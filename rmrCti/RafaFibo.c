// rafa_autonomos.c
// "Anguiar autônomos" = guiar agentes 2D (jogo/sim) com:
// - seek (alvo), avoid (obstáculo), separate/cohere/align (boids light)
// - FSM simples (IDLE/SEEK/FLEE)
// - determinístico (seed)
// Build: cc -O2 -std=c11 rafa_autonomos.c -lm -o rafa_autonomos
// Run:   ./rafa_autonomos

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ---------- Utils ----------
static inline float clampf(float x, float a, float b){
  return (x < a) ? a : (x > b) ? b : x;
}
static inline float sqrf(float x){ return x*x; }

typedef struct { float x,y; } v2;

static inline v2 v2_add(v2 a, v2 b){ return (v2){a.x+b.x, a.y+b.y}; }
static inline v2 v2_sub(v2 a, v2 b){ return (v2){a.x-b.x, a.y-b.y}; }
static inline v2 v2_mul(v2 a, float s){ return (v2){a.x*s, a.y*s}; }
static inline float v2_dot(v2 a, v2 b){ return a.x*b.x + a.y*b.y; }
static inline float v2_len2(v2 a){ return v2_dot(a,a); }
static inline float v2_len(v2 a){ return sqrtf(v2_len2(a)); }

static inline v2 v2_norm(v2 a){
  float l = v2_len(a);
  if(l < 1e-8f) return (v2){0,0};
  return v2_mul(a, 1.0f/l);
}

static inline v2 v2_limit(v2 a, float maxlen){
  float l2 = v2_len2(a);
  float m2 = maxlen*maxlen;
  if(l2 <= m2) return a;
  float l = sqrtf(l2);
  return v2_mul(a, maxlen / (l + 1e-9f));
}

// ---------- Deterministic RNG ----------
typedef struct { uint64_t s; } rng64;
static uint64_t splitmix64(uint64_t *x){
  uint64_t z = (*x += 0x9E3779B97F4A7C15ULL);
  z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
  z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
  return z ^ (z >> 31);
}
static inline void rng_seed(rng64 *r, uint64_t seed){
  r->s = seed ? seed : 0xDEADBEEFCAFEBABEULL;
}
static inline uint32_t rng_u32(rng64 *r){
  uint64_t x = splitmix64(&r->s);
  return (uint32_t)(x >> 32);
}
static inline float rng_f01(rng64 *r){
  return (rng_u32(r) / 4294967296.0f);
}
static inline float rng_frange(rng64 *r, float a, float b){
  return a + (b-a)*rng_f01(r);
}

// ---------- World / Obstacles ----------
typedef struct {
  v2 c;      // center
  float r;   // radius
} circle_ob;

static inline int circle_contains(circle_ob o, v2 p){
  return v2_len2(v2_sub(p,o.c)) <= o.r*o.r;
}

// ---------- Agent ----------
typedef enum { ST_IDLE=0, ST_SEEK=1, ST_FLEE=2 } state_t;

typedef struct {
  v2 p;       // position
  v2 v;       // velocity
  v2 a;       // acceleration (sum forces)
  state_t st;
  float hp;   // optional
} agent;

typedef struct {
  // motion
  float max_speed;
  float max_force;
  float dt;

  // neighborhood
  float neigh_r;      // for align/cohere
  float sep_r;        // for separation
  float avoid_r;      // obstacle influence

  // weights
  float w_seek;
  float w_flee;
  float w_avoid;
  float w_sep;
  float w_coh;
  float w_ali;

  // thresholds
  float arrive_r;     // slow down near target
  float flee_r;       // flee if player near
} params;

// ---------- Steering forces ----------
static v2 steer_seek(v2 p, v2 v, v2 target, float max_speed, float max_force, float arrive_r){
  v2 to = v2_sub(target, p);
  float d = v2_len(to);
  if(d < 1e-6f) return (v2){0,0};
  float sp = max_speed;
  if(d < arrive_r) sp = max_speed * (d/arrive_r); // arrive
  v2 desired = v2_mul(v2_norm(to), sp);
  v2 steer = v2_sub(desired, v);
  return v2_limit(steer, max_force);
}

static v2 steer_flee(v2 p, v2 v, v2 threat, float max_speed, float max_force){
  v2 away = v2_sub(p, threat);
  float d = v2_len(away);
  if(d < 1e-6f) return (v2){0,0};
  v2 desired = v2_mul(v2_norm(away), max_speed);
  v2 steer = v2_sub(desired, v);
  return v2_limit(steer, max_force);
}

// simple obstacle avoidance: push away when inside influence ring
static v2 steer_avoid(v2 p, circle_ob o, float avoid_r, float max_force){
  v2 d = v2_sub(p, o.c);
  float dist = v2_len(d);
  float R = o.r + avoid_r;
  if(dist >= R || dist < 1e-6f) return (v2){0,0};
  float t = 1.0f - (dist / R);     // 0..1
  // stronger near obstacle boundary
  v2 push = v2_mul(v2_norm(d), (t*t) * max_force);
  return push;
}

// boids: separation
static v2 steer_sep(agent *A, int n, int i, float sep_r, float max_force){
  v2 sum = {0,0};
  int cnt = 0;
  float r2 = sep_r*sep_r;
  for(int j=0;j<n;j++){
    if(j==i) continue;
    v2 d = v2_sub(A[i].p, A[j].p);
    float l2 = v2_len2(d);
    if(l2 > 1e-9f && l2 < r2){
      // inverse distance weighting
      sum = v2_add(sum, v2_mul(d, 1.0f/(l2 + 1e-6f)));
      cnt++;
    }
  }
  if(!cnt) return (v2){0,0};
  sum = v2_mul(sum, 1.0f/(float)cnt);
  v2 steer = v2_limit(sum, max_force);
  return steer;
}

// boids: cohesion (toward neighbor centroid)
static v2 steer_coh(agent *A, int n, int i, float neigh_r, float max_speed, float max_force){
  v2 center = {0,0};
  int cnt = 0;
  float r2 = neigh_r*neigh_r;
  for(int j=0;j<n;j++){
    if(j==i) continue;
    v2 d = v2_sub(A[j].p, A[i].p);
    if(v2_len2(d) < r2){
      center = v2_add(center, A[j].p);
      cnt++;
    }
  }
  if(!cnt) return (v2){0,0};
  center = v2_mul(center, 1.0f/(float)cnt);
  return steer_seek(A[i].p, A[i].v, center, max_speed, max_force, neigh_r);
}

// boids: alignment (match neighbor velocity)
static v2 steer_ali(agent *A, int n, int i, float neigh_r, float max_speed, float max_force){
  v2 avg = {0,0};
  int cnt = 0;
  float r2 = neigh_r*neigh_r;
  for(int j=0;j<n;j++){
    if(j==i) continue;
    v2 d = v2_sub(A[j].p, A[i].p);
    if(v2_len2(d) < r2){
      avg = v2_add(avg, A[j].v);
      cnt++;
    }
  }
  if(!cnt) return (v2){0,0};
  avg = v2_mul(avg, 1.0f/(float)cnt);
  v2 desired = v2_mul(v2_norm(avg), max_speed);
  v2 steer = v2_sub(desired, A[i].v);
  return v2_limit(steer, max_force);
}

// ---------- Update ----------
static void step_agents(
  agent *A, int n,
  v2 goal, v2 player,
  circle_ob *obs, int nobs,
  params P
){
  for(int i=0;i<n;i++){
    // FSM rule: if player near -> flee else seek goal
    float dp2 = v2_len2(v2_sub(A[i].p, player));
    if(dp2 < P.flee_r*P.flee_r) A[i].st = ST_FLEE;
    else A[i].st = ST_SEEK;

    v2 f = {0,0};

    // obstacle avoid (sum)
    v2 fav = {0,0};
    for(int k=0;k<nobs;k++){
      fav = v2_add(fav, steer_avoid(A[i].p, obs[k], P.avoid_r, P.max_force));
    }
    f = v2_add(f, v2_mul(fav, P.w_avoid));

    // boids
    v2 fsep = steer_sep(A, n, i, P.sep_r, P.max_force);
    v2 fcoh = steer_coh(A, n, i, P.neigh_r, P.max_speed, P.max_force);
    v2 fali = steer_ali(A, n, i, P.neigh_r, P.max_speed, P.max_force);
    f = v2_add(f, v2_mul(fsep, P.w_sep));
    f = v2_add(f, v2_mul(fcoh, P.w_coh));
    f = v2_add(f, v2_mul(fali, P.w_ali));

    // goal vs flee
    if(A[i].st == ST_SEEK){
      v2 fs = steer_seek(A[i].p, A[i].v, goal, P.max_speed, P.max_force, P.arrive_r);
      f = v2_add(f, v2_mul(fs, P.w_seek));
    } else if(A[i].st == ST_FLEE){
      v2 ff = steer_flee(A[i].p, A[i].v, player, P.max_speed, P.max_force);
      f = v2_add(f, v2_mul(ff, P.w_flee));
    }

    // integrate
    A[i].a = v2_limit(f, P.max_force);
  }

  for(int i=0;i<n;i++){
    A[i].v = v2_add(A[i].v, v2_mul(A[i].a, P.dt));
    A[i].v = v2_limit(A[i].v, P.max_speed);
    A[i].p = v2_add(A[i].p, v2_mul(A[i].v, P.dt));
  }
}

// ---------- Demo ----------
static void print_agents(agent *A, int n, int t){
  printf("t=%04d\n", t);
  for(int i=0;i<n;i++){
    printf("  a%02d st=%d p=(%7.2f,%7.2f) v=(%6.2f,%6.2f)\n",
      i, (int)A[i].st, A[i].p.x, A[i].p.y, A[i].v.x, A[i].v.y);
  }
}

int main(int argc, char **argv){
  int N = 12;
  int STEPS = 600;
  uint64_t seed = 42;

  if(argc >= 2) seed = (uint64_t)strtoull(argv[1], NULL, 10);
  if(argc >= 3) N = atoi(argv[2]);
  if(argc >= 4) STEPS = atoi(argv[3]);
  if(N < 1) N = 1;

  rng64 R; rng_seed(&R, seed);

  agent *A = (agent*)calloc((size_t)N, sizeof(agent));
  if(!A){ fprintf(stderr,"oom\n"); return 1; }

  // init agents in a blob
  for(int i=0;i<N;i++){
    float ang = rng_frange(&R, 0.0f, 2.0f*(float)M_PI);
    float rad = rng_frange(&R, 0.0f, 15.0f);
    A[i].p = (v2){ cosf(ang)*rad, sinf(ang)*rad };
    A[i].v = (v2){ rng_frange(&R,-1.0f,1.0f), rng_frange(&R,-1.0f,1.0f) };
    A[i].st = ST_SEEK;
    A[i].hp = 1.0f;
  }

  // obstacles
  circle_ob obs[2] = {
    { .c = { 12,  0 }, .r = 6 },
    { .c = {-10, -6 }, .r = 5 }
  };

  params P = {
    .max_speed = 6.0f,
    .max_force = 8.0f,
    .dt = 0.04f,

    .neigh_r = 10.0f,
    .sep_r   = 4.0f,
    .avoid_r = 6.0f,

    .w_seek  = 1.20f,
    .w_flee  = 1.60f,
    .w_avoid = 1.40f,
    .w_sep   = 1.10f,
    .w_coh   = 0.55f,
    .w_ali   = 0.35f,

    .arrive_r = 8.0f,
    .flee_r   = 7.5f
  };

  v2 goal   = { 40,  0 };
  v2 player = {  0,  0 };

  for(int t=0;t<STEPS;t++){
    // move player in a circle (demo)
    float a = 0.01f*(float)t;
    player.x = 10.0f*cosf(a);
    player.y = 10.0f*sinf(a);

    // goal drifting (demo)
    goal.y = 10.0f*sinf(0.004f*(float)t);

    step_agents(A, N, goal, player, obs, 2, P);

    if((t % 60) == 0){
      printf("player=(%.2f,%.2f) goal=(%.2f,%.2f)\n",
             player.x, player.y, goal.x, goal.y);
      print_agents(A, N, t);
    }
  }

  free(A);
  return 0;
}
