#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <ncurses.h>

static const char *F_HOT   = "omega_hot_rank.txt";
static const char *F_TH    = "omega_threads_rank.txt";
static const char *F_MODEL = "omega_cross_hot_rank.txt";
static const char *F_MSGS  = "omega_msgs.jsonl";
static const char *F_VMAP  = "omega_value_map.jsonl";

static int file_exists(const char *p){
    FILE *f=fopen(p,"rb");
    if(!f) return 0;
    fclose(f);
    return 1;
}

static void trim_newline(char *s){
    if(!s) return;
    size_t n=strlen(s);
    while(n && (s[n-1]=='\n' || s[n-1]=='\r')) s[--n]=0;
}

static void center_line(int y, const char *s){
    int rows, cols; getmaxyx(stdscr, rows, cols);
    int x = (cols - (int)strlen(s))/2;
    if(x<0) x=0;
    mvprintw(y, x, "%s", s);
}

static void draw_header(const char *title){
    int rows, cols; getmaxyx(stdscr, rows, cols);
    attron(A_REVERSE);
    mvhline(0,0,' ',cols);
    mvprintw(0,2,"OMEGA_NAV");
    mvprintw(0,13,"| %s", title?title:"");
    attroff(A_REVERSE);
}

static void draw_footer(const char *hint){
    int rows, cols; getmaxyx(stdscr, rows, cols);
    attron(A_REVERSE);
    mvhline(rows-1,0,' ',cols);
    mvprintw(rows-1,2,"%s", hint?hint:"");
    attroff(A_REVERSE);
}

static char* slurp_cmd(const char *cmd, size_t *out_len){
    *out_len = 0;
    FILE *p = popen(cmd, "r");
    if(!p) return NULL;

    size_t cap = 1<<16;
    size_t len = 0;
    char *buf = (char*)malloc(cap);
    if(!buf){ pclose(p); return NULL; }

    char tmp[4096];
    while(fgets(tmp, sizeof(tmp), p)){
        size_t n = strlen(tmp);
        if(len + n + 1 > cap){
            cap = (cap*2) + n + 1;
            char *nb = (char*)realloc(buf, cap);
            if(!nb){ free(buf); pclose(p); return NULL; }
            buf = nb;
        }
        memcpy(buf+len, tmp, n);
        len += n;
    }
    buf[len] = 0;
    pclose(p);
    *out_len = len;
    return buf;
}

static char* slurp_file(const char *path, size_t *out_len, size_t max_bytes){
    *out_len = 0;
    FILE *f = fopen(path, "rb");
    if(!f) return NULL;

    size_t cap = (max_bytes?max_bytes:(1<<20)) + 1;
    char *buf = (char*)malloc(cap);
    if(!buf){ fclose(f); return NULL; }

    size_t n = fread(buf,1,cap-1,f);
    buf[n]=0;
    fclose(f);
    *out_len = n;
    return buf;
}

static void viewer(const char *title, const char *content){
    int rows, cols; getmaxyx(stdscr, rows, cols);
    int top = 0;

    // split lines pointers (cheap)
    const char *p = content ? content : "";
    size_t Lcap=4096, L=0;
    const char **lines = (const char**)malloc(Lcap * sizeof(char*));
    if(!lines) return;
    lines[L++] = p;

    for(; *p; p++){
        if(*p=='\n'){
            if(L+1 >= Lcap){
                Lcap*=2;
                const char **nl = (const char**)realloc(lines, Lcap*sizeof(char*));
                if(!nl){ free(lines); return; }
                lines = nl;
            }
            lines[L++] = p+1;
        }
    }

    while(1){
        erase();
        draw_header(title);

        int view_h = rows-2;
        int max_show = view_h;

        for(int i=0;i<max_show;i++){
            int li = top + i;
            if(li >= (int)L) break;
            // print a slice of the line
            const char *s = lines[li];
            // find end
            const char *e = strchr(s,'\n');
            int len = e ? (int)(e-s) : (int)strlen(s);
            if(len < 0) len = 0;
            if(len > cols-2) len = cols-2;

            mvaddnstr(1+i, 1, s, len);
        }

        char foot[256];
        snprintf(foot,sizeof(foot),
            "↑↓ scroll | PgUp/PgDn | Home/End | q back   (line %d/%d)",
            top+1, (int)L);
        draw_footer(foot);

        refresh();

        int ch = getch();
        if(ch=='q' || ch=='Q' || ch==27) break;
        else if(ch==KEY_UP){ if(top>0) top--; }
        else if(ch==KEY_DOWN){ if(top < (int)L-1) top++; }
        else if(ch==KEY_NPAGE){ top += (rows-2); if(top>(int)L-1) top=(int)L-1; }
        else if(ch==KEY_PPAGE){ top -= (rows-2); if(top<0) top=0; }
        else if(ch==KEY_HOME){ top=0; }
        else if(ch==KEY_END){ top=(int)L-1; }
    }

    free(lines);
}

static int prompt_input(const char *label, char *out, size_t out_sz){
    int rows, cols; getmaxyx(stdscr, rows, cols);
    erase();
    draw_header("Search");
    mvprintw(2,2,"%s", label);
    mvprintw(4,2,"> ");
    draw_footer("Enter confirma | ESC cancela");
    echo();
    curs_set(1);
    int rc = 0;

    // input line
    int ch = 0;
    size_t i=0;
    memset(out,0,out_sz);

    while(1){
        ch = getch();
        if(ch==27){ rc=0; break; }
        if(ch=='\n' || ch=='\r'){ rc= (i>0); break; }
        if(ch==KEY_BACKSPACE || ch==127 || ch==8){
            if(i>0){
                i--;
                out[i]=0;
                int y=4, x=4;
                mvprintw(y,x,"%*s", cols-5, " ");
                mvprintw(y,x,"%s", out);
                move(y, x+(int)i);
                refresh();
            }
            continue;
        }
        if(isprint(ch)){
            if(i+1 < out_sz){
                out[i++] = (char)ch;
                out[i] = 0;
                mvprintw(4,4,"%s", out);
                refresh();
            }
        }
    }

    noecho();
    curs_set(0);
    return rc;
}

static void show_help(){
    const char *txt =
        "OMEGA_NAV :: atalhos\n"
        "\n"
        "A  Hot convs (densidade)   -> omega_hot_rank.txt\n"
        "B  Threads (profundidade)  -> omega_threads_rank.txt\n"
        "C  Models em hot           -> omega_cross_hot_rank.txt\n"
        "V  Top Valor (C puro)      -> omega_value_top omega_value_map.jsonl\n"
        "S  Search (streaming)      -> omega_search_fast / omega_query_msgs.py / grep\n"
        "H  Help\n"
        "Q  Quit\n"
        "\n"
        "Viewer:\n"
        "  ↑ ↓  scroll\n"
        "  PgUp/PgDn  page\n"
        "  Home/End   inicio/fim\n"
        "  q          voltar\n";
    viewer("Help", txt);
}

static void action_show_file(const char *title, const char *path){
    if(!file_exists(path)){
        char msg[512];
        snprintf(msg,sizeof(msg),
            "Arquivo não encontrado:\n  %s\n\n"
            "Gere/coloque esse arquivo no diretório atual e tente novamente.\n", path);
        viewer(title, msg);
        return;
    }
    size_t n=0;
    // mobile-safe: lê até 1.5MB por tela
    char *buf = slurp_file(path, &n, (size_t)(1.5*1024*1024));
    if(!buf){
        viewer(title, "Falha ao ler arquivo (OOM ou permissão).");
        return;
    }
    viewer(title, buf);
    free(buf);
}

static void action_top_value(){
    // prefer C puro se existir
    if(file_exists("./omega_value_top") && file_exists(F_VMAP)){
        size_t n=0;
        char cmd[512];
        snprintf(cmd,sizeof(cmd), "./omega_value_top %s 50 2>/dev/null", F_VMAP);
        char *buf = slurp_cmd(cmd, &n);
        if(!buf){
            viewer("Top Value", "Falha ao executar omega_value_top.");
            return;
        }
        viewer("Top Value (rank)", buf);
        free(buf);
        return;
    }

    // fallback: mostrar aviso + hint
    viewer("Top Value",
        "omega_value_top ou omega_value_map.jsonl não encontrados.\n\n"
        "Sugestão:\n"
        "  cc -O3 -Wall -Wextra omega_value_top.c -o omega_value_top\n"
        "  ./omega_value_top omega_value_map.jsonl 50\n");
}

static void action_search(){
    char term[256];
    if(!prompt_input("Termo de busca (ex: jwt, fastapi, gpt-4o, rafaelia):", term, sizeof(term))){
        return;
    }

    char cmd[1024];
    if(file_exists("./omega_search_fast")){
        snprintf(cmd,sizeof(cmd),
            "./omega_search_fast --in %s --term %s --limit 50 2>/dev/null",
            F_MSGS, term);
    } else if(file_exists("./omega_query_msgs.py")){
        snprintf(cmd,sizeof(cmd),
            "./omega_query_msgs.py --in %s --term %s --limit 50 2>/dev/null",
            F_MSGS, term);
    } else {
        snprintf(cmd,sizeof(cmd),
            "grep -i %s %s | head -n 50 2>/dev/null",
            term, F_MSGS);
    }

    size_t n=0;
    char *buf = slurp_cmd(cmd, &n);
    if(!buf || n==0){
        if(buf) free(buf);
        viewer("Search", "Sem resultados (ou falha ao executar busca).");
        return;
    }
    viewer("Search Results", buf);
    free(buf);
}

typedef enum {
    MENU_HOT = 0,
    MENU_THREADS,
    MENU_MODELS,
    MENU_VALUE,
    MENU_SEARCH,
    MENU_HELP,
    MENU_QUIT,
    MENU_COUNT
} MenuAction;

static const char *menu_labels[MENU_COUNT] = {
    "Hot convs (densidade)",
    "Threads (profundidade)",
    "Models em hot",
    "Top Valor (C puro)",
    "Search (streaming)",
    "Help",
    "Quit"
};

static void run_menu_action(int idx, int *running){
    switch((MenuAction)idx){
        case MENU_HOT:     action_show_file("Hot convs (densidade)", F_HOT); break;
        case MENU_THREADS: action_show_file("Threads (profundidade)", F_TH); break;
        case MENU_MODELS:  action_show_file("Models em hot", F_MODEL); break;
        case MENU_VALUE:   action_top_value(); break;
        case MENU_SEARCH:  action_search(); break;
        case MENU_HELP:    show_help(); break;
        case MENU_QUIT:    *running = 0; break;
        default: break;
    }
}

int main(int argc, char **argv){
    (void)argc; (void)argv;

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    int sel = 0;
    int running = 1;

    while(running){
        erase();
        draw_header("Menu");
        center_line(2,  "🧭 OMEGA_NAV :: Navegador de Sessões (estilo DOS Shell)");

        int y0 = 4;
        for(int i=0;i<MENU_COUNT;i++){
            int y = y0 + i;
            if(i == sel) attron(A_REVERSE | A_BOLD);
            mvprintw(y, 6, "%c %s", (i==sel)?'>':' ', menu_labels[i]);
            if(i == sel) attroff(A_REVERSE | A_BOLD);
        }

        draw_footer("↑↓ seleciona | Enter executa | A/B/C/V/S/H/Q atalhos");
        refresh();

        int ch = getch();
        if(ch==KEY_UP || ch=='k' || ch=='K'){
            sel = (sel + MENU_COUNT - 1) % MENU_COUNT;
            continue;
        } else if(ch==KEY_DOWN || ch=='j' || ch=='J'){
            sel = (sel + 1) % MENU_COUNT;
            continue;
        } else if(ch=='\n' || ch=='\r' || ch==KEY_ENTER){
            run_menu_action(sel, &running);
            continue;
        }

        if(ch=='a' || ch=='A') run_menu_action(MENU_HOT, &running);
        else if(ch=='b' || ch=='B') run_menu_action(MENU_THREADS, &running);
        else if(ch=='c' || ch=='C') run_menu_action(MENU_MODELS, &running);
        else if(ch=='v' || ch=='V') run_menu_action(MENU_VALUE, &running);
        else if(ch=='s' || ch=='S') run_menu_action(MENU_SEARCH, &running);
        else if(ch=='h' || ch=='H') run_menu_action(MENU_HELP, &running);
        else if(ch=='q' || ch=='Q' || ch==27) run_menu_action(MENU_QUIT, &running);
    }

    endwin();
    return 0;
}
