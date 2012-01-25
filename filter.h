#ifndef FILTER_H
#define FILTER_H

#include "global.h"

typedef struct _filter_t filter_t;

typedef enum _e_filter_type {
  FILTER_NO_3_AP,
  FILTER_NO_DOUBLE_3_AP,
  FILTER_NO_ADDITIVE_SQUARE,
  FILTER_NO_ODD_LATTICE_AP,
  FILTER_CUSTOM
} e_filter_type;

typedef enum _e_filter_mode {
  MODE_FULL,
  MODE_LAST_ONLY
} e_filter_mode;

struct _filter_t {
  e_filter_type type;
  e_filter_mode mode;

  const char *(*get_type) (const filter_t *);

  bool (*run)      (const ramsey_t *);

  bool (*supports) (const filter_t *, e_ramsey_type);
  bool (*set_mode) (filter_t *, e_filter_mode);
  filter_t *(*clone) (const filter_t *);
  void (*destroy)  (filter_t *);
};

filter_t *filter_new (const char *);
filter_t *filter_new_custom (bool (*run) (const ramsey_t *));

#endif
