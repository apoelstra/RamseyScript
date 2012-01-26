#ifndef GLOBAL_H
#define GLOBAL_H

typedef struct _parser_t parser_t;
typedef struct _ramsey_t ramsey_t;
typedef struct _filter_t filter_t;
typedef struct _data_collector_t data_collector_t;
typedef struct _global_data global_data_t;
typedef int bool;

typedef enum _e_ramsey_type {
  TYPE_SEQUENCE,
  TYPE_COLORING,
  TYPE_WORD,
  TYPE_PERMUTATION,
  TYPE_LATTICE
} e_ramsey_type;

typedef enum _e_setting_type {
  TYPE_STRING,
  TYPE_INTEGER,
  TYPE_RAMSEY
} e_setting_type;

#include "dump/dump.h"
#include "filter/filter.h"
#include "setting.h"
#include "stream.h"

#define VERSION "0.3"
#define SETTING(s) (state->settings->get_setting (state->settings, (s)))

struct _parser_t {
  const char *name;
  const char *help;
  void *(*construct) (const global_data_t *);
};

struct _data_collector_t {
  const char *(*get_type) (const data_collector_t *);

  void (*reset)   (data_collector_t *);
  void (*record)  (data_collector_t *, ramsey_t *);
  void (*output)  (const data_collector_t *);
  void (*destroy) (data_collector_t *);
};

typedef struct _filter_list {
  filter_t *data;
  struct _filter_list *next;
} filter_list;

typedef struct _dc_list {
  data_collector_t *data;
  struct _dc_list *next;
} dc_list;

struct _global_data {
  setting_list_t *settings;

  filter_list *filters;
  dc_list     *dumps, *targets;

  bool interactive;
  bool kill_now;   /* used in threaded implementations */

  stream_t *out_stream;
  stream_t *in_stream;
  stream_t *err_stream;
};

int recursion_preamble (ramsey_t *rt, global_data_t *state);

#endif
