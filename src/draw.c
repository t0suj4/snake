#include <stdlib.h>
#include "list.h"
#include "point.h"
#include "constants.h"

#include <curses.h>

#define TIMES8(x) x,x,x,x,x,x,x,x

typedef List GraphicsList;

typedef unsigned int flags_t;

#define DEFLG(X_X_FNAME_X_X, X_X_POS_X_X) \
  static const flags_t X_X_FNAME_X_X = 1 << X_X_POS_X_X

/* Don't free attached points */
DEFLG(ATTACHED, 0);

/* Offset from window borders */
static const int DRAW_OFFSET = 1;

typedef struct _Graphics {
  int shape;         /* Character representing the object */
  int color;         /* Color of the object */
  flags_t flags;     /* State flags for the object */
  PointList *points; /* Points where the object should be drawn */
} Graphics;

Graphics *graphics_new(int shape, int color)
{
  Graphics *g = malloc(sizeof(*g));
  g->shape = shape;
  g->color = color;
  g->flags = 0;
  g->points = list_new();
  
  return g;
}

Graphics *graphics_attach(int shape, int color, PointList *points)
{
  Graphics *g = malloc(sizeof(*g));
  g->shape = shape;
  g->color = color;
  g->flags = ATTACHED;
  g->points = points;

  return g;
}

void graphics_free(Graphics *g)
{
  if (!(g->flags & ATTACHED))
    list_free(g->points);
  free(g);
}

static void draw_point(Point p, int c, WINDOW *win)
{
  mvwaddch(win, p.y+DRAW_OFFSET, p.x+DRAW_OFFSET, c);
}

static void handle_draw_point(void *data, void *arg)
{
  Point p = *(Point*) data;
  List *l = (List*) arg;
  int c = *(int*) list_get_nth(l, 0);
  WINDOW *win = *(WINDOW**) list_get_nth(l, 1);
  draw_point(p, c, win);
}

static void draw_object(Graphics *g, WINDOW *win)
{
  List *list = list_new();
  list_add(list, g->shape);
  list_add(list, win);
  wattron(win, COLOR_PAIR(g->color));
  list_foreach(g->points, handle_draw_point, list);
  wattroff(win, COLOR_PAIR(g->color));
  list_free(list);
}

static void handle_draw_object(void *data, void *arg)
{
  Graphics *obj = *(Graphics**) data;
  WINDOW *win = (WINDOW*) arg;
  draw_object(obj, win);
}

static void draw_fence(WINDOW *win)
{
  wattron(win, COLOR_PAIR(FENCE_PAIR));
  wborder(win, TIMES8(FENCE_CHAR));
  wattroff(win, COLOR_PAIR(FENCE_PAIR));
}

static void draw_score_border(WINDOW *win)
{
  box(win, 0, 0);
}

typedef struct _GameStatus GameStatus;

int game_score(GameStatus *game);

void draw_score(GameStatus *game, WINDOW *win)
{
  werase(win);
  draw_score_border(win);
  mvwprintw(win, 1, 1, "SCORE:  %*d", 3, game_score(game));
}

void draw_objects(GraphicsList *gs, WINDOW *win)
{
  werase(win);
  list_foreach(gs, handle_draw_object, win);
  draw_fence(win);
}
