#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define LINE 8192
#define MAXZ 512

static int get_int(const char*s,const char*k,int*o){
    const char*p=strstr(s,k); if(!p) return 0;
    p=strchr(p,':'); if(!p) return 0;
    *o=strtol(p+1,NULL,10); return 1;
}

int main(){
    FILE *f=fopen("omega_zone_conv_map.jsonl","r");
    if(!f){perror("io");return 1;}

    long cnt[MAXZ]={0}, total=0;
    char l[LINE];
    while(fgets(l,LINE,f)){
        int z;
        if(!get_int(l,"\"zone\"",&z)) continue;
        if(z>=0 && z<MAXZ){ cnt[z]++; total++; }
    }
    fclose(f);

    for(int z=0;z<MAXZ;z++){
        if(cnt[z]){
            double p=(double)cnt[z]/total;
            double H=-p*log2(p);
            printf("%d\t%.6f\n",z,H);
        }
    }
    return 0;
}
