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
