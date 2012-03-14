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


#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "filter.h"

static bool check_rainbow (const filter_t *f, const ramsey_t *rt)
{
  int ap_length  = rt->get_n_cells (rt);
  int col_length = rt->get_length (rt);
  const int *col = rt->get_alt_priv_data_const (rt);
  int i, j, k;

  int *count = malloc (ap_length * sizeof *count);

  assert (f);
  assert (rt && rt->type == TYPE_COLORING);

  /* Loop over all AP's */
  /* loop i over starting points */
  for (i = 0; i < col_length; ++i)
    /* loop j over gap sizes */
    for (j = 1; i + (ap_length - 1) * j < col_length; ++j)
      {
        memset (count, 0, ap_length * sizeof *count);
        /* loop k over elements of AP, counting colors */
        for (k = 0; k < ap_length; ++k)
          if (!count[col[i + k * j]])
            ++count[col[i + k * j]];
          else
            /* not a rainbow AP */
            break;
        /* did we get through the entire AP without seeing a color twice? */
        if (k == ap_length)
          {
            free (count);
            return 0;
          }
      }

  free (count);
  return 1;
}

static bool cheap_check_rainbow (const filter_t *f, const ramsey_t *rt)
{
  return check_rainbow (f, rt);
}

static const char *_filter_get_type (const filter_t *flt)
{
  (void) flt;
  return "no-rainbow-aps";
}

static bool _filter_supports (const filter_t *flt, e_ramsey_type type)
{
  (void) flt;
  return type == TYPE_COLORING;
}

static bool _filter_set_mode (filter_t *flt, e_filter_mode mode)
{
  flt->mode = mode;
  switch (mode)
    {
    case MODE_FULL:      flt->run  = check_rainbow; break;
    case MODE_LAST_ONLY: flt->run  = cheap_check_rainbow; break;
    }
  return 1;
}

/* CONSTRUCTOR / DESTRUCTOR  */
void *filter_rainbow_ap_new (const global_data_t *state)
{
  filter_t *rv = filter_new_generic ();
  (void) state;
  rv->mode = MODE_LAST_ONLY;
  rv->get_type = _filter_get_type;
  rv->supports = _filter_supports;
  rv->set_mode = _filter_set_mode;
  rv->run  = cheap_check_rainbow;

  return rv;
}

