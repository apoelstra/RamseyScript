
#include <stdlib.h>
#include <assert.h>

#include "filter.h"

/* No double-3-aps */
static bool check_sequence3 (const filter_t *f, const ramsey_t *rt)
{
  int len = rt->get_length (rt);
  const int *val = rt->get_priv_data_const (rt);
  int i, j;

  assert (val != NULL);
  (void) f;

  if (len >= 3)
    for (i = 0; i < len; ++i)
      for (j = i + 2; j < len; j += 2)
        if (2 * val[(i + j)/2] == val[i] + val[j])
          return 0;
  return 1;
}

static bool cheap_check_sequence3 (const filter_t *f, const ramsey_t *rt)
{
  int  len = rt->get_length (rt);
  const int *val = rt->get_priv_data_const (rt);
  int i;

  assert (val != NULL);
  (void) f;

  if (len >= 3)
    for (i = !(len % 2); i < len - 1; i += 2)
      if (2 * val[(i + len - 1)/2] == val[i] + val[len - 1])
        return 0;
  return 1;
}

static const char *_filter_get_type (const filter_t *flt)
{
  (void) flt;
  return "no-double-3-aps";
}

static bool _filter_supports (const filter_t *flt, e_ramsey_type type)
{
  (void) flt;
  return type == TYPE_SEQUENCE ||
         type == TYPE_COLORING ||
         type == TYPE_PERMUTATION;
}

static bool _filter_set_mode (filter_t *flt, e_filter_mode mode)
{
  flt->mode = mode;
  switch (mode)
    {
    case MODE_FULL:      flt->run  = check_sequence3; break;
    case MODE_LAST_ONLY: flt->run  = cheap_check_sequence3; break;
    }
  return 1;
}

/* CONSTRUCTOR / DESTRUCTOR  */
void *filter_double_3_ap_new (const global_data_t *state)
{
  filter_t *rv = filter_new_generic ();
  (void) state;
  rv->mode = MODE_LAST_ONLY;
  rv->get_type = _filter_get_type;
  rv->supports = _filter_supports;
  rv->set_mode = _filter_set_mode;
  rv->run  = cheap_check_sequence3;

  return rv;
}

