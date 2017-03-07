#ifdef _UTILS_H_
#error "utils.h already included"
#endif
#define _UTILS_H_

int in_bounds(int x, int y, int h, int w, int perimeter);
int randint(int min, int max);
unsigned int getticks();
void msleep(unsigned int ms);
_Noreturn void die(const char *fmt, ...);
