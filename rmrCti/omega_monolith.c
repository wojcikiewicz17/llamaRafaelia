typedef unsigned long long u64;
typedef long long i64;
typedef unsigned char u8;

#define SYS_READ   63
#define SYS_WRITE  64
#define SYS_EXIT   93
#define STDOUT     1

typedef struct {
    u64 s_syn;            // Massa simbólica (>127)
    u64 s_emerg;          // Volume total (bytes)
    u64 s_latent;
    u64 entropy_raw;
    u64 burst_factor;     // Delimitadores (space)
    u64 iso_ratio;
    u64 technical_d;
    u64 noise_floor;
    u64 alpha_wave;
    u64 beta_wave;
    u64 phi_integral;
    u64 k_constant;
    u64 sync_clock;
    u64 semantic_gap;
    u64 logic_depth;
    u64 token_ratio;
    u64 bit_depth_10;
    u64 context_span;
    u64 feedback_loop;
    u64 neural_resonance;
    u64 cross_entropy;
    u64 weight_bias_inf;
    u64 fractal_dim;
    u64 hamming_dist;
    u64 kurtosis_val;
    u64 skewness_val;
    u64 chi_square;
    u64 p_value_est;
    u64 omega_point;
} NeuroMetrics;

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

void omega_engine(const char* buf, u64 len, NeuroMetrics* m) {
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
            "add %2, %2, x4\n\t"
            "addv b6, v4.16b\n\t"
            "fmov x5, d6\n\t"
            "add %3, %3, x5\n\t"
            "subs %1, %1, #16\n\t"
            "b.ne 1b\n\t"
            : "+r"(buf), "+r"(aligned),
              "+r"(m->s_syn), "+r"(m->burst_factor)
            :
            : "x4","x5","v0","v1","v2","v3","v4","v5","v6","memory"
        );
    }

    while (rem--) {
        u8 c = (u8)*buf++;
        if (c > 127) m->s_syn++;
        if (c == 32)  m->burst_factor++;
    }
}

void u64_to_str(u64 n, char* s) {
    int i = 19; s[20] = 0;
    if (!n) s[i--] = '0';
    while (n && i >= 0) { s[i--] = (n % 10) + '0'; n /= 10; }
    while (i >= 0) s[i--] = ' ';
}

void _start() {
    NeuroMetrics m = {0};
    char buf[65536];
    char out[21];

    _sc(SYS_WRITE, STDOUT,
        (long)"[OMEGA MONOLITH ACTIVE]\n", 25);

    while (1) {
        i64 r = _sc(SYS_READ, 0, (long)buf, sizeof(buf));
        if (r <= 0) break;
        omega_engine(buf, (u64)r, &m);
        m.s_emerg += r;
    }

    _sc(SYS_WRITE, STDOUT, (long)"S_SYN:  ", 8);
    u64_to_str(m.s_syn, out);
    _sc(SYS_WRITE, STDOUT, (long)out, 21);

    _sc(SYS_WRITE, STDOUT, (long)"\nBYTES: ", 8);
    u64_to_str(m.s_emerg, out);
    _sc(SYS_WRITE, STDOUT, (long)out, 21);

    _sc(SYS_WRITE, STDOUT, (long)"\nBURST: ", 8);
    u64_to_str(m.burst_factor, out);
    _sc(SYS_WRITE, STDOUT, (long)out, 21);

    _sc(SYS_EXIT, 0, 0, 0);
}
