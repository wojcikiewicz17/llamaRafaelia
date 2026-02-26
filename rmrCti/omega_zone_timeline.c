#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE 8192

static int get_int(const char*s,const char*k,int*o){
    const char*p=strstr(s,k); if(!p) return 0;
    p=strchr(p,':'); if(!p) return 0;
    *o=strtol(p+1,NULL,10); return 1;
}

int main(){
    FILE *f=fopen("omega_zone_conv_map.jsonl","r");
    if(!f){perror("io");return 1;}

    char l[LINE];
    long i=0;
    while(fgets(l,LINE,f)){
        int z,c;
        if(!get_int(l,"\"zone\"",&z)) continue;
        if(!get_int(l,"\"conv_i\"",&c)) continue;
        printf("%ld\t%d\t%d\n",i++,z,c);
    }
    fclose(f);
    return 0;
}
