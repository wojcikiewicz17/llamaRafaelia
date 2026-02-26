#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define W 60

static void trim_newline(char *s){
  size_t n=strlen(s);
  while(n && (s[n-1]=='\n' || s[n-1]=='\r')) s[--n]=0;
}
static int file_exists(const char *p){
  FILE *f=fopen(p,"rb");
  if(!f) return 0;
  fclose(f);
  return 1;
}
static void join_path(char *out, size_t cap, const char *a, const char *b){
  snprintf(out, cap, "%s/%s", a, b);
}

static void hr(void){
  puts("------------------------------------------------------------");
}

static void bbs_header(const char *title){
  puts("");
  puts("############################################################");
  puts("#                                                          #");
  printf("#  %-56s#\n", title);
  puts("#                                                          #");
  puts("############################################################");
}

static void bbs_menu(void){
  puts("");
  puts("╔══════════════════════════════════════════════════════════════╗");
  puts("║  GBS2 :: TRIAD_LAB + RAF_FIBO (C)                            ║");
  puts("║  menu: (1) trace stats  (2) sweep top  (3) raf fibo  (0) exit║");
  puts("╚══════════════════════════════════════════════════════════════╝");
}

static int parse_csv_header_map(const char *line, char cols[][64], int maxc){
  // split header by commas into cols[]; returns count
  int c=0;
  const char *p=line;
  while(*p && c<maxc){
    // read token
    char buf[64]={0};
    int bi=0;
    while(*p && *p!=',' && *p!='\n' && *p!='\r'){
      if(bi<63) buf[bi++]=*p;
      p++;
    }
    buf[bi]=0;
    // trim spaces
    int i=0; while(buf[i] && isspace((unsigned char)buf[i])) i++;
    memmove(buf, buf+i, strlen(buf+i)+1);
    int j=(int)strlen(buf);
    while(j>0 && isspace((unsigned char)buf[j-1])) buf[--j]=0;

    strncpy(cols[c], buf, 63);
    cols[c][63]=0;
    c++;
    if(*p==',') p++;
  }
  return c;
}

static int col_index(char cols[][64], int n, const char *name){
  for(int i=0;i<n;i++) if(strcmp(cols[i], name)==0) return i;
  return -1;
}

static int split_csv_row(char *line, char *cells[], int maxc){
  int c=0;
  char *p=line;
  while(*p && c<maxc){
    cells[c++]=p;
    while(*p && *p!=',') p++;
    if(*p==','){ *p=0; p++; }
  }
  return c;
}

static void bar60(char *out, int v, int mx){
  int n = (mx>0) ? (int)((double)W * ((double)v/(double)mx) + 0.5) : 0;
  if(n<0) n=0; if(n>W) n=W;
  for(int i=0;i<W;i++) out[i] = (i<n)?'#':' ';
  out[W]=0;
}

static void show_trace_stats(const char *trace_path){
  FILE *f=fopen(trace_path,"rb");
  if(!f){ printf("no trace: %s\n", trace_path); return; }

  char line[8192];
  if(!fgets(line,sizeof(line),f)){ fclose(f); puts("empty trace"); return; }
  trim_newline(line);

  char cols[128][64];
  int ncols=parse_csv_header_map(line, cols, 128);

  int iJ   = col_index(cols, ncols, "J_n");
  int iesc = col_index(cols, ncols, "escaped");
  int ista = col_index(cols, ncols, "stable_any");
  int igp  = col_index(cols, ncols, "gate_in_peaks");
  int ifm  = col_index(cols, ncols, "fr_matches_gate");
  int igate= col_index(cols, ncols, "gate_9to1");
  if(igate<0) igate = col_index(cols, ncols, "gate");

  if(iJ<0 || iesc<0 || ista<0 || igp<0 || ifm<0){
    puts("trace missing required columns. need: J_n, escaped, stable_any, gate_in_peaks, fr_matches_gate");
    fclose(f);
    return;
  }

  long rows=0, esc=0, stable=0, peak=0, match=0;
  long peaks_total=0, peaks_stable=0, non_total=0, non_stable=0;
  long jhist[4]={0,0,0,0};

  while(fgets(line,sizeof(line),f)){
    trim_newline(line);
    if(!line[0]) continue;

    char *cells[256]={0};
    int nc = split_csv_row(line, cells, 256);

    (void)nc;

    int J      = (iJ   >=0 && cells[iJ])   ? atoi(cells[iJ])   : 0;
    int escaped= (iesc >=0 && cells[iesc]) ? atoi(cells[iesc]) : 0;
    int st     = (ista >=0 && cells[ista]) ? atoi(cells[ista]) : 0;
    int gp     = (igp  >=0 && cells[igp])  ? atoi(cells[igp])  : 0;
    int fm     = (ifm  >=0 && cells[ifm])  ? atoi(cells[ifm])  : 0;

    rows++;
    esc += escaped;
    stable += st;
    peak += gp;
    match += fm;

    if(J>=0 && J<=3) jhist[J]++;

    // peaks bucket: either explicit gate_in_peaks==1 OR gate in {3,4,8} if available
    int is_peak = gp;
    if(!is_peak && igate>=0 && cells[igate]){
      int g = atoi(cells[igate]);
      if(g==3 || g==4 || g==8) is_peak = 1;
    }

    if(is_peak){
      peaks_total++;
      peaks_stable += st;
    }else{
      non_total++;
      non_stable += st;
    }
  }
  fclose(f);

  double p_peaks = (peaks_total>0) ? (double)peaks_stable/(double)peaks_total : 0.0;
  double p_non   = (non_total  >0) ? (double)non_stable  /(double)non_total   : 0.0;
  double delta   = p_peaks - p_non;

  bbs_header("TRIAD TRACE STATS");
  printf("trace: %s\n\n", trace_path);
  printf("rows=%ld  escaped=%ld (%.2f%%)\n", rows, esc, rows? (100.0*(double)esc/(double)rows):0.0);
  printf("stable_any=%ld (%.2f%%)\n", stable, rows? (100.0*(double)stable/(double)rows):0.0);
  printf("gate_in_peaks=%ld (%.2f%%)\n", peak, rows? (100.0*(double)peak/(double)rows):0.0);
  printf("fr_matches_gate=%ld (%.2f%%)\n\n", match, rows? (100.0*(double)match/(double)rows):0.0);

  printf("stable | peaks    : %ld / %ld (%.2f%%)\n", peaks_stable, peaks_total, peaks_total? (100.0*p_peaks):0.0);
  printf("stable | nonpeaks : %ld / %ld (%.2f%%)\n", non_stable,  non_total,  non_total ? (100.0*p_non)  :0.0);
  printf("ΔP = p(stable|peaks) - p(stable|nonpeaks) = %.6f\n\n", delta);

  // heatmap
  long mx=0; for(int j=0;j<4;j++) if(jhist[j]>mx) mx=jhist[j];
  puts("J_hist heatmap (60 cols)");
  for(int j=0;j<4;j++){
    char b[W+1];
    bar60(b, (int)jhist[j], (int)mx);
    double pct = rows? (100.0*(double)jhist[j]/(double)rows):0.0;
    printf("J=%d %4ld (%5.1f%%) |%s|\n", j, jhist[j], pct, b);
  }
}

typedef struct {
  const char *name;
  int seq[64];
  int n;
} Seq;

static void build_seq_01123(int *out, int *n){
  int a[]={0,1,1,2,3};
  memcpy(out,a,sizeof(a)); *n=5;
}
static void build_seq_001123(int *out, int *n){
  int a[]={0,0,1,1,2,3};
  memcpy(out,a,sizeof(a)); *n=6;
}
static void reverse_seq(const int *in, int n, int *out){
  for(int i=0;i<n;i++) out[i]=in[n-1-i];
}
static void invert01_like(const int *in, int n, int *out){
  // "inversa" binária: 0<->1, mantém >1 como está (pra não inventar)
  for(int i=0;i<n;i++){
    int v=in[i];
    if(v==0) out[i]=1;
    else if(v==1) out[i]=0;
    else out[i]=v;
  }
}
static void print_seq(const char *tag, const int *s, int n){
  printf("%-12s: ", tag);
  for(int i=0;i<n;i++) printf("%d", s[i]);
  puts("");
}
static void raf_fibo_screen(void){
  bbs_header("RAF_FIBO :: 01123 / 001123 (fwd / rev / inv / inv+rev)");
  puts("definições (operacionais):");
  puts("  fwd  = sequência original");
  puts("  rev  = reversa (espelho temporal)");
  puts("  inv  = inversa binária (0<->1; >=2 preserva)");
  puts("  invR = inv + rev");
  hr();

  Seq S[2];
  S[0].name="01123";   build_seq_01123(S[0].seq, &S[0].n);
  S[1].name="001123";  build_seq_001123(S[1].seq, &S[1].n);

  for(int k=0;k<2;k++){
    int rev[64], inv[64], invR[64];
    reverse_seq(S[k].seq, S[k].n, rev);
    invert01_like(S[k].seq, S[k].n, inv);
    reverse_seq(inv, S[k].n, invR);

    printf("[%s]\n", S[k].name);
    print_seq("fwd",  S[k].seq, S[k].n);
    print_seq("rev",  rev,      S[k].n);
    print_seq("inv",  inv,      S[k].n);
    print_seq("invR", invR,     S[k].n);
    hr();
  }

  puts("nota:");
  puts("  isso aqui é o 'alfabeto pequeno' (âncora).");
  puts("  você usa ele pra: gate, estados (st), e decisões discretas (0/1).");
}

typedef struct {
  double delta;
  double kappa;
  int lam;
  double alpha;
  double beta;
  int steps;
  int seed;
  double R;
  double r;
  long zcap;
} Row;

static int cmp_row_desc(const void *A, const void *B){
  const Row *a=(const Row*)A, *b=(const Row*)B;
  if(a->delta < b->delta) return 1;
  if(a->delta > b->delta) return -1;
  return 0;
}

static void show_sweep_top(const char *sweep_path, int topN){
  FILE *f=fopen(sweep_path,"rb");
  if(!f){ printf("no sweep: %s\n", sweep_path); return; }

  char line[8192];
  if(!fgets(line,sizeof(line),f)){ fclose(f); puts("empty sweep"); return; }
  trim_newline(line);

  // we assume header contains: steps,seed,R,r,alpha,beta,kappa,lam,zcap,rows,esc,stable_rate,p_peaks,p_non,delta,j0,j1,j2,j3
  // we'll parse needed by split per row with fixed indexes by header lookup
  char cols[128][64];
  int ncols=parse_csv_header_map(line, cols, 128);

  int isteps = col_index(cols,ncols,"steps");
  int iseed  = col_index(cols,ncols,"seed");
  int iR     = col_index(cols,ncols,"R");
  int ir     = col_index(cols,ncols,"r");
  int ialpha = col_index(cols,ncols,"alpha");
  int ibeta  = col_index(cols,ncols,"beta");
  int ikappa = col_index(cols,ncols,"kappa");
  int ilam   = col_index(cols,ncols,"lam");
  int izcap  = col_index(cols,ncols,"zcap");
  int idelta = col_index(cols,ncols,"delta");

  if(isteps<0||iseed<0||iR<0||ir<0||ialpha<0||ibeta<0||ikappa<0||ilam<0||izcap<0||idelta<0){
    puts("sweep missing required columns (steps seed R r alpha beta kappa lam zcap delta)");
    fclose(f);
    return;
  }

  Row *rows = NULL;
  size_t cap=0, n=0;

  while(fgets(line,sizeof(line),f)){
    trim_newline(line);
    if(!line[0]) continue;
    char *cells[256]={0};
    split_csv_row(line, cells, 256);

    if(n==cap){
      cap = cap? cap*2 : 256;
      rows = (Row*)realloc(rows, cap*sizeof(Row));
      if(!rows){ puts("OOM"); fclose(f); return; }
    }

    Row rr;
    rr.steps = atoi(cells[isteps]);
    rr.seed  = atoi(cells[iseed]);
    rr.R     = atof(cells[iR]);
    rr.r     = atof(cells[ir]);
    rr.alpha = atof(cells[ialpha]);
    rr.beta  = atof(cells[ibeta]);
    rr.kappa = atof(cells[ikappa]);
    rr.lam   = atoi(cells[ilam]);
    rr.zcap  = atol(cells[izcap]);
    rr.delta = atof(cells[idelta]);

    rows[n++] = rr;
  }
  fclose(f);

  qsort(rows, n, sizeof(Row), cmp_row_desc);

  bbs_header("SWEEP TOP (by delta)");
  printf("sweep: %s\n\n", sweep_path);
  printf("top %d by ΔP\n", topN);
  puts("");
  puts("  #   delta      kappa    lam   alpha    beta     steps seed  (R,r,zcap)");
  puts("--------------------------------------------------------------------------");
  for(int i=0;i<topN && i<(int)n;i++){
    Row *x=&rows[i];
    printf("%3d  %+0.6f  %0.3f   %4d  %0.3f  %0.3f    %4d  %3d  (%0.1f,%0.1f,%ld)\n",
      i+1, x->delta, x->kappa, x->lam, x->alpha, x->beta, x->steps, x->seed, x->R, x->r, x->zcap
    );
  }

  free(rows);
}

static void show_paths(const char *root){
  char trace[PATH_MAX], sweep[PATH_MAX];
  join_path(trace,sizeof(trace),root,"out/triad_trace.csv");
  join_path(sweep,sizeof(sweep),root,"out_sweep/results.csv");

  bbs_header("PATHS");
  printf("root : %s\n", root);
  printf("trace: %s  [%s]\n", trace, file_exists(trace) ? "OK":"MISSING");
  printf("sweep: %s  [%s]\n", sweep, file_exists(sweep) ? "OK":"MISSING");
}

int main(int argc, char **argv){
  const char *root = (argc>1)? argv[1] : ".";
  char trace[PATH_MAX], sweep[PATH_MAX];
  join_path(trace,sizeof(trace),root,"out/triad_trace.csv");
  join_path(sweep,sizeof(sweep),root,"out_sweep/results.csv");

  while(1){
    bbs_menu();
    printf("root=%s\n", root);
    printf("select> ");
    fflush(stdout);

    char in[64]={0};
    if(!fgets(in,sizeof(in),stdin)) break;
    trim_newline(in);

    if(strcmp(in,"0")==0 || strcasecmp(in,"q")==0) break;

    if(strcmp(in,"1")==0){
      if(file_exists(trace)) show_trace_stats(trace);
      else { puts(""); printf("(no trace) %s\n", trace); }
    } else if(strcmp(in,"2")==0){
      if(file_exists(sweep)) show_sweep_top(sweep, 12);
      else { puts(""); printf("(no sweep) %s\n", sweep); }
    } else if(strcmp(in,"3")==0){
      raf_fibo_screen();
    } else if(strcmp(in,"p")==0 || strcmp(in,"P")==0){
      show_paths(root);
    } else {
      puts("?");
      puts("dica: 1/2/3 ou 0. (p = paths)");
    }
  }

  puts("\nbye.\n");
  return 0;
}
