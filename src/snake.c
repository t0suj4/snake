#include <ncurses.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#ifdef __STDC_NO_THREADS__
#include "c11threads.h"
#else
#include <threads.h>
#endif
#include <signal.h>

#ifdef __STDC_NO_ATOMICS__
#warning "Atomics not supported, may suffer race conditions!"
#define Atomic
#else
#define Atomic _Atomic
#endif

#include "utils.h"
#include "list.h"
#include "snake.h"
#include "game.h"
#include "constants.h"
#include "draw.h"

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))

#define RGB(r,g,b) ((short) (r*1000)), ((short) (g*1000)), ((short) (b*1000))

#define _BOARD_WIDTH (40)
#define _BOARD_HEIGHT (20)

#define _BOARDWIN_WIDTH (_BOARD_WIDTH+3)
#define _BOARDWIN_HEIGHT (_BOARD_HEIGHT+3)

#define _SAFE_DIST (5)

#if (_SAFE_DIST*2 >= _BOARD_WIDTH) || (_SAFE_DIST*2 >= _BOARD_HEIGHT)
#error "Board is too small, adjust width, height or _SAFE_DIST"
#endif

#if _BOARD_WIDTH < 24
#warning "Keys window text may be hard to read"
#endif

#define _APPLE_LIFETIME (((_BOARD_WIDTH+_BOARD_HEIGHT)*1000)/1100)

/* Safe distance from board edge for snake to spawn */
const int SAFE_DIST = _SAFE_DIST;

/* Game board dimensions*/
const int BOARD_WIDTH  = _BOARD_WIDTH;
const int BOARD_HEIGHT = _BOARD_HEIGHT;

const int BOARDWIN_WIDTH = _BOARDWIN_WIDTH;
const int BOARDWIN_HEIGHT = _BOARDWIN_HEIGHT;

/* Height of the score window */
const int SCORE_HEIGHT = 4;

/* Object representations */
const int FENCE_CHAR = '#';
const int SNAKE_CHAR = '@';
const int STONE_CHAR = '*';
const int APPLE_CHAR = '+';
const int PAPPLE_CHAR = '%';

const int BKG_PAIR   = 30;
const int FENCE_PAIR = 31;
const int SNAKE_PAIR = 32;
const int STONE_PAIR = 33;
const int APPLE_PAIR = 34;
const int BLACK_PAIR = 35;
const int PAPPLE_PAIR = 36;

/* How many stones on the board */
const int STONES = 25;

/* How long before apple becomes stale */
const int APPLE_LIFETIME = _APPLE_LIFETIME;

/* NOT DECLARED IN CONSTANTS.H */
const int FENCE_COLOR = COLOR_WHITE;
const int SNAKE_COLOR = COLOR_GREEN;
const int STONE_COLOR = COLOR_MAGENTA;
const int APPLE_COLOR = COLOR_RED;
const int PAPPLE_COLOR = COLOR_YELLOW;

const int BKG_COLOR = COLOR_BLACK;

/* How often we read keys */
const unsigned int INPUT_GRANULARITY = 5;

/* Time between ticks in ms */
const unsigned int GAME_TICKS = 100;

/* Set on terminal window resize */
static Atomic int resizing = 1;

typedef struct _Context {
  mtx_t screenlock; /* Curses screen lock, see curs_threads(3) */
  mtx_t boardlock;  /* Game board lock */
  mtx_t gamelock;   /* Lock before start of the game */
  cnd_t gamecnd;    /* Signals start of the game */
  cnd_t drawcnd;    /* Signaled when game needs redraw */
} Context;


/* Center window with respect to scoreboard and gameboard heights */
static void fix_win(WINDOW *win, int height, int off)
{
  int bw = BOARDWIN_WIDTH;
  int bh = BOARDWIN_HEIGHT;
  int w = bw;
  int h = bh + SCORE_HEIGHT;
  int x = (COLS/2 - w/2);
  int y = (LINES/2 - h/2) + off;
  chtype tmp = getbkgd(win);
  wbkgd(win, COLOR_PAIR(BLACK_PAIR));
  werase(win);
  wnoutrefresh(win);
  wbkgd(win, tmp);
  wresize(win, 1, 1); /* HACK: so mvwin don't fail */
  mvwin(win, MAX(off,y), MAX(0,x));
  wresize(win, height, bw);
}

static void fix_windows(WINDOW *bwin, WINDOW *swin)
{
  fix_win(bwin, BOARDWIN_HEIGHT, 0);
  fix_win(swin, SCORE_HEIGHT, BOARDWIN_HEIGHT);
  
  resizing = 0;
}

static int extract_key()
{
  int key = getch();
  int rest;
  /* Clear buffer from repeating keys */
  while((rest = getch()) == key && rest != ERR);
  if (rest != ERR)
    ungetch(rest);
  return key;
}

static void handle_input(GameStatus *game)
{
  int key = extract_key();
    
  switch (key) {
  case 'h':
    game_direct(game, DIR_UP);
    return;
  case 'n':
    game_direct(game, DIR_DOWN);
    return;
  case 'm':
    game_direct(game, DIR_RIGHT);
    return;
  case 'b':
    game_direct(game, DIR_LEFT);
    return;
  case 'q':
    game_terminate(game);
    return;
  case ' ':
    game_mark_unpausing(game);
    return;
  case KEY_RESIZE:
    resizing = 1;
    return;
  }
  /* Clear unused keys */
  while(getch() != ERR);
}

static void ptext(WINDOW *win, int l, int c, const char *t)
{
  int pos;
  if (c == 0)
    pos = BOARDWIN_WIDTH/8;
  else
    pos = BOARDWIN_WIDTH - 12 - BOARDWIN_WIDTH/8;
  mvwprintw(win, l+1, pos, "%s", t);
}

static void let_the_games_begin(GameStatus *game)
{
  const int WIDTH = _BOARDWIN_WIDTH;
  const int HEIGHT = 5;
  int ch;
  WINDOW *win = newwin(HEIGHT, WIDTH, LINES/2 - HEIGHT/2, COLS/2 - WIDTH/2);
  wbkgdset(win, COLOR_PAIR(BLACK_PAIR));
  timeout(100);
  while (1) {
    /* Watching KEY_RESIZE is a little unreliable here */
    fix_win(win, HEIGHT, BOARDWIN_HEIGHT/2 - HEIGHT/2);
    ptext(win, 0, 0, "H - UP  ");  ptext(win, 0, 1,"    B - LEFT ");
    ptext(win, 1, 0, "N - DOWN");  ptext(win, 1, 1,"    M - RIGHT");
    ptext(win, 2, 0, "Q - QUIT");  ptext(win, 2, 1,"SPACE - START");
    box(win, 0, 0);
    mvwprintw(win, 0, WIDTH/2 - 18/2, " Snake - Controls ");
    wrefresh(win);
    ch = getch();
    if (ch == 'q') {
      game_terminate(game);
      break;
    } else if (ch == ' ') {
      break;
    }
  }
  delwin(win);
  timeout(0);
}

/* FIXME: May deadlock, but I don't know better solution */
static void wait_start(Context *ctx)
{
  mtx_lock(&ctx->gamelock);
  cnd_wait(&ctx->gamecnd, &ctx->gamelock);
  mtx_unlock(&ctx->gamelock);
}

static int draw_thread(void *arg)
{
  List *list = (List*) arg;
  GameStatus *game = *(GameStatus**) list_get_nth(list, 0);
  WINDOW *bwin = *(WINDOW**) list_get_nth(list, 1);
  WINDOW *swin = *(WINDOW**) list_get_nth(list, 2);
  Context *ctx = *(Context**) list_get_nth(list, 3);
  wait_start(ctx);

  mtx_lock(&ctx->boardlock);
  while (!game_terminating(game)) {
    game_draw_board(game, bwin);
    draw_score(game, swin);
    mtx_lock(&ctx->screenlock);
    if (resizing)
      fix_windows(bwin, swin);
    wrefresh(bwin);
    wrefresh(swin);
    mtx_unlock(&ctx->screenlock);
    cnd_wait(&ctx->drawcnd, &ctx->boardlock);
  }
  mtx_unlock(&ctx->boardlock);
  return 0;
}

static int input_thread(void *arg)
{
  List *list = (List*) arg;
  GameStatus *game = *(GameStatus**) list_get_nth(list, 0);
  Context *ctx = *(Context**) list_get_nth(list, 3);
  let_the_games_begin(game);
  cnd_broadcast(&ctx->gamecnd);
  unsigned int ticks = getticks();
  
  while(!game_terminating(game)) {
    unsigned int dt = getticks() - ticks;
    if (dt <= INPUT_GRANULARITY)
      msleep(MIN(dt,INPUT_GRANULARITY));
    mtx_lock(&ctx->screenlock);
    handle_input(game);
    mtx_unlock(&ctx->screenlock);
    ticks = getticks();
  }
  return 0;
}

static void main_thread(void *arg)
{
  List *list = (List*) arg;
  GameStatus *game = *(GameStatus**) list_get_nth(list, 0);
  Context *ctx = *(Context**) list_get_nth(list, 3);
  wait_start(ctx);
  unsigned int ticks = getticks();

  while(!game_terminating(game)) {
    unsigned int dt = getticks() - ticks;
    if (dt >= GAME_TICKS) {
      mtx_lock(&ctx->boardlock);
      if (game_unpausing(game))
        game_unpause(game);
      gamelogic_step(game);
      mtx_unlock(&ctx->boardlock);
      cnd_signal(&ctx->drawcnd);
      ticks = getticks();
    }
    msleep(MIN(dt,GAME_TICKS));
  }
  /* Wake up draw thread */
  cnd_signal(&ctx->drawcnd);
}

static void dispatch(GameStatus *game, WINDOW *bwin, WINDOW *swin, Context *ctx)
{
  List *list = list_new();
  list_add(list, game);  /* 0 */
  list_add(list, bwin);  /* 1 */
  list_add(list, swin);  /* 2 */
  list_add(list, ctx);   /* 3 */

  thrd_t draw, input;
  if (thrd_create(&draw, draw_thread, list) == thrd_error)
    die("Cannot create draw thread!\n");

  if (thrd_create(&input, input_thread, list) == thrd_error)
    die("Cannot create input thread!\n");
  
  main_thread(list);

  thrd_join(draw, NULL);
  thrd_join(input, NULL);
  list_free(list);
}

static short colors[8][3];

const size_t ncolors = sizeof(colors)/sizeof(*colors);

void save_colors()
{
  for (size_t i = 0; i < ncolors; i++) {
    short r, g, b;
    color_content(i, &r, &g, &b);
    colors[i][0] = r;
    colors[i][1] = g;
    colors[i][2] = b;
  }
}

void restore_colors()
{
  for (size_t i = 0; i < ncolors; i++) {
    short r = colors[i][0];
    short g = colors[i][1];
    short b = colors[i][2];
    init_color(i, r, g, b);
  }
}

static void init_graphics()
{
  initscr();
  start_color();
  int coff = 0;
  if (can_change_color()) {
    save_colors();
    coff = 128;
    init_color(FENCE_COLOR+coff,  RGB(0.6, 0.3, 0.2));
    init_color(SNAKE_COLOR+coff,  RGB(  0, 0.6,   0));
    init_color(STONE_COLOR+coff,  RGB(0.7, 0.7, 0.7));
    init_color(APPLE_COLOR+coff,  RGB(0.8,   0,   0));
    init_color(BKG_COLOR+coff,    RGB(  0, 0.2,   0));
    init_color(PAPPLE_COLOR+coff, RGB(0.9, 0.5,   0));
  }
  init_pair(BKG_PAIR, 0, BKG_COLOR+coff);
  init_pair(BLACK_PAIR, COLOR_WHITE, COLOR_BLACK);
  init_pair(FENCE_PAIR, FENCE_COLOR+coff, BKG_COLOR+coff);
  init_pair(SNAKE_PAIR, SNAKE_COLOR+coff, SNAKE_COLOR+coff);
  init_pair(STONE_PAIR, STONE_COLOR+coff, STONE_COLOR+coff);
  init_pair(APPLE_PAIR, APPLE_COLOR+coff, APPLE_COLOR+coff);
  init_pair(PAPPLE_PAIR, PAPPLE_COLOR+coff, PAPPLE_COLOR+coff);
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  curs_set(0);
  bkgd(COLOR_PAIR(BLACK_PAIR));
}

static WINDOW* new_win(int height, int off)
{
  int bw = BOARDWIN_WIDTH;
  int bh = BOARDWIN_HEIGHT;
  int w = bw;
  int h = bh + SCORE_HEIGHT;
  int x = (COLS/2 - w/2);
  int y = (LINES/2 - h/2) + off;
  
  WINDOW *win = newwin(height, bw, MAX(off,y), MAX(0,x));
  return win;
}

static WINDOW* new_board_win()
{
  WINDOW *win = new_win(BOARDWIN_HEIGHT, 0);
  wbkgd(win, COLOR_PAIR(BKG_PAIR));  
  return win;
}

static WINDOW *new_score_win()
{
  WINDOW *win = new_win(SCORE_HEIGHT, BOARDWIN_HEIGHT);

  return win;
}

static Context *init_context()
{
  Context *ctx = malloc(sizeof(*ctx));
  mtx_init(&ctx->screenlock, mtx_plain);
  mtx_init(&ctx->boardlock, mtx_plain);
  mtx_init(&ctx->gamelock, mtx_plain);
  cnd_init(&ctx->gamecnd);
  cnd_init(&ctx->drawcnd);
  return ctx;
}

static void clear_context(Context *ctx)
{
  mtx_destroy(&ctx->screenlock);
  mtx_destroy(&ctx->boardlock);
  mtx_destroy(&ctx->gamelock);
  cnd_destroy(&ctx->gamecnd);
  cnd_destroy(&ctx->drawcnd);
  free(ctx);
}

void abort_handler (int sig)
{
  (void) sig;
  endwin();
  exit(1);
}

int main()
{
  srand(time(NULL));
  init_graphics();
  GameStatus *game = game_new();
  WINDOW *bwin = new_board_win();
  WINDOW *swin = new_score_win();
  Context *ctx = init_context();

  struct sigaction sa;

  sa.sa_handler = abort_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;

  dispatch(game, bwin, swin, ctx);
  
  endwin();
  clear_context(ctx);
  delwin(swin);
  delwin(bwin);
  restore_colors();
  use_default_colors();
  game_free(game);
  
  return 0;
}
