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

struct _priv {
  filter_t parent;

  const ramsey_t *gap_set;
  bool has_symmetry;
};

static bool cheap_check_gap_set (const filter_t *f, const ramsey_t *rt)
{
  struct _priv *priv = (struct _priv *) f;
  const ramsey_t *seq, *gaps;
  const int *data;
  int len;

  assert (f != NULL);
  assert (rt != NULL);

  /* Select the last-modified color, if applicable */
  if (rt->type == TYPE_COLORING)
    {
      const int *cols = rt->get_alt_priv_data_const (rt);
      int last_col = cols[rt->get_length (rt) - 1];
      seq = ((const ramsey_t **) rt->get_priv_data_const (rt)) [last_col];

      if (priv->gap_set->type == TYPE_COLORING)
        {
          if (priv->gap_set->get_n_cells (priv->gap_set) <= last_col)
            return 1;
          else
            gaps = ((const ramsey_t **) priv->gap_set->get_priv_data_const (priv->gap_set))[last_col];
        }
      else
        gaps = priv->gap_set;
    }
  else
    {
      seq = rt;
      if (priv->gap_set->type == TYPE_COLORING)
        gaps = ((const ramsey_t **) priv->gap_set->get_priv_data_const (priv->gap_set))[0];
      else
        gaps = priv->gap_set;
    }

  data = seq->get_priv_data_const (seq);
  len  = seq->get_length (seq);

  return (len < 2 || gaps->find_value (gaps, data[len - 1] - data[len - 2]));
}

/* end ACTUAL FILTER CODE */
static const char *_filter_get_type (const filter_t *flt)
{
  (void) flt;
  return "gap-set";
}

static bool _filter_get_symmetry (const filter_t *flt)
{
  return ((struct _priv *) flt)->has_symmetry;
}

static bool _filter_supports (const filter_t *flt, e_ramsey_type type)
{
  (void) flt;
  return type == TYPE_WORD ||
         type == TYPE_PERMUTATION ||
         type == TYPE_SEQUENCE ||
         type == TYPE_COLORING;
}

static bool _filter_set_mode (filter_t *flt, e_filter_mode mode)
{
  flt->mode = MODE_LAST_ONLY;
  flt->run  = cheap_check_gap_set;
  if (mode != MODE_LAST_ONLY)
    fprintf (stderr, "Warning: enabling full-check on unsupported filter ``%s''\n",
             flt->get_type (flt));
  return (mode == MODE_LAST_ONLY);
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

void *filter_gap_set_new (const global_data_t *state)
{
  struct _priv *priv;
  filter_t *rv;
  const setting_t *gap_set_set = SETTING ("gap_set");
  if (gap_set_set == NULL)
    {
      fprintf (stderr, "Error: filter requires variable ``gap_set'' set.\n");
      return NULL;
    }
  else if (gap_set_set->type != TYPE_RAMSEY)
    {
      fprintf (stderr, "Error: ``gap_set'' must be a sequence or coloring.\n");
      return NULL;
    }
  priv = malloc (sizeof *priv);
  priv->gap_set = gap_set_set->get_ramsey_value (gap_set_set);
  priv->gap_set = priv->gap_set->clone (priv->gap_set);   /* lol C */
  switch (priv->gap_set->type)
  {
  case TYPE_SEQUENCE:
    priv->has_symmetry = 1;
    break;
  case TYPE_COLORING:
    priv->has_symmetry = 0;
    break;
  default:
    free (priv);
    fprintf (stderr, "Error: ``gap_set'' must be a sequence or coloring.\n");
    return NULL;
  }
  rv = (filter_t *) priv;

  (void) state;
  rv->mode = MODE_LAST_ONLY;
  rv->destroy  = _filter_destroy;
  rv->clone    = _filter_clone;
  rv->get_type = _filter_get_type;
  rv->get_symmetry = _filter_get_symmetry;
  rv->supports = _filter_supports;
  rv->set_mode = _filter_set_mode;
  rv->run  = cheap_check_gap_set;
  return priv;
}

