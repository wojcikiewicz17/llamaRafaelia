/* rf_core.h (freestanding, no libc, no includes) */
typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;

void rafa_boot(void);
u32  rafa_run(const u8 *in, u32 in_len, u8 *out, u32 out_cap);
