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

/*! \file sequence.c
 *  \brief Implementation of the sequence type.
 *
 *  A sequence is a list of integers, which is extended recursively
 *  to seek a maximum length given constraints (filters).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "ramsey.h"
#include "sequence.h"

/*! \brief Default allocation size for sequences. */
#define DEFAULT_MAX_LENGTH	400
/*! \brief Default number of filters. */
#define DEFAULT_MAX_FILTERS	20

/*! \brief Private data for the sequence type. */
struct _sequence {
  /*! \brief parent struct. */
  ramsey_t parent;

  /*! \brief List of filters set on the sequence. */
  filter_t **filter;
  /*! \brief Number of filters set. */
  int n_filters;
  /*! \brief Number of filters allocated. */
  int max_filters;

  /*! \brief Content of the sequence. */
  int *value;
  /*! \brief Length of the sequence. */
  int length;
  /*! \brief Maximum length of the sequence without requiring reallocation. */
  int max_length;

  /*! Set of allowable gap sizes when sequence is being recursively extended. */
  ramsey_t *gap_set;
};

static const char *_sequence_get_type (const ramsey_t *rt)
{
  assert (rt && rt->type == TYPE_SEQUENCE);
  return "sequence";
}

static const ramsey_t *_sequence_find_value (const ramsey_t *rt, int value)
{
  const struct _sequence *s = (struct _sequence *) rt;
  int i;
  assert (rt && (rt->type == TYPE_SEQUENCE || rt->type == TYPE_WORD ||
                 rt->type == TYPE_PERMUTATION));

  for (i = 0; i < s->length; ++i)
    if (s->value[i] == value)
      return rt;
  return NULL;
}

/* FILTERS */
static int _sequence_run_filters (const ramsey_t *rt)
{
  struct _sequence *s = (struct _sequence *) rt;
  int i;
  assert (rt && (rt->type == TYPE_SEQUENCE || rt->type == TYPE_WORD ||
                 rt->type == TYPE_PERMUTATION));

  for (i = 0; i < s->n_filters; ++i)
    if (!s->filter[i]->run (s->filter[i], rt))
      return 0;
  return 1;
}

static int _sequence_add_filter (ramsey_t *rt, filter_t *f)
{
  struct _sequence *s = (struct _sequence *) rt;
  assert (f);
  assert (rt && (rt->type == TYPE_SEQUENCE || rt->type == TYPE_WORD ||
                 rt->type == TYPE_PERMUTATION));

  if (!f->supports (f, rt->type))
    {
      fprintf (stderr, "Warning: filter ``%s'' does not support ``%s''\n",
               f->get_type (f), rt->get_type (rt));
      return 0;
    }

  if (s->n_filters == s->max_filters)
    {
      void *new_alloc = realloc (s->filter, 2 * s->max_filters);
      if (new_alloc == NULL)
        return 0;
      s->filter = new_alloc;
      s->max_filters *= 2;
    }

  f->set_mode (f, MODE_LAST_ONLY);
  s->filter[s->n_filters++] = f;
  return 1;
}

/* RECURSION */
static void _sequence_recurse (ramsey_t *rt, global_data_t *state)
{
  int i;
  const int *gap_set;
  int gap_set_len;
  struct _sequence *s = (struct _sequence *) rt;
  assert (rt && (rt->type == TYPE_SEQUENCE || rt->type == TYPE_WORD ||
                 rt->type == TYPE_PERMUTATION));

  if (!recursion_preamble (rt, state))
    return;

  if (s->gap_set == NULL)
    {
      fputs ("Error: cannot search sequences without a gap set.\n", stderr);
      return;
    }

  gap_set = s->gap_set->get_priv_data_const (s->gap_set);
  gap_set_len = s->gap_set->get_length (s->gap_set);
  for (i = 0; i < gap_set_len; ++i)
    {
      rt->append (rt, rt->get_maximum (rt) + gap_set[i]);
      rt->recurse (rt, state);
      rt->deappend (rt);
    }

  recursion_postamble (rt);
}

/* PRINT / PARSE */
static void _sequence_print (const ramsey_t *rt, stream_t *out)
{
  struct _sequence *s = (struct _sequence *) rt;
  int i;

  assert (rt && (rt->type == TYPE_SEQUENCE || rt->type == TYPE_WORD ||
                 rt->type == TYPE_PERMUTATION));

  out->write (out, "[");
  if (s->length > 0)
    stream_printf (out, "%d", s->value[0]);
  for (i = 1; i < s->length; ++i)
    stream_printf (out, ", %d", s->value[i]);
  out->write (out, "]");
}

static const char *_sequence_parse (ramsey_t *rt, const char *data)
{
  int value;

  assert (rt && (rt->type == TYPE_SEQUENCE || rt->type == TYPE_WORD ||
                 rt->type == TYPE_PERMUTATION));

  while (*data && *data != '[')
    ++data;
  if (*data == '[')
    ++data;
  /* [= is the opener for an equalized list */
  if (*data == '=')
    ++data;

  while ((value = strtol (data, (char **) &data, 0)))
    {
      rt->append (rt, value);
      while (*data && (isspace (*data) || *data == ','))
        ++data;
      if (!strncmp (data, "...", 3))
        {
          int next_value;
          data += 3;
          while (*data && (isspace (*data) || *data == ','))
            ++data;
          /* Read ascending run */
          if ((next_value = strtol (data, (char **) &data, 0)) && next_value > value)
            while (value++ < next_value)
              rt->append(rt, value);
          /* Read decending run */
          if ((next_value = strtol (data, (char **) &data, 0)) && next_value < value)
            while (value-- > next_value)
              rt->append(rt, value);
        }
    }

  while (*data && *data != ']')
    ++data;
  if (*data == ']')
    ++data;

  return data;
}

static void _sequence_randomize (ramsey_t *rt, int n)
{
  (void) n;
  fprintf (stderr, "Warning: randomize() unimplemented for ``%s''.\n",
           rt->get_type (rt));
}

/* ACCESSORS */
static int _sequence_get_length (const ramsey_t *rt)
{
  struct _sequence *s = (struct _sequence *) rt;
  assert (rt && (rt->type == TYPE_SEQUENCE || rt->type == TYPE_WORD ||
                 rt->type == TYPE_PERMUTATION));
  return s->length;
}

static int _sequence_get_maximum (const ramsey_t *rt)
{
  struct _sequence *s = (struct _sequence *) rt;
  assert (rt && (rt->type == TYPE_SEQUENCE || rt->type == TYPE_WORD ||
                 rt->type == TYPE_PERMUTATION));
  return (s->length ? s->value[s->length - 1] : 0);
}

static int _sequence_get_n_cells (const ramsey_t *rt)
{
  (void) rt;
  return 1;
}

static void *_sequence_get_priv_data (ramsey_t *rt)
{
  struct _sequence *s = (struct _sequence *) rt;
  assert (rt && (rt->type == TYPE_SEQUENCE || rt->type == TYPE_WORD ||
                 rt->type == TYPE_PERMUTATION));
  return s->value;
}

static const void *_sequence_get_priv_data_const (const ramsey_t *rt)
{
  const struct _sequence *s = (const struct _sequence *) rt;
  assert (rt && (rt->type == TYPE_SEQUENCE || rt->type == TYPE_WORD ||
                 rt->type == TYPE_PERMUTATION));
  return s->value;
}

/* APPEND / DEAPPEND */
static int _sequence_append (ramsey_t *rt, int value)
{
  struct _sequence *s = (struct _sequence *) rt;
  assert (rt && (rt->type == TYPE_SEQUENCE || rt->type == TYPE_WORD ||
                 rt->type == TYPE_PERMUTATION));
  if (s->length == s->max_length)
    {
      void *tmp = realloc (s->value, 2 * s->max_length * sizeof *s->value);
      if (tmp == NULL)
        return 0;
      s->max_length *= 2;
      s->value = tmp;
    }
  s->value[s->length++] = value;
  return 1;
}

static int _sequence_deappend (ramsey_t *rt)
{
  struct _sequence *s = (struct _sequence *) rt;
  assert (rt && (rt->type == TYPE_SEQUENCE || rt->type == TYPE_WORD ||
                 rt->type == TYPE_PERMUTATION));
  if (s->length)
    --s->length;
  return 1;
}

static int _sequence_cell_append (ramsey_t *rt, int value, int cell)
{
  (void) rt;
  (void) value;
  (void) cell;
  return 0;
}

static int _sequence_cell_deappend (ramsey_t *rt, int cell)
{
  (void) rt;
  (void) cell;
  return 0;
}


/* CONSTRUCTOR / DESTRUCTOR */
static void _sequence_empty (ramsey_t *rt)
{
  struct _sequence *s = (struct _sequence *) rt;
  assert (rt && (rt->type == TYPE_SEQUENCE || rt->type == TYPE_WORD ||
                 rt->type == TYPE_PERMUTATION));

  s->length = 0;
}

static void _sequence_reset (ramsey_t *rt)
{
  struct _sequence *s = (struct _sequence *) rt;
  int i;
  assert (rt && (rt->type == TYPE_SEQUENCE || rt->type == TYPE_WORD ||
                 rt->type == TYPE_PERMUTATION));

  for (i = 0; i < s->n_filters; ++i)
    s->filter[i]->destroy (s->filter[i]);

  s->length = 0;
  s->n_filters = 0;
  recursion_init (rt);
}

static ramsey_t *_sequence_clone (const ramsey_t *rt)
{
  const struct _sequence *old_s = (struct _sequence *) rt;
  struct _sequence *s = malloc (sizeof *s);
  int i;

  assert (rt && rt->type == TYPE_SEQUENCE);
  if (s == NULL)
    return NULL;

  memcpy (s, rt, sizeof *s);

  s->filter = malloc (s->max_filters * sizeof *s->filter);
  s->value  = malloc (s->max_length * sizeof *s->value);
  if (s->filter == NULL || s->value == NULL)
    {
      free (s->filter);
      free (s->value);
      free (s);
      return NULL;
    }
  if (old_s->gap_set)
    {
      s->gap_set = old_s->gap_set->clone (old_s->gap_set);
      if (s->gap_set == NULL)
        {
          free (s);
          return NULL;
        }
    }
  memcpy (s->value, old_s->value, s->max_length * sizeof *s->value);
  for (i = 0; i < s->n_filters; ++i)
    s->filter[i] = old_s->filter[i]->clone (old_s->filter[i]);

  return (ramsey_t *) s;
}

static void _sequence_destroy (ramsey_t *rt)
{
  struct _sequence *s = (struct _sequence *) rt;
  int i;
  assert (rt && (rt->type == TYPE_SEQUENCE || rt->type == TYPE_WORD ||
                 rt->type == TYPE_PERMUTATION));

  for (i = 0; i < s->n_filters; ++i)
    s->filter[i]->destroy (s->filter[i]);

  if (s->gap_set)
    s->gap_set->destroy (s->gap_set);
  free (s->filter);
  free (s->value);
  free (s);
}

void *sequence_new_direct ()
{
  struct _sequence *s = malloc (sizeof *s);
  ramsey_t *rv = (ramsey_t *) s;

  if (s == NULL)
    {
      fprintf (stderr, "Out of memory creating sequence!\n");
      return NULL;
    }

  rv->type = TYPE_SEQUENCE;
  rv->get_type = _sequence_get_type;

  rv->print   = _sequence_print;
  rv->parse   = _sequence_parse;
  rv->empty   = _sequence_empty;
  rv->reset   = _sequence_reset;
  rv->clone   = _sequence_clone;
  rv->destroy = _sequence_destroy;
  rv->randomize = _sequence_randomize;
  rv->recurse = _sequence_recurse;
  recursion_init (rv);

  rv->find_value  = _sequence_find_value;
  rv->get_length  = _sequence_get_length;
  rv->get_maximum = _sequence_get_maximum;
  rv->get_n_cells = _sequence_get_n_cells;
  rv->append      = _sequence_append;
  rv->cell_append = _sequence_cell_append;
  rv->deappend    = _sequence_deappend;
  rv->cell_deappend = _sequence_cell_deappend;
  rv->get_priv_data       = _sequence_get_priv_data;
  rv->get_priv_data_const = _sequence_get_priv_data_const;

  rv->add_filter  = _sequence_add_filter;
  rv->run_filters = _sequence_run_filters;

  s->gap_set = NULL;

  s->length    = 0;
  s->n_filters = 0;
  s->max_length = DEFAULT_MAX_LENGTH;
  s->value = malloc (s->max_length * sizeof *s->value);
  s->max_filters = DEFAULT_MAX_FILTERS;
  s->filter = malloc (s->max_filters * sizeof *s->filter);

  if (s->value == NULL || s->filter == NULL)
    {
      fprintf (stderr, "Out of memory creating sequence!\n");
      free (s->value);
      free (s->filter);
      free (s);
      rv = NULL;
    }
  return rv;
}

void *sequence_new (const setting_list_t *vars)
{
  struct _sequence *rv = sequence_new_direct ();
  if (rv == NULL)
    return NULL;
  else
    {
      const setting_t *gap_set_set = vars->get_setting (vars, "gap_set");
      if (gap_set_set && gap_set_set->type == TYPE_RAMSEY)
        {
          const ramsey_t *gs = gap_set_set->get_ramsey_value (gap_set_set);
          rv->gap_set = gs->clone (gs);
        }
    }
  return rv;
}

/* PROTOTYPE */
const ramsey_t *sequence_prototype ()
{
  static ramsey_t *rv;
  if (rv == NULL)
    rv = sequence_new_direct ();
  return rv;
}


