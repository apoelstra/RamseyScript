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

/*! \file lattice.c
 *  \brief Implementation of the lattice type.
 *
 *  A lattice is a 2D array of numbers, which is traversed lengthwise
 *  and upward, like
 *  8 9 ...
 *  4 5 6 7
 *  0 1 2 3
 *
 *  They are used to give a geometric interpretation of various Ramsey
 *  properties.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "ramsey.h"
#include "lattice.h"

/*! \brief Default allocation size for lattices. */
#define DEFAULT_MAX_LENGTH	400
/*! \brief Default number of filters. */
#define DEFAULT_MAX_FILTERS	20

/*! \brief Private data for the lattice type. */
struct _lattice {
  /*! \brief parent struct */
  ramsey_t parent;

  /*! \brief List of filters set on the lattice. */
  filter_t **filter;
  /*! \brief Number of filters set. */
  int n_filters;
  /*! \brief Number of filters allocated. */
  int max_filters;

  /*! \brief Content of the lattice. */
  int *value;
  /*! \brief Maximum length of the lattice without requiring reallocation. */
  int max_value;
  /*! \brief Length of the lattice. */
  int top_value;
  /*! \brief Number of columns wide the lattice is. */
  int n_columns;
  /*! \brief Number of colors used for the lattice. */
  int n_colors;
};

static const char *_lattice_get_type (const ramsey_t *rt)
{
  assert (rt && rt->type == TYPE_LATTICE);
  return "lattice";
}

static const ramsey_t *_lattice_find_value (const ramsey_t *rt, int value)
{
  struct _lattice *lat = (struct _lattice *) rt;
  assert (rt && rt->type == TYPE_LATTICE);
  return (value > 0 && value < lat->top_value) ? rt : NULL;
}

/* FILTERS */
static int _lattice_run_filters (const ramsey_t *rt)
{
  const struct _lattice *lat = (struct _lattice *) rt;
  int i;

  assert (rt && rt->type == TYPE_LATTICE);

  for (i = 0; i < lat->n_filters; ++i)
    if (!lat->filter[i]->run (lat->filter[i], rt))
      return 0;
  return 1;
}

static int _lattice_add_filter (ramsey_t *rt, filter_t *f)
{
  struct _lattice *lat = (struct _lattice *) rt;

  assert (f);
  assert (rt && rt->type == TYPE_LATTICE);

  if (!f->supports (f, rt->type))
    {
      fprintf (stderr, "Warning: filter ``%s'' does not support ``%s''\n",
               f->get_type (f), rt->get_type (rt));
      return 0;
    }

  if (lat->n_filters == lat->max_filters)
    {
      void *new_alloc = realloc (lat->filter, 2 * lat->max_filters);
      if (new_alloc == NULL)
        return 0;
      lat->filter = new_alloc;
      lat->max_filters *= 2;
    }

  f->set_mode (f, MODE_LAST_ONLY);
  lat->filter[lat->n_filters++] = f;
  return 1;
}

/* RECURSION */
static void _lattice_recurse (global_data_t *state)
{
  int i;
  struct _lattice *lat = (struct _lattice *) state->seed;

  assert (state != NULL);
  assert (state->seed && state->seed->type == TYPE_LATTICE);

  if (!recursion_preamble (state))
    return;

  for (i = 1; i <= lat->n_colors; ++i)
    {
      state->seed->append (state->seed, i);
      state->seed->recurse (state);
      state->seed->deappend (state->seed);
    }

  recursion_postamble (state->seed);
}

/* PRINT / PARSE */
static void _lattice_print (const ramsey_t *rt, stream_t *out)
{
  struct _lattice *lat = (struct _lattice *) rt;
  int i, j;
  assert (rt && rt->type == TYPE_LATTICE);

  stream_printf (out, "Lattice of %d columns on %d colors:\n\n", 
                      lat->n_columns, lat->n_colors);

  for (i = (lat->top_value - 1) / lat->n_columns; i >= 0; --i)
    {
      stream_printf (out, "    ");
      for (j = i * lat->n_columns; j < lat->top_value && j < (i + 1) * lat->n_columns; ++j)
        stream_printf (out, "%2d ", lat->value[j]);
      stream_printf (out, "\n");
    }
}

static const char *_lattice_parse (ramsey_t *rt, const char *data)
{
  assert (rt && rt->type == TYPE_LATTICE);
  fprintf (stderr, "Warning: parse() unimplemented for ``%s''\n",
           rt->get_type (rt));
  return data;
}

static void _lattice_randomize (ramsey_t *rt, int n)
{
  (void) n;
  fprintf (stderr, "Warning: randomize() unimplemented for ``%s''.\n",
           rt->get_type (rt));
}

/* ACCESSORS */
static int _lattice_get_length (const ramsey_t *rt)
{
  struct _lattice *lat = (struct _lattice *) rt;
  assert (rt && rt->type == TYPE_LATTICE);
  return lat->top_value / lat->n_columns;
}

static int _lattice_get_maximum (const ramsey_t *rt)
{
  struct _lattice *lat = (struct _lattice *) rt;
  assert (rt && rt->type == TYPE_LATTICE);
  return lat->top_value;
}

static int _lattice_get_n_cells (const ramsey_t *rt)
{
  struct _lattice *lat = (struct _lattice *) rt;
  assert (rt && rt->type == TYPE_LATTICE);
  return lat->n_columns;
}

static void *_lattice_get_priv_data (ramsey_t *rt)
{
  struct _lattice *lat = (struct _lattice *) rt;
  assert (rt && rt->type == TYPE_LATTICE);
  return lat->value;
}

static const void *_lattice_get_priv_data_const (const ramsey_t *rt)
{
  const struct _lattice *lat = (struct _lattice *) rt;
  assert (rt && rt->type == TYPE_LATTICE);
  return lat->value;
}

/* APPEND / DEAPPEND */
static int _lattice_append (ramsey_t *rt, int value)
{
  struct _lattice *lat = (struct _lattice *) rt;
  assert (rt && rt->type == TYPE_LATTICE);

  if (lat->top_value >= lat->max_value)
    {
      void *re = realloc (lat->value, 2 * lat->max_value * sizeof *lat->value);
      if (re == NULL)
        return 0;
      lat->max_value *= 2;
      lat->value = re;
    }

  lat->value[lat->top_value++] = value;
  return 1;
}

static int _lattice_deappend (ramsey_t *rt)
{
  struct _lattice *lat = (struct _lattice *) rt;
  assert (rt && rt->type == TYPE_LATTICE);
  if (lat->top_value)
    --lat->top_value;
  else
    return 0;
  return 1;
}

static int _lattice_cell_append (ramsey_t *rt, int value, int cell)
{
  (void) rt;
  (void) value;
  (void) cell;
  return 0;
}

static int _lattice_cell_deappend (ramsey_t *rt, int cell)
{
  (void) rt;
  (void) cell;
  return 0;
}


/* CONSTRUCTOR / DESTRUCTOR */
static void _lattice_empty (ramsey_t *rt)
{
  struct _lattice *lat = (struct _lattice *) rt;
  assert (rt && rt->type == TYPE_LATTICE);
  lat->top_value = 0;
}

static void _lattice_reset (ramsey_t *rt)
{
  struct _lattice *lat = (struct _lattice *) rt;
  int i;
  assert (rt && rt->type == TYPE_LATTICE);

  for (i = 0; i < lat->n_filters; ++i)
    lat->filter[i]->destroy (lat->filter[i]);

  lat->top_value = 0;
  lat->n_filters = 0;
  recursion_init (rt);
}

static ramsey_t *_lattice_clone (const ramsey_t *rt)
{
  const struct _lattice *old_lat = (struct _lattice *) rt;
  struct _lattice *lat = malloc (sizeof *lat);
  int i;

  assert (rt && rt->type == TYPE_LATTICE);
  if (lat == NULL)
    return NULL;

  memcpy (lat, rt, sizeof *lat);

  lat->filter = malloc (lat->max_filters * sizeof *lat->filter);
  for (i = 0; i < lat->n_filters; ++i)
    lat->filter[i] = old_lat->filter[i]->clone (old_lat->filter[i]);

  return (ramsey_t *) lat;
}

static void _lattice_destroy (ramsey_t *rt)
{
  struct _lattice *lat = (struct _lattice *) rt;
  int i;
  assert (rt && rt->type == TYPE_LATTICE);

  for (i = 0; i < lat->n_filters; ++i)
    lat->filter[i]->destroy (lat->filter[i]);

  free (lat->value);
  free (lat);
}

void *lattice_new (const setting_list_t *vars)
{
  const setting_t *n_colors_set  = vars->get_setting (vars, "n_colors");
  const setting_t *n_columns_set = vars->get_setting (vars, "n_columns");
  struct _lattice *lat = malloc (sizeof *lat);
  ramsey_t *rv = (ramsey_t *) lat;

  if (n_colors_set == NULL)
    {
      fprintf (stderr, "Error: coloring requires variable ``n_colors'' set.\n");
      free (lat);
      return NULL;
    }
  if (n_columns_set == NULL)
    {
      fprintf (stderr, "Error: coloring requires variable ``n_columns'' set.\n");
      free (lat);
      return NULL;
    }
  if (lat == NULL)
    {
      fprintf (stderr, "Out of memory creating lattice!\n");
      return NULL;
    }

  rv->type = TYPE_LATTICE;
  rv->get_type = _lattice_get_type;

  rv->print   = _lattice_print;
  rv->parse   = _lattice_parse;
  rv->empty   = _lattice_empty;
  rv->reset   = _lattice_reset;
  rv->clone   = _lattice_clone;
  rv->destroy = _lattice_destroy;
  rv->randomize = _lattice_randomize;
  rv->recurse = _lattice_recurse;
  recursion_init (rv);

  rv->find_value  = _lattice_find_value;
  rv->get_length  = _lattice_get_length;
  rv->get_maximum = _lattice_get_maximum;
  rv->get_n_cells = _lattice_get_n_cells;
  rv->append      = _lattice_append;
  rv->cell_append = _lattice_cell_append;
  rv->deappend    = _lattice_deappend;
  rv->cell_deappend = _lattice_cell_deappend;
  rv->get_priv_data       = _lattice_get_priv_data;
  rv->get_priv_data_const = _lattice_get_priv_data_const;

  rv->add_filter  = _lattice_add_filter;
  rv->run_filters = _lattice_run_filters;

  lat->n_columns = n_columns_set->get_int_value (n_columns_set);
  lat->n_colors  = n_colors_set->get_int_value (n_colors_set);
  lat->top_value = 0;
  lat->n_filters = 0;
  lat->max_value = DEFAULT_MAX_LENGTH;
  lat->value = malloc (lat->max_value * sizeof *lat->value);
  lat->max_filters = DEFAULT_MAX_FILTERS;
  lat->filter = malloc (lat->max_filters * sizeof *lat->filter);

  if (lat->value == NULL || lat->filter == NULL)
    {
      fprintf (stderr, "Out of memory creating lattice!\n");
      free (lat->value);
      free (lat->filter);
      free (lat);
      rv = NULL;
    }
  return rv;
}

