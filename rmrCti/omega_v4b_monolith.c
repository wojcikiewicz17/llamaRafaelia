typedef unsigned long long u64;
typedef long long i64;
typedef unsigned char u8;

/* SYSCALL ABI ARM64 (Linux kernel underneath Android) */
#define SYS_READ   63
#define SYS_WRITE  64
#define SYS_EXIT   93
#define STDOUT     1

/* ---- BSS (zerado pelo loader) ---- */
static struct {
    u64 s_syn;
    u64 s_emerg;
    u64 burst_factor;
    u64 k_constant;
    u64 sync_clock;
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

/* NEON engine + cauda */
static void omega_engine(const char* p, u64 len) {
    u64 aligned = len & ~0xFULL;
    u64 rem = len & 0xFULL;

    if (aligned) {
        __asm__ __volatile__ (
            "movi v1.16b, #128\n\t"
            "movi v2.16b, #32\n\t"
            "1:\n\t"
            "ld1 {v0.16b}, [%0], #16\n\t"
            "cmhi v3.16b, v0.16b, v1.16b\n\t"
            "cmeq v4.16b, v0.16b, v2.16b\n\t"
            "addv b5, v3.16b\n\t"
            "fmov x4, d5\n\t"
            "add %1, %1, x4\n\t"
            "addv b6, v4.16b\n\t"
            "fmov x5, d6\n\t"
            "add %2, %2, x5\n\t"
            "subs %3, %3, #16\n\t"
            "b.ne 1b\n\t"
            : "+r"(p),
              "+r"(m.s_syn),
              "+r"(m.burst_factor),
              "+r"(aligned)
            :
            : "x4","x5","v0","v1","v2","v3","v4","v5","v6","memory"
        );
    }

    while (rem--) {
        u8 c = (u8)*p++;
        if (c > 127) m.s_syn++;
        if (c == 32)  m.burst_factor++;
    }
}

/* u64 → string (NO-LIBC) */
static void u64_to_str(u64 n) {
    int i = 19; out[20] = 0;
    if (!n) out[i--] = '0';
    while (n && i >= 0) { out[i--] = (n % 10) + '0'; n /= 10; }
    while (i >= 0) out[i--] = ' ';
}

/* IMPORTANTE:
   - NÃO static (global symbol)
   - used/noinline: impede clang/lld de remover por GC */
__attribute__((used,noinline))
void omega_main(void) {
    _sc(SYS_WRITE, STDOUT,
        (long)"[OMEGA V4B MONOLITH ACTIVE - ANDROID(LINUX) ARM64]\n", 53);

    while (1) {
        i64 r = _sc(SYS_READ, 0, (long)buf, sizeof(buf));
        if (r <= 0) break;
        omega_engine(buf, (u64)r);
        m.s_emerg += (u64)r;
    }

    _sc(SYS_WRITE, STDOUT, (long)"S_SYN:  ", 8);
    u64_to_str(m.s_syn);
    _sc(SYS_WRITE, STDOUT, (long)out, 21);

    _sc(SYS_WRITE, STDOUT, (long)"\nBYTES: ", 8);
    u64_to_str(m.s_emerg);
    _sc(SYS_WRITE, STDOUT, (long)out, 21);

    _sc(SYS_WRITE, STDOUT, (long)"\nBURST: ", 8);
    u64_to_str(m.burst_factor);
    _sc(SYS_WRITE, STDOUT, (long)out, 21);

    _sc(SYS_EXIT, 0, 0, 0);
}

/* ENTRYPOINT REAL: naked == só ASM */
__attribute__((naked))
void _start(void) {
    __asm__ __volatile__(
        "mov x29, xzr\n"
        "mov x30, xzr\n"
        "bl omega_main\n"
    );
}
