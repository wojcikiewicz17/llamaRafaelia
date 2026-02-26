#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE 8192
#define MAXZ 512

static int get_int(const char*s,const char*k,int*o){
  const char*p=strstr(s,k); if(!p) return 0;
  p=strchr(p,':'); if(!p) return 0;
  *o=(int)strtol(p+1,NULL,10); return 1;
}

int main(int argc, char **argv){
  const char *in = (argc>1)?argv[1]:"omega_zone_conv_map.jsonl";
  FILE *f=fopen(in,"r");
  if(!f){ perror("io"); return 1; }

  // Markov counts (sparse-ish printing)
  static unsigned int M[MAXZ][MAXZ];
  memset(M,0,sizeof(M));

  // Stability (streak lengths)
  int prev=-1, streak=0;
  long long rows=0;

  char l[LINE];
  while(fgets(l,LINE,f)){
    int z;
    if(!get_int(l,"\"zone\"",&z)) continue;
    if(z<0 || z>=MAXZ) continue;

    // markov
    if(prev!=-1 && prev<MAXZ) M[prev][z]++;

    // stability streaks
    if(z==prev) streak++;
    else{
      if(prev!=-1) printf("STREAK\tzone=%d\tlen=%d\n", prev, streak);
      streak=1; prev=z;
    }
    rows++;
  }
  if(prev!=-1) printf("STREAK\tzone=%d\tlen=%d\n", prev, streak);
  fclose(f);

  // Print transitions
  for(int i=0;i<MAXZ;i++){
    for(int j=0;j<MAXZ;j++){
      if(M[i][j]) printf("EDGE\t%d->%d\t%u\n", i, j, M[i][j]);
    }
  }
  fprintf(stderr,"[omega_zone_markov_stability] OK :: in=%s rows=%lld\n", in, rows);
  return 0;
}
