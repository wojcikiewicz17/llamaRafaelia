// omega_json_reconstruct_fast.c
// Boosted streaming extractor JSON top-level: '{' ... '}' into JSONL
// Termux/Android friendly: sequential read + kernel hints + buffered output
//
// Uso:
//   ./omega_json_reconstruct_fast <in.json> <out.jsonl> [--offset N] [--bytes N] [--progress]
//
// Obs: extrai objetos top-level entre chaves; braces dentro de strings não contam.

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#if defined(__linux__)
  #include <unistd.h>
  #include <fcntl.h>
  #include <sys/types.h>
#endif

typedef uint64_t u64;

static void die(const char *msg){
  fprintf(stderr, "[omega_json_reconstruct_fast] %s\n", msg);
  exit(1);
}

static int streq(const char *a, const char *b){ return strcmp(a,b)==0; }

static u64 parse_u64(const char *s){
  if(!s || !*s) die("parse_u64: empty");
  char *end = 0;
  errno = 0;
  unsigned long long v = strtoull(s, &end, 10);
  if(errno || end==s) die("parse_u64: bad number");
  return (u64)v;
}

static double now_sec(void){
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (double)ts.tv_sec + (double)ts.tv_nsec/1e9;
}

static const char* fmt_bytes(u64 n){
  static char buf[64];
  const char* u[]={"B","KB","MB","GB","TB"};
  double x=(double)n;
  int i=0;
  while(x>=1024.0 && i<4){ x/=1024.0; i++; }
  snprintf(buf,sizeof(buf),"%.2f%s", x, u[i]);
  return buf;
}

#if defined(__GNUC__) || defined(__clang__)
  #define LIKELY(x)   __builtin_expect(!!(x), 1)
  #define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
  #define LIKELY(x) (x)
  #define UNLIKELY(x) (x)
#endif

typedef struct {
  unsigned char *p;
  size_t cap;
  size_t len;
  FILE *f;
} OutBuf;

static void out_flush(OutBuf *ob){
  if(ob->len){
    fwrite(ob->p, 1, ob->len, ob->f);
    ob->len = 0;
  }
}

static inline void out_putc(OutBuf *ob, unsigned char c){
  ob->p[ob->len++] = c;
  if(UNLIKELY(ob->len == ob->cap)) out_flush(ob);
}

int main(int argc, char **argv){
  if(argc < 3){
    fprintf(stderr,
      "usage: %s <in.json> <out.jsonl> [--offset N] [--bytes N] [--progress]\n", argv[0]);
    return 2;
  }

  const char *in_path  = argv[1];
  const char *out_path = argv[2];

  u64 off0 = 0;
  u64 max_bytes = 0; // 0 = até EOF
  int progress = 0;

  for(int i=3;i<argc;i++){
    if(streq(argv[i],"--offset")){
      if(i+1>=argc) die("missing value for --offset");
      off0 = parse_u64(argv[++i]);
    } else if(streq(argv[i],"--bytes")){
      if(i+1>=argc) die("missing value for --bytes");
      max_bytes = parse_u64(argv[++i]);
    } else if(streq(argv[i],"--progress")){
      progress = 1;
    } else {
      fprintf(stderr,"unknown arg: %s\n", argv[i]);
      return 2;
    }
  }

#if defined(__linux__)
  int fd = open(in_path, O_RDONLY | O_CLOEXEC);
  if(fd < 0){ perror("open"); return 3; }

  // Hints pro kernel (quando disponível)
  // max_bytes==0 -> pode passar 0 (kernel interpreta como “do offset em diante” em algumas libc)
  #ifdef POSIX_FADV_SEQUENTIAL
    (void)posix_fadvise(fd, (off_t)off0, (off_t)(max_bytes?max_bytes:0), POSIX_FADV_SEQUENTIAL);
  #endif

  if(lseek(fd, (off_t)off0, SEEK_SET) < 0){ perror("lseek"); close(fd); return 3; }

  FILE *out = fopen(out_path, "wb");
  if(!out){ perror("fopen(out)"); close(fd); return 3; }

  // Buffer maior pro FILE* (reduz syscalls)
  setvbuf(out, NULL, _IOFBF, 1<<20);

  const size_t IN_BUFSZ  = (size_t)(8u<<20);   // 8MB input (ajuste: 4MB se RAM apertar)
  const size_t OUT_BUFSZ = (size_t)(1u<<20);   // 1MB output buffer

  unsigned char *inbuf = (unsigned char*)malloc(IN_BUFSZ);
  if(!inbuf) die("malloc inbuf failed");

  OutBuf ob;
  ob.p = (unsigned char*)malloc(OUT_BUFSZ);
  if(!ob.p) die("malloc outbuf failed");
  ob.cap = OUT_BUFSZ;
  ob.len = 0;
  ob.f   = out;

  // state machine
  int in_str = 0;
  int esc = 0;
  int started = 0;
  u64 depth = 0;

  // counters
  u64 bytes_read_total = 0;
  u64 bytes_scanned = 0;
  u64 obj_count = 0;

  double t0 = now_sec();
  double t_last = t0;

  while(1){
    size_t want = IN_BUFSZ;
    if(max_bytes){
      u64 left = (max_bytes > bytes_read_total) ? (max_bytes - bytes_read_total) : 0;
      if(left == 0) break;
      if(left < (u64)want) want = (size_t)left;
    }

    ssize_t r = read(fd, inbuf, want);
    if(r < 0){ perror("read"); break; }
    if(r == 0) break;

    bytes_read_total += (u64)r;

    for(ssize_t i=0;i<r;i++){
      unsigned char c = inbuf[i];
      bytes_scanned++;

      if(LIKELY(in_str)){
        if(esc) esc = 0;
        else {
          if(c == '\\') esc = 1;
          else if(c == '"') in_str = 0;
        }
        if(started) out_putc(&ob, c);
        continue;
      }

      // fora de string
      if(c == '"'){
        in_str = 1; esc = 0;
        if(started) out_putc(&ob, c);
        continue;
      }

      if(c == '{'){
        depth++;
        if(!started && depth == 1) started = 1;
        if(started) out_putc(&ob, c);
        continue;
      }

      if(c == '}'){
        if(started) out_putc(&ob, c);
        if(depth) depth--;
        if(started && depth == 0){
          out_putc(&ob, '\n');
          obj_count++;
          started = 0;
        }
        continue;
      }

      if(started) out_putc(&ob, c);
    }

    if(progress){
      double t = now_sec();
      if(t - t_last >= 0.5){
        double dt = t - t0;
        double spd = dt>0 ? (double)bytes_read_total/dt : 0.0;
        fprintf(stderr, "\r[omega] read=%s scanned=%s objs=%llu speed=%s/s",
          fmt_bytes(bytes_read_total),
          fmt_bytes(bytes_scanned),
          (unsigned long long)obj_count,
          fmt_bytes((u64)spd)
        );
        fflush(stderr);
        t_last = t;
      }
    }
  }

  out_flush(&ob);

  if(progress){
    fprintf(stderr, "\n[omega] DONE read=%s objs=%llu out=%s\n",
      fmt_bytes(bytes_read_total),
      (unsigned long long)obj_count,
      out_path
    );
  } else {
    fprintf(stderr, "[omega] DONE objs=%llu out=%s\n",
      (unsigned long long)obj_count,
      out_path
    );
  }

  free(inbuf);
  free(ob.p);
  fclose(out);
  close(fd);
  return 0;

#else
  // fallback não-linux (mantém simples)
  FILE *in = fopen(in_path, "rb");
  if(!in){ perror("fopen(in)"); return 3; }
  FILE *out = fopen(out_path, "wb");
  if(!out){ perror("fopen(out)"); fclose(in); return 3; }
  fseek(in, (long)off0, SEEK_SET);

  int in_str=0, esc=0, started=0;
  u64 depth=0, obj_count=0, read_total=0;

  int ch;
  while((ch=fgetc(in))!=EOF){
    read_total++;
    if(max_bytes && read_total>max_bytes) break;
    unsigned char c = (unsigned char)ch;

    if(in_str){
      if(esc) esc=0;
      else {
        if(c=='\\') esc=1;
        else if(c=='"') in_str=0;
      }
      if(started) fputc(c,out);
      continue;
    } else {
      if(c=='"'){ in_str=1; esc=0; if(started) fputc(c,out); continue; }
      if(c=='{'){ depth++; if(!started && depth==1) started=1; if(started) fputc(c,out); continue; }
      if(c=='}'){
        if(started) fputc(c,out);
        if(depth) depth--;
        if(started && depth==0){ fputc('\n',out); obj_count++; started=0; }
        continue;
      }
      if(started) fputc(c,out);
    }
  }

  fprintf(stderr,"[omega] DONE objs=%llu out=%s\n",
    (unsigned long long)obj_count, out_path);
  fclose(in); fclose(out);
  return 0;
#endif
}
