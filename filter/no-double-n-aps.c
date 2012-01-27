
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "no-double-3-aps.h"
#include "no-double-n-aps.h"
#include "filter.h"

struct _priv {
  filter_t parent;

  char name[30];
  int ap_length;
};

static bool check_double_n_ap (const filter_t *flt, const ramsey_t *rt)
{
  const struct _priv *priv = (struct _priv *) flt;
  int len = rt->get_length (rt);
  const int *val = rt->get_priv_data_const (rt);
  int i, j, gap;

  assert (val != NULL);

  if (priv->ap_length == 1 && len > 0)
    return 0;

  for (i = 0; i < len; ++i)
    for (gap = 1; i + (priv->ap_length - 1) * gap < len; ++gap)
      for (j = 0; j < priv->ap_length - 1; ++j)
        {
          int val_gap = val[i] - val[i + gap];
          if (val[i + gap*j] - val[i + gap*(j+1)] != val_gap)
            continue;
          return 0;
        }
  return 1;
}

static bool cheap_check_double_n_ap (const filter_t *flt, const ramsey_t *rt)
{
  const struct _priv *priv = (struct _priv *) flt;
  int len = rt->get_length (rt);
  const int *val = rt->get_priv_data_const (rt);
  int gap, j;

  assert (val != NULL);

  if (priv->ap_length == 1 && len > 0)
    return 0;

  for (gap = 1; (priv->ap_length - 1) * gap < len; ++gap)
    {
      int val_gap = val[len - 1] - val[len - 1 - gap];
      for (j = 0; j < priv->ap_length - 1; ++j)
        {
/*printf ("Comparing %d to %d (idx_gap %d val_gap %d j %d len %d)\n", val[len - 1 - gap*j], val[len - 1 - gap*(j+1)], gap, val_gap, j, len);*/
          if (val[len - 1 - gap*j] - val[len - 1 - gap*(j+1)] != val_gap)
            goto not_an_n_ap;
/*printf ("FOUND %d-AP, failing\n", priv->ap_length);*/
        }
      return 0;
not_an_n_ap:;
    }
  return 1;
}

/* end ACTUAL FILTER CODE */
static const char *_filter_get_type (const filter_t *flt)
{
  struct _priv *priv = (struct _priv *) flt;
  return priv->name;
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
    case MODE_FULL:      flt->run  = check_double_n_ap; break;
    case MODE_LAST_ONLY: flt->run  = cheap_check_double_n_ap; break;
    }
  return 1;
}

/* CONSTRUCTOR / DESTRUCTOR  */
static filter_t *_filter_clone (const filter_t *flt)
{
  struct _priv *rv = malloc (sizeof *rv);
  assert (flt != NULL);
  if (rv != NULL)
    memcpy (rv, flt, sizeof *rv);
  return (filter_t *) rv;
}

static void _filter_destroy (filter_t *flt)
{
  free (flt);
}

void *filter_double_n_ap_new (const global_data_t *state)
{
  const setting_t *ap_length_set = SETTING ("ap_length");
  if (ap_length_set == NULL)
    {
      fprintf (stderr, "Error: filter requires variable ``ap_length'' set.\n");
      return NULL;
    }
  if (ap_length_set->get_int_value (ap_length_set) == 3)
    return filter_double_3_ap_new (state);
  else
    {
      struct _priv *priv = malloc (sizeof *priv);
      filter_t *rv = (filter_t *) priv;

      priv->ap_length = ap_length_set->get_int_value (ap_length_set);
      sprintf (priv->name, "no-%d-aps", priv->ap_length);

      rv->mode = MODE_LAST_ONLY;
      rv->destroy  = _filter_destroy;
      rv->clone    = _filter_clone;
      rv->get_type = _filter_get_type;
      rv->supports = _filter_supports;
      rv->set_mode = _filter_set_mode;
      rv->run  = cheap_check_double_n_ap;
      return rv;
    }
}

