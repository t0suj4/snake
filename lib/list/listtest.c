#include <stdio.h>
#include <signal.h>
#include <execinfo.h>
#include <stdlib.h>
#include "list.h"

void bt_sighandler(int sig, struct sigcontext ctx) {

  void *trace[16];
  char **messages = (char **)NULL;
  int i, trace_size = 0;

  if (sig == SIGSEGV)
    printf("Got signal %d, faulty address is %p, "
           "from %p\n", sig, ctx.cr2, ctx.rip);
  else
    printf("Got signal %d\n", sig);

  trace_size = backtrace(trace, 16);
  /* overwrite sigaction with caller's address */
  trace[1] = (void *)ctx.rip;
  messages = backtrace_symbols(trace, trace_size);
  /* skip first stack frame (points here) */
  printf("[bt] Execution path:\n");
  for (i=1; i<trace_size; ++i)
  {
    printf("[bt] #%d %s\n", i, messages[i]);

    /* find first occurence of '(' or ' ' in message[i] and assume
     * everything before that is the file name. (Don't go beyond 0 though
     * (string terminator)*/
    size_t p = 0;
    while(messages[i][p] != '(' && messages[i][p] != ' '
            && messages[i][p] != 0)
        ++p;

    char syscom[256];
    sprintf(syscom,"addr2line %p -e %.*s", trace[i], p, messages[i]);
        //last parameter is the file name of the symbol
    system(syscom);
  }

  exit(2);
}

void printint(void *data, void *arg)
{
  int rval = printf(" %d,", *(int*) data);
  return;
}

int main()
{
  struct sigaction sa;

  sa.sa_handler = (void *) bt_sighandler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;

  sigaction(SIGABRT, &sa, NULL);
 
  List *list = list_new();
  const int L33T = 1337;

  for (int i = 0; i < 10; i++) {
    list_add(list, i);
  }

  List *clone = list_clone(list);
  List *another = list_new();
  list_add(list, L33T);
  list_foreach(list, printint, NULL);
  printf("\n");
  
  list_foreach(clone, printint, NULL);
  printf("\n");

  list_foreach(list_push(list_delete(list_nth(clone, 5)), L33T), printint, NULL);
  printf("\n");

  list_foreach(clone, printint, NULL);
  printf("\n");

  printf("%d\n%d\n%d\n",*(int *)list_get_nth(list, 0), *(int*) list_get_nth(list, 4), *(int*) list_last(list));

  list_delete(list_tail(list));
  
  list_foreach(list, printint, NULL);
  printf("\n");

  list_concat(list, clone);
  list_foreach(list, printint, NULL);
  printf("\n");
  list_concat(another, list);

  list_detach(another);
  list_detach(clone);
  list_free(list);

  return 0;
}
