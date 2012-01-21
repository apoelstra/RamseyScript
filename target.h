#ifndef TARGET_H
#define TARGEt_H

#include <stddef.h>

#include "global.h"
#include "stream.h"

data_collector_t *target_new (const char *data, stream_t *out);

#endif
