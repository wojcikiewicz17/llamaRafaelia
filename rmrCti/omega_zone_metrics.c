#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

/*
  omega_zone_metrics.c
  --------------------
  Input : omega_events.jsonl
  Output: omega_zone_metrics.jsonl + omega_zone_rank.txt

  Métricas:
    - ZV  (zone volatility): |dppm| normalizado (por 1e5) + hibit_penalty
    - H   (entropy proxy): entropia das "classes" (space/quote/brace/colon/comma/hibit/other)
    - QD  (quote density): quote / zbytes
    - BD  (brace density): brace / zbytes
    - HD  (hibit density): hibit / zbytes
    - AS  (anomaly score): combinação ponderada (ZV + 2*H + 5*HD + event bump)
*/

static int json_get_ll(const char *line, const char *key, long long *out){
    char pat[128];
    snprintf(pat,sizeof(pat),"\"%s\"",key);
    const char *p = strstr(line, pat);
    if(!p) return 0;
    p = strchr(p, ':');
    if(!p) return 0;
    p++;
    while(*p && isspace((unsigned char)*p)) p++;
    int sign = 1;
    if(*p=='-'){ sign=-1; p++; }
    if(!isdigit((unsigned char)*p)) return 0;
    long long v=0;
    while(isdigit((unsigned char)*p)){
        v = v*10 + (*p - '0');
        p++;
    }
    *out = v * sign;
    return 1;
}

static int json_get_str(const char *line, const char *key, char *out, size_t out_sz){
    char pat[128];
    snprintf(pat,sizeof(pat),"\"%s\"",key);
    const char *p = strstr(line, pat);
    if(!p) return 0;
    p = strchr(p, ':');
    if(!p) return 0;
    p++;
    while(*p && isspace((unsigned char)*p)) p++;
    if(*p!='"') return 0;
    p++;
    size_t i=0;
    while(*p && *p!='"'){
        if(i+1<out_sz) out[i++] = *p;
        p++;
    }
    out[i]=0;
    return 1;
}

static double safe_log2(double x){
    return (x<=0.0) ? 0.0 : log(x)/log(2.0);
}

static double entropy7(const double *p, int n){
    // H = -sum p_i log2 p_i
    double H=0.0;
    for(int i=0;i<n;i++){
        if(p[i] > 0.0) H -= p[i] * safe_log2(p[i]);
    }
    return H;
}

typedef struct {
    long long zone, off, zbytes;
    long long space, quote, brace, colon, comma, hibit;
    long long dppm;
    char event[32];
    double QD, BD, HD;
    double ZV, H, AS;
} Row;

static int cmp_as_desc(const void *a, const void *b){
    const Row *A=(const Row*)a, *B=(const Row*)b;
    if(A->AS < B->AS) return 1;
    if(A->AS > B->AS) return -1;
    return 0;
}

int main(int argc, char **argv){
    const char *IN  = (argc>1) ? argv[1] : "omega_events.jsonl";
    const char *OUT = (argc>2) ? argv[2] : "omega_zone_metrics.jsonl";
    const char *RANK= (argc>3) ? argv[3] : "omega_zone_rank.txt";

    FILE *f = fopen(IN,"rb");
    if(!f){ perror("open input"); return 1; }

    FILE *o = fopen(OUT,"wb");
    if(!o){ perror("open output"); fclose(f); return 1; }

    Row *rows=NULL;
    size_t n=0, cap=256;

    rows = (Row*)calloc(cap, sizeof(Row));
    if(!rows){ fprintf(stderr,"OOM\n"); fclose(f); fclose(o); return 2; }

    char *line=NULL;
    size_t lc=0;

    while(getline(&line,&lc,f)!=-1){
        Row r; memset(&r,0,sizeof(r));
        strcpy(r.event,"");

        if(!json_get_ll(line,"zone",&r.zone)) continue;
        json_get_ll(line,"off",&r.off);
        json_get_ll(line,"zbytes",&r.zbytes);
        json_get_ll(line,"dppm",&r.dppm);
        json_get_str(line,"event",r.event,sizeof(r.event));

        // ppm nested keys live flat in the line, então só buscar por nome
        json_get_ll(line,"space",&r.space);
        json_get_ll(line,"quote",&r.quote);
        json_get_ll(line,"brace",&r.brace);
        json_get_ll(line,"colon",&r.colon);
        json_get_ll(line,"comma",&r.comma);
        json_get_ll(line,"hibit",&r.hibit);

        double zb = (r.zbytes>0) ? (double)r.zbytes : 1.0;

        r.QD = (double)r.quote / zb;
        r.BD = (double)r.brace / zb;
        r.HD = (double)r.hibit / zb;

        // proxy volatility: |dppm| normalized (por 1e5) + hibit density * 1e3
        r.ZV = fabs((double)r.dppm) / 100000.0 + r.HD * 1000.0;

        // entropy proxy among 7 buckets: space quote brace colon comma hibit other
        long long known = r.space + r.quote + r.brace + r.colon + r.comma + r.hibit;
        long long other = (r.zbytes>known) ? (r.zbytes - known) : 0;

        double p[7];
        p[0]=(double)r.space/zb;
        p[1]=(double)r.quote/zb;
        p[2]=(double)r.brace/zb;
        p[3]=(double)r.colon/zb;
        p[4]=(double)r.comma/zb;
        p[5]=(double)r.hibit/zb;
        p[6]=(double)other/zb;

        r.H = entropy7(p,7);

        // event bump
        double bump = 0.0;
        if(!strcasecmp(r.event,"boot")) bump = 0.50;
        else if(!strcasecmp(r.event,"hot")) bump = 0.75;
        else if(!strcasecmp(r.event,"stable")) bump = 0.10;

        // anomaly score: combina vol + entropia + hibit + bump
        r.AS = (1.0*r.ZV) + (2.0*r.H) + (5.0*r.HD*1000.0) + bump;

        // write jsonl
        fprintf(o,
            "{\"zone\":%lld,\"off\":%lld,\"zbytes\":%lld,"
            "\"dppm\":%lld,\"event\":\"%s\","
            "\"QD\":%.6f,\"BD\":%.6f,\"HD\":%.6f,"
            "\"ZV\":%.6f,\"H\":%.6f,\"AS\":%.6f}\n",
            r.zone, r.off, r.zbytes, r.dppm, r.event,
            r.QD, r.BD, r.HD, r.ZV, r.H, r.AS
        );

        if(n>=cap){
            cap*=2;
            Row *nr=(Row*)realloc(rows, cap*sizeof(Row));
            if(!nr){ fprintf(stderr,"OOM\n"); free(rows); free(line); fclose(f); fclose(o); return 2; }
            rows=nr;
        }
        rows[n++] = r;
    }

    free(line);
    fclose(f);
    fclose(o);

    // rank
    qsort(rows,n,sizeof(Row),cmp_as_desc);
    FILE *rk = fopen(RANK,"wb");
    if(!rk){ perror("open rank"); free(rows); return 1; }

    fprintf(rk, "rank\tAS\tZV\tH\tHD\tQD\tzone\toff\tevent\n");
    size_t lim = (n<50)?n:50;
    for(size_t i=0;i<lim;i++){
        Row *r=&rows[i];
        fprintf(rk, "%zu\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%lld\t%lld\t%s\n",
            i+1, r->AS, r->ZV, r->H, r->HD, r->QD, r->zone, r->off, r->event
        );
    }
    fclose(rk);

    fprintf(stderr,"[omega_zone_metrics] OK -> %s | %s (rows=%zu)\n", OUT, RANK, n);
    free(rows);
    return 0;
}
