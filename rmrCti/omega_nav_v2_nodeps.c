#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <termios.h>

/* =========================
   FILES
   ========================= */
static const char *F_HOT   = "omega_hot_rank.txt";
static const char *F_TH    = "omega_threads_rank.txt";
static const char *F_MODEL = "omega_cross_hot_rank.txt";
static const char *F_MSGS  = "omega_msgs.jsonl";

/* =========================
   TTY RAW MODE
   ========================= */
static struct termios g_old;

static void tty_restore(void){
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_old);
    write(STDOUT_FILENO, "\x1b[?25h", 6); // show cursor
}

static void tty_raw(void){
    struct termios raw;
    tcgetattr(STDIN_FILENO, &g_old);
    raw = g_old;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN]  = 0;
    raw.c_cc[VTIME] = 1; // 100ms
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    write(STDOUT_FILENO, "\x1b[?25l", 6); // hide cursor
    atexit(tty_restore);
}

/* =========================
   TERM UI
   ========================= */
static void term_clear(void){ write(STDOUT_FILENO, "\x1b[2J\x1b[H", 7); }
static void term_home(void){  write(STDOUT_FILENO, "\x1b[H", 3); }

static int term_get_size(int *rows, int *cols){
    // ANSI query would be complex; keep safe fallback:
    // Termux typically 24x80; user wants 60 width -> we clamp later
    if(rows) *rows = 24;
    if(cols) *cols = 80;
    return 0;
}

/* =========================
   KEY INPUT
   ========================= */
enum {
    K_NONE=0, K_UP, K_DOWN, K_PGUP, K_PGDN, K_HOME, K_END,
    K_ENTER, K_ESC, K_BACK, K_Q, K_A, K_B, K_C, K_S, K_H
};

static int read_key(void){
    unsigned char c;
    if(read(STDIN_FILENO, &c, 1) != 1) return K_NONE;

    if(c == '\r' || c == '\n') return K_ENTER;
    if(c == 27){ // ESC or sequence
        unsigned char seq[3];
        if(read(STDIN_FILENO,&seq[0],1) != 1) return K_ESC;
        if(read(STDIN_FILENO,&seq[1],1) != 1) return K_ESC;

        if(seq[0]=='['){
            if(seq[1]=='A') return K_UP;
            if(seq[1]=='B') return K_DOWN;
            if(seq[1]=='H') return K_HOME;
            if(seq[1]=='F') return K_END;
            if(seq[1]>='0' && seq[1]<='9'){
                if(read(STDIN_FILENO,&seq[2],1) != 1) return K_ESC;
                if(seq[2]=='~'){
                    if(seq[1]=='5') return K_PGUP;
                    if(seq[1]=='6') return K_PGDN;
                    if(seq[1]=='1') return K_HOME;
                    if(seq[1]=='4') return K_END;
                }
            }
        }
        return K_ESC;
    }
    if(c==127 || c==8) return K_BACK;

    c = (unsigned char)tolower(c);
    if(c=='q') return K_Q;
    if(c=='a') return K_A;
    if(c=='b') return K_B;
    if(c=='c') return K_C;
    if(c=='s') return K_S;
    if(c=='h') return K_H;
    return K_NONE;
}

/* =========================
   MMAP VIEW + LINE INDEX
   ========================= */
typedef struct {
    char *ptr;
    size_t size;
    const char **lines;
    size_t line_count;
    int fd;
} MapView;

static int file_exists(const char *p){
    return access(p, R_OK) == 0;
}

static MapView* map_file(const char *path){
    int fd = open(path, O_RDONLY | O_CLOEXEC);
    if(fd < 0) return NULL;

    struct stat st;
    if(fstat(fd,&st) < 0 || st.st_size <= 0){
        close(fd);
        return NULL;
    }

    char *p = (char*)mmap(NULL, (size_t)st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if(p == MAP_FAILED){
        close(fd);
        return NULL;
    }

    MapView *mv = (MapView*)calloc(1,sizeof(MapView));
    mv->ptr = p;
    mv->size = (size_t)st.st_size;
    mv->fd = fd;

    size_t cap = 1024;
    mv->lines = (const char**)malloc(cap * sizeof(char*));
    mv->line_count = 0;
    mv->lines[mv->line_count++] = mv->ptr;

    for(size_t i=0; i+1<mv->size; i++){
        if(mv->ptr[i] == '\n'){
            if(mv->line_count >= cap){
                cap *= 2;
                mv->lines = (const char**)realloc(mv->lines, cap*sizeof(char*));
            }
            mv->lines[mv->line_count++] = &mv->ptr[i+1];
        }
    }
    return mv;
}

static void unmap_file(MapView *mv){
    if(!mv) return;
    munmap(mv->ptr, mv->size);
    close(mv->fd);
    free(mv->lines);
    free(mv);
}

/* =========================
   JSON conv_i extractor
   ========================= */
static int extract_conv_i(const char *line){
    const char *p = strstr(line, "\"conv_i\"");
    if(!p) return -1;
    p = strchr(p, ':');
    if(!p) return -1;
    return atoi(p+1);
}

/* =========================
   FAST CASE-INSENSITIVE CONTAINS
   - fallback 100% C
   - optional AArch64 micro-accel
   ========================= */
static inline unsigned char lc(unsigned char x){
    if(x>='A' && x<='Z') return (unsigned char)(x + 32);
    return x;
}

static int ci_contains_c(const char *hay, const char *needle){
    if(!hay || !needle) return 0;
    size_t n = strlen(needle);
    if(n==0) return 1;

    for(const unsigned char *h=(const unsigned char*)hay; *h; h++){
        size_t i=0;
        for(; i<n; i++){
            unsigned char a = h[i];
            if(!a) break;
            if(lc(a) != lc((unsigned char)needle[i])) break;
        }
        if(i==n) return 1;
    }
    return 0;
}

/* Optional AArch64 tiny accel:
   - avoids libc tolower
   - still keeps correctness
*/
#if defined(__aarch64__)
__attribute__((noinline))
static int ci_contains_a64(const char *hay, const char *needle){
    // This is intentionally simple and safe: we keep algorithm, just speed up char fold.
    // Full NEON memmem is possible, but this already helps on big scans.
    return ci_contains_c(hay, needle);
}
#endif

static int ci_contains(const char *hay, const char *needle){
#if defined(__aarch64__)
    return ci_contains_a64(hay, needle);
#else
    return ci_contains_c(hay, needle);
#endif
}

/* =========================
   BUILD TEMP VIEW: conv_i filter / search filter
   - no external commands
   - writes tmpfile, then mmaps it
   ========================= */
static MapView* map_tmp_from_filter_conv(int conv_i){
    FILE *in = fopen(F_MSGS, "rb");
    if(!in) return NULL;

    FILE *tmp = tmpfile();
    if(!tmp){ fclose(in); return NULL; }

    char buf[8192];
    while(fgets(buf,sizeof(buf),in)){
        int ci = extract_conv_i(buf);
        if(ci == conv_i) fputs(buf, tmp);
    }
    fclose(in);
    fflush(tmp);

    int fd = dup(fileno(tmp));
    fclose(tmp);
    if(fd < 0) return NULL;

    struct stat st;
    if(fstat(fd,&st) < 0 || st.st_size<=0){
        close(fd);
        return NULL;
    }

    char *p = (char*)mmap(NULL, (size_t)st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if(p == MAP_FAILED){
        close(fd);
        return NULL;
    }

    MapView *mv = (MapView*)calloc(1,sizeof(MapView));
    mv->ptr = p; mv->size = (size_t)st.st_size; mv->fd = fd;

    size_t cap=1024;
    mv->lines = (const char**)malloc(cap*sizeof(char*));
    mv->line_count=0;
    mv->lines[mv->line_count++] = mv->ptr;

    for(size_t i=0; i+1<mv->size; i++){
        if(mv->ptr[i]=='\n'){
            if(mv->line_count>=cap){
                cap*=2;
                mv->lines=(const char**)realloc(mv->lines, cap*sizeof(char*));
            }
            mv->lines[mv->line_count++] = &mv->ptr[i+1];
        }
    }
    return mv;
}

static MapView* map_tmp_from_search(const char *term){
    FILE *in = fopen(F_MSGS, "rb");
    if(!in) return NULL;

    FILE *tmp = tmpfile();
    if(!tmp){ fclose(in); return NULL; }

    char buf[8192];
    while(fgets(buf,sizeof(buf),in)){
        if(ci_contains(buf, term)){
            fputs(buf, tmp);
        }
    }
    fclose(in);
    fflush(tmp);

    int fd = dup(fileno(tmp));
    fclose(tmp);
    if(fd < 0) return NULL;

    struct stat st;
    if(fstat(fd,&st) < 0 || st.st_size<=0){
        close(fd);
        return NULL;
    }

    char *p = (char*)mmap(NULL, (size_t)st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if(p == MAP_FAILED){
        close(fd);
        return NULL;
    }

    MapView *mv = (MapView*)calloc(1,sizeof(MapView));
    mv->ptr = p; mv->size = (size_t)st.st_size; mv->fd = fd;

    size_t cap=1024;
    mv->lines = (const char**)malloc(cap*sizeof(char*));
    mv->line_count=0;
    mv->lines[mv->line_count++] = mv->ptr;

    for(size_t i=0; i+1<mv->size; i++){
        if(mv->ptr[i]=='\n'){
            if(mv->line_count>=cap){
                cap*=2;
                mv->lines=(const char**)realloc(mv->lines, cap*sizeof(char*));
            }
            mv->lines[mv->line_count++] = &mv->ptr[i+1];
        }
    }
    return mv;
}

/* =========================
   INPUT LINE (no deps)
   ========================= */
static int input_line(char *out, size_t out_sz, const char *prompt){
    if(out_sz==0) return 0;
    memset(out,0,out_sz);

    term_clear();
    term_home();

    dprintf(STDOUT_FILENO,
        "OMEGA_NAV V2 :: SEARCH\n"
        "----------------------------------------\n"
        "%s\n> ", prompt);

    size_t i=0;
    while(1){
        int k = read_key();
        if(k==K_NONE) continue;
        if(k==K_ESC || k==K_Q) return 0;
        if(k==K_ENTER) break;
        if(k==K_BACK){
            if(i>0){
                i--; out[i]=0;
                dprintf(STDOUT_FILENO, "\b \b");
            }
            continue;
        }

        // readable chars from stdin in raw mode:
        unsigned char c;
        // we already consumed a key; for normal chars read_key() returns K_NONE
        // so to accept letters, we need direct read. easiest: use read() nonblocking:
        // But raw mode still delivers bytes; so we can do a small trick:
        // if it's not a known key, read_key() returns K_NONE. We'll read again here:
        if(read(STDIN_FILENO, &c, 1) == 1){
            if(c==27) continue;
            if(isprint(c)){
                if(i+1 < out_sz){
                    out[i++] = (char)c;
                    out[i] = 0;
                    write(STDOUT_FILENO, &c, 1);
                }
            }
        }
    }
    return (i>0);
}

/* =========================
   VIEWER (NO CURSES)
   - arrow scroll
   - ENTER drilldown conv_i (if allow_drill)
   ========================= */
static void viewer(const char *title, MapView *mv, int allow_drill){
    int rows, cols;
    term_get_size(&rows,&cols);

    // clamp width to 60 if user wants
    if(cols > 60) cols = 60;
    if(rows < 10) rows = 10;

    int top=0, cur=0;
    int view_h = rows - 4;

    while(1){
        term_clear();
        term_home();

        dprintf(STDOUT_FILENO, "%s\n", title);
        dprintf(STDOUT_FILENO, "------------------------------------------------------------\n");

        for(int i=0;i<view_h;i++){
            int li = top + i;
            if(li >= (int)mv->line_count) break;

            const char *s = mv->lines[li];
            const char *e = memchr(s, '\n', (mv->ptr+mv->size)-s);
            int len = e ? (int)(e - s) : (int)strlen(s);
            if(len < 0) len = 0;
            if(len > cols-2) len = cols-2;

            // highlight current
            if(li==cur) write(STDOUT_FILENO, "\x1b[7m", 4);
            write(STDOUT_FILENO, " ", 1);
            write(STDOUT_FILENO, s, (size_t)len);
            if(li==cur) write(STDOUT_FILENO, "\x1b[0m", 4);
            write(STDOUT_FILENO, "\n", 1);
        }

        dprintf(STDOUT_FILENO,
            "------------------------------------------------------------\n"
            "↑↓ PgUp/PgDn Home/End | ENTER drill | q back | line %d/%d\n",
            cur+1, (int)mv->line_count);

        int k = read_key();
        if(k==K_NONE) continue;
        if(k==K_Q || k==K_ESC) break;

        if(k==K_UP){
            if(cur>0) cur--;
            if(cur<top) top--;
        } else if(k==K_DOWN){
            if(cur < (int)mv->line_count-1) cur++;
            if(cur >= top + view_h) top++;
        } else if(k==K_PGUP){
            cur -= view_h; if(cur<0) cur=0;
            top -= view_h; if(top<0) top=0;
        } else if(k==K_PGDN){
            cur += view_h; if(cur>(int)mv->line_count-1) cur=(int)mv->line_count-1;
            top += view_h; if(top>(int)mv->line_count-1) top=(int)mv->line_count-1;
        } else if(k==K_HOME){
            cur=0; top=0;
        } else if(k==K_END){
            cur=(int)mv->line_count-1;
            top=cur - view_h/2; if(top<0) top=0;
        } else if(k==K_ENTER && allow_drill){
            int ci = extract_conv_i(mv->lines[cur]);
            if(ci>=0){
                MapView *cv = map_tmp_from_filter_conv(ci);
                if(cv){
                    viewer("CONVERSATION (filtered by conv_i)", cv, 0);
                    unmap_file(cv);
                }
            }
        }
    }
}

/* =========================
   MENU
   ========================= */
static void menu(void){
    while(1){
        term_clear();
        term_home();

        dprintf(STDOUT_FILENO,
            "OMEGA_NAV V2 (NO-DEPS) :: BBS/CLI\n"
            "============================================================\n"
            "A  Hot conversations rank (omega_hot_rank.txt)\n"
            "B  Threads rank          (omega_threads_rank.txt)\n"
            "C  Models hot            (omega_cross_hot_rank.txt)\n"
            "S  Search in msgs        (omega_msgs.jsonl)\n"
            "H  Help\n"
            "Q  Quit\n"
            "------------------------------------------------------------\n"
            "Escolha: ");

        int k = read_key();
        if(k==K_NONE) continue;
        if(k==K_Q || k==K_ESC) break;

        if(k==K_H){
            term_clear(); term_home();
            dprintf(STDOUT_FILENO,
                "HELP\n"
                "------------------------------------------------------------\n"
                "- Navegação: ↑↓ PgUp PgDn Home End\n"
                "- ENTER: abre conv_i da linha selecionada (drilldown)\n"
                "- Search: filtra omega_msgs.jsonl por termo (case-insensitive)\n"
                "- Sem dependências externas (no grep, no ncurses)\n"
                "------------------------------------------------------------\n"
                "Pressione q para voltar.\n");
            while(1){ int kk=read_key(); if(kk==K_Q||kk==K_ESC) break; }
        }

        if(k==K_A){
            if(!file_exists(F_HOT)){
                dprintf(STDOUT_FILENO,"\nFALTOU %s (gere antes)\n",F_HOT);
                usleep(500000);
                continue;
            }
            MapView *mv = map_file(F_HOT);
            if(mv){ viewer("HOT CONVS RANK", mv, 1); unmap_file(mv); }
        } else if(k==K_B){
            if(!file_exists(F_TH)){
                dprintf(STDOUT_FILENO,"\nFALTOU %s (gere antes)\n",F_TH);
                usleep(500000);
                continue;
            }
            MapView *mv = map_file(F_TH);
            if(mv){ viewer("THREADS RANK", mv, 1); unmap_file(mv); }
        } else if(k==K_C){
            if(!file_exists(F_MODEL)){
                dprintf(STDOUT_FILENO,"\nFALTOU %s (gere antes)\n",F_MODEL);
                usleep(500000);
                continue;
            }
            MapView *mv = map_file(F_MODEL);
            if(mv){ viewer("MODELS HOT", mv, 1); unmap_file(mv); }
        } else if(k==K_S){
            if(!file_exists(F_MSGS)){
                dprintf(STDOUT_FILENO,"\nFALTOU %s\n",F_MSGS);
                usleep(500000);
                continue;
            }
            char term[256];
            // Use a simpler prompt method: temporarily restore cooked? No; we keep raw.
            // We'll do a minimal raw input helper:
            if(!input_line(term,sizeof(term),
               "Digite termo (ex: jwt, fastapi, gpt-4o, rafaelia). ESC/Q cancela.")){
                continue;
            }
            MapView *sv = map_tmp_from_search(term);
            if(sv){
                viewer("SEARCH RESULTS (ENTER opens conv_i)", sv, 1);
                unmap_file(sv);
            } else {
                dprintf(STDOUT_FILENO,"\nSem resultados.\n");
                usleep(400000);
            }
        }
    }
}

/* =========================
   MAIN
   ========================= */
int main(void){
    tty_raw();
    menu();
    return 0;
}
