#ifndef FILTER_H
#define FILTER_H

#include "../global.h"
#include "../ramsey/ramsey.h"

typedef enum _e_filter_mode {
  MODE_FULL,
  MODE_LAST_ONLY
} e_filter_mode;

struct _filter_t {
  e_filter_mode mode;

  const char *(*get_type) (const filter_t *);

  bool (*run)      (const ramsey_t *);

  bool (*supports) (const filter_t *, e_ramsey_type);
  bool (*set_mode) (filter_t *, e_filter_mode);
  filter_t *(*clone) (const filter_t *);
  void (*destroy)  (filter_t *);
};

filter_t *filter_new (const char *, const global_data_t *);
filter_t *filter_new_custom (bool (*run) (const ramsey_t *));
filter_t *filter_new_generic (void);
void filter_usage (stream_t *out);

#endif
