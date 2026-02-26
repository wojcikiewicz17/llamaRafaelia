/*
GBS_BBS :: RAFAELIA TRIAD LAB CONSOLE
- Menu ASCII (BBS style)
- Reads: out/triad_trace.csv, out_sweep/results.csv
- Prints: J histogram, escaped rate, stable, peaks vs nonpeaks, ΔP
- Top sweep by delta
Build: cc -O2 -std=c11 -Wall -Wextra gbs_bbs.c -lm -o gbs
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define LINE_MAXN 4096
#define W 60

static void die(const char* msg){
  fprintf(stderr, "ERR: %s\n", msg);
  exit(1);
}

static int file_exists(const char* path){
  FILE* f = fopen(path, "rb");
  if(!f) return 0;
  fclose(f);
  return 1;
}

static void trim_newline(char* s){
  size_t n = strlen(s);
  while(n && (s[n-1]=='\n' || s[n-1]=='\r')) s[--n]=0;
}

static void bar(int v, int mx){
  int n = 0;
  if(mx > 0){
    double frac = (double)v / (double)mx;
    n = (int)llround((double)W * frac);
    if(n < 0) n = 0;
    if(n > W) n = W;
  }
  for(int i=0;i<n;i++) putchar((unsigned char)0xDB); // █
  for(int i=n;i<W;i++) putchar(' ');
}

static int split_csv(char* line, char** out, int maxf){
  int nf=0;
  char* p=line;
  int inq=0;
  out[nf++] = p;
  for(; *p; ++p){
    if(*p=='"') inq = !inq;
    else if(!inq && *p==',' ){
      *p=0;
      if(nf<maxf) out[nf++] = p+1;
    }
  }
  return nf;
}

static int find_col(char** hdr, int nh, const char* name){
  for(int i=0;i<nh;i++){
    if(strcmp(hdr[i], name)==0) return i;
  }
  return -1;
}

static long to_long(const char* s){
  if(!s || !*s) return 0;
  return strtol(s, NULL, 10);
}

static double to_double(const char* s){
  if(!s || !*s) return 0.0;
  return strtod(s, NULL);
}

static void show_trace_stats(const char* path){
  FILE* f = fopen(path, "rb");
  if(!f){ perror(path); return; }

  char line[LINE_MAXN];
  if(!fgets(line, sizeof(line), f)){ fclose(f); die("trace csv vazio"); }
  trim_newline(line);

  // header parse
  char* hdrf[256]={0};
  int nh = split_csv(line, hdrf, 256);

  int cj = find_col(hdrf, nh, "J_n");
  int ces = find_col(hdrf, nh, "escaped");
  int cst = find_col(hdrf, nh, "stable_any");
  int cpk = find_col(hdrf, nh, "gate_in_peaks");
  int cmg = find_col(hdrf, nh, "fr_matches_gate");

  if(cj<0) die("faltou coluna J_n");
  if(ces<0) die("faltou coluna escaped");
  if(cst<0) die("faltou coluna stable_any");
  if(cpk<0) die("faltou coluna gate_in_peaks");
  if(cmg<0) die("faltou coluna fr_matches_gate");

  long rows=0, esc=0, stable=0, peak=0, match=0;
  long peaks_total=0, peaks_stable=0;
  long non_total=0, non_stable=0;
  long jhist[4]={0,0,0,0};

  while(fgets(line, sizeof(line), f)){
    trim_newline(line);
    if(!line[0]) continue;
    char* fld[256]={0};
    int nf = split_csv(line, fld, 256);
    (void)nf;

    long j = to_long(fld[cj]);
    long e = to_long(fld[ces]);
    long st= to_long(fld[cst]);
    long pk= to_long(fld[cpk]);
    long mg= to_long(fld[cmg]);

    rows++;
    esc += (e!=0);
    stable += (st!=0);
    peak   += (pk!=0);
    match  += (mg!=0);

    if(0<=j && j<=3) jhist[j]++;

    if(pk){
      peaks_total++;
      peaks_stable += (st!=0);
    }else{
      non_total++;
      non_stable += (st!=0);
    }
  }
  fclose(f);

  double p_stable = rows? (double)stable/(double)rows : 0.0;
  double p_peak_s = peaks_total? (double)peaks_stable/(double)peaks_total : NAN;
  double p_non_s  = non_total? (double)non_stable/(double)non_total : NAN;
  double delta    = (isfinite(p_peak_s) && isfinite(p_non_s)) ? (p_peak_s - p_non_s) : NAN;

  int mx=0; for(int i=0;i<4;i++) if((int)jhist[i]>mx) mx=(int)jhist[i];

  printf("\n=== TRIAD TRACE :: %s ===\n", path);
  printf("rows=%ld  escaped=%ld (%.2f%%)\n", rows, esc, rows? (100.0*esc/rows):0.0);
  printf("stable_any=%ld (%.2f%%)\n", stable, rows? (100.0*stable/rows):0.0);
  printf("gate_in_peaks=%ld (%.2f%%)\n", peak, rows? (100.0*peak/rows):0.0);
  printf("fr_matches_gate=%ld (%.2f%%)\n", match, rows? (100.0*match/rows):0.0);

  if(isfinite(p_peak_s) && isfinite(p_non_s)){
    printf("\nstable | peaks    : %ld / %ld (%.2f%%)\n", peaks_stable, peaks_total, 100.0*p_peak_s);
    printf("stable | nonpeaks : %ld / %ld (%.2f%%)\n", non_stable,  non_total,   100.0*p_non_s);
    printf("ΔP = p(stable|peaks) - p(stable|nonpeaks) = %.6f\n", delta);
  }

  printf("\nJ_hist heatmap (60 cols)\n");
  for(int j=0;j<4;j++){
    double pct = rows? (100.0*(double)jhist[j]/(double)rows):0.0;
    printf("J=%d %4ld (%5.1f%%) |", j, jhist[j], pct);
    bar((int)jhist[j], mx);
    printf("|\n");
  }
}

typedef struct {
  double delta;
  int steps, seed;
  double R, r, alpha, beta, kappa;
  int lam, zcap;
  long rows, esc, j0,j1,j2,j3;
} SweepRow;

static int cmp_desc_delta(const void* a, const void* b){
  const SweepRow* A=(const SweepRow*)a;
  const SweepRow* B=(const SweepRow*)b;
  if(A->delta < B->delta) return 1;
  if(A->delta > B->delta) return -1;
  return 0;
}

static void show_sweep_top(const char* path, int topn){
  FILE* f = fopen(path, "rb");
  if(!f){ perror(path); return; }

  char line[LINE_MAXN];
  if(!fgets(line, sizeof(line), f)){ fclose(f); die("results.csv vazio"); }
  trim_newline(line);

  // header
  char* hdr[128]={0};
  int nh = split_csv(line, hdr, 128);

  // indices (minimal)
  int c_delta = find_col(hdr, nh, "delta");
  int c_steps = find_col(hdr, nh, "steps");
  int c_seed  = find_col(hdr, nh, "seed");
  int c_R     = find_col(hdr, nh, "R");
  int c_r     = find_col(hdr, nh, "r");
  int c_alpha = find_col(hdr, nh, "alpha");
  int c_beta  = find_col(hdr, nh, "beta");
  int c_kappa = find_col(hdr, nh, "kappa");
  int c_lam   = find_col(hdr, nh, "lam");
  int c_zcap  = find_col(hdr, nh, "zcap");

  if(c_delta<0) die("faltou coluna delta no results.csv");

  SweepRow* arr=NULL;
  size_t n=0, cap=0;

  while(fgets(line, sizeof(line), f)){
    trim_newline(line);
    if(!line[0]) continue;
    char* fld[128]={0};
    split_csv(line, fld, 128);

    double d = to_double(fld[c_delta]);
    if(!isfinite(d)) continue;

    if(n==cap){
      cap = cap? cap*2 : 256;
      arr = (SweepRow*)realloc(arr, cap*sizeof(SweepRow));
      if(!arr) die("oom");
    }
    SweepRow* s = &arr[n++];
    memset(s, 0, sizeof(*s));
    s->delta = d;
    s->steps = (c_steps>=0)? (int)to_long(fld[c_steps]) : 0;
    s->seed  = (c_seed>=0) ? (int)to_long(fld[c_seed])  : 0;
    s->R     = (c_R>=0)    ? to_double(fld[c_R])         : 0;
    s->r     = (c_r>=0)    ? to_double(fld[c_r])         : 0;
    s->alpha = (c_alpha>=0)? to_double(fld[c_alpha])     : 0;
    s->beta  = (c_beta>=0) ? to_double(fld[c_beta])      : 0;
    s->kappa = (c_kappa>=0)? to_double(fld[c_kappa])     : 0;
    s->lam   = (c_lam>=0)  ? (int)to_long(fld[c_lam])    : 0;
    s->zcap  = (c_zcap>=0) ? (int)to_long(fld[c_zcap])   : 0;
  }
  fclose(f);

  if(n==0){
    printf("\n=== SWEEP TOP :: %s ===\n(no finite rows)\n", path);
    free(arr);
    return;
  }

  qsort(arr, n, sizeof(SweepRow), cmp_desc_delta);

  printf("\n=== SWEEP TOP :: %s ===\n", path);
  printf("top %d by delta\n\n", topn);
  printf("  #   delta      kappa    lam   alpha    beta     steps seed  (R,r,zcap)\n");
  printf("--------------------------------------------------------------------------\n");
  int m = (topn < (int)n)? topn : (int)n;
  for(int i=0;i<m;i++){
    SweepRow* s=&arr[i];
    printf("%3d  %+0.6f  %0.3f  %5d  %0.3f  %0.3f  %5d %4d  (%0.1f,%0.1f,%d)\n",
      i+1, s->delta, s->kappa, s->lam, s->alpha, s->beta, s->steps, s->seed, s->R, s->r, s->zcap
    );
  }
  free(arr);
}

static void banner(void){
  printf("\n");
  printf("╔══════════════════════════════════════════════════════════════╗\n");
  printf("║  GBS_BBS :: RAFAELIA TRIAD LAB                               ║\n");
  printf("║  menu: (1) trace stats  (2) sweep top  (3) paths  (0) exit   ║\n");
  printf("╚══════════════════════════════════════════════════════════════╝\n");
}

static void show_paths(const char* root){
  char a[512], b[512];
  snprintf(a,sizeof(a),"%s/out/triad_trace.csv", root);
  snprintf(b,sizeof(b),"%s/out_sweep/results.csv", root);
  printf("\nPaths:\n");
  printf("  trace: %s  [%s]\n", a, file_exists(a)?"OK":"missing");
  printf("  sweep: %s  [%s]\n", b, file_exists(b)?"OK":"missing");
}

int main(int argc, char** argv){
  const char* root = (argc>=2)? argv[1] : ".";
  char trace[512], sweep[512];
  snprintf(trace,sizeof(trace),"%s/out/triad_trace.csv", root);
  snprintf(sweep,sizeof(sweep),"%s/out_sweep/results.csv", root);

  for(;;){
    banner();
    printf("root=%s\n", root);
    printf("select> ");
    fflush(stdout);

    char in[64]={0};
    if(!fgets(in,sizeof(in),stdin)) break;
    trim_newline(in);

    if(strcmp(in,"0")==0 || strcasecmp(in,"q")==0) break;
    else if(strcmp(in,"1")==0){
      if(file_exists(trace)) show_trace_stats(trace);
      else printf("\n(no trace) %s\n", trace);
    }else if(strcmp(in,"2")==0){
      if(file_exists(sweep)) show_sweep_top(sweep, 12);
      else printf("\n(no sweep) %s\n", sweep);
    }else if(strcmp(in,"3")==0){
      show_paths(root);
    }else{
      printf("?\n");
    }
  }
  printf("\nbye.\n");
  return 0;
}
