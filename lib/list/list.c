#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define GETNEXT() List *next = head->next

typedef struct _List {
  struct _List *next;
  size_t size;
  char data[];
} List;

List *list_new()
{
  List *list = malloc(sizeof(*list));
  list->next = NULL;
  list->size = 0;
  
  assert(list != NULL);
  return list;
}

void *list_get(List *head)
{
  assert(head != NULL);
  GETNEXT();
  assert(next != NULL);
  return (void*) next->data;
}

List *list_get_next(List *head)
{
  assert(head != NULL);
  GETNEXT();
  return next;
}

static List *create_link(const void *data, size_t size)
{
  List *list = malloc(sizeof(*list) + size);
  memcpy(&list->data, data, size);
  list->size = size;
  return list;
}

static size_t get_size(List *head)
{
  assert(head != NULL);
  GETNEXT();
  assert(next != NULL);
  return next->size;
}

int list_has_next(List *head)
{
  assert(head != NULL);
  return list_get_next(head) != NULL;
}

List *list_prepend(List *head, const void *data, size_t size)
{
  assert(head != NULL);
  assert(size != 0);
  List *list = create_link(data, size);
  GETNEXT();
  list->next = next ? next : NULL;
  head->next = list;
  return head;
}

List *list_append(List *head, const void *data, size_t size)
{
  assert(head != NULL);
  assert(size != 0);
  GETNEXT();
  if (next) {
    list_append(next, data, size);
    return head;
  }
  next = create_link(data, size);
  next->next = NULL;
  head->next = next;
  return head;
}

void clear(List *head)
{
  GETNEXT();
  assert(head->size != 0);
  free(head);
  if(next)
    clear(next);
}

List *list_clear(List *head)
{
  assert(head != NULL);
  GETNEXT();
  if (next)
    clear(next);
  head->next = NULL;
  return head;
}

List *list_delete(List *head)
{
  assert(head != NULL);
  GETNEXT();
  if (next == NULL)
    return head;

  List *nnext = next->next;
  free(next);
  head->next = nnext;
  return head;
}

int list_empty(List *head)
{
  assert(head != NULL);
  GETNEXT();
  return next == NULL;
}

void list_free(List *list)
{
  assert(list != NULL);
  list_clear(list);
  free(list);
}

void list_detach(List *list)
{
  assert(list != NULL);
  assert(list->size == 0);
  free(list);
}

void list_detach_head(List *head)
{
  assert(head != NULL);
  assert(head->next != NULL);
  head->next = NULL;
}

void list_foreach(List *head, void (*each_fn)(void *data, void *arg), void *arg)
{
  assert(head != NULL);
  for (List *it = head; list_has_next(it); it = list_get_next(it)) {
    each_fn(list_get(it), arg);
  }
}

void *list_find(List *head, int (*find_fn)(void *data, void *arg), void *arg)
{
  assert(head != NULL);
  for (List *it = head; list_has_next(it); it = list_get_next(it)) {
    void *data = list_get(it);
    if (find_fn(data, arg))
      return data;
  }
  return NULL;
}

List *list_nth(List *head, int n)
{
  assert(head != NULL);
  assert(n >= 0);
  GETNEXT();
  if (!next)
    return NULL;
  
  if (n)
    return list_nth(next, --n);

  return head;
}

List *list_tail(List *head)
{
  assert(head != NULL);
  GETNEXT();
  assert(next != NULL);
  if (next->next)
    return list_tail(next);
  return head;
}

void *list_last(List *head)
{
  return list_get(list_tail(head));
}

void *list_get_nth(List *head, int n)
{
  assert(head != NULL);
  
  return list_get(list_nth(head, n));
}

List *list_clone(List *head)
{
  assert(head != NULL);
  List *list = list_new();
  for (List *it = head; list_has_next(it); it = list_get_next(it)) {
    list_append(list, list_get(it), get_size(it));
  }
  return list;
}

List *list_concat(List *head, List *head2)
{
  assert(head != NULL);
  assert(head2 != NULL);
  assert(head2->next != NULL);
  List *tail = head->next ? list_tail(head)->next : head;
  tail->next = head2->next;
  return head;
}
