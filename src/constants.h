#ifdef _CONSTANTS_H_
#error "constants.h already included"
#endif
#define _CONSTANTS_H_

/* Safe distance from board edge for snake to spawn */
extern const int SAFE_DIST;

/* Game board dimensions*/
extern const int BOARD_WIDTH;
extern const int BOARD_HEIGHT;

/* Object representations */
extern const int FENCE_CHAR;
extern const int SNAKE_CHAR;
extern const int STONE_CHAR;
extern const int APPLE_CHAR;
extern const int PAPPLE_CHAR;

extern const int FENCE_PAIR;
extern const int SNAKE_PAIR;
extern const int STONE_PAIR;
extern const int APPLE_PAIR;
extern const int PAPPLE_PAIR;

extern const int BKG_COLOR;

/* How many stones on the board */
extern const int STONES;

/* How long befor an apple becomes stale */
extern const int APPLE_LIFETIME;
