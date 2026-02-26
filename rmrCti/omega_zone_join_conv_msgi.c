#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE 16384
#define INIT 256

typedef struct {
  int zone;
  long first_msg;   // inclusive
  long last_msg;    // exclusive
} ZR;

static int get_long(const char*s,const char*k,long*o){
  const char*p=strstr(s,k); if(!p) return 0;
  p=strchr(p,':'); if(!p) return 0;
  char *e=NULL;
  long v=strtol(p+1,&e,10);
  if(e==(p+1)) return 0;
  *o=v; return 1;
}

static int get_int(const char*s,const char*k,int*o){
  long v; if(!get_long(s,k,&v)) return 0;
  *o=(int)v; return 1;
}

/* zonas são ranges em bytes; precisamos converter para ranges em msg index.
   Estratégia REAL: usamos zone_timeline.txt / zone_timeline map se existir,
   SENÃO: derivamos "first_msg/last_msg" por contagem proporcional a zbytes
   (fallback determinístico, mas o ideal é ter timeline real). */

int main(int argc,char**argv){
  const char *FZ = "omega_zone_metrics.jsonl";
  const char *FM = "omega_msgs.jsonl";
  const char *FO = "omega_zone_conv_map.jsonl";
  const char *FT = "zone_timeline.txt"; // opcional

  FILE *fz=fopen(FZ,"r");
  FILE *fm=fopen(FM,"r");
  if(!fz||!fm){ perror("open"); return 1; }

  // 1) carregar zonas (zone, zbytes) e tentar timeline (zone->first_msg/last_msg)
  ZR *z = (ZR*)malloc(INIT*sizeof(ZR));
  int zn=0, zc=INIT;
  char l[LINE];

  // se tiver zone_timeline.txt, usamos ela diretamente
  FILE *ft=fopen(FT,"r");
  if(ft){
    // formato esperado: msg_i zone (ou msg_i zone conv_i) — aceitamos 2+ cols
    // vamos achar para cada zone: min msg_i e max msg_i+1
    long minm[4096], maxm[4096];
    for(int i=0;i<4096;i++){ minm[i]=999999999; maxm[i]=-1; }

    long msg_i; int zone;
    while(fgets(l,LINE,ft)){
      // tenta ler 2 primeiros ints/longs
      char *p=l;
      msg_i=strtol(p,&p,10);
      while(*p==' '||*p=='\t') p++;
      zone=(int)strtol(p,&p,10);
      if(zone>=0 && zone<4096){
        if(msg_i<minm[zone]) minm[zone]=msg_i;
        if(msg_i>maxm[zone]) maxm[zone]=msg_i;
      }
    }
    fclose(ft);

    for(int zone=0; zone<4096; zone++){
      if(maxm[zone]>=0){
        if(zn>=zc){ zc<<=1; z=(ZR*)realloc(z,zc*sizeof(ZR)); }
        z[zn].zone=zone;
        z[zn].first_msg=minm[zone];
        z[zn].last_msg=maxm[zone]+1;
        zn++;
      }
    }

  } else {
    // fallback: cria ranges aproximados por zbytes, usando total_msgs
    // 1) conta msgs totais
    long total_msgs=0;
    while(fgets(l,LINE,fm)) total_msgs++;
    rewind(fm);

    // 2) soma zbytes total
    long zsum=0;
    while(fgets(l,LINE,fz)){
      int zone; long zbytes;
      if(!get_int(l,"\"zone\"",&zone)) continue;
      if(!get_long(l,"\"zbytes\"",&zbytes)) zbytes=0;
      zsum += (zbytes>0? zbytes:0);
    }
    rewind(fz);

    // 3) cria ranges proporcionais (determinístico)
    long cursor=0;
    while(fgets(l,LINE,fz)){
      int zone; long zbytes;
      if(!get_int(l,"\"zone\"",&zone)) continue;
      if(!get_long(l,"\"zbytes\"",&zbytes)) zbytes=0;
      if(zbytes<=0) continue;

      long span = (zsum>0) ? ( (zbytes * total_msgs) / zsum ) : 0;
      if(span<=0) span=1;

      if(zn>=zc){ zc<<=1; z=(ZR*)realloc(z,zc*sizeof(ZR)); }
      z[zn].zone=zone;
      z[zn].first_msg=cursor;
      z[zn].last_msg=cursor+span;
      cursor += span;
      zn++;
    }
  }
  fclose(fz);

  // 2) join msgs -> zone -> conv
  FILE *fo=fopen(FO,"w");
  if(!fo){ perror("out"); return 2; }

  long rows=0, miss=0, bad=0;
  long msg_i=-1; int conv_i=-1;

  while(fgets(l,LINE,fm)){
    if(!get_long(l,"\"msg_i\"",&msg_i)) { miss++; continue; }
    if(!get_int(l,"\"conv_i\"",&conv_i)) { miss++; continue; }

    int hit=-1;
    for(int i=0;i<zn;i++){
      if(msg_i>=z[i].first_msg && msg_i<z[i].last_msg){
        hit=z[i].zone; break;
      }
    }
    if(hit<0){ bad++; continue; }

    fprintf(fo,"{\"zone\":%d,\"conv_i\":%d,\"msg_i\":%ld}\n",hit,conv_i,msg_i);
    rows++;
  }

  fclose(fm);
  fclose(fo);
  free(z);

  fprintf(stderr,
    "[join_msgi] OK -> %s zones=%d rows=%ld miss=%ld bad=%ld\n",
    FO, zn, rows, miss, bad
  );

  if(rows==0){
    fprintf(stderr,
      "[join_msgi] FAIL-SAFE: rows=0. Provável: omega_msgs.jsonl não tem \"msg_i\".\n"
    );
    return 3;
  }
  return 0;
}
