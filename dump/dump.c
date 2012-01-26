
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "dump.h"

/* INSTALL DUMPS HERE */
#include "iters-per-length.h"
const parser_t g_dump[] = {
  { "iterations_per_length", "Output number of iterations spent on each object length.",
    dump_iters_per_length_new },
  { "iters_per_length",      "Synonym of ``iterations-per-length''.",
    dump_iters_per_length_new }
};
const int g_n_dumps = sizeof g_dump / sizeof g_dump[0];
/* end INSTALL DUMPS HERE */

data_collector_t *dump_new (const char *data, const global_data_t *state)
{
  assert (data != NULL);
  if (data && *data)
    {
      int i;
      for (i = 0; i < g_n_dumps; ++i)
        if (!strcmp (g_dump[i].name, data))
          return g_dump[i].construct (state);
      fprintf (stderr, "Error: unknown dump type ``%s''.\n", data);
    }
  return NULL;
}

