#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <errno.h>

typedef uint64_t u64;

/* ------------------------- utils ------------------------- */

static void die(const char *msg){
    fprintf(stderr, "[omega_metrics_v2] %s\n", msg);
    exit(2);
}

static int file_exists(const char *p){
    FILE *f=fopen(p,"rb");
    if(!f) return 0;
    fclose(f);
    return 1;
}

/* Find `"key"` then ':' then parse int (ignores spaces) */
static int json_get_int(const char *line, const char *key, int *out){
    if(!line || !key || !out) return 0;
    char pat[128];
    snprintf(pat, sizeof(pat), "\"%s\"", key);
    const char *p = strstr(line, pat);
    if(!p) return 0;
    p = strchr(p, ':'); if(!p) return 0; p++;
    while(*p && isspace((unsigned char)*p)) p++;
    int sign=1; if(*p=='-'){ sign=-1; p++; }
    if(!isdigit((unsigned char)*p)) return 0;
    long v=0;
    while(isdigit((unsigned char)*p)){ v = v*10 + (*p - '0'); p++; }
    *out = (int)(sign * v);
    return 1;
}

/* Extract JSON string value for `"key":"..."`
   - returns malloc'd string in *out (caller frees)
   - unescapes \" \\ \n \t \r minimally
*/
static int json_get_string_unescape(const char *line, const char *key, char **out){
    *out = NULL;
    if(!line || !key) return 0;

    char pat[128];
    snprintf(pat, sizeof(pat), "\"%s\"", key);

    const char *p = strstr(line, pat);
    if(!p) return 0;

    p = strchr(p, ':'); if(!p) return 0; p++;
    while(*p && isspace((unsigned char)*p)) p++;

    if(*p != '"') return 0;
    p++;

    const char *q = p;
    int esc = 0;
    while(*q){
        if(!esc && *q=='"') break;
        if(!esc && *q=='\\') esc=1;
        else esc=0;
        q++;
    }
    if(*q!='"') return 0;

    size_t raw_len = (size_t)(q - p);
    char *buf = (char*)malloc(raw_len + 1);
    if(!buf) return 0;

    size_t j=0;
    for(size_t i=0;i<raw_len;i++){
        char c=p[i];
        if(c=='\\' && i+1<raw_len){
            char n=p[i+1];
            switch(n){
                case 'n': buf[j++]='\n'; i++; break;
                case 't': buf[j++]='\t'; i++; break;
                case 'r': buf[j++]='\r'; i++; break;
                case '\\': buf[j++]='\\'; i++; break;
                case '"': buf[j++]='"'; i++; break;
                default: buf[j++]=n; i++; break;
            }
        } else {
            buf[j++]=c;
        }
    }
    buf[j]=0;
    *out=buf;
    return 1;
}

static int has_kw_ci(const char *s, const char *kw){
    if(!s || !kw) return 0;
    size_t n=strlen(kw);
    if(!n) return 0;

    for(const char *p=s; *p; p++){
        size_t i=0;
        for(; i<n && p[i]; i++){
            char a=(char)tolower((unsigned char)p[i]);
            char b=(char)tolower((unsigned char)kw[i]);
            if(a!=b) break;
        }
        if(i==n) return 1;
    }
    return 0;
}

/* -------------------- hashing for OL -------------------- */

static inline u64 fnv1a64(const void *data, size_t len){
    const unsigned char *p=(const unsigned char*)data;
    u64 h=1469598103934665603ULL;
    for(size_t i=0;i<len;i++){
        h ^= (u64)p[i];
        h *= 1099511628211ULL;
    }
    return h;
}

typedef struct {
    u64 *keys;
    uint8_t *used;
    size_t cap;
    size_t len;
    int frozen; /* if we hit cap-limit */
} HashSet;

static void hs_free(HashSet *hs){
    if(!hs) return;
    free(hs->keys);
    free(hs->used);
    memset(hs,0,sizeof(*hs));
}

static void hs_init(HashSet *hs, size_t cap){
    memset(hs,0,sizeof(*hs));
    if(cap < 1024) cap = 1024;
    hs->cap = cap;
    hs->keys = (u64*)calloc(hs->cap, sizeof(u64));
    hs->used = (uint8_t*)calloc(hs->cap, 1);
    if(!hs->keys || !hs->used) die("OOM hs_init");
}

static void hs_rehash(HashSet *hs, size_t newcap){
    u64 *oldk = hs->keys;
    uint8_t *oldu = hs->used;
    size_t oldcap = hs->cap;

    hs->keys = (u64*)calloc(newcap, sizeof(u64));
    hs->used = (uint8_t*)calloc(newcap, 1);
    if(!hs->keys || !hs->used) die("OOM hs_rehash");

    hs->cap = newcap;
    hs->len = 0;

    for(size_t i=0;i<oldcap;i++){
        if(oldu[i]){
            u64 k=oldk[i];
            size_t m = hs->cap;
            size_t idx = (size_t)(k % m);
            while(hs->used[idx]) idx = (idx + 1) % m;
            hs->used[idx]=1;
            hs->keys[idx]=k;
            hs->len++;
        }
    }

    free(oldk);
    free(oldu);
}

static int hs_add(HashSet *hs, u64 k){
    if(hs->frozen) return 0;

    /* load factor ~0.7 */
    if(hs->len * 10 >= hs->cap * 7){
        size_t newcap = hs->cap * 2;
        if(newcap > (1u<<20)){ /* hard cap: ~1M slots */
            hs->frozen = 1;
            return 0;
        }
        hs_rehash(hs, newcap);
    }

    size_t m=hs->cap;
    size_t idx=(size_t)(k % m);
    while(hs->used[idx]){
        if(hs->keys[idx]==k) return 0; /* already */
        idx = (idx + 1) % m;
    }
    hs->used[idx]=1;
    hs->keys[idx]=k;
    hs->len++;
    return 1;
}

/* -------------------- keyword sets -------------------- */

static int is_word_char(int c){
    return isalpha(c); /* simples, rápido e robusto */
}

#define KW_MAX 32
typedef struct {
    u64 keyhash[KW_MAX];
    int used;
    uint8_t seen[KW_MAX];
} KeySet;

static void ks_reset(KeySet *ks){
    ks->used=0;
    memset(ks->keyhash,0,sizeof(ks->keyhash));
    memset(ks->seen,0,sizeof(ks->seen));
}

static void ks_add(KeySet *ks, u64 h){
    if(ks->used >= KW_MAX) return;
    /* evita duplicar */
    for(int i=0;i<ks->used;i++) if(ks->keyhash[i]==h) return;
    ks->keyhash[ks->used++] = h;
}

static void ks_mark_seen(KeySet *ks, u64 h){
    for(int i=0;i<ks->used;i++){
        if(ks->keyhash[i]==h){
            ks->seen[i]=1;
            return;
        }
    }
}

/* -------------------- classifiers per message -------------------- */

static int msg_has_infra(const char *t){
    return has_kw_ci(t,"bash")||has_kw_ci(t,"zsh")||has_kw_ci(t,"termux")||
           has_kw_ci(t,"android")||has_kw_ci(t,"apk")||has_kw_ci(t,"clang")||
           has_kw_ci(t,"gcc")||has_kw_ci(t,"make")||has_kw_ci(t,"kernel")||
           has_kw_ci(t,"ld")||has_kw_ci(t,"linker")||has_kw_ci(t,"elf")||
           has_kw_ci(t,"aarch64")||has_kw_ci(t,"arm64")||has_kw_ci(t,"qemu");
}

static int msg_has_security(const char *t){
    return has_kw_ci(t,"jwt")||has_kw_ci(t,"token")||has_kw_ci(t,"oauth")||
           has_kw_ci(t,"tls")||has_kw_ci(t,"ssh")||has_kw_ci(t,"pki")||
           has_kw_ci(t,"xss")||has_kw_ci(t,"csrf")||has_kw_ci(t,"cve")||
           has_kw_ci(t,"crypto")||has_kw_ci(t,"encrypt")||has_kw_ci(t,"hmac");
}

static int msg_has_ai(const char *t){
    return has_kw_ci(t,"gpt")||has_kw_ci(t,"model")||has_kw_ci(t,"llm")||
           has_kw_ci(t,"inference")||has_kw_ci(t,"training")||
           has_kw_ci(t,"thinking")||has_kw_ci(t,"prompt");
}

static int msg_has_symbolic(const char *t){
    return has_kw_ci(t,"rafael")||has_kw_ci(t,"rafaelia")||has_kw_ci(t,"zipraf")||
           has_kw_ci(t,"ethica")||has_kw_ci(t,"bitraf")||has_kw_ci(t,"verbo")||
           has_kw_ci(t,"fiat lux")||has_kw_ci(t,"nihil relictum");
}

/* -------------------- value_map arrays -------------------- */

typedef struct {
    int ok;
    int infra, sec, ai, symb, total, msgs;
} VRow;

static VRow *vrows = NULL;
static int vrows_cap = 0;
static int vrows_max = -1;

static void ensure_vrows(int idx){
    if(idx < vrows_cap) return;
    int newcap = vrows_cap ? vrows_cap : 4096;
    while(newcap <= idx) newcap *= 2;
    VRow *nv = (VRow*)calloc((size_t)newcap, sizeof(VRow));
    if(!nv) die("OOM ensure_vrows");
    if(vrows){
        memcpy(nv, vrows, (size_t)vrows_cap * sizeof(VRow));
        free(vrows);
    }
    vrows = nv;
    vrows_cap = newcap;
}

static void load_value_map(const char *path){
    FILE *f = fopen(path,"r");
    if(!f) die("open omega_value_map.jsonl failed");

    char *line=NULL; size_t cap=0;
    while(getline(&line,&cap,f)!=-1){
        int conv_i=-1;
        if(!json_get_int(line,"conv_i",&conv_i)) continue;
        ensure_vrows(conv_i);
        VRow *r = &vrows[conv_i];
        r->ok = 1;
        json_get_int(line,"infra",&r->infra);
        json_get_int(line,"security",&r->sec);
        json_get_int(line,"ai",&r->ai);
        json_get_int(line,"symbolic",&r->symb);
        json_get_int(line,"total",&r->total);
        json_get_int(line,"msgs",&r->msgs);
        if(conv_i > vrows_max) vrows_max = conv_i;
    }
    free(line);
    fclose(f);
}

/* -------------------- per-conv metrics -------------------- */

typedef struct {
    int conv_i;
    int msgs;
    int tool_msgs;

    /* CV buckets (0..4) based on per-message category presence */
    int bcnt[5];

    /* OL (unique tokens) */
    HashSet hs;

    /* PT (theme persistence) */
    KeySet ks;
    int seed_msgs;        /* how many msgs used for seed */
    int seed_target;      /* how many msgs to seed */
} MState;

static void ms_reset(MState *ms, int conv_i){
    ms->conv_i = conv_i;
    ms->msgs = 0;
    ms->tool_msgs = 0;
    memset(ms->bcnt,0,sizeof(ms->bcnt));

    hs_free(&ms->hs);
    hs_init(&ms->hs, 2048);

    ks_reset(&ms->ks);
    ms->seed_msgs = 0;
    ms->seed_target = 10; /* seed from first 10 msgs */
}

/* tokenize text and update OL/PT */
static void process_text(MState *ms, const char *text){
    if(!text || !*text) return;

    char wbuf[128];
    int wlen = 0;

    const unsigned char *p = (const unsigned char*)text;
    for(; *p; p++){
        int c = *p;
        if(is_word_char(c)){
            if(wlen < (int)sizeof(wbuf)-1){
                wbuf[wlen++] = (char)tolower((unsigned char)c);
            } else {
                /* long word, keep hashing later */
            }
        } else {
            if(wlen >= 5){
                u64 h = fnv1a64(wbuf, (size_t)wlen);
                hs_add(&ms->hs, h);

                /* seed keywords for PT */
                if(ms->seed_msgs < ms->seed_target){
                    ks_add(&ms->ks, h);
                } else {
                    ks_mark_seen(&ms->ks, h);
                }
            }
            wlen = 0;
        }
    }
    if(wlen >= 5){
        u64 h = fnv1a64(wbuf, (size_t)wlen);
        hs_add(&ms->hs, h);
        if(ms->seed_msgs < ms->seed_target) ks_add(&ms->ks, h);
        else ks_mark_seen(&ms->ks, h);
    }
}

static double clamp01(double x){
    if(x < 0.0) return 0.0;
    if(x > 1.0) return 1.0;
    return x;
}

static const char* classify(double IC, double PP, double CV, double OL, double PT){
    /* simples e determinístico */
    if(IC > 0.60 && OL > 6.0 && PT > 0.60) return "framework_autoral";
    if(PP > 1.00 && CV < 0.55)            return "produto_tecnico";
    if(CV > 0.60 && IC > 0.50)            return "insight_nuclear";
    if(PP > 0.60 && IC < 0.35)            return "execucao_infra";
    return "processual";
}

static void emit_conv(FILE *out, const MState *ms){
    int ci = ms->conv_i;
    VRow vr = {0};
    if(ci >= 0 && ci < vrows_cap && vrows && vrows[ci].ok) vr = vrows[ci];

    int msgs = ms->msgs ? ms->msgs : (vr.msgs ? vr.msgs : 1);
    int tool_msgs = ms->tool_msgs;

    /* DF */
    double DF = (double)tool_msgs / (double)msgs;

    /* IC / PP from value_map totals per msg */
    double IC = (double)(vr.ai + vr.symb) / (double)msgs;
    double PP = (double)(vr.infra + vr.sec + vr.ai) / (double)msgs;

    /* OL = unique tokens per msg (approx) */
    double OL = (double)ms->hs.len / (double)msgs;

    /* PT = % keywords seen after seed */
    int kN = ms->ks.used;
    int seen=0;
    for(int i=0;i<kN;i++) if(ms->ks.seen[i]) seen++;
    double PT = (kN>0) ? ((double)seen / (double)kN) : 0.0;

    /* CV = top10% value share using buckets */
    int n_top = (int)((msgs + 9) / 10); /* ceil(0.1*msgs) */
    int total_val = 0;
    for(int v=0; v<=4; v++) total_val += v * ms->bcnt[v];

    int rem = n_top;
    int top_val = 0;
    for(int v=4; v>=0 && rem>0; v--){
        int take = ms->bcnt[v] < rem ? ms->bcnt[v] : rem;
        top_val += take * v;
        rem -= take;
    }
    double CV = (total_val>0) ? ((double)top_val / (double)total_val) : 0.0;

    const char *cls = classify(IC, PP, CV, OL, PT);

    /* emissão */
    fprintf(out,
        "{"
        "\"conv_i\":%d,"
        "\"msgs\":%d,"
        "\"tool_msgs\":%d,"
        "\"value_total\":%d,"
        "\"infra\":%d,\"security\":%d,\"ai\":%d,\"symbolic\":%d,"
        "\"DF\":%.4f,"
        "\"IC\":%.4f,"
        "\"PP\":%.4f,"
        "\"OL\":%.4f,"
        "\"PT\":%.4f,"
        "\"CV\":%.4f,"
        "\"class\":\"%s\""
        "}\n",
        ci,
        msgs,
        tool_msgs,
        vr.total,
        vr.infra, vr.sec, vr.ai, vr.symb,
        clamp01(DF),
        IC,
        PP,
        OL,
        clamp01(PT),
        clamp01(CV),
        cls
    );
}

/* ----------------------------- main ----------------------------- */

int main(int argc, char **argv){
    const char *F_MSGS  = (argc > 1) ? argv[1] : "omega_msgs.jsonl";
    const char *F_VMAP  = (argc > 2) ? argv[2] : "omega_value_map.jsonl";
    const char *F_OUT   = (argc > 3) ? argv[3] : "omega_metrics_v2.jsonl";

    if(!file_exists(F_MSGS)) die("faltou omega_msgs.jsonl");
    if(!file_exists(F_VMAP)) die("faltou omega_value_map.jsonl");

    load_value_map(F_VMAP);

    FILE *fin = fopen(F_MSGS,"r");
    if(!fin) die("open omega_msgs.jsonl failed");
    FILE *fout = fopen(F_OUT,"w");
    if(!fout) die("open output failed");

    char *line=NULL; size_t cap=0;

    MState ms;
    ms.conv_i = -1;
    memset(&ms,0,sizeof(ms));

    int cur_ci = -1;
    int first = 1;

    while(getline(&line,&cap,fin)!=-1){
        int ci=-1;
        if(!json_get_int(line,"conv_i",&ci)) continue;

        if(first){
            cur_ci = ci;
            ms_reset(&ms, cur_ci);
            first = 0;
        }

        if(ci != cur_ci){
            emit_conv(fout, &ms);
            ms_reset(&ms, ci);
            cur_ci = ci;
        }

        ms.msgs++;

        int ht=0;
        if(json_get_int(line,"has_tool",&ht) && ht) ms.tool_msgs++;

        /* text */
        char *text=NULL;
        if(json_get_string_unescape(line,"text",&text)){
            /* seed logic */
            if(ms.seed_msgs >= ms.seed_target){
                /* when seeding finished, enable seen marking */
            }

            /* CV per-message value (0..4) from category presence (boolean) */
            int infra = msg_has_infra(text);
            int sec   = msg_has_security(text);
            int ai    = msg_has_ai(text);
            int symb  = msg_has_symbolic(text);
            int v = infra + sec + ai + symb;
            if(v < 0) v = 0;
            if(v > 4) v = 4;
            ms.bcnt[v]++;

            /* OL/PT tokenization */
            process_text(&ms, text);

            free(text);
        } else {
            /* mesmo sem texto, ainda conta CV como 0 */
            ms.bcnt[0]++;
        }

        if(ms.seed_msgs < ms.seed_target) ms.seed_msgs++;
    }

    if(!first){
        emit_conv(fout, &ms);
    }

    hs_free(&ms.hs);
    free(line);
    fclose(fin);
    fclose(fout);

    fprintf(stderr, "[omega_metrics_v2] OK -> %s\n", F_OUT);
    return 0;
}
