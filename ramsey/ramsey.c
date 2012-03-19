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
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "ramsey.h"

/* INSTALL OBJECTS HERE */
#include "coloring.h"
#include "lattice.h"
#include "permutation.h"
#include "sequence.h"
#include "word.h"
const parser_t g_ramsey[] = {
  { "colorings",   "Partitions of [1, N] for increasing N.",   coloring_new },
  { "partitions",  "Synonym of ``coloring''.",                 coloring_new },
  { "lattices",    "2D lattice of colored points.",            lattice_new },
  { "permutations","Permutations of [1, N] for increasing N.", permutation_new },
  { "sequences",   "Strictly increasing sequences.",           sequence_new },
  { "words",       "Sequences of words on a given alphabet.",  word_new },
};
const int g_n_ramseys = sizeof g_ramsey / sizeof g_ramsey[0];
/* end INSTALL OBJECTS HERE */

ramsey_t *ramsey_new (const char *data, const setting_list_t *vars)
{
  assert (data != NULL);
  if (data && *data)
    {
      int i;
      for (i = 0; i < g_n_ramseys; ++i)
        if (!strcmp (g_ramsey[i].name, data))
          return g_ramsey[i].construct (vars);
      fprintf (stderr, "Error: unknown ramsey object type ``%s''.\n", data);
    }
  return NULL;
}

ramsey_t *ramsey_new_from_parse (const char *data)
{
  ramsey_t *rv = NULL;
  const char *scan = data;

  /* Skip whitespace */
  while (*scan && isspace (*scan))
    ++scan;

  if (*scan != '[')
    {
      fprintf (stderr, "Error: ``%s'' is not a valid Ramsey object (does not start with '[').\n", data);
      return NULL;
    }

  /* Skip '[' and more whitespace */
  ++scan;
  while (*scan && isspace (*scan))
    ++scan;

  /* Do we have a sequence? */
  if (isdigit (*scan) || *scan == '-')
    rv = sequence_new_direct ();
  /* A coloring? */
  else if (*scan == '[')
    {
      int n_colors = 0;
      while (*scan)
        {
          if (*scan == '[')
            ++n_colors;
          ++scan;
        }
      rv = coloring_new_direct (n_colors);
    }
  else
    {
      fprintf (stderr, "Error: ``%s'' is not a valid Ramsey object (could not be parsed).\n", data);
      return NULL;
    }

  if (rv)
    rv->parse (rv, data);
  return rv;
}

void ramsey_usage (stream_t *out)
{
  int i;
  stream_printf (out,
    "Usage: \n"
    "  search <space>  Recursively explore a space of Ramsey objects\n"
    "\n"
    "Available spaces:\n");
  for (i = 0; i < g_n_ramseys; ++i)
    stream_printf (out, "  %20s  %s\n", g_ramsey[i].name, g_ramsey[i].help);
}

