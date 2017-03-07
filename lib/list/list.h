struct _List;

typedef struct _List List;

#define list_add(X_X_HEAD_X_X, X_X_DATA_X_X) \
  list_append(X_X_HEAD_X_X, &X_X_DATA_X_X, sizeof(X_X_DATA_X_X))

#define list_push(X_X_HEAD_X_X, X_X_DATA_X_X) \
  list_prepend(X_X_HEAD_X_X, &X_X_DATA_X_X, sizeof(X_X_DATA_X_X))

List *list_new();

void *list_get(List *list);

List *list_prepend(List *list, const void *data, size_t size);

List *list_append(List *list, const void *data, size_t size);

List *list_get_next(List *list);

int list_has_next(List *list);
  
List *list_clear(List *list);

List *list_delete(List *list);

List *list_tail(List *list);

void *list_last(List *list);

int list_empty(List *list);

void list_free(List *list);

/* Detach head from the rest of the list */
void list_detach(List *list);

/* Detach head, without freeing */
void list_detach_head(List *list);

void *list_find(List *head, int (*find_fn)(void *data, void *arg), void *arg);

void list_foreach(List *list, void (*each_fn)(void *data, void *arg), void *arg);

List *list_clone(List *list);

List *list_nth(List *list, int n);

void *list_get_nth(List *head, int n);

/* Concatenate responsibly */
List *list_concat(List *head, List *list);
