#ifndef RECURSE_H
#define RECURSE_H

#include "sequence.h"
#include "filters.h"

void reset_max ();

void recurse_sequence (Sequence *seed, int min, int max, filter_func filter, long max_iterations);
void recurse_words (Sequence *seed, Sequence *alphabet, filter_func filter, long max_iterations);
void recurse_colorings_breadth_first (Coloring *seed, int max_value, int min,
                                      int max, filter_func filter, long max_iterations);
void recurse_colorings(Coloring *seed, int max_value, int min,
                       int max, filter_func filter, long max_iterations);

#endif
