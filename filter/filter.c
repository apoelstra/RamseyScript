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

/* INSTALL FILTERS HERE */
#include "gap-set.h"
#include "no-double-3-aps.h"
#include "no-double-n-aps.h"
#include "no-additive-squares.h"
#include "no-3-aps.h"
#include "no-n-aps.h"
#include "no-odd-lattice-aps.h"
#include "no-rainbow-aps.h"
#include "no-schur-solutions.h"
static const parser_t g_filter[] = {
  { "gap_set",             "Eliminate sequences whose gaps are outside of gap_set.", filter_gap_set_new },
  { "no_double_3_aps",     "Eliminate objects containing double 3-AP's.",  filter_double_3_ap_new },
  { "no_double_n_aps",     "Eliminate objects containing double n-AP's, with n == ap_length.",
    filter_double_n_ap_new },
  { "no_additive_squares", "Eliminate words containing additive squares.", filter_additive_square_new },
  { "no_3_aps",            "Eliminate objects containing 3-AP's.",         filter_3_ap_new },
  { "no_n_aps",            "Eliminate objects containing n-AP's, with n == ap_length", filter_n_ap_new },
  { "no_odd_lattice_aps",  "Eliminate lattices containing monochromatic lines.", filter_odd_lattice_ap_new },
  { "no_rainbow_aps",      "Eliminate colorings containing rainbow AP's.", filter_rainbow_ap_new },
  { "no_schur_solutions",  "Eliminate objects with solutions to X + Y = Z.", filter_schur_new }
};
static const int g_n_filters = sizeof g_filter / sizeof g_filter[0];
/* end INSTALL FILTERS HERE */


/*! \brief Private data for single-function filters. */
struct _custom_priv {
  /*! \brief parent struct. */
  filter_t parent;
  /*! \brief The user-visible name of the filter. */
  const char *name;
};

/* Generic filter functions */ 
static bool _filter_get_symmetry (const filter_t *flt)
{
  (void) flt;
  return 1;
}

/* Custom filter functions */
static const char *_filter_custom_get_type (const filter_t *flt)
{
  struct _custom_priv *priv = (struct _custom_priv *) flt;
  return priv->name;
}

static bool _filter_custom_supports (const filter_t *flt, e_ramsey_type type)
{
  (void) flt;
  (void) type;
  return 1;
}

static bool _filter_custom_set_mode (filter_t *flt, e_filter_mode mode)
{
  flt->mode = mode;
  return 1;
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

filter_t *filter_new (const char *data, const setting_list_t *vars)
{
  assert (data != NULL);
  if (data && *data)
    {
      int i;
      for (i = 0; i < g_n_filters; ++i)
        if (!strcmp (g_filter[i].name, data))
          return g_filter[i].construct (vars);
      fprintf (stderr, "Error: unknown filter ``%s''.\n", data);
    }

  return NULL;
}

filter_t *filter_new_custom (const char *name,
                             bool (*run) (const filter_t *f, const ramsey_t *))
{
  struct _custom_priv *priv = malloc (sizeof *priv);
  filter_t *rv = (filter_t *) priv;
  if (rv != NULL)
    {
      rv->run = run;
      rv->get_symmetry = _filter_get_symmetry;
      rv->get_type = _filter_custom_get_type;
      rv->supports = _filter_custom_supports;
      rv->set_mode = _filter_custom_set_mode;
      rv->clone    = _filter_clone;
      rv->destroy  = _filter_destroy;
      priv->name = name;
    }
  return rv;
}

filter_t *filter_new_generic ()
{
  filter_t *rv = malloc (sizeof *rv);
  if (rv == NULL)
    {
      fprintf (stderr, "filter_new: out of memory!\n");
      return NULL;
    }

  rv->get_symmetry = _filter_get_symmetry;
  rv->clone   = _filter_clone;
  rv->destroy = _filter_destroy;
  return rv;
}

void filter_usage (stream_t *out)
{
  int i;
  stream_printf (out,
    "Usage: \n"
    "  filter clear      Remove all filters\n"
    "  filter <filter>   Add a filter\n"
    "\n"
    "Available filters:\n");
  for (i = 0; i < g_n_filters; ++i)
    stream_printf (out, "  %20s  %s\n", g_filter[i].name, g_filter[i].help);
}


