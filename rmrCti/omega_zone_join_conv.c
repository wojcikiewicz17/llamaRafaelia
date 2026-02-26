#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/*
 * OMEGA :: ZONE ↔ CONV JOINER (V4)
 * Objetivo: mapear mensagens (offset) → zona → conversa
 * Complexidade: O(n log n)
 * Dependências: libc apenas
 */

#define LINE_BUF 16384
#define INIT_CAP 512

typedef struct {
    int   zone;
    long  off;
    long  zbytes;
} Zone;

/* ---------- util ---------- */

static int extract_long(const char *s, const char *key, long *out){
    const char *p = strstr(s, key);
    if(!p) return 0;
    p = strchr(p, ':');
    if(!p) return 0;
    *out = strtol(p+1, NULL, 10);
    return 1;
}

static int extract_int(const char *s, const char *key, int *out){
    long v;
    if(!extract_long(s, key, &v)) return 0;
    *out = (int)v;
    return 1;
}

/* ---------- sort ---------- */

static int cmp_zone(const void *a, const void *b){
    const Zone *x = (const Zone*)a;
    const Zone *y = (const Zone*)b;
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
        fprintf(stderr, "[FATAL] erro abrindo arquivos\n");
        return 1;
    }

    /* carregar zonas */
    size_t zn = 0, zcap = INIT_CAP;
    Zone *zones = malloc(zcap * sizeof(Zone));
    if(!zones) return 1;

    char buf[LINE_BUF];
    while(fgets(buf, sizeof(buf), fz)){
        Zone z = {0};
        if(!extract_int(buf, "\"zone\"", &z.zone)) continue;
        extract_long(buf, "\"off\"",    &z.off);
        extract_long(buf, "\"zbytes\"", &z.zbytes);

        if(zn == zcap){
            zcap <<= 1;
            zones = realloc(zones, zcap * sizeof(Zone));
            if(!zones) return 1;
        }
        zones[zn++] = z;
    }
    fclose(fz);

    /* ordenar por offset */
    qsort(zones, zn, sizeof(Zone), cmp_zone);

    /* join streaming */
    while(fgets(buf, sizeof(buf), fm)){
        int  conv_i;
        long msg_off;

        if(!extract_int(buf, "\"conv_i\"", &conv_i)) continue;
        if(!extract_long(buf, "\"off\"",    &msg_off)) continue;

        /* busca binária manual */
        size_t lo = 0, hi = zn;
        while(lo < hi){
            size_t mid = lo + ((hi - lo) >> 1);
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
        "[omega_zone_join_conv] OK -> %s (zones=%zu)\n",
        F_OUT, zn
    );
    return 0;
}
