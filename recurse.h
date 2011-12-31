#ifndef RECURSE_H
#define RECURSE_H

#include "sequence.h"
#include "filters.h"

void reset_max ();
long int get_iterations ();

void recurse_sequence (Sequence *seed);
void recurse_words (Sequence *seed);
void recurse_colorings (Coloring *seed, int max_value);

#endif
