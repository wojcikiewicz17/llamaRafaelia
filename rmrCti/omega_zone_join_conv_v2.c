#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define LINE_BUF 8192
#define INIT_CAP 512

typedef struct {
    int  zone;
    long off;
    long zbytes;
} Zone;

/* ---------- parsing mínimo ---------- */

static int get_long(const char *s, const char *key, long *out){
    const char *p = strstr(s, key);
    if(!p) return 0;
    p = strchr(p, ':');
    if(!p) return 0;
    *out = strtol(p+1, NULL, 10);
    return 1;
}

static int get_int(const char *s, const char *key, int *out){
    long v;
    if(!get_long(s, key, &v)) return 0;
    *out = (int)v;
    return 1;
}

/* ---------- ordenação ---------- */

static int cmp_zone(const void *a, const void *b){
    const Zone *x = a;
    const Zone *y = b;
    if(x->off < y->off) return -1;
    if(x->off > y->off) return  1;
    return 0;
}

/* ---------- main ---------- */

int main(void){
    const char *F_ZONES = "omega_zone_metrics.jsonl";
    const char *F_MSGS  = "omega_msgs.jsonl";
    const char *F_OUT   = "omega_zone_conv_map.jsonl";

    FILE *fz = fopen(F_ZONES, "r");
    FILE *fm = fopen(F_MSGS,  "r");
    FILE *fo = fopen(F_OUT,   "w");

    if(!fz || !fm || !fo){
        fprintf(stderr,"[ERR] arquivos ausentes\n");
        return 1;
    }

    /* ---- carregar zonas ---- */

    size_t zn = 0, zcap = INIT_CAP;
    Zone *zones = malloc(zcap * sizeof(Zone));
    if(!zones) return 2;

    char line[LINE_BUF];

    while(fgets(line, sizeof(line), fz)){
        Zone z = {0};
        if(!get_int(line, "\"zone\"", &z.zone)) continue;
        if(!get_long(line, "\"off\"", &z.off)) continue;
        get_long(line, "\"zbytes\"", &z.zbytes);

        if(zn >= zcap){
            zcap <<= 1;
            zones = realloc(zones, zcap * sizeof(Zone));
            if(!zones) return 3;
        }
        zones[zn++] = z;
    }
    fclose(fz);

    /* ---- ordenar por offset ---- */

    qsort(zones, zn, sizeof(Zone), cmp_zone);

    /* ---- join mensagens ---- */

    while(fgets(line, sizeof(line), fm)){
        int  conv_i;
        long msg_off;

        if(!get_int(line, "\"conv_i\"", &conv_i)) continue;
        if(!get_long(line, "\"off\"",    &msg_off)) continue;

        /* busca binária */
        size_t lo = 0, hi = zn;
        while(lo < hi){
            size_t mid = (lo + hi) >> 1;
            long start = zones[mid].off;
            long end   = start + zones[mid].zbytes;

            if(msg_off >= start && msg_off < end){
                fprintf(fo,
                    "{\"zone\":%d,\"conv_i\":%d,\"msg_off\":%ld}\n",
                    zones[mid].zone, conv_i, msg_off
                );
                break;
            }
            if(msg_off < start) hi = mid;
            else lo = mid + 1;
        }
    }

    fclose(fm);
    fclose(fo);
    free(zones);

    fprintf(stderr,
        "[omega_zone_join_conv_v2] OK -> %s (zones=%zu)\n",
        F_OUT, zn
    );
    return 0;
}
