#!/data/data/com.termux/files/usr/bin/bash
set -e

cat > rafa_graph_patch_2048_nolib.c <<'C_EOF'
/*
  rafa_graph_patch_2048_nolib.c
  RAFAELIA :: 100x2048-bit graph + 10 patches 5x5 overlay + invariants
  FREESTANDING / NO LIBC / NO INCLUDES

  Build (Android/Termux aarch64 PIE):
    clang -O3 -fno-builtin -fno-stack-protector -nostdlib -ffreestanding \
      -fPIE -pie -Wl,-e,_start -Wl,-dynamic-linker,/system/bin/linker64 \
      rafa_graph_patch_2048_nolib.c -o rafa_graph_patch_2048
*/

typedef unsigned long long u64;
typedef unsigned int       u32;
typedef unsigned char      u8;
typedef unsigned long      uptr;

#define N        100
#define BITS     2048
#define WORDS    (BITS / 64)
#define PATCHES  10
#define PATCH_N  25

/* ---------- syscalls (Linux/Android aarch64) ---------- */
static inline long sys_write(int fd, const void *buf, uptr len) {
  register long x0 asm("x0") = (long)fd;
  register long x1 asm("x1") = (long)buf;
  register long x2 asm("x2") = (long)len;
  register long x8 asm("x8") = 64; /* __NR_write */
  asm volatile("svc #0" : "+r"(x0) : "r"(x1), "r"(x2), "r"(x8) : "memory");
  return x0;
}

__attribute__((noreturn))
static inline void sys_exit(int code) {
  register long x0 asm("x0") = (long)code;
  register long x8 asm("x8") = 93; /* __NR_exit */
  asm volatile("svc #0" : : "r"(x0), "r"(x8) : "memory");
  __builtin_unreachable();
}

/* ---------- tiny utils (no libc) ---------- */
static inline void *my_memcpy(void *dst, const void *src, uptr n) {
  u8 *d = (u8*)dst;
  const u8 *s = (const u8*)src;
  for (uptr i = 0; i < n; i++) d[i] = s[i];
  return dst;
}

static inline void *my_memset(void *dst, int v, uptr n) {
  u8 *d = (u8*)dst;
  u8 b = (u8)v;
  for (uptr i = 0; i < n; i++) d[i] = b;
  return dst;
}

static inline uptr my_strlen(const char *s) {
  uptr n = 0;
  while (s[n]) n++;
  return n;
}

static inline void out_str(const char *s) { sys_write(1, s, my_strlen(s)); }
static inline void out_ch(char c) { sys_write(1, &c, 1); }
static inline void out_nl(void) { out_ch('\n'); }

static inline void out_u64(u64 x) {
  char buf[32];
  int i = 0;
  if (x == 0) { out_ch('0'); return; }
  while (x > 0 && i < 31) {
    u64 q = x / 10ULL;
    u64 r = x - q * 10ULL;
    buf[i++] = (char)('0' + (char)r);
    x = q;
  }
  while (i--) out_ch(buf[i]);
}

static inline void out_u32(u32 x) { out_u64((u64)x); }

static inline u32 parse_u32(const char *s) {
  u32 v = 0;
  while (*s >= '0' && *s <= '9') {
    v = (u32)(v * 10u + (u32)(*s - '0'));
    s++;
  }
  return v;
}

/* ---------- bit ops ---------- */
static inline u32 xorshift32(u32 *st) {
  u32 x = *st;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  *st = x;
  return x;
}

static inline u64 rotl64(u64 x, int r) {
  return (x << r) | (x >> (64 - r));
}

static inline int popcnt64(u64 x) {
  x = x - ((x >> 1) & 0x5555555555555555ULL);
  x = (x & 0x3333333333333333ULL) + ((x >> 2) & 0x3333333333333333ULL);
  x = (x + (x >> 4)) & 0x0F0F0F0F0F0F0F0FULL;
  return (int)((x * 0x0101010101010101ULL) >> 56);
}

/* ---------- core data ---------- */
typedef struct __attribute__((aligned(64))) {
  u64 w[WORDS]; /* 256 bytes */
} bitrow2048_t;

typedef struct {
  bitrow2048_t row[N];
} graph_t;

/* ---------- bitset primitives ---------- */
static inline void row_zero(bitrow2048_t *r) {
  for (int i = 0; i < WORDS; i++) r->w[i] = 0ULL;
}

static inline void bitset_set(bitrow2048_t *r, int bit) {
  int wi = bit >> 6;
  int bi = bit & 63;
  r->w[wi] |= (1ULL << (u64)bi);
}

static inline int bitset_test(const bitrow2048_t *r, int bit) {
  int wi = bit >> 6;
  int bi = bit & 63;
  return (int)((r->w[wi] >> (u64)bi) & 1ULL);
}

static inline void row_or(bitrow2048_t *dst, const bitrow2048_t *src) {
  for (int i = 0; i < WORDS; i++) dst->w[i] |= src->w[i];
}

static inline u64 row_popcount(const bitrow2048_t *r) {
  u64 s = 0;
  for (int i = 0; i < WORDS; i++) s += (u64)popcnt64(r->w[i]);
  return s;
}

static inline void graph_copy(graph_t *dst, const graph_t *src) {
  my_memcpy(dst, src, (uptr)sizeof(graph_t));
}

static inline u64 graph_pop_total(const graph_t *g) {
  u64 total = 0;
  for (int i = 0; i < N; i++) total += row_popcount(&g->row[i]);
  return total;
}

static inline u64 graph_flip_total(const graph_t *g, const graph_t *prev) {
  u64 flips = 0;
  for (int i = 0; i < N; i++) {
    for (int w = 0; w < WORDS; w++) {
      u64 x = g->row[i].w[w] ^ prev->row[i].w[w];
      flips += (u64)popcnt64(x);
    }
  }
  return flips;
}

/* ---------- initialization ---------- */
static void graph_init_background(graph_t *g, u32 seed) {
  for (int i = 0; i < N; i++) row_zero(&g->row[i]);

  for (int i = 0; i < N; i++) {
    u32 s = seed ^ (u32)(0x9E3779B9u * (u32)(i + 1));

    for (int k = 0; k < 16; k++) {
      u32 r = xorshift32(&s);
      int bit = (int)(r & (BITS - 1));
      bitset_set(&g->row[i], bit);
    }

    int lane = (i * 17) & (BITS - 1);
    bitset_set(&g->row[i], lane);
    bitset_set(&g->row[i], (lane + 1) & (BITS - 1));
  }
}

static void build_patch_maps(u8 patch_map[PATCHES][PATCH_N], u32 seed) {
  for (int p = 0; p < PATCHES; p++) {
    u32 s = seed ^ (u32)(0xA341316Cu + (u32)(97u * (u32)p));
    u8 used[N];
    my_memset(used, 0, (uptr)N);

    for (int j = 0; j < PATCH_N; j++) {
      int pick;
      do { pick = (int)(xorshift32(&s) % (u32)N); } while (used[pick]);
      used[pick] = 1;
      patch_map[p][j] = (u8)pick;
    }
  }
}

/* ---------- patch overlay ---------- */
static void apply_patch_overlay(graph_t *g, const u8 patch_map[PATCHES][PATCH_N], int p, u32 tick) {
  int phase = (int)(tick & (BITS - 1));
  for (int lu = 0; lu < PATCH_N; lu++) {
    int u = (int)patch_map[p][lu];
    for (int lv = 0; lv < PATCH_N; lv++) {
      if (lv == lu) continue;
      int v = (int)patch_map[p][lv];
      int slot = (p * PATCH_N + lv) % 20;
      int bit  = (v + 100 * slot + phase) & (BITS - 1);
      bitset_set(&g->row[u], bit);
    }
  }
}

/* ---------- diffusion ---------- */
static void diffuse_step(graph_t *g, u32 tick) {
  int r = (int)(1 + (tick % 7));
  bitrow2048_t tmp;

  for (int i = 0; i < N; i++) {
    for (int w = 0; w < WORDS; w++) tmp.w[w] = rotl64(g->row[i].w[w], r);
    row_or(&g->row[i], &tmp);
    int j = (i + 1) % N;
    row_or(&g->row[i], &g->row[j]);
  }
}

/* ---------- minimal entry (_start) ---------- */
__attribute__((noreturn))
void _start(void) {
  uptr *sp;
  asm volatile("mov %0, sp" : "=r"(sp));

  int argc = (int)sp[0];
  char **argv = (char**)&sp[1];

  u32 ticks = 200;
  u32 seed  = 1337;

  if (argc >= 2) ticks = parse_u32(argv[1]);
  if (argc >= 3) seed  = parse_u32(argv[2]);

  graph_t g, prev;
  u8 patch_map[PATCHES][PATCH_N];

  graph_init_background(&g, seed);
  build_patch_maps(patch_map, seed ^ 0xC0FFEEu);
  graph_copy(&prev, &g);

  const u64 total_bits = (u64)N * (u64)BITS;

  out_str("RAFAELIA :: graph N="); out_u32((u32)N);
  out_str(" BITS="); out_u32((u32)BITS);
  out_str(" row_bytes="); out_u32((u32)(WORDS * 8));
  out_str(" total_kb="); out_u64((u64)(sizeof(graph_t) / 1024ULL));
  out_nl();

  out_str("ticks="); out_u32(ticks);
  out_str(" seed="); out_u32(seed);
  out_str(" patches="); out_u32((u32)PATCHES);
  out_str(" patch_n="); out_u32((u32)PATCH_N);
  out_nl(); out_nl();

  for (u32 t = 1; t <= ticks; t++) {
    graph_copy(&prev, &g);

    int p = (int)(t % (u32)PATCHES);
    apply_patch_overlay(&g, patch_map, p, t);
    diffuse_step(&g, t);

    u64 pop_total  = graph_pop_total(&g);
    u64 flip_total = graph_flip_total(&g, &prev);

    if (t <= 5 || (t % 25u == 0u) || t == ticks) {
      out_str("t="); out_u32(t);
      out_str(" patch="); out_u32((u32)p);
      out_str(" pop="); out_u64(pop_total);
      out_str(" flips="); out_u64(flip_total);

      u64 coherence_ppm = 0;
      if (flip_total >= total_bits) coherence_ppm = 0;
      else coherence_ppm = (u64)(1000000ULL - (flip_total * 1000000ULL) / total_bits);

      out_str(" coherence_ppm="); out_u64(coherence_ppm);
      out_nl();
    }
  }

  out_nl();
  out_str("Sanity: node=0 bit=123 => ");
  out_u32((u32)bitset_test(&g.row[0], 123));
  out_nl();

  sys_exit(0);
}
C_EOF

echo "==[A] build (PIE/ET_DYN; no libc; no includes)=="
clang -O3 -ffreestanding -fno-builtin -fno-stack-protector -nostdlib \
  -fPIE -pie -Wl,-e,_start -Wl,-dynamic-linker,/system/bin/linker64 \
  rafa_graph_patch_2048_nolib.c -o rafa_graph_patch_2048

echo "==[B] verify ELF type =="
readelf -h rafa_graph_patch_2048 | grep -E 'Type:|Machine:|Entry point|OS/ABI' || true

echo
echo "==[C] RUN default =="
./rafa_graph_patch_2048 || true

echo
echo "==[D] RUN 200 1337 =="
./rafa_graph_patch_2048 200 1337 || true

echo
echo "==[E] RUN 1000 42 =="
./rafa_graph_patch_2048 1000 42 || true

echo
echo "OK :: done ✅"
