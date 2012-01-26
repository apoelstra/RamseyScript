
#include <stdio.h>
#include <assert.h>

#include "filter.h"

/* No additive squares */
static bool cheap_check_additive_square (const ramsey_t *rt)
{
  int  len = rt->get_length (rt);
  const int *val = rt->get_priv_data_const (rt);
  int i, sum1, sum2;

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
void *filter_additive_square_new (const global_data_t *state)
{
  filter_t *rv = filter_new_generic ();
  (void) state;
  rv->mode = MODE_LAST_ONLY;
  rv->get_type = _filter_get_type;
  rv->supports = _filter_supports;
  rv->set_mode = _filter_set_mode;
  rv->run  = cheap_check_additive_square;

  return rv;
}

