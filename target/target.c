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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "target.h"

/* INSTALL TARGETS HERE */
#include "max-length.h"
#include "any-length.h"
static const parser_t g_target[] = {
  { "max_length", "Seek objects of maximal length.", target_max_length_new },
  { "any_length", "Output any valid object.",        target_any_length_new }
};
static const int g_n_targets = sizeof g_target / sizeof g_target[0];
/* end INSTALL TARGETS HERE */

data_collector_t *target_new (const char *data, const setting_list_t *vars)
{
  assert (data != NULL);
  if (data && *data)
    {
      int i;
      for (i = 0; i < g_n_targets; ++i)
        if (!strcmp (g_target[i].name, data))
          return g_target[i].construct (vars);
      fprintf (stderr, "Error: unknown target type ``%s''.\n", data);
    }
  return NULL;
}

void target_usage (stream_t *out)
{
  int i;
  stream_printf (out,
    "Usage: \n"
    "  target clear     Remove all targets\n"
    "  target <target>  Add a target\n"
    "\n"
    "Available targets:\n");
  for (i = 0; i < g_n_targets; ++i)
    stream_printf (out, "  %20s  %s\n", g_target[i].name, g_target[i].help);
}

