#ifndef GLOBAL_H
#define GLOBAL_H

typedef struct _ramsey_t ramsey_t;
typedef struct _global_data global_data_t;
typedef struct _dump_t dump_t;
typedef int bool;

typedef enum _e_ramsey_type {
  TYPE_SEQUENCE,
  TYPE_COLORING,
  TYPE_WORD,
  TYPE_PERMUTATION
} e_ramsey_type;

typedef enum _e_setting_type {
  TYPE_STRING,
  TYPE_INTEGER,
  TYPE_RAMSEY
} e_setting_type;

#include "dump.h"
#include "filter.h"
#include "setting.h"
#include "stream.h"

#define VERSION "0.3"
#define SETTING(s) (state->settings->get_setting (state->settings, (s)))

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

typedef struct _filter_list {
  filter_t *data;
  struct _filter_list *next;
} filter_list;

typedef struct _dump_list {
  dump_t *data;
  struct _dump_list *next;
} dump_list;

struct _global_data {
  setting_list_t *settings;

  filter_list *filters;
  dump_list   *dumps;

  bool interactive;
  bool kill_now;   /* used in threaded implementations */

  stream_t *out_stream;
  stream_t *in_stream;
  stream_t *err_stream;
};

int recursion_preamble (ramsey_t *rt, global_data_t *state);

#endif
