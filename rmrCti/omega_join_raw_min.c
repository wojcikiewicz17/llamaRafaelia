/*
  omega_join_raw_min.c
  - NÃO cole isso no terminal como C
  - compile para ELF e rode
  - objetivo: provar o fluxo correto (arquivo -> binário -> execução)

  build (Termux):
    cc -O3 -nostdlib -fno-builtin -Wl,-e,_start omega_join_raw_min.c -o omega_join_raw_min

  run:
    ./omega_join_raw_min
*/

#define SYS_READ    63
#define SYS_WRITE   64
#define SYS_OPENAT  56
#define SYS_CLOSE   57
#define SYS_EXIT    93

#define AT_FDCWD   (-100)
#define O_RDONLY     0

typedef unsigned long u64;
typedef long          s64;

static inline s64 sys_call(u64 n,u64 a,u64 b,u64 c,u64 d,u64 e,u64 f){
  register u64 x8 asm("x8") = n;
  register u64 x0 asm("x0") = a;
  register u64 x1 asm("x1") = b;
  register u64 x2 asm("x2") = c;
  register u64 x3 asm("x3") = d;
  register u64 x4 asm("x4") = e;
  register u64 x5 asm("x5") = f;
  asm volatile("svc 0" : "+r"(x0)
    : "r"(x8),"r"(x1),"r"(x2),"r"(x3),"r"(x4),"r"(x5)
    : "memory");
  return (s64)x0;
}

static inline s64 sys_openat(int dirfd, const char*path, int flags, int mode){
  return sys_call(SYS_OPENAT, (u64)dirfd, (u64)path, (u64)flags, (u64)mode, 0, 0);
}
static inline s64 sys_read(int fd, void *buf, u64 n){
  return sys_call(SYS_READ, (u64)fd, (u64)buf, n, 0, 0, 0);
}
static inline s64 sys_write(int fd, const void *buf, u64 n){
  return sys_call(SYS_WRITE, (u64)fd, (u64)buf, n, 0, 0, 0);
}
static inline s64 sys_close(int fd){
  return sys_call(SYS_CLOSE, (u64)fd, 0,0,0,0,0);
}
static inline void sys_exit(int code){
  sys_call(SYS_EXIT, (u64)code, 0,0,0,0,0);
  for(;;){} // noreturn hard
}

static inline u64 cstrlen(const char*s){
  u64 n=0; while(s && s[n]) n++; return n;
}

__attribute__((noreturn))
void _start(void){
  const char *path = "omega_msgs.jsonl";
  int fd = (int)sys_openat(AT_FDCWD, path, O_RDONLY, 0);
  if(fd < 0){
    const char *m = "ERR open omega_msgs.jsonl\n";
    sys_write(2, m, cstrlen(m));
    sys_exit(1);
  }

  char buf[4096];
  s64 total = 0;

  for(;;){
    s64 r = sys_read(fd, buf, sizeof(buf));
    if(r == 0) break;
    if(r < 0){
      const char *m = "ERR read\n";
      sys_write(2, m, cstrlen(m));
      sys_close(fd);
      sys_exit(2);
    }
    total += r;
  }

  sys_close(fd);

  // print total bytes (sem printf)
  // output: "OK bytes=<n>\n"
  char out[64];
  int i=0;
  const char *pfx="OK bytes=";
  for(; pfx[i]; i++) out[i]=pfx[i];

  // itoa decimal
  s64 v = total;
  if(v < 0) v = 0;
  char tmp[32];
  int t=0;
  if(v==0){ tmp[t++]='0'; }
  else{
    while(v>0 && t<(int)sizeof(tmp)){
      tmp[t++] = (char)('0' + (v % 10));
      v/=10;
    }
  }
  while(t--) out[i++]=tmp[t];
  out[i++]='\n';

  sys_write(1, out, (u64)i);
  sys_exit(0);
}
