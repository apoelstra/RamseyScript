#ifndef DUMP_H
#define DUMP_H

#include <stddef.h>

#include "global.h"
#include "stream.h"

struct _dump_t {
  const char *(*get_type) (const dump_t *);

  void (*reset)   (dump_t *);
  void (*record)  (dump_t *, ramsey_t *);
  void (*output)  (const dump_t *);
  void (*destroy) (dump_t *);
};

dump_t *dump_new (const char *data, size_t size, stream_t *out);

#endif
