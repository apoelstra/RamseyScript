#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "ramsey.h"

void *sequence_new (const global_data_t *);
void *sequence_new_direct (void);
const ramsey_t *sequence_prototype (void);

#endif
