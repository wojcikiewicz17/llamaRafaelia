#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

static void die(const char *m){
  fprintf(stderr,"[omega_search_fast] %s\n", m);
  exit(2);
}

static char lower_ascii(char c){
  if(c>='A' && c<='Z') return (char)(c + 32);
  return c;
}

/* contains_icase: busca term dentro de hay (case-insensitive ASCII) */
static int contains_icase(const char *hay, const char *term){
  if(!hay || !term) return 0;
  size_t n = strlen(hay), m = strlen(term);
  if(m==0) return 1;
  if(n < m) return 0;

  for(size_t i=0; i + m <= n; i++){
    size_t j=0;
    for(; j<m; j++){
      char a = lower_ascii(hay[i+j]);
      char b = lower_ascii(term[j]);
      if(a != b) break;
    }
    if(j==m) return 1;
  }
  return 0;
}

/* role filter: procura exatamente "role":"X" (X = tool/user/assistant/system) */
static int match_role(const char *line, const char *role){
  if(!role || !*role) return 1;
  char pat[64];
  snprintf(pat, sizeof(pat), "\"role\":\"%s\"", role);
  return strstr(line, pat) != NULL;
}

int main(int argc, char **argv){
  const char *in_path = "omega_msgs.jsonl";
  const char *term = NULL;
  const char *role = NULL;
  long limit = 20;

  for(int i=1;i<argc;i++){
    if(strcmp(argv[i],"--in")==0){
      if(i+1>=argc) die("missing --in");
      in_path = argv[++i];
    } else if(strcmp(argv[i],"--term")==0){
      if(i+1>=argc) die("missing --term");
      term = argv[++i];
    } else if(strcmp(argv[i],"--role")==0){
      if(i+1>=argc) die("missing --role");
      role = argv[++i];
    } else if(strcmp(argv[i],"--limit")==0){
      if(i+1>=argc) die("missing --limit");
      limit = strtol(argv[++i], NULL, 10);
      if(limit <= 0) limit = 1;
    } else {
      fprintf(stderr,"usage: %s --term X [--in omega_msgs.jsonl] [--role tool] [--limit 20]\n", argv[0]);
      return 2;
    }
  }

  if(!term || !*term) die("term vazio");

  FILE *f = fopen(in_path, "rb");
  if(!f){
    fprintf(stderr,"[omega_search_fast] cannot open: %s (%s)\n", in_path, strerror(errno));
    return 3;
  }

  char *line = NULL;
  size_t cap = 0;
  long hits = 0;
  long scanned = 0;

  while(1){
    ssize_t r = getline(&line, &cap, f);
    if(r < 0) break;
    scanned++;

    if(!match_role(line, role)) continue;

    if(contains_icase(line, term)){
      fputs(line, stdout);
      hits++;
      if(hits >= limit) break;
    }
  }

  fprintf(stderr,"[omega_search_fast] hits=%ld scanned_lines=%ld\n", hits, scanned);

  free(line);
  fclose(f);
  return 0;
}
