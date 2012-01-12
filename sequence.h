#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "global.h"
#include "stream.h"

ramsey_t *sequence_new ();
ramsey_t *sequence_new_zeros (int size, bool one_indexed);

#endif
