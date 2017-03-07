#ifdef _POINT_H_
#error "point.h already included"
#endif
#define _POINT_H_

typedef List PointList;

typedef struct _Point {
  int x;
  int y;
} Point;

Point point_make(int x, int y);

Point point_random(int w, int h, int per);

int point_collide(Point p1, Point p2);

int points_collide_with_point(PointList *points, Point point);
