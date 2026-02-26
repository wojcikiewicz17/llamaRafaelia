#include <stdio.h>
#include <stdint.h>

typedef uint8_t  u8;
typedef uint32_t u32;

void rafa_boot(void);
u32  rafa_run(const u8 *in, u32 in_len, u8 *out, u32 out_cap);

static void hexdump(const u8 *p, u32 n) {
  for (u32 i=0;i<n;i++) {
    printf("%02X", (unsigned)p[i]);
    if ((i+1u)%16u==0u) printf("\n");
    else printf(" ");
  }
  if (n%16u) printf("\n");
}

int main(void){
  const u8 msg[] = {1,2,3,4,5,6,7,8,9};
  u8 out[64];
  rafa_boot();
  u32 n = rafa_run(msg, (u32)sizeof(msg), out, (u32)sizeof(out));
  printf("out_len=%u\n", (unsigned)n);
  hexdump(out, n);
  return 0;
}
