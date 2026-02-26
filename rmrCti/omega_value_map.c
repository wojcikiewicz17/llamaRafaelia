#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    int conv_i;
    int infra, sec, ai, symb;
    int msgs;
} ConvScore;

/* ---- helpers ---- */
static int has_kw_ci(const char *s, const char *kw){
    if(!s || !kw) return 0;
    size_t n=strlen(kw); if(!n) return 0;
    for(const char *p=s; *p; p++){
        size_t i=0;
        for(; i<n && p[i]; i++){
            if(tolower((unsigned char)p[i]) != tolower((unsigned char)kw[i])) break;
        }
        if(i==n) return 1;
    }
    return 0;
}

static int json_get_int(const char *line, const char *key, int *out){
    char pat[80];
    snprintf(pat,sizeof(pat),"\"%s\"",key);
    const char *p=strstr(line,pat); if(!p) return 0;
    p=strchr(p,':'); if(!p) return 0; p++;
    while(*p && isspace((unsigned char)*p)) p++;
    *out = atoi(p);
    return 1;
}

static int json_get_string_unescape(const char *line,const char *key,char **out){
    *out=NULL;
    char pat[80];
    snprintf(pat,sizeof(pat),"\"%s\"",key);
    const char *p=strstr(line,pat); if(!p) return 0;
    p=strchr(p,':'); if(!p) return 0; p++;
    while(*p && isspace((unsigned char)*p)) p++;
    if(*p!='"') return 0;
    p++;

    const char *q=p; int esc=0;
    while(*q){
        if(!esc && *q=='"') break;
        esc = (!esc && *q=='\\');
        q++;
    }
    if(*q!='"') return 0;

    size_t len=(size_t)(q-p);
    char *buf=(char*)malloc(len+1); if(!buf) return 0;

    size_t j=0;
    for(size_t i=0;i<len;i++){
        if(p[i]=='\\' && i+1<len){
            i++;
            if(p[i]=='n') buf[j++]='\n';
            else if(p[i]=='t') buf[j++]='\t';
            else if(p[i]=='r') buf[j++]='\r';
            else buf[j++]=p[i];
        } else buf[j++]=p[i];
    }
    buf[j]=0;
    *out=buf;
    return 1;
}

/* ---- classifier ---- */
static void classify(ConvScore *c, const char *t){
    if(!t) return;

    /* infra */
    if(has_kw_ci(t,"bash")||has_kw_ci(t,"zsh")||has_kw_ci(t,"termux")||
       has_kw_ci(t,"android")||has_kw_ci(t,"clang")||has_kw_ci(t,"gcc")||
       has_kw_ci(t,"make")||has_kw_ci(t,"kernel")||has_kw_ci(t,"qemu"))
        c->infra++;

    /* security */
    if(has_kw_ci(t,"jwt")||has_kw_ci(t,"token")||has_kw_ci(t,"oauth")||
       has_kw_ci(t,"tls")||has_kw_ci(t,"ssh")||has_kw_ci(t,"pki")||
       has_kw_ci(t,"cve")||has_kw_ci(t,"crypto")||has_kw_ci(t,"encrypt"))
        c->sec++;

    /* AI */
    if(has_kw_ci(t,"gpt")||has_kw_ci(t,"model")||has_kw_ci(t,"llm")||
       has_kw_ci(t,"inference")||has_kw_ci(t,"training")||has_kw_ci(t,"thinking"))
        c->ai++;

    /* symbolic/IP */
    if(has_kw_ci(t,"rafael")||has_kw_ci(t,"rafaelia")||has_kw_ci(t,"zipraf")||
       has_kw_ci(t,"bitraf")||has_kw_ci(t,"ethica")||has_kw_ci(t,"fiat lux")||
       has_kw_ci(t,"verbo"))
        c->symb++;
}

static void emit(FILE *o, const ConvScore *c){
    int total = c->infra + c->sec + c->ai + c->symb;

    const char *reuse =
        (c->infra + c->sec + c->ai) >= 6 ? "alta" :
        (c->infra + c->sec + c->ai) >= 3 ? "media" : "baixa";

    const char *monetizable =
        (c->sec + c->ai) >= 3 ? "true" : "false";

    fprintf(o,
      "{\"conv_i\":%d,\"infra\":%d,\"security\":%d,\"ai\":%d,\"symbolic\":%d,"
      "\"total\":%d,\"msgs\":%d,\"reuse\":\"%s\",\"monetizable\":%s}\n",
      c->conv_i,c->infra,c->sec,c->ai,c->symb,total,c->msgs,reuse,monetizable);
}

int main(int argc,char **argv){
    const char *in  = (argc>1)?argv[1]:"omega_msgs.jsonl";
    const char *out = (argc>2)?argv[2]:"omega_value_map.jsonl";

    FILE *f=fopen(in,"r");  if(!f){ perror(in); return 1; }
    FILE *o=fopen(out,"w"); if(!o){ perror(out); fclose(f); return 1; }

    ConvScore cur = (ConvScore){-1,0,0,0,0,0};
    char *line=NULL; size_t cap=0;

    while(getline(&line,&cap,f)!=-1){
        int ci=-1;
        if(!json_get_int(line,"conv_i",&ci)) continue;

        if(cur.conv_i==-1) cur.conv_i=ci;

        if(ci!=cur.conv_i){
            emit(o,&cur);
            cur = (ConvScore){ci,0,0,0,0,0};
        }

        char *txt=NULL;
        if(json_get_string_unescape(line,"text",&txt)){
            classify(&cur, txt);
            free(txt);
        }
        cur.msgs++;
    }

    if(cur.conv_i!=-1) emit(o,&cur);

    free(line);
    fclose(f); fclose(o);
    fprintf(stderr,"[omega_value_map] OK -> %s\n", out);
    return 0;
}
