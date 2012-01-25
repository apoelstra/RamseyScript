
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "global.h"
#include "filter.h"
#include "ramsey.h"

#define strmatch(s, r) (!strcmp ((s), (r)))

/* ACTUAL FILTER CODE */

/* No double-3-aps */
static bool check_sequence3 (const ramsey_t *rt)
{
  int len = rt->get_length (rt);
  const int *val = rt->get_priv_data_const (rt);
  int i, j;

  assert (val != NULL);

  if (len >= 3)
    for (i = 0; i < len; ++i)
      for (j = i + 2; j < len; j += 2)
        if (2 * val[(i + j)/2] == val[i] + val[j])
          return 0;
  return 1;
}

static bool cheap_check_sequence3 (const ramsey_t *rt)
{
  int  len = rt->get_length (rt);
  const int *val = rt->get_priv_data_const (rt);
  int i;

  assert (val != NULL);

  if (len >= 3)
    for (i = !(len % 2); i < len - 1; i += 2)
      if (2 * val[(i + len - 1)/2] == val[i] + val[len - 1])
        return 0;
  return 1;
}

/* No 3-AP's */
static bool check_3_ap (const ramsey_t *rt)
{
  int len = rt->get_length (rt);
  const int *val = rt->get_priv_data_const (rt);
  int i, j, k;

  assert (val != NULL);

  for (i = 0; i < len; ++i)
    for (j = i + 1; j < len; ++j)
      for (k = j + 1; k < len; ++k)
        if (2 * val[j] == val[i] + val[k])
          return 0;
  return 1;
}

static bool cheap_check_3_ap (const ramsey_t *rt)
{
  int len = rt->get_length (rt);
  const int *val = rt->get_priv_data_const (rt);
  int i, j;

  assert (val != NULL);

  for (i = 0; i < len; ++i)
    for (j = i + 1; j < len; ++j)
      if (2 * val[j] == val[i] + val[len - 1])
        return 0;
  return 1;
}

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

/* No odd-length AP's (on lattices) */
static bool cheap_check_odd_lattice_ap (const ramsey_t *rt)
{
  int len = rt->get_maximum (rt);
  int wid = rt->get_n_cells (rt);
  int i;
  int gap;
  const int *val = rt->get_priv_data_const (rt);

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
  switch (flt->type)
    {
    case FILTER_CUSTOM:             return "custom";
    case FILTER_NO_DOUBLE_3_AP:     return "no-double-3-aps";
    case FILTER_NO_ADDITIVE_SQUARE: return "no-additive-squares";
    case FILTER_NO_3_AP:            return "no-3-aps";
    case FILTER_NO_ODD_LATTICE_AP:  return "no-odd-lattice-aps";
    }
  return "unknown";
}

static bool _filter_supports (const filter_t *flt, e_ramsey_type type)
{
  switch (flt->type)
    {
    case FILTER_CUSTOM:
      return 1;
    case FILTER_NO_3_AP:
    case FILTER_NO_DOUBLE_3_AP:
      return (type == TYPE_SEQUENCE || type == TYPE_COLORING || type == TYPE_PERMUTATION);
    case FILTER_NO_ADDITIVE_SQUARE:
      return (type == TYPE_WORD);
    case FILTER_NO_ODD_LATTICE_AP:
      return (type == TYPE_SEQUENCE || type == TYPE_PERMUTATION || type == TYPE_LATTICE);
    }
  return 0;
}

static bool _filter_set_mode (filter_t *flt, e_filter_mode mode)
{
  switch (flt->type)
    {
    /* Only full-mode supported */
    /* Only cheap-mode supported */
    case FILTER_NO_ODD_LATTICE_AP:
      flt->mode = MODE_LAST_ONLY;
      flt->run  = cheap_check_odd_lattice_ap;
      if (mode != MODE_LAST_ONLY)
        fprintf (stderr, "Warning: enabling full-check on unsupported filter ``%s''\n",
                 flt->get_type (flt));
      return (mode == MODE_LAST_ONLY);
    case FILTER_NO_ADDITIVE_SQUARE:
      flt->mode = MODE_LAST_ONLY;
      flt->run  = cheap_check_additive_square;
      if (mode != MODE_LAST_ONLY)
        fprintf (stderr, "Warning: enabling full-check on unsupported filter ``%s''\n",
                 flt->get_type (flt));
      return (mode == MODE_LAST_ONLY);
    /* All modes supported */
    case FILTER_NO_DOUBLE_3_AP:
      flt->mode = mode;
      switch (mode)
        {
        case MODE_FULL:      flt->run  = check_sequence3; break;
        case MODE_LAST_ONLY: flt->run  = cheap_check_sequence3; break;
        }
      return 1;
    case FILTER_NO_3_AP:
      flt->mode = mode;
      switch (mode)
        {
        case MODE_FULL:      flt->run  = check_3_ap; break;
        case MODE_LAST_ONLY: flt->run  = cheap_check_3_ap; break;
        }
      return 1;
    case FILTER_CUSTOM:
      flt->mode = mode;
      return 1;
    }
  return 0;
}

/* CONSTRUCTOR / DESTRUCTOR  */

static filter_t *_filter_clone (const filter_t *flt)
{
  filter_t *rv = malloc (sizeof *rv);

  assert (flt != NULL);
  if (rv != NULL)
    memcpy (rv, flt, sizeof *flt);
  return rv;
}

static void _filter_destroy (filter_t *flt)
{
  free (flt);
}

filter_t *filter_new (const char *data)
{
  filter_t *rv = malloc (sizeof *rv);

  assert (data != NULL);
  if (rv == NULL)
    {
      fprintf (stderr, "filter_new: out of memory!\n");
      return NULL;
    }

  rv->mode = MODE_LAST_ONLY;
  rv->get_type = _filter_get_type;
  rv->supports = _filter_supports;
  rv->set_mode = _filter_set_mode;
  rv->clone    = _filter_clone;
  rv->destroy  = _filter_destroy;

  if (strmatch (data, "no_double_3_aps"))
    {
      rv->type = FILTER_NO_DOUBLE_3_AP;
      rv->run  = cheap_check_sequence3;
    }
  else if (strmatch (data, "no_additive_squares"))
    {
      rv->type = FILTER_NO_ADDITIVE_SQUARE;
      rv->run  = cheap_check_additive_square;
    }
  else if (strmatch (data, "no_3_aps"))
    {
      rv->type = FILTER_NO_3_AP;
      rv->run  = cheap_check_3_ap;
    }
  else if (strmatch (data, "no_odd_lattice_aps"))
    {
      rv->type = FILTER_NO_ODD_LATTICE_AP;
      rv->run  = cheap_check_odd_lattice_ap;
    }
  else
    {
      fprintf (stderr, "Warning: unrecognized filter ``%s''.\n", data);
      free (rv);
      rv = NULL;
    }

  return rv;
}

filter_t *filter_new_custom (bool (*run) (const ramsey_t *))
{
  filter_t *rv = malloc (sizeof *rv);
  if (rv != NULL)
    {
      rv->run = run;
      rv->type = FILTER_CUSTOM;
      rv->get_type = _filter_get_type;
      rv->supports = _filter_supports;
      rv->set_mode = _filter_set_mode;
      rv->clone    = _filter_clone;
      rv->destroy  = _filter_destroy;
    }
  return rv;
}


