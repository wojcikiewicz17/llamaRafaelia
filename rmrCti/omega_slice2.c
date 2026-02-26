#define _GNU_SOURCE
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

typedef uint64_t u64;

int main(int argc, char** argv){
  if(argc<4){
    fprintf(stderr,"usage: %s <file> <offset> <bytes>\n", argv[0]);
    return 2;
  }
  const char* path = argv[1];
  u64 off = (u64)strtoull(argv[2], 0, 10);
  u64 need = (u64)strtoull(argv[3], 0, 10);

  int fd = open(path, O_RDONLY);
  if(fd<0){ perror("open"); return 3; }

  static unsigned char buf[1<<20]; // 1MB buffer
  u64 done = 0;

  while(done < need){
    u64 want = need - done;
    if(want > sizeof(buf)) want = sizeof(buf);

    ssize_t r = pread(fd, buf, (size_t)want, (off_t)(off + done));
    if(r < 0){ perror("pread"); close(fd); return 4; }
    if(r == 0) break;

    ssize_t w = write(1, buf, (size_t)r);
    if(w < 0){ perror("write"); close(fd); return 5; }

    done += (u64)r;
  }

  if(done == 0){
    fprintf(stderr,"[slice2] zero bytes read (off=%llu)\n",
            (unsigned long long)off);
    close(fd);
    return 6;
  }

  close(fd);
  return 0;
}
