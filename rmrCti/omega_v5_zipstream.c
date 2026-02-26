// OMEGA V5 — ZIPSTREAM + DEFLATE (Android / ARM64)
// streaming puro, buffers fixos, sem libc pesada
typedef unsigned long long u64;
typedef long long i64;
typedef unsigned int u32;
typedef unsigned char u8;

/* syscalls ARM64 */
#define SYS_READ   63
#define SYS_WRITE  64
#define SYS_EXIT   93
#define STDOUT     1

/* ZIP */
#define ZIP_LOCAL_SIG 0x04034b50
#define METHOD_STORED 0
#define METHOD_DEFLATE 8

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
    u64 files_ok;
    u64 files_skip;
} m;

static u8 inbuf[65536];
static u8 outbuf[65536];
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

/* contadores byte-a-byte */
static inline void omega_engine(const u8* p, u64 n) {
    for (u64 i = 0; i < n; i++) {
        u8 c = p[i];
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

/* u64 → string */
static void u64_to_str(u64 n) {
    int i = 19; out[20] = 0;
    if (!n) out[i--] = '0';
    while (n && i >= 0) { out[i--] = (n % 10) + '0'; n /= 10; }
    while (i >= 0) out[i--] = ' ';
}

/* --- miniz tinfl (inflate mínimo) --- */
#define TINFL_IMPLEMENTATION
#define TINFL_NO_MALLOC
#define TINFL_LZ_DICT_SIZE 32768
#include "tinfl.c"

/* leitura exata */
static int readn(u8* p, u32 n) {
    u32 got = 0;
    while (got < n) {
        i64 r = _sc(SYS_READ, 0, (long)(p + got), n - got);
        if (r <= 0) return -1;
        got += (u32)r;
    }
    return 0;
}

static u32 rd32(const u8* p) {
    return (u32)p[0] | ((u32)p[1] << 8) | ((u32)p[2] << 16) | ((u32)p[3] << 24);
}
static u16 rd16(const u8* p) {
    return (u16)p[0] | ((u16)p[1] << 8);
}

__attribute__((used,noinline))
void omega_main(void) {
    _sc(SYS_WRITE, STDOUT,
        (long)"[OMEGA V5 ZIPSTREAM + DEFLATE]\n", 32);

    for (;;) {
        u8 hdr[30];
        if (readn(hdr, 4) < 0) break;
        if (rd32(hdr) != ZIP_LOCAL_SIG) break;

        if (readn(hdr + 4, 26) < 0) break;

        u16 method = rd16(hdr + 8);
        u32 csize  = rd32(hdr + 18);
        u16 nlen   = rd16(hdr + 26);
        u16 xlen   = rd16(hdr + 28);

        /* pula nome + extra */
        if (nlen) readn(inbuf, nlen);
        if (xlen) readn(inbuf, xlen);

        if (method == METHOD_STORED) {
            u32 left = csize;
            while (left) {
                u32 chunk = left > sizeof(inbuf) ? sizeof(inbuf) : left;
                if (readn(inbuf, chunk) < 0) break;
                omega_engine(inbuf, chunk);
                left -= chunk;
            }
            m.files_ok++;
        } else if (method == METHOD_DEFLATE) {
            tinfl_decompressor d;
            tinfl_init(&d);
            u32 left = csize;
            size_t in_ofs = 0, out_ofs = 0;
            int status = TINFL_STATUS_NEEDS_MORE_INPUT;

            while (status > 0) {
                if (!in_ofs && left) {
                    u32 chunk = left > sizeof(inbuf) ? sizeof(inbuf) : left;
                    if (readn(inbuf, chunk) < 0) break;
                    in_ofs = chunk;
                    left -= chunk;
                }
                size_t in_bytes = in_ofs;
                size_t out_bytes = sizeof(outbuf);
                status = tinfl_decompress(&d,
                    inbuf, &in_bytes,
                    outbuf, outbuf, &out_bytes,
                    left ? TINFL_FLAG_HAS_MORE_INPUT : 0);

                omega_engine(outbuf, out_bytes);
                in_ofs -= in_bytes;
            }
            m.files_ok++;
        } else {
            /* método não suportado */
            u32 left = csize;
            while (left) {
                u32 chunk = left > sizeof(inbuf) ? sizeof(inbuf) : left;
                if (readn(inbuf, chunk) < 0) break;
                left -= chunk;
            }
            m.files_skip++;
        }
    }

    #define P(lbl,val) do{ \
        _sc(SYS_WRITE, STDOUT, (long)lbl, sizeof(lbl)-1); \
        u64_to_str(val); _sc(SYS_WRITE, STDOUT, (long)out, 21); \
        _sc(SYS_WRITE, STDOUT, (long)"\n", 1); \
    }while(0)

    P("BYTES:      ", m.bytes);
    P("HIBIT:      ", m.hibit);
    P("SPACE:      ", m.space);
    P("NL:         ", m.nl);
    P("BRACE:      ", m.brace);
    P("QUOTE:      ", m.quote);
    P("COLON:      ", m.colon);
    P("COMMA:      ", m.comma);
    P("FILES_OK:   ", m.files_ok);
    P("FILES_SKIP: ", m.files_skip);

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
