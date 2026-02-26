#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE 8192
#define ZN   136

static int get_int(const char*s,const char*k,int*o){
    const char*p=strstr(s,k); if(!p) return 0;
    p=strchr(p,':'); if(!p) return 0;
    *o=strtol(p+1,NULL,10); return 1;
}

int main(){
    FILE *fm=fopen("omega_msgs.jsonl","r");
    FILE *fo=fopen("omega_zone_conv_map.logic.jsonl","w");
    if(!fm||!fo){perror("io");return 1;}

    char l[LINE]; long rows=0;
    while(fgets(l,LINE,fm)){
        int conv;
        if(!get_int(l,"\"conv_i\"",&conv)) continue;
        int zone = conv % ZN;
        fprintf(fo,"{\"zone\":%d,\"conv_i\":%d}\n",zone,conv);
        rows++;
    }
    fclose(fm); fclose(fo);
    fprintf(stderr,
      "[logic] OK -> omega_zone_conv_map.logic.jsonl rows=%ld\n",rows);
    return 0;
}
