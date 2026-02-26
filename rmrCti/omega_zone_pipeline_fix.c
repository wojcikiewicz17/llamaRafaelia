/*
  omega_zone_pipeline_fix.c  (Termux/Android 15 safe)
  Build: cc -O3 -pipe -Wall -Wextra -std=c11 omega_zone_pipeline_fix.c -o ozp
  Run:   ./ozp omega_zone_metrics.jsonl omega_msgs.jsonl omega_metrics_v3.jsonl
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#ifndef LINE
#define LINE 16384
#endif

#ifndef ZMAX
#define ZMAX 4096
#endif

typedef struct { int zone; long off; long zbytes; } Zone;
typedef struct { int conv_i; double IC,PP,CV; } MRow;

typedef struct { int zone; long a,b; } ZRange;

/* ---------------- tiny parsing: "key": number ---------------- */

static const char* skip_ws(const char *p){
  while(*p==' '||*p=='\t'||*p=='\r'||*p=='\n') p++;
  return p;
}
static const char* find_key(const char *s, const char *key){
  size_t klen = strlen(key);
  const char *p = s;
  while((p = strstr(p, key))){
    const char *q = p + klen;
    q = skip_ws(q);
    if(*q==':') return q+1;
    p = p + 1;
  }
  return NULL;
}
static int get_i32(const char *s, const char *key, int *out){
  const char *p = find_key(s,key);
  if(!p) return 0;
  p = skip_ws(p);
  char *e=NULL;
  long v = strtol(p,&e,10);
  if(e==p) return 0;
  if(v < INT32_MIN || v > INT32_MAX) return 0;
  *out = (int)v;
  return 1;
}
static int get_i64(const char *s, const char *key, long *out){
  const char *p = find_key(s,key);
  if(!p) return 0;
  p = skip_ws(p);
  char *e=NULL;
  long v = strtol(p,&e,10);
  if(e==p) return 0;
  *out = v;
  return 1;
}
static int get_f64(const char *s, const char *key, double *out){
  const char *p = find_key(s,key);
  if(!p) return 0;
  p = skip_ws(p);
  char *e=NULL;
  double v = strtod(p,&e);
  if(e==p) return 0;
  *out = v;
  return 1;
}

/* ---------------- helpers ---------------- */

static void die(const char *m){ perror(m); exit(1); }

static long fsize_bytes(const char *path){
  FILE *f=fopen(path,"rb");
  if(!f) return -1;
  if(fseek(f,0,SEEK_END)!=0){ fclose(f); return -1; }
  long sz = ftell(f);
  fclose(f);
  return sz;
}

static int cmp_zone_off(const void*a,const void*b){
  const Zone*x=(const Zone*)a,*y=(const Zone*)b;
  if(x->off < y->off) return -1;
  if(x->off > y->off) return  1;
  /* desempate: range maior primeiro */
  if(x->zbytes > y->zbytes) return -1;
  if(x->zbytes < y->zbytes) return  1;
  return 0;
}

static int cmp_mrow_conv(const void*a,const void*b){
  const MRow*x=(const MRow*)a,*y=(const MRow*)b;
  if(x->conv_i < y->conv_i) return -1;
  if(x->conv_i > y->conv_i) return  1;
  return 0;
}

static MRow* bsearch_m(MRow*v,int n,int key){
  int lo=0,hi=n-1;
  while(lo<=hi){
    int mid=(lo+hi)>>1;
    int c=v[mid].conv_i;
    if(c==key) return &v[mid];
    if(key<c) hi=mid-1; else lo=mid+1;
  }
  return NULL;
}

/* ---------------- load zones (needs "zone","off","zbytes") ---------------- */

static int load_zones(const char *FZ, Zone **out, int *outn, int *out_maxz){
  FILE *f=fopen(FZ,"r");
  if(!f) return 0;

  int cap=256,n=0,maxz=-1;
  Zone *z=(Zone*)malloc((size_t)cap*sizeof(Zone));
  if(!z) die("oom zones");

  char line[LINE];
  while(fgets(line,sizeof(line),f)){
    Zone t; memset(&t,0,sizeof(t));
    if(!get_i32(line,"\"zone\"",&t.zone)) continue;
    if(!get_i64(line,"\"off\"",&t.off)) continue;
    if(!get_i64(line,"\"zbytes\"",&t.zbytes)) continue;
    if(t.zbytes<=0) continue;
    if(t.zone>maxz) maxz=t.zone;
    if(n>=cap){
      cap<<=1;
      Zone *nz=(Zone*)realloc(z,(size_t)cap*sizeof(Zone));
      if(!nz) die("realloc zones");
      z=nz;
    }
    z[n++]=t;
  }
  fclose(f);

  if(n==0){ free(z); return 0; }

  qsort(z,n,sizeof(Zone),cmp_zone_off);
  *out=z; *outn=n; *out_maxz=maxz;
  return 1;
}

/* ---------------- method A: JOIN by ftello offsets ---------------- */

static int join_by_offset(
  const char *FM, const char *FO, Zone *z, int zn,
  long msgs_sz, long *out_rows, long *out_miss, long *out_bad
){
  FILE *fm=fopen(FM,"r");
  if(!fm) return 0;

  FILE *fo=fopen("omega_zone_conv_map.tmp","w");
  if(!fo){ fclose(fm); return 0; }

  char line[LINE];
  long rows=0, miss=0, bad=0;

  while(1){
    long pos = ftell(fm);         /* offset REAL do início da linha */
    if(pos<0) break;
    if(!fgets(line,sizeof(line),fm)) break;

    int conv=-1;
    if(!get_i32(line,"\"conv_i\"",&conv)){ miss++; continue; }

    /* binsearch in zones by pos */
    int hit=-1;
    int lo=0,hi=zn;
    while(lo<hi){
      int m=(lo+hi)>>1;
      long a=z[m].off, b=a+z[m].zbytes;
      if(pos>=a && pos<b){ hit=z[m].zone; break; }
      if(pos<a) hi=m; else lo=m+1;
    }
    if(hit<0){ bad++; continue; }

    fprintf(fo,"{\"zone\":%d,\"conv_i\":%d,\"off\":%ld}\n",hit,conv,pos);
    rows++;
  }

  fclose(fm);
  fclose(fo);

  if(rows>0){
    rename("omega_zone_conv_map.tmp",FO);
  }else{
    remove("omega_zone_conv_map.tmp");
  }

  *out_rows=rows; *out_miss=miss; *out_bad=bad;

  /* Heurística de mismatch: muitos bad ou offsets fora do file */
  if(rows==0) return 2;

  /* se zone ranges claramente fora do arquivo, consideramos mismatch */
  long out_of=0;
  for(int i=0;i<zn;i++){
    long a=z[i].off, b=a+z[i].zbytes;
    if(a<0||b<0||a>=msgs_sz||b>msgs_sz) out_of++;
  }
  if(out_of>0){
    fprintf(stderr,
      "[join_offset] WARN out_of=%ld/%d (offsets fora do omega_msgs)\n",
      out_of, zn
    );
    /* ainda pode ter gerado algo, mas indicamos fallback opcional */
    if((double)out_of/(double)zn > 0.20) return 3;
  }
  return 1;
}

/* ---------------- load timeline -> zone ranges (msg index) ----------------
   Espera linhas do tipo:
     <msg_i> <zone> ...
   A gente calcula min/max msg_i por zone.
*/

static int load_timeline_ranges(const char *FT, ZRange **out, int *outn, int *out_maxz){
  FILE *f=fopen(FT,"r");
  if(!f) return 0;

  long minm[ZMAX], maxm[ZMAX];
  for(int i=0;i<ZMAX;i++){ minm[i]=LONG_MAX; maxm[i]=-1; }

  char line[LINE];
  while(fgets(line,sizeof(line),f)){
    char *p=line;
    long msg_i = strtol(p,&p,10);
    while(*p==' '||*p=='\t') p++;
    long zone  = strtol(p,&p,10);
    if(zone<0 || zone>=ZMAX) continue;
    if(msg_i<0) continue;
    if(msg_i<minm[zone]) minm[zone]=msg_i;
    if(msg_i>maxm[zone]) maxm[zone]=msg_i;
  }
  fclose(f);

  int cap=256,n=0,maxz=-1;
  ZRange *vr=(ZRange*)malloc((size_t)cap*sizeof(ZRange));
  if(!vr) die("oom ranges");

  for(int z=0; z<ZMAX; z++){
    if(maxm[z]>=0 && minm[z]!=LONG_MAX){
      if(n>=cap){
        cap<<=1;
        ZRange *nv=(ZRange*)realloc(vr,(size_t)cap*sizeof(ZRange));
        if(!nv) die("realloc ranges");
        vr=nv;
      }
      vr[n].zone=z;
      vr[n].a=minm[z];
      vr[n].b=maxm[z]+1; /* exclusive */
      if(z>maxz) maxz=z;
      n++;
    }
  }

  if(n==0){ free(vr); return 0; }

  *out=vr; *outn=n; *out_maxz=maxz;
  return 1;
}

/* ---------------- method B: JOIN by timeline (msg_i = line index) --------- */

static int join_by_timeline(
  const char *FM, const char *FO, ZRange *rg, int rn,
  long *out_rows, long *out_miss, long *out_bad
){
  FILE *fm=fopen(FM,"r");
  if(!fm) return 0;

  FILE *fo=fopen("omega_zone_conv_map.tmp","w");
  if(!fo){ fclose(fm); return 0; }

  char line[LINE];
  long rows=0, miss=0, bad=0;
  long msg_i=0;

  while(fgets(line,sizeof(line),fm)){
    int conv=-1;
    if(!get_i32(line,"\"conv_i\"",&conv)){ miss++; msg_i++; continue; }

    int hit=-1;
    /* rn ~ 136 => linear OK */
    for(int i=0;i<rn;i++){
      if(msg_i>=rg[i].a && msg_i<rg[i].b){ hit=rg[i].zone; break; }
    }
    if(hit<0){ bad++; msg_i++; continue; }

    fprintf(fo,"{\"zone\":%d,\"conv_i\":%d,\"msg_i\":%ld}\n",hit,conv,msg_i);
    rows++;
    msg_i++;
  }

  fclose(fm);
  fclose(fo);

  if(rows>0){
    rename("omega_zone_conv_map.tmp",FO);
  }else{
    remove("omega_zone_conv_map.tmp");
  }

  *out_rows=rows; *out_miss=miss; *out_bad=bad;
  return (rows>0);
}

/* ---------------- rank zones ---------------- */

static int rank_zones(const char *F_MAP, const char *F_RANK, int topN, int zcap){
  FILE *f=fopen(F_MAP,"r");
  if(!f) return 0;

  long *cnt=(long*)calloc((size_t)zcap,sizeof(long));
  if(!cnt) die("oom rank cnt");

  char line[LINE];
  long rows=0,bad=0;
  int z=0;

  while(fgets(line,sizeof(line),f)){
    if(!get_i32(line,"\"zone\"",&z)){ bad++; continue; }
    if(z<0||z>=zcap){ bad++; continue; }
    cnt[z]++; rows++;
  }
  fclose(f);

  FILE *o=fopen(F_RANK,"w");
  if(!o){ free(cnt); return 0; }

  fprintf(o,"rank\tzone\trefs\tshare\n");
  fprintf(stdout,"rank\tzone\trefs\tshare\n");

  for(int k=1;k<=topN;k++){
    long best=-1; int bestz=-1;
    for(int i=0;i<zcap;i++){
      if(cnt[i]>best){ best=cnt[i]; bestz=i; }
    }
    if(bestz<0 || best<=0) break;
    double share = rows? (100.0*(double)best/(double)rows):0.0;
    fprintf(o,"%d\t%d\t%ld\t%.2f%%\n",k,bestz,best,share);
    fprintf(stdout,"%d\t%d\t%ld\t%.2f%%\n",k,bestz,best,share);
    cnt[bestz] = -cnt[bestz];
  }

  fclose(o);
  free(cnt);

  fprintf(stderr,"[rank] OK rows=%ld bad=%ld -> %s\n",rows,bad,F_RANK);
  return (rows>0);
}

/* ---------------- metrics load + join ---------------- */

static int load_metrics(const char *FMET, MRow **out, int *outn){
  FILE *f=fopen(FMET,"r");
  if(!f) return 0;

  int cap=4096,n=0;
  MRow *v=(MRow*)malloc((size_t)cap*sizeof(MRow));
  if(!v) die("oom metrics");

  char line[LINE];
  while(fgets(line,sizeof(line),f)){
    MRow r; memset(&r,0,sizeof(r));
    if(!get_i32(line,"\"conv_i\"",&r.conv_i)) continue;
    (void)get_f64(line,"\"IC\"",&r.IC);
    (void)get_f64(line,"\"PP\"",&r.PP);
    (void)get_f64(line,"\"CV\"",&r.CV);
    if(n>=cap){
      cap<<=1;
      MRow *nv=(MRow*)realloc(v,(size_t)cap*sizeof(MRow));
      if(!nv) die("realloc metrics");
      v=nv;
    }
    v[n++]=r;
  }
  fclose(f);

  if(n==0){ free(v); return 0; }
  qsort(v,n,sizeof(MRow),cmp_mrow_conv);
  *out=v; *outn=n;
  return 1;
}

static int join_zone_metrics(
  const char *F_MAP, const char *F_X, const char *F_S,
  MRow *mv, int mn, int zcap
){
  FILE *f=fopen(F_MAP,"r");
  if(!f) return 0;
  FILE *ox=fopen(F_X,"w");
  if(!ox){ fclose(f); return 0; }
  FILE *os=fopen(F_S,"w");
  if(!os){ fclose(f); fclose(ox); return 0; }

  double *sumIC=(double*)calloc((size_t)zcap,sizeof(double));
  double *sumPP=(double*)calloc((size_t)zcap,sizeof(double));
  double *sumCV=(double*)calloc((size_t)zcap,sizeof(double));
  long   *cnt  =(long*)calloc((size_t)zcap,sizeof(long));
  if(!sumIC||!sumPP||!sumCV||!cnt) die("oom zone stats");

  char line[LINE];
  long rows=0,miss=0,bad=0;
  int z=0,c=0;

  while(fgets(line,sizeof(line),f)){
    if(!get_i32(line,"\"zone\"",&z)){ bad++; continue; }
    if(!get_i32(line,"\"conv_i\"",&c)){ bad++; continue; }
    if(z<0||z>=zcap){ bad++; continue; }

    MRow *r=bsearch_m(mv,mn,c);
    if(!r){ miss++; continue; }

    fprintf(ox,"%d\t%d\t%.6f\t%.6f\t%.6f\n",z,c,r->IC,r->PP,r->CV);
    sumIC[z]+=r->IC; sumPP[z]+=r->PP; sumCV[z]+=r->CV; cnt[z]++;
    rows++;
  }

  fclose(f); fclose(ox);

  fprintf(os,"zone\trefs\tIC_mean\tPP_mean\tCV_mean\n");
  for(int i=0;i<zcap;i++){
    if(cnt[i]){
      fprintf(os,"%d\t%ld\t%.6f\t%.6f\t%.6f\n",
        i,cnt[i],
        sumIC[i]/(double)cnt[i],
        sumPP[i]/(double)cnt[i],
        sumCV[i]/(double)cnt[i]
      );
    }
  }
  fclose(os);

  free(sumIC); free(sumPP); free(sumCV); free(cnt);

  fprintf(stderr,
    "[metrics_join] OK rows=%ld miss=%ld bad=%ld -> %s + %s\n",
    rows,miss,bad,F_X,F_S
  );
  return (rows>0);
}

/* ---------------- markov edges: sparse hash ---------------- */

typedef struct { uint32_t a,b,used,cnt; } Edge;
static uint32_t mix32(uint32_t x){
  x ^= x>>16; x *= 0x7feb352dU;
  x ^= x>>15; x *= 0x846ca68bU;
  x ^= x>>16; return x;
}
static uint32_t eh(uint32_t a,uint32_t b){
  return mix32(a*1315423911u ^ (b + 0x9e3779b9u));
}
static void edge_inc(Edge *t, uint32_t cap, uint32_t a, uint32_t b){
  uint32_t m=cap-1, h=eh(a,b)&m;
  for(;;){
    Edge *s=&t[h];
    if(!s->used){ s->used=1; s->a=a; s->b=b; s->cnt=1; return; }
    if(s->a==a && s->b==b){ s->cnt++; return; }
    h=(h+1)&m;
  }
}

static int markov_streaks(const char *F_MAP, const char *F_E, const char *F_ST){
  FILE *f=fopen(F_MAP,"r");
  if(!f) return 0;
  FILE *fe=fopen(F_E,"w");
  FILE *fs=fopen(F_ST,"w");
  if(!fe||!fs){ if(fe)fclose(fe); if(fs)fclose(fs); fclose(f); return 0; }

  uint32_t cap = 1u<<18; /* 262k slots */
  Edge *T=(Edge*)calloc(cap,sizeof(Edge));
  if(!T) die("oom edges");

  char line[LINE];
  long rows=0;
  int last=-1,cur=-1,streak=0;

  while(fgets(line,sizeof(line),f)){
    if(!get_i32(line,"\"zone\"",&cur)) continue;
    rows++;
    if(last!=-1){
      edge_inc(T,cap,(uint32_t)last,(uint32_t)cur);
      if(cur==last) streak++;
      else{ fprintf(fs,"STREAK\tzone=%d\tlen=%d\n",last,streak); streak=1; }
    }else streak=1;
    last=cur;
  }
  fclose(f);
  if(last!=-1) fprintf(fs,"STREAK\tzone=%d\tlen=%d\n",last,streak);

  for(uint32_t i=0;i<cap;i++){
    if(T[i].used) fprintf(fe,"EDGE\t%u->%u\t%u\n",T[i].a,T[i].b,T[i].cnt);
  }

  fclose(fe); fclose(fs);
  free(T);

  fprintf(stderr,"[markov] OK rows=%ld -> %s + %s\n",rows,F_E,F_ST);
  return (rows>0);
}

/* ---------------- main ---------------- */

int main(int argc, char **argv){
  const char *FZ   = (argc>1)?argv[1]:"omega_zone_metrics.jsonl";
  const char *FM   = (argc>2)?argv[2]:"omega_msgs.jsonl";
  const char *FMET = (argc>3)?argv[3]:"omega_metrics_v3.jsonl";

  const char *FT   = "zone_timeline.txt";

  const char *F_MAP  = "omega_zone_conv_map.jsonl";
  const char *F_RANK = "omega_zone_conv_rank.txt";
  const char *F_X    = "zone_x_metrics.txt";
  const char *F_S    = "zone_stats.txt";
  const char *F_E    = "zone_markov_edges.txt";
  const char *F_ST   = "zone_conv_streaks.txt";

  long zsz=fsize_bytes(FZ), msz=fsize_bytes(FM), ksz=fsize_bytes(FMET);
  if(zsz<0) die("open zone_metrics");
  if(msz<0) die("open msgs");
  if(ksz<0) die("open metrics");

  fprintf(stderr,"[sanity] %s %ldB\n",FZ,zsz);
  fprintf(stderr,"[sanity] %s %ldB\n",FM,msz);
  fprintf(stderr,"[sanity] %s %ldB\n",FMET,ksz);

  Zone *zones=NULL; int zn=0; int maxzone=-1;
  if(!load_zones(FZ,&zones,&zn,&maxzone)){
    fprintf(stderr,"[zones] FAIL: nao carregou zones (precisa zone/off/zbytes)\n");
    return 2;
  }
  int zcap = (maxzone>=0 && maxzone<ZMAX) ? (maxzone+1) : ZMAX;
  fprintf(stderr,"[zones] OK zn=%d maxzone=%d zcap=%d\n",zn,maxzone,zcap);

  long rows=0,miss=0,bad=0;
  int j = join_by_offset(FM,F_MAP,zones,zn,msz,&rows,&miss,&bad);
  fprintf(stderr,"[join_offset] rows=%ld miss=%ld bad=%ld code=%d\n",rows,miss,bad,j);

  if(j==2 || j==3){
    fprintf(stderr,"[fallback] tentando timeline: %s\n",FT);
    ZRange *rg=NULL; int rn=0; int tmax=-1;
    if(load_timeline_ranges(FT,&rg,&rn,&tmax)){
      long r2=0,m2=0,b2=0;
      if(join_by_timeline(FM,F_MAP,rg,rn,&r2,&m2,&b2)){
        fprintf(stderr,"[join_timeline] OK rows=%ld miss=%ld bad=%ld rn=%d\n",r2,m2,b2,rn);
        rows=r2;
      }else{
        fprintf(stderr,"[join_timeline] FAIL rows=0\n");
      }
      free(rg);
    }else{
      fprintf(stderr,"[fallback] FAIL: nao conseguiu ler %s\n",FT);
    }
  }

  free(zones);

  if(rows==0){
    fprintf(stderr,
      "[FAIL] join deu 0.\n"
      "Causas:\n"
      " - omega_msgs.jsonl sem \"conv_i\" (miss alto)\n"
      " - offsets de zone_metrics nao pertencem ao omega_msgs\n"
      " - zone_timeline.txt ausente/incompativel\n"
    );
    return 3;
  }

  if(!rank_zones(F_MAP,F_RANK,30,zcap)){
    fprintf(stderr,"[rank] FAIL\n");
    return 4;
  }

  MRow *mv=NULL; int mn=0;
  if(!load_metrics(FMET,&mv,&mn)){
    fprintf(stderr,"[metrics] FAIL: metrics_v3 vazio?\n");
    return 5;
  }
  fprintf(stderr,"[metrics] OK mn=%d\n",mn);

  if(!join_zone_metrics(F_MAP,F_X,F_S,mv,mn,zcap)){
    fprintf(stderr,"[metrics_join] WARN rows=0 (conv_i mismatch)\n");
  }
  free(mv);

  if(!markov_streaks(F_MAP,F_E,F_ST)){
    fprintf(stderr,"[markov] WARN rows=0\n");
  }

  fprintf(stderr,"== DONE ==\n");
  return 0;
}
