typedef unsigned long long u64;
typedef long long i64;
typedef unsigned char u8;

/* SYSCALL ABI ARM64 (Linux kernel underneath Android) */
#define SYS_READ   63
#define SYS_WRITE  64
#define SYS_EXIT   93
#define STDOUT     1

/* ---- BSS ---- */
static struct {
  u64 bytes;
  u64 hibit;     /* bytes with bit7 set (>=128) */
  u64 space;     /* 0x20 */
  u64 nl;        /* '\n' */
  u64 brace;     /* '{' + '}' */
  u64 quote;     /* '"' */
  u64 colon;     /* ':' */
  u64 comma;     /* ',' */
} m;

static char buf[65536];
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

/* NEON texture engine (counts exact 0/1) */
static void omega_texture(const char* p, u64 len) {
  u64 aligned = len & ~0xFULL;
  u64 rem = len & 0xFULL;

  if (aligned) {
    /* constants in NEON regs */
    __asm__ __volatile__(
      "movi v1.16b, #32\n\t"   /* space */
      "movi v2.16b, #10\n\t"   /* \\n */
      "movi v3.16b, #123\n\t"  /* { */
      "movi v4.16b, #125\n\t"  /* } */
      "movi v5.16b, #34\n\t"   /* \" */
      "movi v6.16b, #58\n\t"   /* : */
      "movi v7.16b, #44\n\t"   /* , */
      "1:\n\t"
      "ld1 {v0.16b}, [%0], #16\n\t"

      /* hibit: (byte >> 7) gives 0/1 */
      "ushr v8.16b, v0.16b, #7\n\t"
      "uaddlv h8, v8.16b\n\t"         /* sum 16 lanes -> h8 */
      "umov w9, v8.h[0]\n\t"
      "add %1, %1, x9\n\t"

      /* space */
      "cmeq v10.16b, v0.16b, v1.16b\n\t"
      "ushr v10.16b, v10.16b, #7\n\t"
      "uaddlv h10, v10.16b\n\t"
      "umov w9, v10.h[0]\n\t"
      "add %2, %2, x9\n\t"

      /* nl */
      "cmeq v11.16b, v0.16b, v2.16b\n\t"
      "ushr v11.16b, v11.16b, #7\n\t"
      "uaddlv h11, v11.16b\n\t"
      "umov w9, v11.h[0]\n\t"
      "add %3, %3, x9\n\t"

      /* brace: '{' or '}' */
      "cmeq v12.16b, v0.16b, v3.16b\n\t"
      "cmeq v13.16b, v0.16b, v4.16b\n\t"
      "orr  v12.16b, v12.16b, v13.16b\n\t"
      "ushr v12.16b, v12.16b, #7\n\t"
      "uaddlv h12, v12.16b\n\t"
      "umov w9, v12.h[0]\n\t"
      "add %4, %4, x9\n\t"

      /* quote */
      "cmeq v14.16b, v0.16b, v5.16b\n\t"
      "ushr v14.16b, v14.16b, #7\n\t"
      "uaddlv h14, v14.16b\n\t"
      "umov w9, v14.h[0]\n\t"
      "add %5, %5, x9\n\t"

      /* colon */
      "cmeq v15.16b, v0.16b, v6.16b\n\t"
      "ushr v15.16b, v15.16b, #7\n\t"
      "uaddlv h15, v15.16b\n\t"
      "umov w9, v15.h[0]\n\t"
      "add %6, %6, x9\n\t"

      /* comma */
      "cmeq v16.16b, v0.16b, v7.16b\n\t"
      "ushr v16.16b, v16.16b, #7\n\t"
      "uaddlv h16, v16.16b\n\t"
      "umov w9, v16.h[0]\n\t"
      "add %7, %7, x9\n\t"

      "subs %8, %8, #16\n\t"
      "b.ne 1b\n\t"
      : "+r"(p),
        "+r"(m.hibit),
        "+r"(m.space),
        "+r"(m.nl),
        "+r"(m.brace),
        "+r"(m.quote),
        "+r"(m.colon),
        "+r"(m.comma),
        "+r"(aligned)
      :
      : "x9",
        "v0","v1","v2","v3","v4","v5","v6","v7",
        "v8","v10","v11","v12","v13","v14","v15","v16",
        "memory"
    );
  }

  while (rem--) {
    u8 c = (u8)*p++;
    if (c & 0x80) m.hibit++;
    if (c == 32)  m.space++;
    if (c == 10)  m.nl++;
    if (c == 123 || c == 125) m.brace++;
    if (c == 34)  m.quote++;
    if (c == 58)  m.colon++;
    if (c == 44)  m.comma++;
  }
}

__attribute__((used,noinline))
void omega_main(void) {
  const char *hdr = "[OMEGA V4B ZIPTEXTURE ACTIVE - ANDROID(LINUX) ARM64]\n";
  _sc(SYS_WRITE, STDOUT, (long)hdr, 56);

  while (1) {
    i64 r = _sc(SYS_READ, 0, (long)buf, sizeof(buf));
    if (r <= 0) break;
    omega_texture(buf, (u64)r);
    m.bytes += (u64)r;
  }

  _sc(SYS_WRITE, STDOUT, (long)"BYTES: ", 7); u64_to_str(m.bytes); _sc(SYS_WRITE, STDOUT, (long)out, 21);
  _sc(SYS_WRITE, STDOUT, (long)"\nHIBIT: ", 8); u64_to_str(m.hibit); _sc(SYS_WRITE, STDOUT, (long)out, 21);
  _sc(SYS_WRITE, STDOUT, (long)"\nSPACE: ", 8); u64_to_str(m.space); _sc(SYS_WRITE, STDOUT, (long)out, 21);
  _sc(SYS_WRITE, STDOUT, (long)"\nNL:    ", 8); u64_to_str(m.nl); _sc(SYS_WRITE, STDOUT, (long)out, 21);
  _sc(SYS_WRITE, STDOUT, (long)"\nBRACE: ", 8); u64_to_str(m.brace); _sc(SYS_WRITE, STDOUT, (long)out, 21);
  _sc(SYS_WRITE, STDOUT, (long)"\nQUOTE: ", 8); u64_to_str(m.quote); _sc(SYS_WRITE, STDOUT, (long)out, 21);
  _sc(SYS_WRITE, STDOUT, (long)"\nCOLON: ", 8); u64_to_str(m.colon); _sc(SYS_WRITE, STDOUT, (long)out, 21);
  _sc(SYS_WRITE, STDOUT, (long)"\nCOMMA: ", 8); u64_to_str(m.comma); _sc(SYS_WRITE, STDOUT, (long)out, 21);
  _sc(SYS_WRITE, STDOUT, (long)"\n", 1);

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
