/*
  rafa_cti_scan.c — CTI streaming audit (no decompression)
  - JSONL append-only + CSV opcional
  - Aceita POSICIONAL e FLAGS: -o/--out  -c/--chunk  --csv
  Build (Termux):
    cc -O2 -std=c11 -Wall -Wextra -o rafa_cti_scan rafa_cti_scan.c -lm
*/

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <math.h>

/* -----------------------------
   CRC32C (Castagnoli) — small
   ----------------------------- */

static uint32_t crc32c_table[256];
static int crc32c_init_done = 0;

static void crc32c_init(void){
  if (crc32c_init_done) return;
  uint32_t poly = 0x1EDC6F41u; /* normal */
  for (uint32_t i=0;i<256;i++){
    uint32_t c=i<<24;
    for(int k=0;k<8;k++){
      c = (c & 0x80000000u) ? (c<<1) ^ poly : (c<<1);
    }
    crc32c_table[i]=c;
  }
  crc32c_init_done = 1;
}

static uint32_t crc32c_update(uint32_t crc, const uint8_t* buf, size_t n){
  for(size_t i=0;i<n;i++){
    uint8_t idx = (uint8_t)((crc>>24) ^ buf[i]);
    crc = (crc<<8) ^ crc32c_table[idx];
  }
  return crc;
}

/* -----------------------------
   bit count + entropy
   ----------------------------- */

static int popcnt8(uint8_t x){
  /* builtin if available */
#if defined(__GNUC__) || defined(__clang__)
  return __builtin_popcount((unsigned)x);
#else
  int c=0; while(x){ c += x&1; x >>= 1; } return c;
#endif
}

/* Shannon entropy (bits) from ones/zeros */
static double shannon_bits(int ones, int zeros){
  int n = ones + zeros;
  if (n <= 0) return 0.0;
  double p1 = (double)ones / (double)n;
  double p0 = (double)zeros / (double)n;
  double h = 0.0;
  if (p1 > 0) h -= p1 * (log(p1)/log(2.0));
  if (p0 > 0) h -= p0 * (log(p0)/log(2.0));
  return h; /* 0..1 for Bernoulli */
}

/* -----------------------------
   args
   ----------------------------- */

static void usage(const char* argv0){
  fprintf(stderr,
    "Usage:\n"
    "  %s <input_file> <out_bitstack.jsonl> [--csv out.csv]\n"
    "  %s <input_file> -o out.jsonl [-c chunk] [--csv out.csv]\n"
    "\nNotes:\n"
    "  - No decompression. Streaming audit only.\n"
    "  - Writes JSONL append-only.\n",
    argv0, argv0
  );
}

static int is_flag(const char* a, const char* b){
  return (a && b && strcmp(a,b)==0);
}

int main(int argc, char** argv){
  const char* in_path = NULL;
  const char* out_path = NULL;
  const char* csv_path = NULL;
  size_t chunk = 4096;

  if (argc < 2){
    usage(argv[0]);
    return 2;
  }

  /* --- parse:
     allow positional:
       argv1=input
       argv2=out
     plus flags:
       -o/--out
       -c/--chunk
       --csv
  */
  int i=1;

  /* first positional input if not a flag */
  if (i < argc && argv[i][0] != '-') {
    in_path = argv[i++];
  }

  /* second positional out if present and not a flag */
  if (i < argc && argv[i][0] != '-') {
    out_path = argv[i++];
  }

  for(; i<argc; i++){
    if (is_flag(argv[i], "-o") || is_flag(argv[i], "--out")){
      if (i+1 >= argc){ fprintf(stderr,"-o needs value\n"); return 2; }
      out_path = argv[++i];
      continue;
    }
    if (is_flag(argv[i], "-c") || is_flag(argv[i], "--chunk")){
      if (i+1 >= argc){ fprintf(stderr,"-c needs value\n"); return 2; }
      long v = strtol(argv[++i], NULL, 10);
      if (v < 256) v = 256;
      chunk = (size_t)v;
      continue;
    }
    if (is_flag(argv[i], "--csv")){
      if (i+1 >= argc){ fprintf(stderr,"--csv needs value\n"); return 2; }
      csv_path = argv[++i];
      continue;
    }

    fprintf(stderr, "Unknown arg: %s\n", argv[i]);
    usage(argv[0]);
    return 2;
  }

  if (!in_path || !out_path){
    usage(argv[0]);
    return 2;
  }

  FILE* fin = fopen(in_path, "rb");
  if (!fin){
    fprintf(stderr, "open input failed: %s (%s)\n", in_path, strerror(errno));
    return 1;
  }

  FILE* fout = fopen(out_path, "ab"); /* append-only */
  if (!fout){
    fprintf(stderr, "open out failed: %s (%s)\n", out_path, strerror(errno));
    fclose(fin);
    return 1;
  }

  FILE* fcsv = NULL;
  if (csv_path){
    fcsv = fopen(csv_path, "ab");
    if (!fcsv){
      fprintf(stderr, "open csv failed: %s (%s)\n", csv_path, strerror(errno));
      fclose(fout);
      fclose(fin);
      return 1;
    }
    /* if empty file, write header */
    fseek(fcsv, 0, SEEK_END);
    long sz = ftell(fcsv);
    if (sz == 0){
      fprintf(fcsv, "idx,off,size,ts,fid_crc32c,E,F,ones,H,X_bad\n");
      fflush(fcsv);
    }
  }

  crc32c_init();

  uint8_t* buf = (uint8_t*)malloc(chunk);
  if (!buf){
    fprintf(stderr, "malloc failed (chunk=%zu)\n", chunk);
    if (fcsv) fclose(fcsv);
    fclose(fout);
    fclose(fin);
    return 1;
  }

  uint64_t off = 0;
  uint64_t idx = 0;

  while (1){
    size_t n = fread(buf, 1, chunk, fin);
    if (n == 0){
      if (feof(fin)) break;
      if (ferror(fin)){
        fprintf(stderr, "read error (%s)\n", strerror(errno));
        break;
      }
    }

    /* metrics */
    int ones = 0;
    for (size_t k=0;k<n;k++) ones += popcnt8(buf[k]);
    int bits = (int)(n * 8);
    int zeros = bits - ones;

    double H = shannon_bits(ones, zeros);

    /* fid crc32c of raw bytes */
    uint32_t crc = 0u;
    crc = crc32c_update(crc, buf, n);
    char fid_hex[9];
    snprintf(fid_hex, sizeof(fid_hex), "%08x", (unsigned)crc);

    /* E/F placeholders (keep your schema stable) */
    int E = (int)(bits - ones); /* “zeros energy” */
    int F = 0;                  /* reserved */

    int X_bad = (H < 0.05) ? 1 : 0; /* simplistic anomaly flag */

    long ts = (long)time(NULL);

    /* JSONL line */
    fprintf(fout,
      "{\"idx\":%llu,\"off\":%llu,\"size\":%zu,\"ts\":%ld,"
      "\"fid_crc32c\":\"%s\",\"E\":%d,\"F\":%d,\"ones\":%d,"
      "\"H\":%.6f,\"X_bad\":%d}\n",
      (unsigned long long)idx,
      (unsigned long long)off,
      n,
      ts,
      fid_hex,
      E, F, ones,
      H,
      X_bad
    );

    if (fcsv){
      fprintf(fcsv,
        "%llu,%llu,%zu,%ld,%s,%d,%d,%d,%.6f,%d\n",
        (unsigned long long)idx,
        (unsigned long long)off,
        n,
        ts,
        fid_hex,
        E, F, ones,
        H,
        X_bad
      );
    }

    fflush(fout);
    if (fcsv) fflush(fcsv);

    off += (uint64_t)n;
    idx += 1;

    if (n < chunk) break; /* eof */
  }

  free(buf);
  if (fcsv) fclose(fcsv);
  fclose(fout);
  fclose(fin);

  return 0;
}
