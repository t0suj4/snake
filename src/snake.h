#ifdef _SNAKE_H_
#error "snake.h already included"
#endif
#define _SNAKE_H_

typedef List ObjectList;
typedef List PointList;

typedef enum _Direction {
  DIR_LEFT,
  DIR_RIGHT,
  DIR_UP,
  DIR_DOWN
} Direction;

