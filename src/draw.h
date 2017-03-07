#ifdef _DRAW_H_
#error "draw.h already included"
#endif
#define _DRAW_H_

typedef struct _Graphics Graphics;

typedef List GraphicsList;

Graphics *graphics_new(int shape, int color);

Graphics *graphics_attach(int shape, int color, PointList *points);

void graphics_free(Graphics *g);

void draw_objects(GraphicsList *gl, void *win);

void draw_score(GameStatus *game, void *win);
