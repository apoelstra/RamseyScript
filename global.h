#ifndef GLOBAL_H
#define GLOBAL_H

typedef struct _ramsey_t ramsey_t;
typedef struct _global_data global_data_t;
typedef int bool;

#include "stream.h"
#include "filters.h"

#define VERSION "0.1"

typedef enum _e_ramsey_type {
  TYPE_INVALID,
  TYPE_SEQUENCE,
  TYPE_COLORING,
  N_TYPES
} e_ramsey_type;

struct _ramsey_t {
  e_ramsey_type type;

  const char *(*parse) (ramsey_t *, const char *data);
  void (*randomize)    (ramsey_t *, int);
  void (*print)        (const ramsey_t *, Stream *);
  void (*empty)        (ramsey_t *);
  void (*reset)        (ramsey_t *);
  void (*destroy)      (ramsey_t *);

  int (*add_filter)  (ramsey_t *, filter_func);
  int (*add_gap_set) (ramsey_t *, const ramsey_t *);
  int (*run_filters) (ramsey_t *);

  void (*recurse)       (ramsey_t *, global_data_t *);
  void (*recurse_reset) (ramsey_t *);
  int  (*recurse_get_iterations) (const ramsey_t *);
  int  (*recurse_get_max_length) (const ramsey_t *);

  int (*get_length)  (const ramsey_t *);
  int (*get_maximum) (const ramsey_t *);
  int (*get_n_cells) (const ramsey_t *);
  int (*append)      (ramsey_t *, int value);
  int (*cell_append) (ramsey_t *, int value, int cell);
  int (*deappend)    (ramsey_t *);
  int (*cell_deappend) (ramsey_t *, int cell);
  struct _ramsey_t **(*get_cells) (ramsey_t *);
  const struct _ramsey_t **(*get_cells_const) (const ramsey_t *);

  const ramsey_t *(*find_value) (const ramsey_t *, int);

  void *(*get_priv_data) (ramsey_t *);
  const void *(*get_priv_data_const) (const ramsey_t *);
};

struct _global_data {
  int max_iterations;
  int n_colors;
  int ap_length;
  int random_length;
  ramsey_t *alphabet;
  ramsey_t *gap_set;
  filter_func filter;

  Stream *dump_stream;
  bool dump_iters;
  int dump_depth;
  ramsey_t *iters_data;

  int kill_now;

  Stream *out_stream;
  Stream *in_stream;
  Stream *err_stream;
};

#endif
