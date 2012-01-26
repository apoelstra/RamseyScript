
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "filter.h"

/* INSTALL FILTERS HERE */
#include "no-double-3-aps.h"
#include "no-additive-squares.h"
#include "no-3-aps.h"
#include "no-odd-lattice-aps.h"
const parser_t g_filter[] = {
  { "no_double_3_aps",     "Eliminate objects containing double 3-AP's.",  filter_double_3_ap_new },
  { "no_additive_squares", "Eliminate words containing additive squares.", filter_additive_square_new },
  { "no_3_aps",            "Eliminate objects containing 3-AP's.",         filter_3_ap_new },
  { "no_odd_lattice_aps",  "Eliminate lattices containing monochromatic lines.", filter_odd_lattice_ap_new }
};
const int g_n_filters = sizeof g_filter / sizeof g_filter[0];
/* end INSTALL FILTERS HERE */


/* Custom filter functions */
static const char *_filter_custom_get_type (const filter_t *flt)
{
  (void) flt;
  return "(unnamed)";
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

filter_t *filter_new (const char *data, const global_data_t *state)
{
  assert (data != NULL);
  if (data && *data)
    {
      int i;
      for (i = 0; i < g_n_filters; ++i)
        if (!strcmp (g_filter[i].name, data))
          return g_filter[i].construct (state);
      fprintf (stderr, "Error: unknown filter ``%s''.\n", data);
    }

  return NULL;
}

filter_t *filter_new_custom (bool (*run) (const ramsey_t *))
{
  filter_t *rv = filter_new_generic ();
  if (rv != NULL)
    {
      rv->run = run;
      rv->get_type = _filter_custom_get_type;
      rv->supports = _filter_custom_supports;
      rv->set_mode = _filter_custom_set_mode;
      rv->clone    = _filter_clone;
      rv->destroy  = _filter_destroy;
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


