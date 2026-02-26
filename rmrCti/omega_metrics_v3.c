#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

static double getnum(const char *line, const char *key){
    char pat[64];
    snprintf(pat,sizeof(pat),"\"%s\":",key);
    const char *p = strstr(line, pat);
    if(!p) return 0.0;
    p += strlen(pat);
    return strtod(p, NULL);
}

static int getint(const char *line, const char *key){
    return (int)getnum(line,key);
}

static void getstr(const char *line, const char *key, char *out, size_t sz){
    char pat[64];
    snprintf(pat,sizeof(pat),"\"%s\":\"",key);
    const char *p = strstr(line, pat);
    if(!p){ out[0]=0; return; }
    p += strlen(pat);
    size_t i=0;
    while(*p && *p!='"' && i+1<sz){
        out[i++] = *p++;
    }
    out[i]=0;
}

int main(int argc, char **argv){
    const char *in  = (argc>1)?argv[1]:"omega_metrics_v2.jsonl";
    const char *out = (argc>2)?argv[2]:"omega_metrics_v3.jsonl";

    FILE *f = fopen(in,"r");
    if(!f){ perror("open in"); return 1; }
    FILE *o = fopen(out,"w");
    if(!o){ perror("open out"); return 1; }

    char *line=NULL;
    size_t cap=0;

    while(getline(&line,&cap,f)!=-1){
        int conv_i = getint(line,"conv_i");
        int msgs   = getint(line,"msgs");

        double PP = getnum(line,"PP");
        double IC = getnum(line,"IC");
        double CV = getnum(line,"CV");
        double DF = getnum(line,"DF");

        double PP_adj = PP * (1.0 - DF);
        double IC_adj = (msgs>0)? IC / sqrt((double)msgs) : 0.0;
        double CV_adj = (msgs>0)? CV * log2((double)msgs + 1.0) : 0.0;

        const char *cls =
            (PP_adj > 0.8) ? "produto_maduro" :
            (IC_adj > 0.7) ? "framework_autoral" :
                             "processual";

        fprintf(o,
            "{\"conv_i\":%d,"
            "\"msgs\":%d,"
            "\"PP\":%.6f,"
            "\"PP_adj\":%.6f,"
            "\"IC\":%.6f,"
            "\"IC_adj\":%.6f,"
            "\"CV\":%.6f,"
            "\"CV_adj\":%.6f,"
            "\"class\":\"%s\"}\n",
            conv_i, msgs,
            PP, PP_adj,
            IC, IC_adj,
            CV, CV_adj,
            cls
        );
    }

    free(line);
    fclose(f);
    fclose(o);

    fprintf(stderr,"[omega_metrics_v3] OK -> %s\n", out);
    return 0;
}
