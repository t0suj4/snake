#ifdef _GAMEOBJECT_H_
#error "gameobject.h already included"
#endif
#define _GAMEOBJECT_H_

typedef struct _Graphics Graphics;

typedef struct _GameObject {
  PointList  *points;   /* Points it occupies on the board */
  void       *data;     /* Arbitrary data */
  Graphics   *graphics; /* Graphics representing the object */
  const char *name;     /* Name of the object */
} GameObject;
