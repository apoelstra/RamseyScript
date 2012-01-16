#ifndef GLOBAL_H
#define GLOBAL_H

typedef struct _ramsey_t ramsey_t;
typedef struct _global_data global_data_t;
typedef int bool;

typedef enum _e_ramsey_type {
  TYPE_INVALID,
  TYPE_SEQUENCE,
  TYPE_COLORING,
  TYPE_WORD,
  TYPE_PERMUTATION
} e_ramsey_type;


#include "stream.h"
#include "filter.h"

#define VERSION "0.1"

struct _ramsey_t {
  e_ramsey_type type;

  /* Recursion state variables */
  int r_depth;
  int r_iterations;
  int r_max_found;

  const char *(*get_type) (const ramsey_t *);

  const char *(*parse) (ramsey_t *, const char *data);
  void (*randomize)    (ramsey_t *, int);
  void (*print)        (const ramsey_t *, Stream *);
  void (*empty)        (ramsey_t *);
  void (*reset)        (ramsey_t *);
  void (*destroy)      (ramsey_t *);

  /* WARNING: when a filter is added to a ramsey_t, the
   *          ramsey_t assumes ownership of the filter
   *          and may modify or free it at its discretion. */
  int (*add_filter)  (ramsey_t *, filter_t *);
  int (*add_gap_set) (ramsey_t *, const ramsey_t *);
  int (*run_filters) (ramsey_t *);

  void (*recurse)       (ramsey_t *, global_data_t *);

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

struct _filter_cell {
  filter_t *data;
  struct _filter_cell *next;
};

struct _global_data {
  int max_iterations;
  int max_depth;

  bool prune_tree;
  int n_colors;
  int ap_length;
  int random_length;
  ramsey_t *alphabet;
  ramsey_t *gap_set;
  struct _filter_cell *filters;

  Stream *dump_stream;
  bool dump_iters;
  int dump_depth;
  ramsey_t *iters_data;

  int kill_now;

  Stream *out_stream;
  Stream *in_stream;
  Stream *err_stream;
};

int recursion_preamble (ramsey_t *rt, global_data_t *state);

#endif
