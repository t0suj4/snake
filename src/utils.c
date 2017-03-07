#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>

int in_bounds(int x, int y, int w, int h, int perimeter)
{
  if (x > (w - perimeter) || x < perimeter)
    return 0;
  if (y > (h - perimeter) || y < perimeter)
    return 0;

  return 1;
}

int randint(int min, int max)
{
  return (rand() % (max - min)) + min;
}

unsigned int getticks()
{
  struct timespec time;
  clock_gettime(CLOCK_MONOTONIC, &time);
  return (unsigned int) (time.tv_sec * 1000) + (time.tv_nsec / 1000000);
}

void msleep (unsigned int ms)
{
  struct timespec time;
  time.tv_sec = ms / 1000;
  time.tv_nsec = (ms % 1000) * 1000000;
  nanosleep(&time, NULL);
}

_Noreturn void die(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  abort(); /* Have abort handler to cleanup resources */
}
