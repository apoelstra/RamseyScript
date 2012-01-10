#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "global.h"
#include "stream.h"

ramsey_t *sequence_new ();
ramsey_t *sequence_new_zeros (int size, bool one_indexed);
void sequence_print_real (ramsey_t *s, int start, Stream *out);

#define sequence_max(s) ((s)->values[(s)->length - 1])

#endif
