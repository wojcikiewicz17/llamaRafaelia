#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE 8192
#define MAXZ 512

typedef struct { int conv; long cnt; } Row;

static int get_int(const char*s,const char*k,int*o){
    const char*p=strstr(s,k); if(!p) return 0;
    p=strchr(p,':'); if(!p) return 0;
    *o=strtol(p+1,NULL,10); return 1;
}

int cmp(const void*a,const void*b){
    return ((Row*)b)->cnt - ((Row*)a)->cnt;
}

int main(){
    FILE *f=fopen("omega_zone_conv_map.jsonl","r");
    if(!f){perror("io");return 1;}

    Row v[100000]; int n=0;
    char l[LINE];
    while(fgets(l,LINE,f)){
        int z,c;
        if(!get_int(l,"\"zone\"",&z)) continue;
        if(!get_int(l,"\"conv_i\"",&c)) continue;
        int found=0;
        for(int i=0;i<n;i++) if(v[i].conv==c){ v[i].cnt++; found=1; break; }
        if(!found){ v[n].conv=c; v[n].cnt=1; n++; }
    }
    fclose(f);

    qsort(v,n,sizeof(Row),cmp);
    for(int i=0;i<20 && i<n;i++)
        printf("%d\t%ld\n",v[i].conv,v[i].cnt);
    return 0;
}
