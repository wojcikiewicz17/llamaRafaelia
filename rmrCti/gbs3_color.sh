#!/data/data/com.termux/files/usr/bin/bash
set -euo pipefail

ROOT="${1:-$PWD}"
BIN="$ROOT/gbs3_color"
SRC="$ROOT/gbs3_color.c"

cat > "$SRC" <<'C_EOF'
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define WBAR 60

// ---------- ANSI colors (Termux ok) ----------
#define C_RESET "\x1b[0m"
#define C_BOLD  "\x1b[1m"
#define C_DIM   "\x1b[2m"

#define C_RED   "\x1b[31m"
#define C_GRN   "\x1b[32m"
#define C_YEL   "\x1b[33m"
#define C_BLU   "\x1b[34m"
#define C_MAG   "\x1b[35m"
#define C_CYN   "\x1b[36m"
#define C_WHT   "\x1b[37m"

static int g_use_color = 1;   // default ON (Termux)
static int g_use_emoji = 0;   // default OFF (emoji may break grid)

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
static void hr(void){ puts("------------------------------------------------------------"); }

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
  puts("║  GBS3_COLOR :: TRIAD_LAB + RAF_FIBO + ARENA (C)              ║");
  puts("║  menu: (1) trace stats  (2) sweep top  (3) raf fibo (4) arena ║");
  puts("║        (p) paths (c) color on/off (e) emoji on/off (0) exit  ║");
  puts("╚══════════════════════════════════════════════════════════════╝");
}

/* ---------- CSV helpers ---------- */

static int parse_csv_header_map(const char *line, char cols[][64], int maxc){
  int c=0; const char *p=line;
  while(*p && c<maxc){
    char buf[64]={0}; int bi=0;
    while(*p && *p!=',' && *p!='\n' && *p!='\r'){
      if(bi<63) buf[bi++]=*p;
      p++;
    }
    buf[bi]=0;
    int i=0; while(buf[i] && isspace((unsigned char)buf[i])) i++;
    memmove(buf, buf+i, strlen(buf+i)+1);
    int j=(int)strlen(buf);
    while(j>0 && isspace((unsigned char)buf[j-1])) buf[--j]=0;

    strncpy(cols[c], buf, 63); cols[c][63]=0;
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
  int c=0; char *p=line;
  while(*p && c<maxc){
    cells[c++]=p;
    while(*p && *p!=',') p++;
    if(*p==','){ *p=0; p++; }
  }
  return c;
}
static void bar60(char *out, int v, int mx){
  int n = (mx>0) ? (int)((double)WBAR * ((double)v/(double)mx) + 0.5) : 0;
  if(n<0) n=0; if(n>WBAR) n=WBAR;
  for(int i=0;i<WBAR;i++) out[i] = (i<n)?'#':' ';
  out[WBAR]=0;
}

/* ---------- TRIAD TRACE stats ---------- */

static double g_cached_deltaP = 0.0;
static int g_has_deltaP = 0;

static double compute_trace_deltaP(const char *trace_path){
  FILE *f=fopen(trace_path,"rb");
  if(!f) return 0.0;

  char line[8192];
  if(!fgets(line,sizeof(line),f)){ fclose(f); return 0.0; }
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
    fclose(f);
    return 0.0;
  }

  long rows=0;
  long peaks_total=0, peaks_stable=0, non_total=0, non_stable=0;

  while(fgets(line,sizeof(line),f)){
    trim_newline(line);
    if(!line[0]) continue;

    char *cells[256]={0};
    split_csv_row(line, cells, 256);

    int st     = (ista >=0 && cells[ista]) ? atoi(cells[ista]) : 0;
    int gp     = (igp  >=0 && cells[igp])  ? atoi(cells[igp])  : 0;

    rows++;

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
  return p_peaks - p_non;
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
    split_csv_row(line, cells, 256);

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

  g_cached_deltaP = delta;
  g_has_deltaP = 1;

  bbs_header("TRIAD TRACE STATS");
  printf("trace: %s\n\n", trace_path);
  printf("rows=%ld  escaped=%ld (%.2f%%)\n", rows, esc, rows? (100.0*(double)esc/(double)rows):0.0);
  printf("stable_any=%ld (%.2f%%)\n", stable, rows? (100.0*(double)stable/(double)rows):0.0);
  printf("gate_in_peaks=%ld (%.2f%%)\n", peak, rows? (100.0*(double)peak/(double)rows):0.0);
  printf("fr_matches_gate=%ld (%.2f%%)\n\n", match, rows? (100.0*(double)match/(double)rows):0.0);

  printf("stable | peaks    : %ld / %ld (%.2f%%)\n", peaks_stable, peaks_total, peaks_total? (100.0*p_peaks):0.0);
  printf("stable | nonpeaks : %ld / %ld (%.2f%%)\n", non_stable,  non_total,  non_total ? (100.0*p_non)  :0.0);
  printf("ΔP = p(stable|peaks) - p(stable|nonpeaks) = %.6f\n\n", delta);

  long mx=0; for(int j=0;j<4;j++) if(jhist[j]>mx) mx=jhist[j];
  puts("J_hist heatmap (60 cols)");
  for(int j=0;j<4;j++){
    char b[WBAR+1];
    bar60(b, (int)jhist[j], (int)mx);
    double pct = rows? (100.0*(double)jhist[j]/(double)rows):0.0;
    printf("J=%d %4ld (%5.1f%%) |%s|\n", j, jhist[j], pct, b);
  }
}

/* ---------- RAF_FIBO small alphabet ---------- */

typedef struct { const char *name; int seq[64]; int n; } Seq;

static void build_seq_01123(int *out, int *n){ int a[]={0,1,1,2,3}; memcpy(out,a,sizeof(a)); *n=5; }
static void build_seq_001123(int *out, int *n){ int a[]={0,0,1,1,2,3}; memcpy(out,a,sizeof(a)); *n=6; }

static void reverse_seq(const int *in, int n, int *out){ for(int i=0;i<n;i++) out[i]=in[n-1-i]; }
static void invert01_like(const int *in, int n, int *out){
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
  puts("  'alfabeto pequeno' = gate discreto p/ estados e escolhas.");
}

/* ---------- SWEEP top ---------- */

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
  printf("top %d by ΔP\n\n", topN);
  puts("  #   delta      kappa    lam   alpha    beta     steps seed  (R,r,zcap)");
  puts("--------------------------------------------------------------------------");
  for(int i=0;i<topN && i<(int)n;i++){
    Row *x=&rows[i];
    printf("%3d  %+0.6f  %0.3f   %4d  %0.3f  %0.3f      %4d  %3d  (%0.1f,%0.1f,%ld)\n",
      i+1, x->delta, x->kappa, x->lam, x->alpha, x->beta, x->steps, x->seed, x->R, x->r, x->zcap
    );
  }

  free(rows);
}

/* ---------- ARENA ---------- */

#define GW 41
#define GH 17
#define NAI 12

typedef struct { int x,y; int vx,vy; int alive; } Ent;

static unsigned rng_u32 = 0xC0FFEE01u;
static unsigned xorshift32(void){
  unsigned x=rng_u32;
  x ^= x<<13; x ^= x>>17; x ^= x<<5;
  rng_u32=x;
  return x;
}
static int irand(int a, int b){
  unsigned r=xorshift32();
  int span=b-a+1;
  return a + (int)(r % (unsigned)span);
}

static int clampi(int v,int a,int b){ if(v<a) return a; if(v>b) return b; return v; }

static int fibo_gate_step(int t){
  int s1[]={0,1,1,2,3};
  int s2[]={0,0,1,1,2,3};
  int a = s1[t%5];
  int b = s2[t%6];
  int j = (a + 2*b + (t%4)) & 3;
  return j;
}

static void ai_update(Ent *e, Ent *P, int jgate, double deltaP){
  int dx = (P->x > e->x) ? 1 : (P->x < e->x ? -1 : 0);
  int dy = (P->y > e->y) ? 1 : (P->y < e->y ? -1 : 0);

  int bias = (deltaP > 0.20) ? 2 : (deltaP > 0.05 ? 1 : 0);

  int mvx=0,mvy=0;
  if(jgate==0){
    mvx = -dx; mvy = -dy;
  }else if(jgate==1){
    mvx = dx;  mvy = (bias? dy:0);
  }else if(jgate==2){
    mvx = (bias? dx:0); mvy = dy;
  }else{
    mvx = dy;
    mvy = -dx;
  }

  if((xorshift32() & 7u)==0u){
    mvx += irand(-1,1);
    mvy += irand(-1,1);
  }

  e->vx = clampi(mvx, -1, 1);
  e->vy = clampi(mvy, -1, 1);
}

static int is_block(char g[GH][GW+1], int x, int y){
  if(x<=0||x>=GW-1||y<=0||y>=GH-1) return 1;
  return (g[y][x]=='#');
}

static const char* ai_color(int i){
  // simple palette cycle
  switch(i%6){
    case 0: return C_RED;
    case 1: return C_GRN;
    case 2: return C_YEL;
    case 3: return C_BLU;
    case 4: return C_MAG;
    default:return C_CYN;
  }
}

static void put_cell(char c, int ai_index){
  if(!g_use_color){ putchar(c); return; }

  if(c=='@'){
    fputs(C_BOLD C_WHT, stdout); putchar('@'); fputs(C_RESET, stdout);
  }else if(c=='#'){
    fputs(C_DIM C_WHT, stdout); putchar('#'); fputs(C_RESET, stdout);
  }else if(c=='|'||c=='-'||c=='+'){
    fputs(C_BLU, stdout); putchar(c); fputs(C_RESET, stdout);
  }else if(c>='a' && c<='l'){
    fputs(ai_color(ai_index), stdout); putchar(c); fputs(C_RESET, stdout);
  }else{
    putchar(c);
  }
}

static void arena_draw(char g[GH][GW+1], Ent *P, Ent A[NAI], int tick, double deltaP){
  for(int y=0;y<GH;y++){
    for(int x=0;x<GW;x++) g[y][x]=' ';
    g[y][GW]=0;
  }

  for(int x=0;x<GW;x++){ g[0][x]='-'; g[GH-1][x]='-'; }
  for(int y=0;y<GH;y++){ g[y][0]='|'; g[y][GW-1]='|'; }
  g[0][0]=g[0][GW-1]=g[GH-1][0]=g[GH-1][GW-1]='+';

  for(int x=6;x<GW-6;x+=8){
    int y=3 + (x%5);
    if(y>1 && y<GH-1) g[y][x]='#';
  }

  g[P->y][P->x]='@';

  for(int i=0;i<NAI;i++){
    if(!A[i].alive) continue;
    char c = (char)('a'+i);
    g[A[i].y][A[i].x]=c;
  }

  int gate = fibo_gate_step(tick);

  printf("\n");
  if(g_use_emoji){
    printf("YOU=🧍  AI=🤖  tick=%d gate=%d ΔP=%+.6f  (WASD, q)\n", tick, gate, deltaP);
  }else{
    printf("YOU=@  AI=a..l  tick=%d gate=%d ΔP=%+.6f  (WASD, q)\n", tick, gate, deltaP);
  }

  for(int y=0;y<GH;y++){
    for(int x=0;x<GW;x++){
      char c=g[y][x];
      int ai_index = (c>='a' && c<='l') ? (c-'a') : -1;
      put_cell(c, ai_index);
    }
    putchar('\n');
  }
}

static void arena_run(const char *root){
  char trace[PATH_MAX];
  join_path(trace,sizeof(trace),root,"out/triad_trace.csv");

  double deltaP = 0.0;
  if(file_exists(trace)){
    deltaP = compute_trace_deltaP(trace);
    g_cached_deltaP = deltaP;
    g_has_deltaP = 1;
  }else{
    g_has_deltaP = 0;
  }

  rng_u32 ^= (unsigned)time(NULL);

  Ent P = { .x=2, .y=2, .vx=0, .vy=0, .alive=1 };
  Ent A[NAI];
  for(int i=0;i<NAI;i++){
    A[i].x = irand(3, GW-4);
    A[i].y = irand(2, GH-3);
    A[i].vx = A[i].vy = 0;
    A[i].alive = 1;
  }

  char grid[GH][GW+1];
  int tick=0;

  while(1){
    arena_draw(grid, &P, A, tick, deltaP);

    printf("cmd> ");
    fflush(stdout);
    char in[64]={0};
    if(!fgets(in,sizeof(in),stdin)) break;
    trim_newline(in);
    if(!in[0]) in[0]='.';

    if(in[0]=='q' || in[0]=='Q') break;

    int px=P.x, py=P.y;
    if(in[0]=='w' || in[0]=='W') py--;
    else if(in[0]=='s' || in[0]=='S') py++;
    else if(in[0]=='a' || in[0]=='A') px--;
    else if(in[0]=='d' || in[0]=='D') px++;

    if(!is_block(grid, px, py)){
      P.x=px; P.y=py;
    }

    int jgate = fibo_gate_step(tick);
    for(int i=0;i<NAI;i++){
      if(!A[i].alive) continue;

      ai_update(&A[i], &P, jgate, deltaP);

      int nx = A[i].x + A[i].vx;
      int ny = A[i].y + A[i].vy;

      if(is_block(grid, nx, ny)){
        int sx = A[i].x + A[i].vx;
        int sy = A[i].y;
        if(!is_block(grid, sx, sy)){ nx=sx; ny=sy; }
        else{
          sx = A[i].x; sy = A[i].y + A[i].vy;
          if(!is_block(grid, sx, sy)){ nx=sx; ny=sy; }
          else { nx=A[i].x; ny=A[i].y; }
        }
      }

      A[i].x = nx; A[i].y = ny;

      if(A[i].x==P.x && A[i].y==P.y){
        bbs_header("HIT");
        printf("autonomo '%c' pegou o player em tick=%d (gate=%d)\n", 'a'+i, tick, jgate);
        puts("enter -> respawn, q -> quit");
        char tbuf[8]={0};
        fgets(tbuf,sizeof(tbuf),stdin);
        if(tbuf[0]=='q' || tbuf[0]=='Q') return;
        P.x=2; P.y=2;
      }
    }

    tick++;
  }
}

/* ---------- paths ---------- */

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
    printf("root=%s  (color=%s emoji=%s)\n", root, g_use_color?"ON":"OFF", g_use_emoji?"ON":"OFF");
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
    } else if(strcmp(in,"4")==0){
      bbs_header("ARENA");
      puts("WASD move, q exit. (enter em branco = no-op)");
      arena_run(root);
    } else if(strcmp(in,"p")==0 || strcmp(in,"P")==0){
      show_paths(root);
    } else if(strcmp(in,"c")==0 || strcmp(in,"C")==0){
      g_use_color = !g_use_color;
      printf("color -> %s\n", g_use_color?"ON":"OFF");
    } else if(strcmp(in,"e")==0 || strcmp(in,"E")==0){
      g_use_emoji = !g_use_emoji;
      printf("emoji(HUD) -> %s\n", g_use_emoji?"ON":"OFF");
    } else {
      puts("?");
    }
  }

  puts("\nbye.\n");
  return 0;
}
C_EOF

echo "[GBS3_COLOR] build -> $BIN"
cc -O2 -std=c11 -Wall -Wextra "$SRC" -lm -o "$BIN"
echo "[GBS3_COLOR] run"
exec "$BIN" "$ROOT"
