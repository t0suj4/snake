#include <stdlib.h>
#include "list.h"
#include "point.h"
#include <assert.h>
#include "utils.h"

Point point_make(int x, int y)
{
  Point p = {.x = x, .y = y};
  return p;
}

Point point_random(int w, int h, int per)
{
  assert(w > per*2);
  assert(h > per*2);
  Point point = point_make(randint(per, w-per),
			   randint(per, h-per));

  assert(in_bounds(point.x, point.y, w, h, per));
  return point;
}


int point_collide(Point p1 , Point p2)
{
  if (p1.x == p2.x && p1.y == p2.y)
    return 1;
  return 0;
}

static int handle_collide_points(void *data, void *arg)
{
  Point p1 = *(Point*) data;
  Point p2 = *(Point*) arg;
  return point_collide(p1, p2);
}

int points_collide_with_point(PointList *points, Point point)
{
  return list_find(points, handle_collide_points, &point) ? 1 : 0;
}
