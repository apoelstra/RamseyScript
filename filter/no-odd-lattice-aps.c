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


#include <stdio.h>
#include <assert.h>

#include "filter.h"

/* No odd-length AP's (on lattices) */
static bool cheap_check_odd_lattice_ap (const filter_t *f, const ramsey_t *rt)
{
  int len = rt->get_maximum (rt);
  int wid = rt->get_n_cells (rt);
  int i;
  int gap;
  const int *val = rt->get_priv_data_const (rt);

  (void) f;

  /* Only AP's involving the latest addition */

  --len;  /* array is zero-indexed, so adjust len */

  /* Start pointing immediately down and right, then two down
   *  one right, three down one right, ... */
  for (gap = -wid + 1; len + 3*gap > 0; gap -= 4)
    for (i = 1; i < wid; ++i)
      {
        /* A change in color means, no an AP */
        if (val[len + i*gap] != val[len + (i + 1)*gap])
          continue;
        /* NO change in color means, FOUND an AP */
        return 0;
      }
  /* Start pointing immediately left, then one down one left,
   *  then two down one left, three down one left, ... */
  for (gap = -1; len + 3*gap >= 0; gap -= 4)
    for (i = 1; i < wid; ++i)
      {
        /* A change in color means, no an AP */
        if (val[len + i*gap] != val[len + (i + 1)*gap])
          continue;
        /* NO change in color means, FOUND an AP */
        return 0;
      }

  return 1;
}

/* end ACTUAL FILTER CODE */
static const char *_filter_get_type (const filter_t *flt)
{
  (void) flt;
  return "no-odd-lattice-aps";
}

static bool _filter_supports (const filter_t *flt, e_ramsey_type type)
{
  (void) flt;
  return type == TYPE_SEQUENCE ||
         type == TYPE_PERMUTATION ||
         type == TYPE_LATTICE;
}

static bool _filter_set_mode (filter_t *flt, e_filter_mode mode)
{
  flt->mode = MODE_LAST_ONLY;
  flt->run  = cheap_check_odd_lattice_ap;
  if (mode != MODE_LAST_ONLY)
    fprintf (stderr, "Warning: enabling full-check on unsupported filter ``%s''\n",
             flt->get_type (flt));
  return mode == MODE_LAST_ONLY;
}

/* CONSTRUCTOR / DESTRUCTOR  */
void *filter_odd_lattice_ap_new (const setting_list_t *vars)
{
  filter_t *rv = filter_new_generic ();
  (void) vars;
  rv->mode = MODE_LAST_ONLY;
  rv->get_type = _filter_get_type;
  rv->supports = _filter_supports;
  rv->set_mode = _filter_set_mode;
  rv->run  = cheap_check_odd_lattice_ap;

  return rv;
}

