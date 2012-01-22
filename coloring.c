
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>

#include "global.h"
#include "coloring.h"
#include "ramsey.h"
#include "recurse.h"
#include "sequence.h"
#include "stream.h"

struct _coloring {
  ramsey_t parent;

  int n_cells;
  ramsey_t **sequence;
};

static const char *_coloring_get_type (const ramsey_t *rt)
{
  (void) rt;
  return "coloring";
}

static const ramsey_t *_coloring_find_value (const ramsey_t *rt, int value)
{
  struct _coloring *c = (struct _coloring *) rt;
  int i;
  assert (rt && rt->type == TYPE_SEQUENCE);

  for (i = 0; i < c->n_cells; ++i)
    if (c->sequence[i]->find_value (c->sequence[i], value))
      return c->sequence[i];
  return NULL;
}

/* FILTERS */
static int _coloring_run_filters (const ramsey_t *rt)
{
  const struct _coloring *c = (struct _coloring *) rt;
  int i;
  assert (rt && rt->type == TYPE_COLORING);

  for (i = 0; i < c->n_cells; ++i)
    {
      if (!c->sequence[i]->run_filters (c->sequence[i]))
        return 0;
    }
  return 1;
}

static int _coloring_add_filter (ramsey_t *rt, filter_t *f)
{
  struct _coloring *c = (struct _coloring *) rt;
  int i;
  assert (rt && rt->type == TYPE_COLORING);

  for (i = 0; i < c->n_cells; ++i)
    {
      if (i > 0)
        f = f->clone (f);
      if (!c->sequence[i]->add_filter (c->sequence[i], f))
        return 0;
    }

  return 1;
}

static int _coloring_add_gap_set (ramsey_t *rt, const ramsey_t *gap_set)
{
  struct _coloring *c = (struct _coloring *) rt;
  assert (rt && rt->type == TYPE_COLORING);

  if (gap_set == NULL)
    return 0;

  if (gap_set->type == TYPE_SEQUENCE)
    {
      int i;
      for (i = 0; i < c->n_cells; ++i)
        if (!c->sequence[i]->add_gap_set (c->sequence[i], gap_set))
          return 0;
    }
  else if (gap_set->type == TYPE_COLORING)
    {
      int i;
      const ramsey_t **cell = gap_set->get_cells_const (gap_set);
      for (i = 0; i < c->n_cells; ++i)
        if (!c->sequence[i]->add_gap_set (c->sequence[i], cell[i]))
          return 0;
    }
  else
    {
      fprintf (stderr, "Bad gap set type ``%s'' for coloring search, sorry.\n",
               gap_set->get_type (gap_set));
      return 0;
    }

  return 1;
}

/* RECURSION */
static void _coloring_real_recurse (ramsey_t *rt, int max_value, global_data_t *state)
{
  struct _coloring *c = (struct _coloring *) rt;
  int i;

  assert (rt && rt->type == TYPE_COLORING);
  assert (state != NULL);

  if (!recursion_preamble (rt, state))
    return; 

  for (i = 0; i < c->n_cells; ++i)
    {
      c->sequence[i]->append (c->sequence[i], max_value + 1);
      _coloring_real_recurse (rt, max_value + 1, state);
      c->sequence[i]->deappend (c->sequence[i]);

      /* Only bother with one empty cell, since by symmetry they'll
       *  all behave the same. */
      if (c->sequence[i]->get_length (c->sequence[i]) == 0)
        break;
    }

  recursion_postamble (rt);
}

static void _coloring_recurse (ramsey_t *rt, global_data_t *state)
{
  _coloring_real_recurse (rt, rt->get_maximum (rt), state);
}

/* PRINT / PARSE */

static const char *_coloring_parse (ramsey_t *rt, const char *data)
{
  struct _coloring *c = (struct _coloring *) rt;
  int i;

  assert (rt && rt->type == TYPE_COLORING);
  assert (data != NULL);

  while (*data && isspace (*data))
    ++data;
  if (*data == '[')
    ++data;
  for (i = 0; i < c->n_cells; ++i)
    data = c->sequence[i]->parse (c->sequence[i], data);

  return data;
}

static void _coloring_print (const ramsey_t *rt, stream_t *out)
{
  int i;
  const struct _coloring *c = (const struct _coloring *) rt;

  assert (rt && rt->type == TYPE_COLORING);
  assert (out != NULL);

  out->write (out, "[");
  c->sequence[0]->print (c->sequence[0], out);
  for (i = 1; i < c->n_cells; ++i)
    {
      out->write (out, " ");
      c->sequence[i]->print (c->sequence[i], out);
    }
  out->write (out, "]");
}

static void _coloring_randomize (ramsey_t *rt, int n)
{
  struct _coloring *c = (struct _coloring *) rt;
  bool success = 0;
  int i;
  assert (rt && rt->type == TYPE_COLORING);

  while (!success)
    {
      rt->empty (rt);
      success = 1;
      for (i = 1; success && i <= n; ++i)
        {
          rt->cell_append (rt, i, rand () % c->n_cells);
          success &= rt->run_filters (rt);
        }
    }
}

/* ACCESSORS */
static int _coloring_get_length (const ramsey_t *rt)
{
  const struct _coloring *c = (const struct _coloring *) rt;
  int sum = 0;
  int i;

  assert (rt && rt->type == TYPE_COLORING);
  for (i = 0; i < c->n_cells; ++i)
    sum += c->sequence[i]->get_length (c->sequence[i]);
  return sum;
}

static int _coloring_get_n_cells (const ramsey_t *rt)
{
  assert (rt && rt->type == TYPE_COLORING);
  return ((const struct _coloring *) rt)->n_cells;
}

static ramsey_t **_coloring_get_cells (ramsey_t *rt)
{
  assert (rt && rt->type == TYPE_COLORING);
  return ((struct _coloring *) rt)->sequence;
}

static const ramsey_t **_coloring_get_cells_const (const ramsey_t *rt)
{
  assert (rt && rt->type == TYPE_COLORING);
  return (const ramsey_t **) _coloring_get_cells ((ramsey_t *) rt);
}

static void *_coloring_get_priv_data (ramsey_t *rt)
{
  (void) rt;
  return NULL;
}

static const void *_coloring_get_priv_data_const (const ramsey_t *rt)
{
  (void) rt;
  return NULL;
}

/* APPEND / DEAPPEND */
static int _coloring_cell_append (ramsey_t *rt, int value, int cell)
{
  ramsey_t *seq = ((struct _coloring *) rt)->sequence[cell];
  assert (rt && rt->type == TYPE_COLORING);
  return seq->append (seq, value);
}

static int _coloring_append (ramsey_t *rt, int value)
{
  assert (rt && rt->type == TYPE_COLORING);
  return _coloring_cell_append (rt, value, 0);
}

static int _coloring_cell_deappend (ramsey_t *rt, int cell)
{
  ramsey_t *seq = ((struct _coloring *) rt)->sequence[cell];
  assert (rt && rt->type == TYPE_COLORING);
  return seq->deappend (seq);
}

static int _coloring_deappend (ramsey_t *rt)
{
  assert (rt && rt->type == TYPE_COLORING);
  return _coloring_cell_deappend (rt, 0);
}


/* CONSTRUCTOR / DESTRUCTOR */
static void _coloring_empty (ramsey_t *rt)
{
  struct _coloring *c = (struct _coloring *) rt;
  int i;

  assert (rt && rt->type == TYPE_COLORING);

  for (i = 0; i < c->n_cells; ++i)
    c->sequence[i]->empty (c->sequence[i]);
}

static void _coloring_reset (ramsey_t *rt)
{
  struct _coloring *c = (struct _coloring *) rt;
  int i;

  assert (rt && rt->type == TYPE_COLORING);

  for (i = 0; i < c->n_cells; ++i)
    c->sequence[i]->reset (c->sequence[i]);
  recursion_init (rt);
}

static void _coloring_destroy (ramsey_t *rt)
{
  struct _coloring *c = (struct _coloring *) rt;
  int i;

  assert (rt && rt->type == TYPE_COLORING);

  for (i = 0; i < c->n_cells; ++i)
    c->sequence[i]->destroy (c->sequence[i]);
  free (c->sequence);
  free (rt);
}

ramsey_t *coloring_new (int n_colors)
{
  struct _coloring *c = malloc (sizeof *c);
  ramsey_t *rv = (ramsey_t *) c;

  if (c != NULL)
    {
      rv->type = TYPE_COLORING;
      rv->get_type = _coloring_get_type;

      rv->print   = _coloring_print;
      rv->parse   = _coloring_parse;
      rv->empty   = _coloring_empty;
      rv->reset   = _coloring_reset;
      rv->destroy = _coloring_destroy;
      rv->randomize = _coloring_randomize;
      rv->recurse = _coloring_recurse;
      recursion_init (rv);

      rv->find_value  = _coloring_find_value;
      rv->get_length  = _coloring_get_length;
      rv->get_maximum = _coloring_get_length;
      rv->get_n_cells = _coloring_get_n_cells;
      rv->append      = _coloring_append;
      rv->cell_append = _coloring_cell_append;
      rv->deappend    = _coloring_deappend;
      rv->cell_deappend = _coloring_cell_deappend;
      rv->get_cells   = _coloring_get_cells;
      rv->get_cells_const = _coloring_get_cells_const;
      rv->get_priv_data   = _coloring_get_priv_data;
      rv->get_priv_data_const = _coloring_get_priv_data_const;

      rv->add_filter  = _coloring_add_filter;
      rv->add_gap_set = _coloring_add_gap_set;
      rv->run_filters = _coloring_run_filters;

      c->n_cells = n_colors;
      c->sequence = malloc (n_colors * sizeof *c->sequence);
      if (c->sequence == NULL)
        {
          free (c);
          rv = NULL;
        }
      else
        {
          int i;
          for (i = 0; i < n_colors; ++i)
            c->sequence[i] = sequence_new ();
        }
    }
  return rv;
}

/* PROTOTYPE */
const ramsey_t *coloring_prototype ()
{
  static ramsey_t *rv;
  if (rv == NULL)
    rv = coloring_new (1);
  return rv;
}

