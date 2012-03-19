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

/* No additive squares */
static bool cheap_check_additive_square (const filter_t *f, const ramsey_t *rt)
{
  int  len = rt->get_length (rt);
  const int *val = rt->get_priv_data_const (rt);
  int i, sum1, sum2;

  (void) f;

  for (i = 1; i <= len / 2; ++i)
    {
      int j;
      sum1 = sum2 = 0;
      for (j = 1; j <= i; ++j)
        {
          sum1 += val[len - j];
          sum2 += val[len - i - j];
        }
      if (sum1 == sum2)
        return 0;
    }
  return 1;
}

static const char *_filter_get_type (const filter_t *flt)
{
  (void) flt;
  return "no-additive-squares";
}

static bool _filter_supports (const filter_t *flt, e_ramsey_type type)
{
  (void) flt;
  return type == TYPE_WORD;
}

static bool _filter_set_mode (filter_t *flt, e_filter_mode mode)
{
  flt->mode = MODE_LAST_ONLY;
  flt->run  = cheap_check_additive_square;
  if (mode != MODE_LAST_ONLY)
    fprintf (stderr, "Warning: enabling full-check on unsupported filter ``%s''\n",
             flt->get_type (flt));
  return mode == MODE_LAST_ONLY;
}

/* CONSTRUCTOR / DESTRUCTOR  */
void *filter_additive_square_new (const setting_list_t *vars)
{
  filter_t *rv = filter_new_generic ();
  (void) vars;
  rv->mode = MODE_LAST_ONLY;
  rv->get_type = _filter_get_type;
  rv->supports = _filter_supports;
  rv->set_mode = _filter_set_mode;
  rv->run  = cheap_check_additive_square;

  return rv;
}

