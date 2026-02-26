#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE 8192
#define MAXZ 512

static int get_int(const char*s,const char*k,int*o){
    const char*p=strstr(s,k); if(!p) return 0;
    p=strchr(p,':'); if(!p) return 0;
    *o=strtol(p+1,NULL,10); return 1;
}

int main(int argc,char**argv){
    const char*in = argc>1?argv[1]:"omega_zone_conv_map.jsonl";
    FILE *f=fopen(in,"r"); if(!f){perror("in");return 1;}

    static unsigned M[MAXZ][MAXZ]={0};
    char l[LINE]; int last=-1,cur;
    long rows=0;

    FILE *fe=fopen("zone_markov_edges.txt","w");
    FILE *fs=fopen("zone_conv_streaks.txt","w");

    int streak=0;
    while(fgets(l,LINE,f)){
        if(!get_int(l,"\"zone\"",&cur)) continue;
        rows++;
        if(last!=-1){
            M[last][cur]++;
            if(cur==last) streak++;
            else{
                fprintf(fs,"STREAK\tzone=%d\tlen=%d\n",last,streak);
                streak=1;
            }
        } else streak=1;
        last=cur;
    }
    fclose(f);

    for(int i=0;i<MAXZ;i++)
      for(int j=0;j<MAXZ;j++)
        if(M[i][j])
          fprintf(fe,"EDGE\t%d->%d\t%u\n",i,j,M[i][j]);

    fclose(fe); fclose(fs);
    fprintf(stderr,
      "[markov] OK :: rows=%ld edges+streaks written\n",rows);
    return 0;
}
