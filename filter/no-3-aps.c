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
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "filter.h"

static bool check_3_ap (const filter_t *f, const ramsey_t *rt)
{
  int len = rt->get_length (rt);
  const int *val = rt->get_priv_data_const (rt);
  int i, j, k;

  assert (val != NULL);
  (void) f;

  for (i = 0; i < len; ++i)
    for (j = i + 1; j < len; ++j)
      for (k = j + 1; k < len; ++k)
        if (2 * val[j] == val[i] + val[k])
          return 0;
  return 1;
}

static bool cheap_check_3_ap (const filter_t *f, const ramsey_t *rt)
{
  int len = rt->get_length (rt);
  const int *val = rt->get_priv_data_const (rt);
  int i, j;

  assert (val != NULL);
  (void) f;

  for (i = 0; i < len; ++i)
    for (j = i + 1; j < len; ++j)
      if (2 * val[j] == val[i] + val[len - 1])
        return 0;
  return 1;
}

/* end ACTUAL FILTER CODE */
static const char *_filter_get_type (const filter_t *flt)
{
  (void) flt;
  return "no-3-aps";
}

static bool _filter_supports (const filter_t *flt, e_ramsey_type type)
{
  (void) flt;
  return type == TYPE_WORD ||
         type == TYPE_PERMUTATION ||
         type == TYPE_SEQUENCE;
}

static bool _filter_set_mode (filter_t *flt, e_filter_mode mode)
{
  flt->mode = mode;
  switch (mode)
    {
    case MODE_FULL:      flt->run  = check_3_ap; break;
    case MODE_LAST_ONLY: flt->run  = cheap_check_3_ap; break;
    }
  return 1;
}

/* CONSTRUCTOR / DESTRUCTOR  */
void *filter_3_ap_new (const setting_list_t *vars)
{
  filter_t *rv = filter_new_generic ();
  (void) vars;
  rv->mode = MODE_LAST_ONLY;
  rv->get_type = _filter_get_type;
  rv->supports = _filter_supports;
  rv->set_mode = _filter_set_mode;
  rv->run  = cheap_check_3_ap;
  return rv;
}

