#include <stdlib.h>
#include <stddef.h>
#include <assert.h>

#include "list.h"
#include "utils.h"

struct _GameObject;

typedef struct _GameObject GameObject;

typedef struct _GameBoard {
  int width;       /* Width of the board  */
  int height;      /* Height of the board */
  List *objects;   /* List of all objects */
} GameBoard;

GameBoard *board_new(int w, int h)
{
  GameBoard *board = malloc(sizeof(*board));
  board->width = w;
  board->height = h;
  board->objects = list_new();

  return board;
}

int board_width(GameBoard *board)
{
  return board->width;
}

int board_height(GameBoard *board)
{
  return board->height;
}

int board_is_inside(GameBoard *board, int x, int y)
{
  return in_bounds(x, y, board->width, board->height, 0);
}

List *board_objects(GameBoard *board)
{
  return board->objects;
}

void board_free(GameBoard *board)
{
  list_free(board->objects);
  free(board);
}
