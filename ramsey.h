#ifndef RAMSEY_H
#define RAMSEY_H

#include "global.h"
#include "stream.h"

struct _global_data *set_defaults (stream_t *in, stream_t *out, stream_t *err);
void process (struct _global_data *state);

struct _ramsey_t {
  e_ramsey_type type;

  /* Recursion state variables */
  int r_depth;
  int r_iterations;
  int r_max_depth;
  int r_max_iterations;
  int r_max_found;
  int r_prune_tree;
  const ramsey_t *r_gap_set;
  const ramsey_t *r_alphabet;

  /* vtable */
  const char *(*get_type) (const ramsey_t *);

  const char *(*parse) (ramsey_t *, const char *data);
  void (*randomize)    (ramsey_t *, int);
  void (*print)        (const ramsey_t *, stream_t *);
  void (*empty)        (ramsey_t *);
  void (*reset)        (ramsey_t *);
  void (*destroy)      (ramsey_t *);

  /* WARNING: when a filter is added to a ramsey_t, the
   *          ramsey_t assumes ownership of the filter
   *          and may modify or free it at its discretion. */
  int (*add_filter)  (ramsey_t *, filter_t *);
  int (*add_gap_set) (ramsey_t *, const ramsey_t *);
  int (*run_filters) (const ramsey_t *);

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

#endif
