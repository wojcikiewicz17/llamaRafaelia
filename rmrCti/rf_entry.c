/* rf_entry.c (freestanding glue)
   - Saída em RAM: um buffer global exportado
   - ABI u64: compatível com loaders simples
*/
typedef unsigned char      u8;
typedef unsigned int       u32;
typedef unsigned long long u64;

void rafa_boot(void);
u32  rafa_run(const u8 *in, u32 in_len, u8 *out, u32 out_cap);

/* “telemetria” em RAM (o loader/QEMU pode inspecionar) */
volatile u8  g_out[64];
volatile u32 g_out_len;
volatile u32 g_booted;

static const u8 g_msg[] = {1,2,3,4,5,6,7,8,9};

/* entry genérica: se in_ptr==0, usa msg default */
u64 rf_entry(u64 in_ptr, u64 in_len) {
  if (!g_booted) { rafa_boot(); g_booted = 1u; }

  const u8 *in = (const u8*)in_ptr;
  u32 len = (u32)in_len;
  if (!in || !len) { in = g_msg; len = (u32)sizeof(g_msg); }

  g_out_len = rafa_run(in, len, (u8*)g_out, (u32)sizeof(g_out));
  return (u64)g_out_len;
}

/* “main” baremetal: chama entry e para */
void rf_main(void) {
  (void)rf_entry(0ull, 0ull);
  for (;;) { /* halt loop */ }
}
