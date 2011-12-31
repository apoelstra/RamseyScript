#ifndef GLOBAL_H
#define GLOBAL_H

#include "sequence.h"
#include "filters.h"

typedef int bool;

struct _global_data {
  long iterations;
  int min_gap;
  int max_gap;
  int n_colors;
  int ap_length;
  Sequence *alphabet;
  Sequence *gap_set;
  filter_func filter;

  FILE *dump_fh;
  bool dump_iters;
  int dump_depth;
  Sequence *iters_data;
};

extern struct _global_data global;

#endif
