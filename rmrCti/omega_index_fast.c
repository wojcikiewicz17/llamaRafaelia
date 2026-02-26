// omega_index_fast.c
// Lê omega_objs.jsonl (1 JSON por linha) e gera omega_index.jsonl (1 índice por linha)
// Extração "pattern-based" (sem JSON parser) porque o JSON já está em linhas independentes.
//
// Campos extraídos (quando existirem):
// - kind: "conversation" (tem "mapping") | "message/tool" (tem "message_type"/"author") | "other"
// - title (se existir)
// - model_slug / default_model_slug (se existir)
// - role (author.role se existir)
// - create_time / update_time (se existir)
// - line_bytes
//
// Uso:
//   ./omega_index_fast omega_objs.jsonl omega_index.jsonl

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

static const char* find_key(const char* s, const char* key){
  return strstr(s, key);
}

// extrai string JSON simples após "key": "...." (sem decodificar escapes)
static int extract_json_string(const char* line, const char* key, char* out, size_t outsz){
  const char* p = find_key(line, key);
  if(!p) return 0;
  p = strchr(p, ':'); if(!p) return 0;
  p++;
  while(*p==' '||*p=='\t') p++;
  if(*p!='"') return 0;
  p++;
  size_t n=0;
  int esc=0;
  while(*p && n+1<outsz){
    char c=*p++;
    if(esc){ out[n++]=c; esc=0; continue; }
    if(c=='\\'){ esc=1; continue; }
    if(c=='"') break;
    out[n++]=c;
  }
  out[n]=0;
  return (n>0);
}

static int extract_json_number(const char* line, const char* key, double* val){
  const char* p = find_key(line, key);
  if(!p) return 0;
  p = strchr(p, ':'); if(!p) return 0;
  p++;
  while(*p==' '||*p=='\t') p++;
  char* end=0;
  double x = strtod(p, &end);
  if(end==p) return 0;
  *val = x;
  return 1;
}

static const char* detect_kind(const char* line){
  if(strstr(line, "\"mapping\"")) return "conversation";
  if(strstr(line, "\"message_type\"") || strstr(line, "\"author\"")) return "message";
  if(strstr(line, "\"command\"") && strstr(line, "\"message\"")) return "toolmsg";
  if(strstr(line, "\"command\"")) return "command";
  return "other";
}

int main(int argc, char** argv){
  if(argc<3){
    fprintf(stderr,"usage: %s <in.jsonl> <out.index.jsonl>\n", argv[0]);
    return 2;
  }
  FILE* in = fopen(argv[1],"rb");
  if(!in){ perror("open in"); return 3; }
  FILE* out = fopen(argv[2],"wb");
  if(!out){ perror("open out"); fclose(in); return 3; }

  char* line = NULL;
  size_t cap = 0;
  uint64_t i = 0;

  while(1){
    ssize_t r = getline(&line, &cap, in);
    if(r<0) break;

    // tira \n
    while(r>0 && (line[r-1]=='\n' || line[r-1]=='\r')) line[--r]=0;

    char title[256]={0};
    char model[128]={0};
    char dmodel[128]={0};
    char role[64]={0};
    double ct=0.0, ut=0.0;

    extract_json_string(line, "\"title\"", title, sizeof(title));
    extract_json_string(line, "\"model_slug\"", model, sizeof(model));
    extract_json_string(line, "\"default_model_slug\"", dmodel, sizeof(dmodel));
    extract_json_string(line, "\"role\"", role, sizeof(role));
    extract_json_number(line, "\"create_time\"", &ct);
    extract_json_number(line, "\"update_time\"", &ut);

    const char* kind = detect_kind(line);

    // emite JSONL do índice (sem libs)
    fprintf(out,
      "{\"i\":%llu,\"kind\":\"%s\",\"line_bytes\":%llu",
      (unsigned long long)i, kind, (unsigned long long)r
    );

    if(title[0])  fprintf(out, ",\"title\":\"%s\"", title);
    if(model[0])  fprintf(out, ",\"model\":\"%s\"", model);
    if(dmodel[0]) fprintf(out, ",\"dmodel\":\"%s\"", dmodel);
    if(role[0])   fprintf(out, ",\"role\":\"%s\"", role);
    if(ct>0.0)    fprintf(out, ",\"create_time\":%.6f", ct);
    if(ut>0.0)    fprintf(out, ",\"update_time\":%.6f", ut);

    fprintf(out, "}\n");
    i++;
  }

  free(line);
  fclose(in);
  fclose(out);
  fprintf(stderr,"[omega_index_fast] OK :: indexed %llu lines\n", (unsigned long long)i);
  return 0;
}
