#include <stdlib.h>
#include "list.h"
#include "point.h"
#include "gameboard.h"
#include "gameobject.h"
#include "snake.h"
#include "game.h"
#include "constants.h"

GameObject *get_board_object(GameBoard *board, const char *name);

static Direction dir_opposite(Direction dir)
{
  switch(dir) {
  case DIR_UP:
    dir = DIR_DOWN;
    break;
  case DIR_DOWN:
    dir = DIR_UP;
    break;
  case DIR_LEFT:
    dir = DIR_RIGHT;
    break;
  case DIR_RIGHT:
    dir = DIR_LEFT;
    break;
  default:
    abort();
  }
  return dir;
}

static void snake_grow(PointList *snake)
{
  PointList *tail = list_tail(snake);
  PointList *clone = list_clone(tail);
  list_concat(tail, clone);
  list_detach(clone);
}

static Point snake_head(PointList *snake)
{
  return *(Point*) list_get_nth(snake, 0);
}

static PointList *snake_body(PointList *snake)
{
  return list_nth(snake, 1);
}

static Direction snake_gdir(GameObject *snake)
{
  return (Direction) snake->data;
}

static void snake_sdir(GameObject *snake, Direction dir)
{
  snake->data = (void*) dir;
}

static int collide_with_object(GameObject *obj, Point point)
{
  return points_collide_with_point(obj->points, point);
}

static int handle_collide_with_object(void *data, void *arg)
{
  GameObject *obj = (GameObject*) data;
  Point *point = (Point*) arg;
  return collide_with_object(obj, *point);
}

static int collide_everything(ObjectList *list, Point point)
{
  return list_find(list, handle_collide_with_object, &point) ? 1 : 0;
}

static int create_empty_board_point(GameBoard *board, Point *point)
{
  Point p = point_random(board_width(board), board_height(board), 0);
  *point = p;
  return collide_everything(board_objects(board), p) ? 0 : 1;
}

static void snake_check_collisions(GameBoard *board, GameStatus *game)
{
  PointList *snake = get_board_object(board, "snake")->points;
  Point head = snake_head(snake);
  if (!board_is_inside(board, head.x, head.y))
    game_kill(game);

  PointList *body = snake_body(snake);
  if (points_collide_with_point(body, head))
    game_kill(game);

  PointList *stones = get_board_object(board, "stones")->points;
  if (points_collide_with_point(stones, head))
    game_kill(game);

  PointList *papples = get_board_object(board, "papples")->points;
  if (points_collide_with_point(papples, head))
    game_kill(game);

  GameObject *apples = get_board_object(board, "apples");
  if (points_collide_with_point(apples->points, head)) {
    snake_grow(snake);
    game_incscore(game);
    list_clear(apples->points);
    apples->data = (void*) 0;
  }  
}

static void snake_move(GameBoard *board, GameStatus *game)
{
  GameObject *sobj = get_board_object(board, "snake");
  PointList *snake = sobj->points;
  Point head = snake_head(snake);
  int x = head.x;
  int y = head.y;

  /* Don't go backwards */
  Direction dir = game_direction(game);
  Direction ldir = snake_gdir(sobj);
  if (ldir != dir_opposite(dir)) {
    snake_sdir(sobj, dir);
    ldir = dir;
  }
  
  switch(ldir) {
  case DIR_RIGHT:
    x +=1;
    break;
  case DIR_LEFT:
    x -=1;
    break;
  case DIR_DOWN:
    y +=1;
    break;
  case DIR_UP:
    y -=1;
    break;
  default:
    abort();
  }
  
  Point nhead = point_make(x, y);
  list_push(snake, nhead);
  list_clear(list_tail(snake));
}

static void regen_apples(GameBoard *board)
{
  ObjectList *apples = get_board_object(board, "apples")->points;
  if (!list_empty(apples))
    return;
  
  Point apple;
  if (!create_empty_board_point(board, &apple))
    return;

  list_add(apples, apple);
}

static void apples_tick(GameBoard *board)
{
  GameObject *apples = get_board_object(board, "apples");

  if (((int) (long) apples->data) < APPLE_LIFETIME) {
    apples->data += 1;
    return;
  }

  GameObject *papples = get_board_object(board, "papples");
  list_concat(papples->points, apples->points);
  list_detach_head(apples->points);
  apples->data = (void*) 0;
}

void gamelogic_step(GameStatus *game)
{
  GameBoard *board = game_board(game);
  if (game_running(game)) {
    snake_check_collisions(board, game);
    apples_tick(board);
    if (game_dead(game))
      return;
    snake_move(board, game);
  }
  regen_apples(board);
}
