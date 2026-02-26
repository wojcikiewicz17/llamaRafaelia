// OMEGA V4B — TEXTURE NEON (Termux/Android ARM64) — NO inline ASM
typedef unsigned long long u64;
typedef long long i64;
typedef unsigned char u8;

/* syscalls ARM64 */
#define SYS_READ   63
#define SYS_WRITE  64
#define SYS_EXIT   93
#define STDOUT     1

#include <arm_neon.h>

/* ---- BSS ---- */
static struct {
  u64 bytes;
  u64 hibit;
  u64 space;
  u64 nl;
  u64 brace;
  u64 quote;
  u64 colon;
  u64 comma;
} m;

static u8 buf[1<<16];
static char out[21];

/* syscall direto */
static inline long _sc(long n, long a, long b, long c) {
  register long x0 __asm__("x0") = a;
  register long x1 __asm__("x1") = b;
  register long x2 __asm__("x2") = c;
  register long x8 __asm__("x8") = n;
  __asm__ __volatile__("svc 0"
    : "+r"(x0)
    : "r"(x1), "r"(x2), "r"(x8)
    : "memory");
  return x0;
}

/* u64 → string (NO-LIBC) */
static void u64_to_str(u64 n) {
  int i = 19; out[20] = 0;
  if (!n) out[i--] = '0';
  while (n && i >= 0) { out[i--] = (n % 10) + '0'; n /= 10; }
  while (i >= 0) out[i--] = ' ';
}

static inline u64 sum_mask_u8(uint8x16_t mask_ff) {
  // mask é 0x00/0xFF; vira 0/1 com shift >>7 e soma horizontal
  uint8x16_t ones = vshrq_n_u8(mask_ff, 7);
  return (u64)vaddvq_u8(ones);
}

static void omega_engine_neon(const u8* p, u64 n) {
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

    // hibit: (byte >> 7)
    m.hibit += (u64)vaddvq_u8(vshrq_n_u8(v, 7));

    // space / nl / quote / colon / comma
    m.space += sum_mask_u8(vceqq_u8(v, v_space));
    m.nl    += sum_mask_u8(vceqq_u8(v, v_nl));
    m.quote += sum_mask_u8(vceqq_u8(v, v_q));
    m.colon += sum_mask_u8(vceqq_u8(v, v_col));
    m.comma += sum_mask_u8(vceqq_u8(v, v_com));

    // brace: '{' OR '}'
    uint8x16_t ml = vceqq_u8(v, v_lc);
    uint8x16_t mr = vceqq_u8(v, v_rc);
    m.brace += sum_mask_u8(vorrq_u8(ml, mr));

    m.bytes += 16;

    p += 16;
    aligned -= 16;
  }

  while (rem--) {
    u8 c = *p++;
    m.bytes++;
    if (c & 0x80) m.hibit++;
    if (c == ' ') m.space++;
    if (c == '\n') m.nl++;
    if (c == '{' || c == '}') m.brace++;
    if (c == '"') m.quote++;
    if (c == ':') m.colon++;
    if (c == ',') m.comma++;
  }
}

__attribute__((used,noinline))
void omega_main(void) {
  _sc(SYS_WRITE, STDOUT, (long)"[OMEGA TEXTURE NEON]\n", 21);

  while (1) {
    i64 r = _sc(SYS_READ, 0, (long)buf, sizeof(buf));
    if (r <= 0) break;
    omega_engine_neon(buf, (u64)r);
  }

  #define P(lbl,val) do{ \
    _sc(SYS_WRITE, STDOUT, (long)lbl, sizeof(lbl)-1); \
    u64_to_str(val); _sc(SYS_WRITE, STDOUT, (long)out, 21); \
    _sc(SYS_WRITE, STDOUT, (long)"\n", 1); \
  }while(0)

  P("BYTES:  ", m.bytes);
  P("HIBIT:  ", m.hibit);
  P("SPACE:  ", m.space);
  P("NL:     ", m.nl);
  P("BRACE:  ", m.brace);
  P("QUOTE:  ", m.quote);
  P("COLON:  ", m.colon);
  P("COMMA:  ", m.comma);

  _sc(SYS_EXIT, 0, 0, 0);
}

__attribute__((naked))
void _start(void) {
  __asm__ __volatile__(
    "mov x29, xzr\n"
    "mov x30, xzr\n"
    "bl omega_main\n"
  );
}
