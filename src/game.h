#ifdef _GAME_H_
#error "game.h already included"
#endif
#define _GAME_H_

typedef struct _GameStatus GameStatus;
typedef struct _GameBoard GameBoard;

GameStatus *game_new();

void game_free(GameStatus *game);

void gamelogic_step(GameStatus *game);

void game_incscore(GameStatus *game);

int game_score(GameStatus *game);

void game_direct(GameStatus *game, Direction dir);

Direction game_direction(GameStatus *game);

void game_pause(GameStatus *game);

int game_paused(GameStatus *game);

int game_unpausing(GameStatus *game);

int game_running(GameStatus *game);

void game_mark_unpausing(GameStatus *game);

void game_unpause(GameStatus *game);

void game_terminate(GameStatus *game);

int game_terminating(GameStatus *game);

void game_kill(GameStatus *game);

int game_dead(GameStatus *game);

GameBoard *game_board(GameStatus *game);

void game_draw_board(GameStatus *game, void *win);
