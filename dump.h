#ifndef DUMP_H
#define DUMP_H

#include <stddef.h>

#include "global.h"
#include "stream.h"

data_collector_t *dump_new (const char *data, size_t size, stream_t *out);

#endif
