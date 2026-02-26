/*
  cognitio_v2_ppm.c — pontos + simetria (espelho X/Y) + simetria rotacional k-fold + saída PPM
  Build: clang -O3 -std=c11 cognitio_v2_ppm.c -lm -o cognitio_v2_ppm
  Run:   ./cognitio_v2_ppm 60 0.03 1 out.ppm

  Args:
    N         (default 60)
    amp       (default 0.03)
    fibo_mode (0/1 default 0)
    out.ppm   (default out.ppm)
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct { double x, y; } Pt;

static unsigned rng_u32(void){
  static unsigned s = 0;
  if(!s) s = (unsigned)time(NULL) ^ 0x9E3779B9u ^ 0xA5A5A5A5u;
  s ^= s << 13; s ^= s >> 17; s ^= s << 5;
  return s;
}
static double rng_f01(void){ return rng_u32() / 4294967296.0; }
static double rng_f11(void){ return 2.0*rng_f01() - 1.0; }

static unsigned fib_u(unsigned n){
  if(n <= 1) return 1;
  unsigned a=1,b=1;
  for(unsigned i=2;i<=n;i++){ unsigned c=a+b; a=b; b=c; }
  return b;
}

static void gen_polygon(Pt *p, int n){
  for(int i=0;i<n;i++){
    double t = (2.0*M_PI*i)/(double)n;
    p[i].x = cos(t);
    p[i].y = sin(t);
  }
}

static void apply_noise(Pt *p, int n, double amp, int fibo_mode){
  for(int i=0;i<n;i++){
    double a = amp;
    if(fibo_mode){
      unsigned f = fib_u((unsigned)(i % 10));
      a *= (1.0 + 0.03*(double)f);
    }
    p[i].x += a * rng_f11();
    p[i].y += a * rng_f11();
  }
}

static inline double dist2(Pt a, Pt b){
  double dx=a.x-b.x, dy=a.y-b.y;
  return dx*dx + dy*dy;
}

static double symmetry_score_axis(const Pt *p, int n, int axis){
  // axis: 0 = X (espelho em x: (x,-y)), 1 = Y (espelho em y: (-x,y))
  double sum=0.0;
  for(int i=0;i<n;i++){
    Pt m=p[i];
    if(axis==0) m.y = -m.y; else m.x = -m.x;

    double best=1e300;
    for(int j=0;j<n;j++){
      double d=dist2(m,p[j]);
      if(d<best) best=d;
    }
    sum += sqrt(best);
  }
  return sum/(double)n;
}

static Pt rot(Pt a, double ang){
  double c=cos(ang), s=sin(ang);
  Pt r; r.x = c*a.x - s*a.y; r.y = s*a.x + c*a.y;
  return r;
}

// k-fold: compara p com p rotacionado por 2π/k (nearest neighbor)
static double symmetry_score_rot_k(const Pt *p, int n, int k){
  double ang = 2.0*M_PI/(double)k;
  double sum=0.0;
  for(int i=0;i<n;i++){
    Pt r = rot(p[i], ang);
    double best=1e300;
    for(int j=0;j<n;j++){
      double d=dist2(r,p[j]);
      if(d<best) best=d;
    }
    sum += sqrt(best);
  }
  return sum/(double)n;
}

/* ---------------- PPM DRAW ---------------- */

typedef struct { unsigned char r,g,b; } RGB;

static RGB rgb(unsigned char r, unsigned char g, unsigned char b){
  RGB c; c.r=r; c.g=g; c.b=b; return c;
}

static void set_px(RGB *img, int W, int H, int x, int y, RGB c){
  if((unsigned)x >= (unsigned)W || (unsigned)y >= (unsigned)H) return;
  img[y*W + x] = c;
}

static void draw_line(RGB *img, int W, int H, int x0, int y0, int x1, int y1, RGB c){
  int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
  int dy = -abs(y1-y0), sy = y0<y1 ? 1 : -1;
  int err = dx + dy;
  for(;;){
    set_px(img,W,H,x0,y0,c);
    if(x0==x1 && y0==y1) break;
    int e2 = 2*err;
    if(e2 >= dy){ err += dy; x0 += sx; }
    if(e2 <= dx){ err += dx; y0 += sy; }
  }
}

static void draw_circle(RGB *img, int W, int H, int cx, int cy, int r, RGB c){
  int x=r, y=0, err=0;
  while(x>=y){
    set_px(img,W,H,cx+x,cy+y,c);
    set_px(img,W,H,cx+y,cy+x,c);
    set_px(img,W,H,cx-y,cy+x,c);
    set_px(img,W,H,cx-x,cy+y,c);
    set_px(img,W,H,cx-x,cy-y,c);
    set_px(img,W,H,cx-y,cy-x,c);
    set_px(img,W,H,cx+y,cy-x,c);
    set_px(img,W,H,cx+x,cy-y,c);
    y++;
    if(err <= 0) err += 2*y+1;
    if(err > 0){ x--; err -= 2*x+1; }
  }
}

static void draw_disk(RGB *img, int W, int H, int cx, int cy, int r, RGB c){
  for(int y=-r;y<=r;y++){
    for(int x=-r;x<=r;x++){
      if(x*x + y*y <= r*r) set_px(img,W,H,cx+x,cy+y,c);
    }
  }
}

static void write_ppm(const char *path, const RGB *img, int W, int H){
  FILE *f = fopen(path,"wb");
  if(!f){ perror("fopen"); exit(1); }
  fprintf(f, "P6\n%d %d\n255\n", W, H);
  fwrite(img, sizeof(RGB), (size_t)W*(size_t)H, f);
  fclose(f);
}

// mapa [-S,S] -> pixels
static void to_px(double x, double y, double S, int W, int H, int *ix, int *iy){
  double u = (x + S) / (2.0*S);
  double v = (y + S) / (2.0*S);
  int px = (int)lrint(u*(W-1));
  int py = (int)lrint((1.0-v)*(H-1));
  *ix = px; *iy = py;
}

int main(int argc, char **argv){
  int n = 60;
  double amp = 0.03;
  int fibo_mode = 0;
  const char *out = "out.ppm";

  if(argc > 1) n = atoi(argv[1]);
  if(argc > 2) amp = atof(argv[2]);
  if(argc > 3) fibo_mode = atoi(argv[3]) ? 1 : 0;
  if(argc > 4) out = argv[4];

  if(n < 3){ fprintf(stderr,"N mínimo=3\n"); return 1; }
  if(amp < 0){ fprintf(stderr,"amp >= 0\n"); return 1; }

  Pt *p = (Pt*)calloc((size_t)n, sizeof(Pt));
  if(!p){ perror("calloc"); return 1; }

  gen_polygon(p,n);
  apply_noise(p,n,amp,fibo_mode);

  double sx = symmetry_score_axis(p,n,0);
  double sy = symmetry_score_axis(p,n,1);

  // varredura k-fold (3..12)
  int bestk = 0;
  double bestk_score = 1e300;
  for(int k=3;k<=12;k++){
    double sc = symmetry_score_rot_k(p,n,k);
    if(sc < bestk_score){ bestk_score=sc; bestk=k; }
  }

  const char *bestAxis = (sx < sy) ? "X (espelho vertical)" : "Y (espelho horizontal)";
  double bestAxisScore = (sx < sy) ? sx : sy;

  printf("N=%d  amp=%.6f  fibo_mode=%d\n", n, amp, fibo_mode);
  printf("score_sym(X)=%.8f\n", sx);
  printf("score_sym(Y)=%.8f\n", sy);
  printf("BEST_AXIS: %s | score=%.8f\n", bestAxis, bestAxisScore);
  printf("BEST_ROT:  k=%d | score=%.8f\n", bestk, bestk_score);

  // ---- render PPM ----
  const int W=1024, H=1024;
  RGB *img = (RGB*)malloc((size_t)W*(size_t)H*sizeof(RGB));
  if(!img){ perror("malloc"); return 1; }

  // background
  RGB bg = rgb(8,10,14);
  for(int i=0;i<W*H;i++) img[i]=bg;

  // escala
  double S = 1.25; // margem
  int cx=W/2, cy=H/2;

  // alvo/círculos
  draw_circle(img,W,H,cx,cy,(int)(0.40*W/2), rgb(30,40,55));
  draw_circle(img,W,H,cx,cy,(int)(0.80*W/2), rgb(20,28,38));
  draw_circle(img,W,H,cx,cy,(int)(0.98*W/2), rgb(16,20,28));

  // eixos
  draw_line(img,W,H, 0,cy, W-1,cy, rgb(20,28,38));
  draw_line(img,W,H, cx,0, cx,H-1, rgb(20,28,38));

  // melhor eixo (color highlight)
  if(sx < sy){
    // eixo X => espelho vertical => linha horizontal y=0 (já tem), realça
    draw_line(img,W,H, 0,cy, W-1,cy, rgb(220,180,90));
  }else{
    // eixo Y => espelho horizontal => linha vertical x=0 (já tem), realça
    draw_line(img,W,H, cx,0, cx,H-1, rgb(220,180,90));
  }

  // desenho do “k-fold” (radiais)
  for(int i=0;i<bestk;i++){
    double ang = (2.0*M_PI*(double)i)/(double)bestk;
    double x = cos(ang)*1.05;
    double y = sin(ang)*1.05;
    int x1,y1;
    to_px(x,y,S,W,H,&x1,&y1);
    draw_line(img,W,H,cx,cy,x1,y1, rgb(60,120,220));
  }

  // pontos
  for(int i=0;i<n;i++){
    int x,y;
    to_px(p[i].x, p[i].y, S, W, H, &x, &y);
    draw_disk(img,W,H,x,y,2, rgb(240,245,255));
  }

  write_ppm(out, img, W, H);
  printf("WROTE: %s (PPM P6 %dx%d)\n", out, W, H);

  // dump dos pontos (pra você reaproveitar)
  printf("\n# points (x y)\n");
  for(int i=0;i<n;i++) printf("%.8f %.8f\n", p[i].x, p[i].y);

  free(img);
  free(p);
  return 0;
}
