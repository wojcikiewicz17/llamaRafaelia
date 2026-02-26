// omega_json_reconstruct.c
// Extrai objetos JSON top-level (entre '{' ... '}') de um stream compacto (sem \n)
// - Não depende de newline
// - Streaming: não carrega tudo na RAM
// - Respeita strings/escapes: braces dentro de "..." não contam
//
// Uso:
//   ./omega_json_reconstruct <in.json> <out.jsonl> [--offset N] [--bytes N] [--progress]
// Ex:
//   ./omega_json_reconstruct ~/storage/downloads/conversations.json omega_objs.jsonl --progress
//   ./omega_json_reconstruct ~/storage/downloads/conversations.json z53.jsonl --offset 452984832 --bytes 8388608 --progress

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
#endif

typedef uint64_t u64;

static void die(const char *msg){
  fprintf(stderr, "[omega_json_reconstruct] %s\n", msg);
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

int main(int argc, char **argv){
  if(argc < 3){
    fprintf(stderr,
      "usage: %s <in.json> <out.jsonl> [--offset N] [--bytes N] [--progress]\n", argv[0]);
    return 2;
  }

  const char *in_path = argv[1];
  const char *out_path = argv[2];

  u64 off0 = 0;
  u64 max_bytes = 0;   // 0 = até EOF
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
  int fd = open(in_path, O_RDONLY);
  if(fd<0){ perror("open"); return 3; }

  FILE *out = fopen(out_path, "wb");
  if(!out){ perror("fopen(out)"); close(fd); return 3; }

  const size_t BUFSZ = 1<<20; // 1MB
  unsigned char *buf = (unsigned char*)malloc(BUFSZ);
  if(!buf) die("malloc failed");

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
    size_t want = BUFSZ;
    if(max_bytes){
      u64 left = (max_bytes > bytes_read_total) ? (max_bytes - bytes_read_total) : 0;
      if(left == 0) break;
      if(left < (u64)want) want = (size_t)left;
    }

    ssize_t r = pread(fd, buf, want, (off_t)(off0 + bytes_read_total));
    if(r < 0){ perror("pread"); break; }
    if(r == 0) break;

    bytes_read_total += (u64)r;

    for(ssize_t i=0;i<r;i++){
      unsigned char c = buf[i];
      bytes_scanned++;

      if(in_str){
        // dentro de string: só sai em '"' não-escapado
        if(esc){
          esc = 0;
        } else {
          if(c == '\\') esc = 1;
          else if(c == '"') in_str = 0;
        }
        if(started) fputc((int)c, out);
        continue;
      } else {
        // fora de string
        if(c == '"'){
          in_str = 1;
          esc = 0;
          if(started) fputc((int)c, out);
          continue;
        }

        if(c == '{'){
          depth++;
          if(!started && depth == 1){
            started = 1; // começou objeto top-level
          }
          if(started) fputc((int)c, out);
          continue;
        }

        if(c == '}'){
          if(started) fputc((int)c, out);
          if(depth > 0) depth--;
          // se fechou o top-level -> finaliza linha JSONL
          if(started && depth == 0){
            fputc('\n', out);
            obj_count++;
            started = 0;
          }
          continue;
        }

        // qualquer outro char
        if(started) fputc((int)c, out);
      }
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

  free(buf);
  fclose(out);
  close(fd);
  return 0;

#else
  // fallback não-linux (improvável no Termux)
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
