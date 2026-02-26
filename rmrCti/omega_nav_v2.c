#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <ncurses.h>

/* =========================
   CONFIG / PATHS
   ========================= */
static const char *F_HOT   = "omega_hot_rank.txt";
static const char *F_TH    = "omega_threads_rank.txt";
static const char *F_MODEL = "omega_cross_hot_rank.txt";
static const char *F_MSGS  = "omega_msgs.jsonl";

/* =========================
   MAP VIEW
   ========================= */
typedef struct {
    char *ptr;
    size_t size;
    const char **lines;
    size_t line_count;
} MapView;

/* =========================
   UTILS
   ========================= */
static int file_exists(const char *p){
    return access(p, R_OK) == 0;
}

static int extract_conv_i(const char *line){
    const char *p = strstr(line, "\"conv_i\"");
    if(!p) return -1;
    p = strchr(p, ':');
    if(!p) return -1;
    return atoi(p+1);
}

/* =========================
   MAP FILE (mmap)
   ========================= */
static MapView* map_file(const char *path){
    int fd = open(path, O_RDONLY);
    if(fd < 0) return NULL;

    struct stat st;
    if(fstat(fd, &st) < 0 || st.st_size == 0){
        close(fd);
        return NULL;
    }

    MapView *mv = calloc(1,sizeof(MapView));
    mv->size = st.st_size;
    mv->ptr = mmap(NULL, mv->size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    if(mv->ptr == MAP_FAILED){
        free(mv);
        return NULL;
    }

    size_t cap = 1024;
    mv->lines = malloc(cap * sizeof(char*));
    mv->line_count = 0;
    mv->lines[mv->line_count++] = mv->ptr;

    for(size_t i=0;i<mv->size-1;i++){
        if(mv->ptr[i]=='\n'){
            if(mv->line_count >= cap){
                cap*=2;
                mv->lines = realloc(mv->lines, cap*sizeof(char*));
            }
            mv->lines[mv->line_count++] = &mv->ptr[i+1];
        }
    }
    return mv;
}

static void unmap_file(MapView *mv){
    if(!mv) return;
    munmap(mv->ptr, mv->size);
    free(mv->lines);
    free(mv);
}

/* =========================
   MAP CONVERSATION (conv_i)
   ========================= */
static MapView* map_conv(int conv_i){
    FILE *in = fopen(F_MSGS,"r");
    if(!in) return NULL;

    FILE *tmp = tmpfile();
    if(!tmp){ fclose(in); return NULL; }

    char buf[8192];
    while(fgets(buf,sizeof(buf),in)){
        int ci = extract_conv_i(buf);
        if(ci == conv_i)
            fputs(buf,tmp);
    }
    fclose(in);
    rewind(tmp);

    int fd = fileno(tmp);
    struct stat st;
    fstat(fd,&st);
    if(st.st_size == 0){
        fclose(tmp);
        return NULL;
    }

    MapView *mv = calloc(1,sizeof(MapView));
    mv->size = st.st_size;
    mv->ptr = mmap(NULL,mv->size,PROT_READ,MAP_PRIVATE,fd,0);
    fclose(tmp);

    size_t cap=512;
    mv->lines = malloc(cap*sizeof(char*));
    mv->line_count=0;
    mv->lines[mv->line_count++] = mv->ptr;

    for(size_t i=0;i<mv->size-1;i++){
        if(mv->ptr[i]=='\n'){
            if(mv->line_count>=cap){
                cap*=2;
                mv->lines=realloc(mv->lines,cap*sizeof(char*));
            }
            mv->lines[mv->line_count++] = &mv->ptr[i+1];
        }
    }
    return mv;
}

/* =========================
   MAP SEARCH (nativo)
   ========================= */
static MapView* map_search(const char *term){
    FILE *in = fopen(F_MSGS,"r");
    if(!in) return NULL;

    FILE *tmp = tmpfile();
    if(!tmp){ fclose(in); return NULL; }

    char buf[8192];
    while(fgets(buf,sizeof(buf),in)){
        if(strcasestr(buf,term))
            fputs(buf,tmp);
    }
    fclose(in);
    rewind(tmp);

    int fd = fileno(tmp);
    struct stat st;
    fstat(fd,&st);
    if(st.st_size==0){
        fclose(tmp);
        return NULL;
    }

    MapView *mv = calloc(1,sizeof(MapView));
    mv->size = st.st_size;
    mv->ptr = mmap(NULL,mv->size,PROT_READ,MAP_PRIVATE,fd,0);
    fclose(tmp);

    size_t cap=512;
    mv->lines = malloc(cap*sizeof(char*));
    mv->line_count=0;
    mv->lines[mv->line_count++] = mv->ptr;

    for(size_t i=0;i<mv->size-1;i++){
        if(mv->ptr[i]=='\n'){
            if(mv->line_count>=cap){
                cap*=2;
                mv->lines=realloc(mv->lines,cap*sizeof(char*));
            }
            mv->lines[mv->line_count++] = &mv->ptr[i+1];
        }
    }
    return mv;
}

/* =========================
   VIEWER ÚNICO
   ========================= */
static void viewer(const char *title, MapView *mv, int allow_drill){
    int rows,cols;
    int top=0, cur=0;

    while(1){
        getmaxyx(stdscr,rows,cols);
        erase();

        attron(A_REVERSE);
        mvhline(0,0,' ',cols);
        mvprintw(0,2,"%s",title);
        attroff(A_REVERSE);

        int view_h = rows-2;
        for(int i=0;i<view_h;i++){
            int li = top+i;
            if(li >= (int)mv->line_count) break;
            const char *s = mv->lines[li];
            const char *e = strchr(s,'\n');
            int len = e ? (int)(e-s) : strlen(s);
            if(len > cols-4) len = cols-4;
            if(li==cur) attron(A_REVERSE);
            mvaddnstr(i+1,2,s,len);
            if(li==cur) attroff(A_REVERSE);
        }

        mvhline(rows-1,0,' ',cols);
        mvprintw(rows-1,2,"↑↓ scroll  ENTER drilldown  q back");
        refresh();

        int ch = getch();
        if(ch=='q' || ch=='Q') break;
        else if(ch==KEY_UP){
            if(cur>0) cur--;
            if(cur<top) top--;
        } else if(ch==KEY_DOWN){
            if(cur<(int)mv->line_count-1) cur++;
            if(cur>=top+view_h) top++;
        } else if(ch=='\n' && allow_drill){
            int ci = extract_conv_i(mv->lines[cur]);
            if(ci>=0){
                MapView *cv = map_conv(ci);
                if(cv){
                    viewer("Conversation",cv,0);
                    unmap_file(cv);
                }
            }
        }
    }
}

/* =========================
   PROMPT SEARCH
   ========================= */
static int prompt(char *out, size_t sz){
    echo();
    curs_set(1);
    mvprintw(5,5,"Search term: ");
    getnstr(out,sz-1);
    noecho();
    curs_set(0);
    return strlen(out)>0;
}

/* =========================
   MAIN MENU
   ========================= */
int main(){
    initscr();
    cbreak();
    noecho();
    keypad(stdscr,TRUE);
    curs_set(0);

    while(1){
        erase();
        mvprintw(3,5,"OMEGA_NAV V2");
        mvprintw(5,5,"A - Hot conversations");
        mvprintw(6,5,"B - Threads rank");
        mvprintw(7,5,"C - Models hot");
        mvprintw(8,5,"S - Search");
        mvprintw(10,5,"Q - Quit");
        refresh();

        int ch = getch();
        if(ch=='q'||ch=='Q') break;

        if(ch=='A'){
            MapView *mv = map_file(F_HOT);
            if(mv){ viewer("Hot Convs",mv,1); unmap_file(mv); }
        } else if(ch=='B'){
            MapView *mv = map_file(F_TH);
            if(mv){ viewer("Threads",mv,1); unmap_file(mv); }
        } else if(ch=='C'){
            MapView *mv = map_file(F_MODEL);
            if(mv){ viewer("Models Hot",mv,1); unmap_file(mv); }
        } else if(ch=='S'){
            char term[256]={0};
            if(prompt(term,sizeof(term))){
                MapView *mv = map_search(term);
                if(mv){ viewer("Search Results",mv,1); unmap_file(mv); }
            }
        }
    }

    endwin();
    return 0;
}
