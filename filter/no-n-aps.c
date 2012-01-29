
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "no-3-aps.h"
#include "no-n-aps.h"
#include "filter.h"

struct _priv {
  filter_t parent;

  char name[50];
  int ap_length;
};

static int _check_recurse (const int *val, int len, int seek, int gap, int ap_len)
{
  if (ap_len == 0)
    return 1;
  else if (len == 0)
    return 0;
  else
    {
      int i = len - 1;
      for (i = len - 1; i >= 0; --i)
        if (val[i] == seek)
          return _check_recurse (val, i, val[i] - gap, gap, ap_len - 1);
      return 0;
    }
}

static bool check_n_ap (const filter_t *flt, const ramsey_t *rt)
{
  const struct _priv *priv = (struct _priv *) flt;
  int len = rt->get_length (rt);
  const int *val = rt->get_priv_data_const (rt);
  int i, gap, max_gap = 0;
  int min, max;
  bool found = 0;

  assert (val != NULL);

  if (priv->ap_length == 1 && len > 0)
    return 0;

  min = max = val[0];
  for (i = 0; i < len - 1; ++i)
    {
      if (val[i] < min)
        min = val[i];
      if (val[i] > max)
        max = val[i];
    }
  max_gap = (max - min + 1) / (priv->ap_length - 1) + 1;

  for (i = 0; i < len && !found; ++i)
    for (gap = -max_gap; gap <= max_gap && !found; ++gap)
      found = _check_recurse (val, i, val[i] - gap, gap, priv->ap_length - 1);

  return !found;
}

static bool cheap_check_n_ap (const filter_t *flt, const ramsey_t *rt)
{
  const struct _priv *priv = (struct _priv *) flt;
  int len = rt->get_length (rt);
  const int *val = rt->get_priv_data_const (rt);
  int i, gap;
  int min, max, max_gap = 0;
  bool found = 0;

  assert (val != NULL);

  min = max = val[0];
  for (i = 0; i < len - 1; ++i)
    {
      if (val[i] < min)
        min = val[i];
      if (val[i] > max)
        max = val[i];
    }
  max_gap = (max - min + 1) / (priv->ap_length - 1) + 1;

  for (gap = -max_gap; gap <= max_gap && !found; ++gap)
    found = _check_recurse (val, len, val[len - 1] - gap, gap, priv->ap_length - 1);

  return !found;
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
    case MODE_FULL:      flt->run  = check_n_ap; break;
    case MODE_LAST_ONLY: flt->run  = cheap_check_n_ap; break;
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

void *filter_n_ap_new (const global_data_t *state)
{
  const setting_t *ap_length_set = SETTING ("ap_length");
  if (ap_length_set == NULL)
    {
      fprintf (stderr, "Error: filter requires variable ``ap_length'' set.\n");
      return NULL;
    }
  if (ap_length_set->get_int_value (ap_length_set) == 3)
    return filter_3_ap_new (state);
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
      rv->run  = cheap_check_n_ap;
      return rv;
    }
}

