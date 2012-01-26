
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "ramsey.h"
#include "lattice.h"

#define DEFAULT_MAX_LENGTH	400
#define DEFAULT_MAX_FILTERS	20

struct _lattice {
  ramsey_t parent;

  filter_t **filter;
  int n_filters;
  int max_filters;

  int *value;
  int top_value;  /* actual highest value */
  int max_value;  /* allocation limit */
  int n_columns;
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
    if (!lat->filter[i]->run (rt))
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

static int _lattice_add_gap_set (ramsey_t *rt, const ramsey_t *gap_set)
{
  assert (rt && rt->type == TYPE_LATTICE);
  (void) gap_set;
  fprintf (stderr, "Warning: add_gap_set() unimplemented for ``%s''\n",
           rt->get_type (rt));
  return 0;
}

/* RECURSION */
static void _lattice_recurse (ramsey_t *rt, global_data_t *state)
{
  int i;
  struct _lattice *lat = (struct _lattice *) rt;

  assert (rt && rt->type == TYPE_LATTICE);

  if (!recursion_preamble (rt, state))
    return;

  for (i = 1; i <= lat->n_colors; ++i)
    {
      rt->append (rt, i);
      rt->recurse (rt, state);
      rt->deappend (rt);
    }

  recursion_postamble (rt);
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

static ramsey_t **_lattice_get_cells (ramsey_t *rt)
{
  (void) rt;
  return NULL;
}

static const ramsey_t **_lattice_get_cells_const (const ramsey_t *rt)
{
  (void) rt;
  return NULL;
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
int _lattice_append (ramsey_t *rt, int value)
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

int _lattice_deappend (ramsey_t *rt)
{
  struct _lattice *lat = (struct _lattice *) rt;
  assert (rt && rt->type == TYPE_LATTICE);
  if (lat->top_value)
    --lat->top_value;
  else
    return 0;
  return 1;
}

int _lattice_cell_append (ramsey_t *rt, int value, int cell)
{
  (void) rt;
  (void) value;
  (void) cell;
  return 0;
}

int _lattice_cell_deappend (ramsey_t *rt, int cell)
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

void *lattice_new (const global_data_t *state)
{
  setting_t *n_colors_set  = SETTING("n_colors");
  setting_t *n_columns_set = SETTING("n_columns");
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
  rv->get_cells   = _lattice_get_cells;
  rv->get_cells_const = _lattice_get_cells_const;
  rv->get_priv_data       = _lattice_get_priv_data;
  rv->get_priv_data_const = _lattice_get_priv_data_const;

  rv->add_filter  = _lattice_add_filter;
  rv->add_gap_set = _lattice_add_gap_set;
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

