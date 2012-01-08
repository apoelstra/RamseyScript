#ifndef GLOBAL_H
#define GLOBAL_H

#include "stream.h"
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

  Stream *dump_stream;
  bool dump_iters;
  int dump_depth;
  Sequence *iters_data;

  int kill_now;

  Stream *out_stream;
  Stream *in_stream;
  Stream *err_stream;
};

#endif
