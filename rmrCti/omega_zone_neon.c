// OMEGA ZONE NEON — streaming + zonas térmicas + eventos JSONL
// Termux/Android ARM64 | syscall-only | NEON intrinsics | NO libc
typedef unsigned long long u64;
typedef long long i64;
typedef unsigned char u8;

#define SYS_READ   63
#define SYS_WRITE  64
#define SYS_OPENAT 56
#define SYS_CLOSE  57
#define SYS_EXIT   93

#define AT_FDCWD   -100
#define O_CREAT    0100
#define O_TRUNC   01000
#define O_WRONLY     01

#include <arm_neon.h>

static inline long _sc(long n, long a, long b, long c, long d) {
  register long x0 __asm__("x0") = a;
  register long x1 __asm__("x1") = b;
  register long x2 __asm__("x2") = c;
  register long x3 __asm__("x3") = d;
  register long x8 __asm__("x8") = n;
  __asm__ __volatile__("svc 0"
    : "+r"(x0)
    : "r"(x1), "r"(x2), "r"(x3), "r"(x8)
    : "memory");
  return x0;
}
static inline long sc3(long n,long a,long b,long c){ return _sc(n,a,b,c,0); }
static inline long sc4(long n,long a,long b,long c,long d){ return _sc(n,a,b,c,d); }

static u8 buf[1<<16];     // 64KB hot buffer
static char line[1024];   // JSONL line buffer

typedef struct {
  u64 bytes, hibit, space, nl, brace, quote, colon, comma;
} Cnt;

static inline u64 sum_mask_u8(uint8x16_t mask_ff) {
  uint8x16_t ones = vshrq_n_u8(mask_ff, 7);
  return (u64)vaddvq_u8(ones);
}

static inline void omega_engine_neon(Cnt* c, const u8* p, u64 n) {
  u64 aligned = n & ~15ULL;
  u64 rem = n & 15ULL;

  const uint8x16_t v_space = vdupq_n_u8((u8)' ');
  const uint8x16_t v_nl    = vdupq_n_u8((u8)'\n');
  const uint8x16_t v_lc    = vdupq_n_u8((u8)'{');
  const uint8x16_t v_rc    = vdupq_n_u8((u8)'}');
  const uint8x16_t v_q     = vdupq_n_u8((u8)'"');
  const uint8x16_t v_col   = vdupq_n_u8((u8)':');
  const uint8x16_t v_com   = vdupq_n_u8((u8)',');

  while (aligned) {
    uint8x16_t v = vld1q_u8(p);

    c->hibit += (u64)vaddvq_u8(vshrq_n_u8(v, 7));
    c->space += sum_mask_u8(vceqq_u8(v, v_space));
    c->nl    += sum_mask_u8(vceqq_u8(v, v_nl));
    c->quote += sum_mask_u8(vceqq_u8(v, v_q));
    c->colon += sum_mask_u8(vceqq_u8(v, v_col));
    c->comma += sum_mask_u8(vceqq_u8(v, v_com));

    uint8x16_t ml = vceqq_u8(v, v_lc);
    uint8x16_t mr = vceqq_u8(v, v_rc);
    c->brace += sum_mask_u8(vorrq_u8(ml, mr));

    c->bytes += 16;
    p += 16;
    aligned -= 16;
  }

  while (rem--) {
    u8 x = *p++;
    c->bytes++;
    if (x & 0x80) c->hibit++;
    if (x == ' ') c->space++;
    if (x == '\n') c->nl++;
    if (x == '{' || x == '}') c->brace++;
    if (x == '"') c->quote++;
    if (x == ':') c->colon++;
    if (x == ',') c->comma++;
  }
}

static inline void wstr(int fd, const char* s, u64 n) {
  sc3(SYS_WRITE, fd, (long)s, (long)n);
}

static char* u64_to_dec(char* p, u64 v) {
  char tmp[32];
  int i=0;
  if (!v) { *p++='0'; return p; }
  while (v) { tmp[i++] = (char)('0' + (v%10)); v/=10; }
  while (i--) *p++ = tmp[i];
  return p;
}

// ppm = (a*1e6)/b (overflow-safe)
static u64 ppm(u64 a, u64 b){
  if (!b) return 0;
  u64 q = a / b;
  u64 r = a % b;
  return q*1000000ULL + (r*1000000ULL)/b;
}
static inline u64 abs64(u64 a, u64 b){ return a>b ? a-b : b-a; }

// append literal
static inline char* app(char* p, const char* s){
  while (*s) *p++ = *s++;
  return p;
}

__attribute__((used,noinline))
void omega_main(void) {
  const u64 ZONE_BYTES = 8ULL*1024*1024;   // 8MB por zona térmica
  const u64 EVENT_THR_PPM = 45000;         // ~4.5% em soma de deltas (ppm)

  int fd = (int)sc4(SYS_OPENAT, AT_FDCWD, (long)"omega_events.jsonl",
                    O_WRONLY|O_CREAT|O_TRUNC, 0600);
  if (fd < 0) fd = 1;

  wstr(1, "[OMEGA ZONE NEON] events -> omega_events.jsonl\n", 49);

  Cnt cur = (Cnt){0}, prev = (Cnt){0};
  u64 zone_bytes = 0, zone_idx = 0, off_total = 0;
  int have_prev = 0;

  while (1) {
    i64 r = sc3(SYS_READ, 0, (long)buf, sizeof(buf));
    if (r <= 0) break;

    omega_engine_neon(&cur, buf, (u64)r);
    zone_bytes += (u64)r;
    off_total  += (u64)r;

    if (zone_bytes >= ZONE_BYTES) {
      u64 b = cur.bytes;

      u64 r_space = ppm(cur.space, b);
      u64 r_quote = ppm(cur.quote, b);
      u64 r_brace = ppm(cur.brace, b);
      u64 r_colon = ppm(cur.colon, b);
      u64 r_comma = ppm(cur.comma, b);
      u64 r_hibit = ppm(cur.hibit, b);

      u64 dsum = 0;
      if (have_prev) {
        dsum += abs64(r_space, ppm(prev.space, prev.bytes));
        dsum += abs64(r_quote, ppm(prev.quote, prev.bytes));
        dsum += abs64(r_brace, ppm(prev.brace, prev.bytes));
        dsum += abs64(r_colon, ppm(prev.colon, prev.bytes));
        dsum += abs64(r_comma, ppm(prev.comma, prev.bytes));
        dsum += abs64(r_hibit, ppm(prev.hibit, prev.bytes));
      }

      char* p = line;
      *p++='{';

      p = app(p, "\"zone\":");   p = u64_to_dec(p, zone_idx); *p++=',';
      p = app(p, "\"off\":");    p = u64_to_dec(p, off_total); *p++=',';
      p = app(p, "\"zbytes\":"); p = u64_to_dec(p, b); *p++=',';

      p = app(p, "\"ppm\":{");
      p = app(p, "\"space\":"); p = u64_to_dec(p, r_space); *p++=',';
      p = app(p, "\"quote\":"); p = u64_to_dec(p, r_quote); *p++=',';
      p = app(p, "\"brace\":"); p = u64_to_dec(p, r_brace); *p++=',';
      p = app(p, "\"colon\":"); p = u64_to_dec(p, r_colon); *p++=',';
      p = app(p, "\"comma\":"); p = u64_to_dec(p, r_comma); *p++=',';
      p = app(p, "\"hibit\":"); p = u64_to_dec(p, r_hibit);
      *p++='}'; *p++=',';

      p = app(p, "\"dppm\":"); p = u64_to_dec(p, dsum); *p++=',';
      p = app(p, "\"event\":");

      if (!have_prev)       p = app(p, "\"boot\"");
      else if (dsum > EVENT_THR_PPM) p = app(p, "\"regime\"");
      else                  p = app(p, "\"stable\"");

      *p++='}'; *p++='\n';

      wstr(fd, line, (u64)(p - line));

      prev = cur;
      have_prev = 1;
      cur = (Cnt){0};
      zone_bytes = 0;
      zone_idx++;
    }
  }

  if (fd != 1) sc3(SYS_CLOSE, fd, 0, 0);
  sc3(SYS_EXIT, 0, 0, 0);
}

__attribute__((naked))
void _start(void) {
  __asm__ __volatile__(
    "mov x29, xzr\n"
    "mov x30, xzr\n"
    "bl omega_main\n"
  );
}
