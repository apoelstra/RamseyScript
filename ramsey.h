#ifndef RAMSEY_H
#define RAMSEY_H

#include <stddef.h>

typedef struct {
  char *(*read_line) (char *buf, size_t len, void *obj);
  void *data;
} Stream;

void set_defaults ();
void process (Stream *stm);

#endif
