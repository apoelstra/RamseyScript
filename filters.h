#ifndef FILTER_H
#define FILTER_H

#include "global.h"

typedef int (*filter_func) (ramsey_t *);

int add_filter (ramsey_t *rt, filter_func f);
int run_filters (ramsey_t *rt);


#endif
