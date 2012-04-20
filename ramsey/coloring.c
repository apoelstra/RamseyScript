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

/*! \file coloring.c
 *  \brief Implementation of the coloring type.
 *
 *  A coloring is a partition of the integers [1, N] into r subsets,
 *  where r (the ``number of colors'') is given, and N is attempted
 *  to be maximised given some constraints (set by filters).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "ramsey.h"
#include "coloring.h"
#include "sequence.h"

/*! \brief Default allocation size for colorings. */
#define DEFAULT_MAX_INTLIST	400
/*! \brief Default number of filters. */
#define DEFAULT_MAX_FILTERS	20

/*! \brief Private data for the coloring type. */
struct _coloring {
  /*! \brief parent struct */
  ramsey_t parent;

  /*! \brief List of filters set on the coloring (the whole coloring,
   *         not its constituent sequences). */
  filter_t **filter;
  /*! \brief Number of filters set. */
  int n_filters;
  /*! \brief Number of filters allocated. */
  int max_filters;

  /*! \brief Whether the coloring is symmetrical or not.
   *
   *  A symmetrical coloring is one in which all colors are, up to
   *  relabelling, the same. In this case, the recursion saves time
   *  by not checking colorings which are simply rearrangements of
   *  each other.
   *
   *  An example of a non-symmetrical coloring would be a 2-coloring
   *  in which blue gaps were restricted to be < 10, and red gaps
   *  to be < 5. Then the colorings [[1 2] []] and [[] [1 2]], even
   *  though they are relabellings of each other, will have different
   *  recursion sub-trees, so they both need to be checked.
   */
  int has_symmetry;
  /*! \brief Number of colors used. */
  int n_cells;
  /*! \brief Base sequence, or NULL if we are just using consecutive numbers */
  ramsey_t *base_sequence;

  /*! \brief Representation of coloring as a word on the alphabet [0,(r-1)],
   *         where r is the number of colors. */
  int *int_list;
  /*! \brief Actual length of the coloring. */
  int n_int_list;
  /*! \brief Allocated length of the coloring. */
  int max_int_list;

  /*! \brief Representation of the coloring as an array of sequences (i.e.,
   *         a partition. */
  ramsey_t **sequence;
};

static int _coloring_cell_append (ramsey_t *rt, int value, int cell);
static int _coloring_cell_deappend (ramsey_t *rt, int cell);

static const char *_coloring_get_type (const ramsey_t *rt)
{
  (void) rt;
  return "coloring";
}

static const ramsey_t *_coloring_find_value (const ramsey_t *rt, int value)
{
  struct _coloring *c = (struct _coloring *) rt;
  assert (rt && rt->type == TYPE_SEQUENCE);

  if (value > 0 && value <= c->n_int_list)
    return c->sequence[c->int_list[value - 1]];
  return NULL;
}

/* FILTERS */
static int _coloring_run_filters (const ramsey_t *rt)
{
  const struct _coloring *c = (struct _coloring *) rt;
  int i;
  assert (rt && rt->type == TYPE_COLORING);

  for (i = 0; i < c->n_filters; ++i)
    if (!c->filter[i]->run (c->filter[i], rt))
      return 0;

  for (i = 0; i < c->n_cells; ++i)
    if (!c->sequence[i]->run_filters (c->sequence[i]))
      return 0;

  return 1;
}

static int _coloring_add_filter (ramsey_t *rt, filter_t *f)
{
  struct _coloring *c = (struct _coloring *) rt;
  assert (rt && rt->type == TYPE_COLORING);


  if (f->supports (f, TYPE_COLORING))
    {
      if (c->n_filters == c->max_filters)
        {
          void *new_alloc = realloc (c->filter, 2 * c->max_filters);
          if (new_alloc == NULL)
            return 0;
          c->filter = new_alloc;
          c->max_filters *= 2;
        }

      c->has_symmetry &= f->get_symmetry (f);
      f->set_mode (f, MODE_LAST_ONLY);
      c->filter[c->n_filters++] = f;
      return 1;
    }
  else if (f->supports (f, TYPE_SEQUENCE))
    {
      int i;
      for (i = 0; i < c->n_cells; ++i)
        {
          if (i > 0)
            f = f->clone (f);
          if (!c->sequence[i]->add_filter (c->sequence[i], f))
            return 0;
        }
      return 1;
    }

  fprintf (stderr, "Warning: filter ``%s'' does not support colorings.\n",
           f->get_type (f));
  return 0;
}

/* RECURSION */
static void *_coloring_real_thread_recurse (void *rtv)
{
  ramsey_t *rt = rtv;
  struct _coloring *c = rtv;
  int i;

  assert (rt && rt->type == TYPE_COLORING);
  
  if (!recursion_preamble_statefree (rt))
    return rt; 

  for (i = 0; i < c->n_cells; ++i)
    {
      _coloring_cell_append ((ramsey_t *) c, c->n_int_list + 1, i);
      _coloring_real_thread_recurse (rt);
      _coloring_cell_deappend ((ramsey_t *) c, i);

      /* Only bother with one empty cell, since by symmetry they'll
       *  all behave the same. */
      if (c->has_symmetry && c->sequence[i]->get_length (c->sequence[i]) == 0)
        break;
    }

  recursion_postamble (rt);
  return rt; 
}

static void _coloring_real_recurse (int max_value, const global_data_t *state)
{
  struct _coloring *c = (struct _coloring *) state->seed;
  const int *base_sequence_values;
  int next_val;
  int i;

  pthread_t thread[10];
  int thread_idx = 0;

  assert (state != NULL);
  assert (state->seed && state->seed->type == TYPE_COLORING);

  if (c->base_sequence != NULL &&
      state->seed->get_length (state->seed) > c->base_sequence->get_length (c->base_sequence))
    return;
  if (!recursion_preamble (state))
    return;

  if (c->base_sequence != NULL)
    {
      base_sequence_values = c->base_sequence->get_priv_data_const (c->base_sequence);
      next_val = base_sequence_values[max_value];
    }
  else
    next_val = max_value + 1;

  for (i = 0; i < c->n_cells; ++i)
    {
      _coloring_cell_append (state->seed, next_val, i);
      /* Try to spawn a new thread */
      if (state->seed->r_depth == 3 && thread_idx < 10 &&
          recursion_thread_spawn (&thread[thread_idx], state->seed,
                                 _coloring_real_thread_recurse))
        ++thread_idx;
      /* Failing that, recurse normally */
      else
        _coloring_real_recurse (max_value + 1, state);

      _coloring_cell_deappend ((ramsey_t *) c, i);
      /* Only bother with one empty cell, since by symmetry they'll
       *  all behave the same. */
      if (c->has_symmetry && c->sequence[i]->get_length (c->sequence[i]) == 0)
        break;
    }

  /* Catch any threads that were spawned */
  while (thread_idx--)
    recursion_thread_join (thread[thread_idx], state->seed);

  recursion_postamble (state->seed);
}

static void _coloring_recurse (global_data_t *state)
{
  _coloring_real_recurse (state->seed->get_maximum (state->seed), state);
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

  /* Copy sequences into int_list */
  {
    int sum = 0;
    for (i = 0; i < c->n_cells; ++i)
      sum += c->sequence[i]->get_length (c->sequence[i]);
    if (c->max_int_list <= sum)
      {
        void *tmp = realloc (c->int_list, 2 * sum * sizeof *c->int_list);
        if (tmp)
          {
            c->max_int_list = sum * 2;
            c->int_list = tmp;
          }
        else
          fprintf (stderr, "OOM in coloring_parse. Bad Things will happen.\n");
      }
    c->n_int_list = sum;
    for (i = 0; i < c->n_cells; ++i)
      {
        const int *data = c->sequence[i]->get_priv_data_const (c->sequence[i]);
        int j;
        for (j = 0; j < c->sequence[i]->get_length (c->sequence[i]); ++j)
          c->int_list[data[j]] = i;
      }
  }

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
  assert (rt && rt->type == TYPE_COLORING);
  return ((const struct _coloring *) rt)->n_int_list;
}

static int _coloring_get_n_cells (const ramsey_t *rt)
{
  assert (rt && rt->type == TYPE_COLORING);
  return ((const struct _coloring *) rt)->n_cells;
}

static void *_coloring_get_priv_data (ramsey_t *rt)
{
  assert (rt && rt->type == TYPE_COLORING);
  return ((struct _coloring *) rt)->sequence;
}

static const void *_coloring_get_priv_data_const (const ramsey_t *rt)
{
  assert (rt && rt->type == TYPE_COLORING);
  return ((struct _coloring *) rt)->sequence;
}

static const void *_coloring_get_alt_priv_data_const (const ramsey_t *rt)
{
  assert (rt && rt->type == TYPE_COLORING);
  return ((struct _coloring *) rt)->int_list;
}

/* APPEND / DEAPPEND */
static int _coloring_cell_append (ramsey_t *rt, int value, int cell)
{
  struct _coloring *c = (struct _coloring *) rt;
  ramsey_t *seq;
  assert (rt && rt->type == TYPE_COLORING);

  seq = c->sequence[cell];
  if (seq->append (seq, value))
    {
      c->int_list[c->n_int_list] = cell;
      if (c->n_int_list == c->max_int_list - 1)
        {
          void *tmp = realloc (c->int_list, c->max_int_list * 2 *
                                            sizeof *c->int_list);
          if (tmp)
            {
              c->int_list = tmp;
              c->max_int_list *= 2;
            }
          else return 0;
        }
      ++c->n_int_list;
      return 1;
    }
  return 0;
}

static int _coloring_append (ramsey_t *rt, int value)
{
  assert (rt && rt->type == TYPE_COLORING);
  return _coloring_cell_append (rt, value, 0);
}

static int _coloring_cell_deappend (ramsey_t *rt, int cell)
{
  struct _coloring *c = (struct _coloring *) rt;
  ramsey_t *seq = c->sequence[cell];
  assert (rt && rt->type == TYPE_COLORING);
  if (seq->deappend (seq))
    {
      --c->n_int_list;
      return 1;
    }
  return 0;
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
  c->n_int_list = 0;
}

static void _coloring_reset (ramsey_t *rt)
{
  struct _coloring *c = (struct _coloring *) rt;
  int i;

  assert (rt && rt->type == TYPE_COLORING);

  for (i = 0; i < c->n_cells; ++i)
    c->sequence[i]->reset (c->sequence[i]);
  c->n_int_list = 0;
  recursion_init (rt);
}

static ramsey_t *_coloring_clone (const ramsey_t *rt)
{
  const struct _coloring *old_c = (struct _coloring *) rt;
  struct _coloring *c = malloc (sizeof *c);
  int i;

  assert (rt && rt->type == TYPE_COLORING);

  if (c == NULL)
    return NULL;

  memcpy (c, rt, sizeof *c);

  c->filter   = malloc (c->max_filters * sizeof *c->filter);
  c->int_list = malloc (c->max_int_list * sizeof *c->int_list);
  c->sequence = malloc (c->n_cells * sizeof *c->sequence);

  if (old_c->base_sequence)
    c->base_sequence = old_c->base_sequence->clone (old_c->base_sequence);
  else
    c->base_sequence = NULL;

  memcpy (c->int_list, old_c->int_list, c->max_int_list * sizeof *c->int_list);
  for (i = 0; i < c->n_filters; ++i)
    c->filter[i] = old_c->filter[i]->clone (old_c->filter[i]);
  for (i = 0; i < c->n_cells; ++i)
    c->sequence[i] = old_c->sequence[i]->clone (old_c->sequence[i]);

  return (ramsey_t *) c;
}

static void _coloring_destroy (ramsey_t *rt)
{
  struct _coloring *c = (struct _coloring *) rt;
  int i;

  assert (rt && rt->type == TYPE_COLORING);

  if (c->base_sequence)
    c->base_sequence->destroy (c->base_sequence);
  for (i = 0; i < c->n_cells; ++i)
    c->sequence[i]->destroy (c->sequence[i]);
  for (i = 0; i < c->n_filters; ++i)
    c->filter[i]->destroy (c->filter[i]);
  free (c->filter);
  free (c->int_list);
  free (c->sequence);
  free (rt);
}

void *coloring_new_direct (int n_colors, const ramsey_t *base_sequence)
{
  struct _coloring *c = malloc (sizeof *c);
  ramsey_t *rv = (ramsey_t *) c;
  if (c == NULL)
    {
      fprintf (stderr, "Out of memory creating coloring!\n");
      return NULL;
    }

  rv->type = TYPE_COLORING;
  rv->get_type = _coloring_get_type;

  rv->print   = _coloring_print;
  rv->parse   = _coloring_parse;
  rv->empty   = _coloring_empty;
  rv->reset   = _coloring_reset;
  rv->clone   = _coloring_clone;
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
  rv->get_priv_data = _coloring_get_priv_data;
  rv->get_priv_data_const = _coloring_get_priv_data_const;
  rv->get_alt_priv_data_const = _coloring_get_alt_priv_data_const;

  rv->add_filter  = _coloring_add_filter;
  rv->run_filters = _coloring_run_filters;

  c->n_filters = 0;
  c->max_filters = DEFAULT_MAX_FILTERS;
  c->filter = malloc (c->max_filters * sizeof *c->filter);

  c->n_int_list = 0;
  c->max_int_list = DEFAULT_MAX_INTLIST;
  c->int_list = malloc (c->max_int_list * sizeof *c->int_list);

  c->has_symmetry = 1;
  c->n_cells = n_colors;
  if (base_sequence)
    c->base_sequence = base_sequence->clone (base_sequence);
  else
    c->base_sequence = NULL;
  c->sequence = malloc (c->n_cells * sizeof *c->sequence);
  if (c->sequence == NULL || c->filter == NULL || c->int_list == NULL)
    {
      fprintf (stderr, "Out of memory creating coloring!\n");
      free (c->int_list);
      free (c->sequence);
      free (c->filter);
      free (c);
      return NULL;
    }
  else
    {
      int i;
      for (i = 0; i < c->n_cells; ++i)
        c->sequence[i] = sequence_new_direct ();
    }
  return c;
}

void *coloring_new (const setting_list_t *vars)
{
  const setting_t *n_colors_set = vars->get_setting (vars, "n_colors");
  const setting_t *base_sequence_set = vars->get_setting (vars, "base_sequence");
  if (n_colors_set == NULL)
    {
      fprintf (stderr, "Error: coloring requires variable ``n_colors'' set.\n");
      return NULL;
    }
  return coloring_new_direct (n_colors_set->get_int_value (n_colors_set),
                              base_sequence_set == NULL ? NULL :
                              base_sequence_set->get_ramsey_value (base_sequence_set));
}

