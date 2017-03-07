#include <stdlib.h>
#include "list.h"
#include "gameboard.h"
#include "constants.h"
#include "snake.h"
#include "game.h"
#include "draw.h"
#include "gameobject.h"
#include <string.h>
#include "point.h"

#ifdef __STDC_NO_ATOMICS__
#warning "Atomics not supported, may suffer race conditions!"
#define Atomic
#else
#define Atomic _Atomic
#endif

static const Direction INIT_DIRECTION = DIR_RIGHT;

typedef enum _Pause {
  PAUSE_NO,
  PAUSE_PAUSED,
  PAUSE_UNPAUSING,
} Pause;

typedef struct _GameStatus {
  GameBoard *board;          /* Game board */
  _Atomic int score;         /* Number of consumed apples */
  _Atomic Direction direction;   /* New direction, where should snake go */
  _Atomic Pause paused;      /* Paused game, nothing moves */
  _Atomic int dead;          /* Player is dead */
  _Atomic int terminating;   /* Are we leaving the game? */
} GameStatus;

static void game_reset_status(GameStatus *game)
{
  game->score = 0;
  game->direction = INIT_DIRECTION;
  game->paused = PAUSE_PAUSED;
  game->dead = 0;
  /* Cannot clear terminating, may cause deadlock */
}

void game_incscore(GameStatus *game)
{
  game->score +=1;
}

int game_score(GameStatus *game)
{
  return game->score;
}

void game_direct(GameStatus *game, Direction dir)
{
  game->direction = dir;
}

Direction game_direction(GameStatus *game)
{
  return game->direction;
}

void game_pause(GameStatus *game)
{
  game->paused = PAUSE_PAUSED;
}

int game_paused(GameStatus *game)
{
  return game->paused == PAUSE_PAUSED;
}

int game_unpausing(GameStatus *game)
{
  return game->paused == PAUSE_UNPAUSING;
}

int game_running(GameStatus *game)
{
  return game->paused == PAUSE_NO;
}

/* Ugliest function name in the entire codebase */
void game_mark_unpausing(GameStatus *game)
{
  if (game_paused(game))
    game->paused = PAUSE_UNPAUSING;
}

void game_terminate(GameStatus *game)
{
  game->terminating = 1;
}

int game_terminating(GameStatus *game)
{
  return game->terminating;
}

/* Kill the player and pause game */
void game_kill(GameStatus *game)
{
  game_pause(game);
  game->dead = 1;
}

int game_dead(GameStatus *game)
{
  return game->dead;
}

GameBoard *game_board(GameStatus *game)
{
  return game->board;
}

int object_has_name(GameObject *obj, char *name)
{
  return strcmp(obj->name, name) ? 0 : 1;
}

static int handle_object_has_name(void *data, void *arg)
{
  return object_has_name(data, arg);
}

GameObject *get_board_object(GameBoard *board, const char *name)
{
  return (GameObject *) list_find(board_objects(board), handle_object_has_name, (void*) name);
}

static void object_clear(GameObject *obj)
{
  list_free(obj->points);
  graphics_free(obj->graphics);
}

static void handle_clear_objects(void *data, void *arg)
{
  GameObject *obj = (GameObject*) data;
  (void) arg;
  object_clear(obj);
}

static void clear_objects(GameBoard *board)
{
  list_foreach(board_objects(board), handle_clear_objects, NULL);
}

static GameObject object_make(const char *name, int shape, int color, PointList *points)
{
  Graphics *g = graphics_attach(shape, color, points);
  GameObject obj = {.name = name,
		    .graphics = g,
		    .data = NULL,
		    .points = points};
  return obj;
}

/* Place objects on the board */
static void repopulate_board(GameBoard *board, int nstones)
{
  GameObject *snake = get_board_object(board, "snake");
  GameObject *stones = get_board_object(board, "stones");
  GameObject *apples = get_board_object(board, "apples");
  GameObject *papples = get_board_object(board, "papples");

  list_clear(snake->points);
  {
    Point head = point_random(board_width(board), board_height(board), SAFE_DIST);
    PointList *point = snake->points;
    list_add(point, head);
    head.x -=1;
    list_add(point, head);
    head.x -=1;
    list_add(point, head);
    snake->data = (void*) INIT_DIRECTION;
  }

  list_clear(stones->points);
  for (int i = 0; i < nstones; i++) {
    Point stone = point_random(board_width(board), board_height(board), 0);
    list_add(stones->points, stone);
  }
  /* Apples will be generated in game loop with additional checks */
  list_clear(apples->points);
  apples->data = (void*) 0;
  list_clear(papples->points);
}

/* Populate game board with objects */
static void init_board(GameBoard *board)
{
  GameObject snake   = object_make("snake", SNAKE_CHAR, SNAKE_PAIR, list_new());
  GameObject apples  = object_make("apples", APPLE_CHAR, APPLE_PAIR, list_new());
  GameObject stones  = object_make("stones", STONE_CHAR, STONE_PAIR, list_new());
  GameObject papples = object_make("papples", PAPPLE_CHAR, PAPPLE_PAIR, list_new());

  ObjectList *objects = board_objects(board);

  list_add(objects, apples);
  list_add(objects, snake);
  list_add(objects, stones);
  list_add(objects, papples);
  
  repopulate_board(board, STONES);
}

GameStatus *game_new()
{
  GameStatus *game = malloc(sizeof(*game));
  GameBoard *board = board_new(BOARD_WIDTH, BOARD_HEIGHT);
  game_reset_status(game);
  game->terminating = 0;
  game->board = board;
  init_board(board);
  
  return game;
}

void game_free(GameStatus *game)
{
  clear_objects(game_board(game));
  board_free(game_board(game));
  free(game);
}

void game_reset(GameStatus *game)
{
  game_reset_status(game);
  repopulate_board(game_board(game), STONES);
}

/* Reset game if dead, then unpause */
void game_unpause(GameStatus *game)
{
  if (game_dead(game))
    game_reset(game);
  
  game->paused = PAUSE_NO;
}

static void collect_graphics(GameObject *obj, GraphicsList *gs)
{
  list_add(gs, obj->graphics);
}

static void handle_collect_graphics(void *data, void *arg)
{
  GameObject *obj = (GameObject*) data;
  GraphicsList *gs = (GraphicsList*) arg;
  collect_graphics(obj, gs);
}

void game_draw_board(GameStatus *game, void *win)
{
  GraphicsList *gs = list_new();
  list_foreach(board_objects(game_board(game)), handle_collect_graphics, gs);
  draw_objects(gs, win);
  list_free(gs);
}
