/* RamseyScript
 * Written in 2012 by
 *   Andrew Poelstra <apoelstra@wpsoftware.net>
 *
 * To the extent possible under law, the author(s) have dedicated all
 * copyright and related and neighboring rights to this software to
 * the public domain worldwide. This software is distributed without
 * any warranty.
 *
 * You should have received a copy of the CC0 Public Domain Dedication
 * along with this software.
 * If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
 */


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

void dump_usage (stream_t *out)
{
  int i;
  stream_printf (out,
    "Usage: \n"
    "  dump clear   Remove all data dumps\n"
    "  dump <dump>  Add a data dump\n"
    "\n"
    "Available dump types:\n");
  for (i = 0; i < g_n_dumps; ++i)
    stream_printf (out, "  %20s  %s\n", g_dump[i].name, g_dump[i].help);
}


