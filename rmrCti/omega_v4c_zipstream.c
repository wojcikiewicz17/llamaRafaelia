typedef unsigned long long u64;
typedef unsigned int u32;
typedef unsigned short u16;
typedef long long i64;
typedef unsigned char u8;

/* syscalls ARM64 */
#define SYS_READ   63
#define SYS_WRITE  64
#define SYS_EXIT   93
#define STDOUT     1

static struct {
  u64 bytes;
  u64 hibit;
  u64 space;
  u64 nl;
  u64 brace;
  u64 quote;
  u64 colon;
  u64 comma;
  u64 files_ok;
  u64 files_skip;
} m;

static u8 buf[65536];
static char out[21];

/* syscall */
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

/* u64 → string */
static void u64_to_str(u64 n) {
  int i = 19; out[20] = 0;
  if (!n) out[i--] = '0';
  while (n && i >= 0) { out[i--] = (n % 10) + '0'; n /= 10; }
  while (i >= 0) out[i--] = ' ';
}

/* texture scalar (fallback; simples e correto) */
static void omega_texture_scalar(const u8* p, u64 len) {
  while (len--) {
    u8 c = *p++;
    if (c & 0x80) m.hibit++;
    if (c == 32)  m.space++;
    if (c == 10)  m.nl++;
    if (c == 123 || c == 125) m.brace++;
    if (c == 34)  m.quote++;
    if (c == 58)  m.colon++;
    if (c == 44)  m.comma++;
    m.bytes++;
  }
}

/* leitura exata */
static int read_exact(u8* p, u64 n) {
  while (n) {
    i64 r = _sc(SYS_READ, 0, (long)p, n);
    if (r <= 0) return -1;
    p += r;
    n -= r;
  }
  return 0;
}

/* ZIP streaming: só STORED */
static void omega_zipstream(void) {
  while (1) {
    u32 sig;
    if (read_exact((u8*)&sig, 4) < 0) break;
    if (sig != 0x04034b50) break; /* PK\003\004 */

    u8 hdr[26];
    if (read_exact(hdr, 26) < 0) break;

    u16 method = *(u16*)(hdr + 4);
    u32 comp_sz = *(u32*)(hdr + 14);
    u16 fn_len  = *(u16*)(hdr + 22);
    u16 ex_len  = *(u16*)(hdr + 24);

    /* skip filename + extra */
    for (u64 skip = fn_len + ex_len; skip; ) {
      u64 k = skip > sizeof(buf) ? sizeof(buf) : skip;
      if (read_exact(buf, k) < 0) return;
      skip -= k;
    }

    if (method == 0) {
      /* STORED */
      u64 left = comp_sz;
      while (left) {
        u64 k = left > sizeof(buf) ? sizeof(buf) : left;
        if (read_exact(buf, k) < 0) return;
        omega_texture_scalar(buf, k);
        left -= k;
      }
      m.files_ok++;
    } else {
      /* compressed → skip */
      u64 skip = comp_sz;
      while (skip) {
        u64 k = skip > sizeof(buf) ? sizeof(buf) : skip;
        if (read_exact(buf, k) < 0) return;
        skip -= k;
      }
      m.files_skip++;
    }
  }
}

__attribute__((used,noinline))
void omega_main(void) {
  const char *hdr = "[OMEGA V4C ZIPSTREAM - STORED ONLY]\n";
  _sc(SYS_WRITE, STDOUT, (long)hdr, 40);

  omega_zipstream();

  _sc(SYS_WRITE, STDOUT, (long)"BYTES: ", 7); u64_to_str(m.bytes); _sc(SYS_WRITE, STDOUT, (long)out, 21);
  _sc(SYS_WRITE, STDOUT, (long)"\nHIBIT: ", 8); u64_to_str(m.hibit); _sc(SYS_WRITE, STDOUT, (long)out, 21);
  _sc(SYS_WRITE, STDOUT, (long)"\nSPACE: ", 8); u64_to_str(m.space); _sc(SYS_WRITE, STDOUT, (long)out, 21);
  _sc(SYS_WRITE, STDOUT, (long)"\nNL:    ", 8); u64_to_str(m.nl); _sc(SYS_WRITE, STDOUT, (long)out, 21);
  _sc(SYS_WRITE, STDOUT, (long)"\nBRACE: ", 8); u64_to_str(m.brace); _sc(SYS_WRITE, STDOUT, (long)out, 21);
  _sc(SYS_WRITE, STDOUT, (long)"\nQUOTE: ", 8); u64_to_str(m.quote); _sc(SYS_WRITE, STDOUT, (long)out, 21);
  _sc(SYS_WRITE, STDOUT, (long)"\nCOLON: ", 8); u64_to_str(m.colon); _sc(SYS_WRITE, STDOUT, (long)out, 21);
  _sc(SYS_WRITE, STDOUT, (long)"\nCOMMA: ", 8); u64_to_str(m.comma); _sc(SYS_WRITE, STDOUT, (long)out, 21);
  _sc(SYS_WRITE, STDOUT, (long)"\nFILES_OK:   ", 12); u64_to_str(m.files_ok); _sc(SYS_WRITE, STDOUT, (long)out, 21);
  _sc(SYS_WRITE, STDOUT, (long)"\nFILES_SKIP: ", 13); u64_to_str(m.files_skip); _sc(SYS_WRITE, STDOUT, (long)out, 21);
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
