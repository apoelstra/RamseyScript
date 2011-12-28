#ifndef COLORING_H
#define COLORING_H

#include "sequence.h"

typedef struct {
  Sequence **sequences;
  int n_colors;
} Coloring;

Coloring *coloring_new (int n_colors);
int coloring_append (Coloring *s, int value, int position);
void coloring_deappend (Coloring *s, int position);
void coloring_print (Coloring *s);
void coloring_delete (Coloring *s);

#endif
