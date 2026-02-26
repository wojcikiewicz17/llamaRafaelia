typedef unsigned long long u64;
typedef long long i64;

/* Linux ARM64 syscalls */
#define SYS_OPENAT 56
#define SYS_READ   63
#define SYS_WRITE  64
#define SYS_LSEEK  62
#define SYS_CLOSE  57
#define SYS_EXIT   93

#define AT_FDCWD  -100
#define STDOUT     1

static inline long sc(long n, long a, long b, long c) {
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

static char buf[65536];

/*
 argv layout (stack):
  sp+8  = path
  sp+16 = offset
  sp+24 = size
*/

__attribute__((used,noinline))
void omega_main(void) {
  const char* path;
  u64 off;
  u64 len;

  __asm__("ldr %0, [sp, #8]"  : "=r"(path));
  __asm__("ldr %0, [sp, #16]" : "=r"(off));
  __asm__("ldr %0, [sp, #24]" : "=r"(len));

  int fd = sc(SYS_OPENAT, AT_FDCWD, (long)path, 0);
  if (fd < 0) sc(SYS_EXIT, 1, 0, 0);

  sc(SYS_LSEEK, fd, off, 0);

  while (len) {
    u64 n = len > sizeof(buf) ? sizeof(buf) : len;
    i64 r = sc(SYS_READ, fd, (long)buf, n);
    if (r <= 0) break;
    sc(SYS_WRITE, STDOUT, (long)buf, r);
    len -= r;
  }

  sc(SYS_CLOSE, fd, 0, 0);
  sc(SYS_EXIT, 0, 0, 0);
}

__attribute__((naked))
void _start(void) {
  __asm__ __volatile__(
    "mov x29, xzr\n"
    "mov x30, xzr\n"
    "bl omega_main\n"
  );
}
