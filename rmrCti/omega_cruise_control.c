#define _GNU_SOURCE
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>

typedef uint64_t u64;

/* -------- tuning -------- */
#define CHUNK_MIN   (1ULL<<20)   /* 1 MB */
#define CHUNK_MAX   (32ULL<<20)  /* 32 MB */
#define CHUNK_INIT  (8ULL<<20)
#define STABLE_HITS 3

/* -------- utils -------- */
static inline u64 ns_now(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (u64)ts.tv_sec*1000000000ULL + ts.tv_nsec;
}

/* -------- controller state -------- */
struct cruise {
    u64 chunk;
    int stable;
};

static void adapt(struct cruise *c, u64 t_read, u64 t_comp) {
    /* compute slower than IO → shrink */
    if (t_comp > t_read * 12 / 10) {
        if (c->chunk > CHUNK_MIN) c->chunk >>= 1;
        c->stable = 0;
        return;
    }
    /* IO slower than compute → grow */
    if (t_read > t_comp * 12 / 10) {
        if (c->chunk < CHUNK_MAX) c->chunk <<= 1;
        c->stable = 0;
        return;
    }
    /* stable */
    if (++c->stable >= STABLE_HITS) {
        if (c->chunk < CHUNK_MAX) c->chunk += (c->chunk >> 2); /* +25% */
        c->stable = 0;
    }
}

/* -------- placeholder for NEON engine -------- */
/* replace this with your omega_neon / zone engine */
static void process_block(const unsigned char *p, u64 n) {
    volatile u64 s = 0;
    for (u64 i = 0; i < n; i++) s += p[i];
}

/* -------- main loop -------- */
int main(int argc, char **argv) {
    if (argc < 2) {
        write(2, "usage: omega_cc <file>\n", 24);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) return 1;

    static unsigned char buf[32*1024*1024]; /* max chunk */
    struct cruise cc = { CHUNK_INIT, 0 };

    off_t off = 0;

    while (1) {
        u64 t0 = ns_now();
        ssize_t r = pread(fd, buf, cc.chunk, off);
        u64 t1 = ns_now();
        if (r <= 0) break;

        process_block(buf, (u64)r);
        u64 t2 = ns_now();

        adapt(&cc, t1 - t0, t2 - t1);

        /* live status */
        dprintf(2,
            "\r[cc] off=%llu  chunk=%lluMB  read=%lluµs  comp=%lluµs   ",
            (unsigned long long)off,
            (unsigned long long)(cc.chunk >> 20),
            (unsigned long long)((t1-t0)/1000),
            (unsigned long long)((t2-t1)/1000)
        );

        off += r;
    }

    write(2, "\n[cc] done\n", 11);
    close(fd);
    return 0;
}
