/*
  cognitio_min.c — núcleo geométrico + simetria (Termux-friendly)
  - Gera N pontos no círculo
  - Aplica ruído (amplitude A)
  - Mede simetria por espelhamento nos eixos X e Y
  Build: clang -O3 -std=c11 cognitio_min.c -lm -o cognitio_min
  Run:   ./cognitio_min 60 0.03 1
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct { double x, y; } Pt;

static unsigned rng_u32(void){
  static unsigned s = 0;
  if(!s) s = (unsigned)time(NULL) ^ 0xA5A5A5A5u;
  s ^= s << 13;
  s ^= s >> 17;
  s ^= s << 5;
  return s;
}
static double rng_f01(void){
  return (rng_u32() / 4294967296.0); // [0,1)
}
static double rng_f11(void){
  return 2.0*rng_f01() - 1.0; // [-1,1)
}

static unsigned fib_u(unsigned n){
  if(n <= 1) return 1;
  unsigned a = 1, b = 1;
  for(unsigned i=2;i<=n;i++){
    unsigned c = a + b;
    a = b; b = c;
  }
  return b;
}

static void gen_polygon(Pt *p, int n){
  for(int i=0;i<n;i++){
    double t = (2.0*M_PI*i)/ (double)n;
    p[i].x = cos(t);
    p[i].y = sin(t);
  }
}

static void apply_noise(Pt *p, int n, double amp, int fibo_mode){
  for(int i=0;i<n;i++){
    double a = amp;
    if(fibo_mode){
      // ruído modulado por Fibonacci (bem leve)
      unsigned f = fib_u((unsigned)(i % 10));
      a *= (1.0 + 0.03*(double)f); // cresce um pouco
    }
    p[i].x += a * rng_f11();
    p[i].y += a * rng_f11();
  }
}

static double dist2(Pt a, Pt b){
  double dx = a.x - b.x;
  double dy = a.y - b.y;
  return dx*dx + dy*dy;
}

/*
  score de simetria:
  Para cada ponto, procura o mais próximo do seu espelho.
  Quanto menor a soma das distâncias -> mais simétrico.
*/
static double symmetry_score_axis(const Pt *p, int n, int axis){
  // axis: 0 = X (espelho em x: (x,-y)), 1 = Y (espelho em y: (-x,y))
  double sum = 0.0;
  for(int i=0;i<n;i++){
    Pt m = p[i];
    if(axis==0) m.y = -m.y;
    else        m.x = -m.x;

    double best = 1e300;
    for(int j=0;j<n;j++){
      double d = dist2(m, p[j]);
      if(d < best) best = d;
    }
    sum += sqrt(best);
  }
  // normaliza pelo número de pontos
  return sum / (double)n;
}

int main(int argc, char **argv){
  int n = 60;
  double amp = 0.03;
  int fibo_mode = 0;

  if(argc > 1) n = atoi(argv[1]);
  if(argc > 2) amp = atof(argv[2]);
  if(argc > 3) fibo_mode = atoi(argv[3]) ? 1 : 0;

  if(n < 3){ fprintf(stderr, "N mínimo = 3\n"); return 1; }
  if(amp < 0){ fprintf(stderr, "amp >= 0\n"); return 1; }

  Pt *p = (Pt*)calloc((size_t)n, sizeof(Pt));
  if(!p){ perror("calloc"); return 1; }

  gen_polygon(p, n);
  apply_noise(p, n, amp, fibo_mode);

  double sx = symmetry_score_axis(p, n, 0);
  double sy = symmetry_score_axis(p, n, 1);

  const char *best = (sx < sy) ? "Eixo X (espelho vertical)" : "Eixo Y (espelho horizontal)";
  double smin = (sx < sy) ? sx : sy;

  printf("N=%d  amp=%.6f  fibo_mode=%d\n", n, amp, fibo_mode);
  printf("score_sym(X)=%.8f\n", sx);
  printf("score_sym(Y)=%.8f\n", sy);
  printf("BEST: %s  | score=%.8f\n", best, smin);

  // Dump simples (pra você plugar em plot depois)
  printf("\n# points (x y)\n");
  for(int i=0;i<n;i++){
    printf("%.8f %.8f\n", p[i].x, p[i].y);
  }

  free(p);
  return 0;
}
