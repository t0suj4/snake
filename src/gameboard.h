typedef struct _GameBoard GameBoard;

GameBoard *board_new(int w, int h);

int board_width(GameBoard *board);

int board_height(GameBoard *board);

int board_is_inside(GameBoard *board, int x, int y);

List *board_objects(GameBoard *board);

void board_free(GameBoard *board);
