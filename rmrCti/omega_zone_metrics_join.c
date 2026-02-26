#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE 8192

static int get_int(const char*s,const char*k,int*o){
    const char*p=strstr(s,k); if(!p) return 0;
    p=strchr(p,':'); if(!p) return 0;
    *o=strtol(p+1,NULL,10); return 1;
}
static int get_d(const char*s,const char*k,double*o){
    const char*p=strstr(s,k); if(!p) return 0;
    p=strchr(p,':'); if(!p) return 0;
    *o=strtod(p+1,NULL); return 1;
}

int main(){
    FILE *fz=fopen("omega_zone_conv_map.jsonl","r");
    FILE *fm=fopen("omega_metrics_v3.jsonl","r");
    if(!fz||!fm){perror("io");return 1;}

    char zl[LINE], ml[LINE];
    while(fgets(zl,LINE,fz) && fgets(ml,LINE,fm)){
        int z,c; double ic;
        if(!get_int(zl,"\"zone\"",&z)) continue;
        if(!get_int(zl,"\"conv_i\"",&c)) continue;
        if(!get_d(ml,"\"IC\"",&ic)) continue;
        printf("%d\t%d\t%.6f\n",z,c,ic);
    }
    fclose(fz); fclose(fm);
    return 0;
}
