
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "global.h"
#include "ramsey.h"
#include "recurse.h"
#include "stream.h"
#include "sequence.h"

#define DEFAULT_MAX_LENGTH	400
#define DEFAULT_MAX_FILTERS	20

struct _sequence {
  ramsey_t parent;

  filter_t **filter;
  int n_filters;
  int max_filters;

  int *value;
  int length;
  int max_length;
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
    if (!s->filter[i]->run (rt))
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

static int _cheap_gap_set_filter (const ramsey_t *rt)
{
  const struct _sequence *s = (const struct _sequence *) rt;
  return (s->length < 2 || rt->r_gap_set->find_value
             (rt->r_gap_set, s->value[s->length - 1] - s->value[s->length - 2]));
}

static int _sequence_add_gap_set (ramsey_t *rt, const ramsey_t *gap_set)
{
  assert (rt && (rt->type == TYPE_SEQUENCE || rt->type == TYPE_WORD ||
                 rt->type == TYPE_PERMUTATION));

  if (gap_set == NULL)
    return 0;

  if (gap_set->type != TYPE_SEQUENCE)
    {
      fprintf (stderr, "Warning: bad gap set type ``%s'' for sequence search.\n",
                       gap_set->get_type (gap_set));
      return 0;
    }

  rt->r_gap_set = gap_set;
  return rt->add_filter (rt, filter_new_custom (_cheap_gap_set_filter));
}

/* RECURSION */
static void _sequence_recurse (ramsey_t *rt, global_data_t *state)
{
  int i;
  const int *gap_set;
  int gap_set_len;
  assert (rt && (rt->type == TYPE_SEQUENCE || rt->type == TYPE_WORD ||
                 rt->type == TYPE_PERMUTATION));

  if (!recursion_preamble (rt, state))
    return;

  if (rt->r_gap_set == NULL)
    {
      fputs ("Error: cannot search sequences without a gap set.\n", stderr);
      return;
    }

  gap_set = rt->r_gap_set->get_priv_data_const (rt->r_gap_set);
  gap_set_len = rt->r_gap_set->get_length (rt->r_gap_set);
  for (i = 0; i < gap_set_len; ++i)
    {
      rt->append (rt, rt->get_maximum (rt) + gap_set[i]);
      rt->recurse (rt, state);
      rt->deappend (rt);
    }

  recursion_postamble (rt);
}

/* PRINT / PARSE */
static void _sequence_print_real (const ramsey_t *rt, int start, stream_t *out)
{
  struct _sequence *s = (struct _sequence *) rt;
  int i;

  assert (rt && (rt->type == TYPE_SEQUENCE || rt->type == TYPE_WORD ||
                 rt->type == TYPE_PERMUTATION));
  assert (start >= 0);

  out->write (out, "[");
  if (start < s->length)
    stream_printf (out, "%d", s->value[start]);
  for (i = start + 1; i < s->length; ++i)
    stream_printf (out, ", %d", s->value[i]);
  out->write (out, "]");
}

static void _sequence_print (const ramsey_t *rt, stream_t *out)
{
  _sequence_print_real (rt, 0, out);
}

static void _sequence_print1 (const ramsey_t *rt, stream_t *out)
{
  _sequence_print_real (rt, 1, out);
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
  return s->value[s->length - 1];
}

static int _sequence_get_n_cells (const ramsey_t *rt)
{
  (void) rt;
  return 1;
}

static ramsey_t **_sequence_get_cells (ramsey_t *rt)
{
  (void) rt;
  return NULL;
}

static const ramsey_t **_sequence_get_cells_const (const ramsey_t *rt)
{
  (void) rt;
  return NULL;
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

static void _sequence_destroy (ramsey_t *rt)
{
  struct _sequence *s = (struct _sequence *) rt;
  int i;
  assert (rt && (rt->type == TYPE_SEQUENCE || rt->type == TYPE_WORD ||
                 rt->type == TYPE_PERMUTATION));

  for (i = 0; i < s->n_filters; ++i)
    s->filter[i]->destroy (s->filter[i]);

  free (s->value);
  free (s);
}

ramsey_t *sequence_new ()
{
  struct _sequence *s = malloc (sizeof *s);
  ramsey_t *rv = (ramsey_t *) s;

  if (s != NULL)
    {
      rv->type = TYPE_SEQUENCE;
      rv->get_type = _sequence_get_type;

      rv->print   = _sequence_print;
      rv->parse   = _sequence_parse;
      rv->empty   = _sequence_empty;
      rv->reset   = _sequence_reset;
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
      rv->get_cells   = _sequence_get_cells;
      rv->get_cells_const = _sequence_get_cells_const;
      rv->get_priv_data       = _sequence_get_priv_data;
      rv->get_priv_data_const = _sequence_get_priv_data_const;

      rv->add_filter  = _sequence_add_filter;
      rv->add_gap_set = _sequence_add_gap_set;
      rv->run_filters = _sequence_run_filters;

      s->length    = 0;
      s->n_filters = 0;
      s->max_length = DEFAULT_MAX_LENGTH;
      s->value = malloc (s->max_length * sizeof *s->value);
      s->max_filters = DEFAULT_MAX_FILTERS;
      s->filter = malloc (s->max_filters * sizeof *s->filter);

      if (s->value == NULL || s->filter == NULL)
        {
          free (s->value);
          free (s->filter);
          free (s);
          rv = NULL;
        }
    }
  return rv;
}

ramsey_t *sequence_new_zeros (int size, bool one_indexed)
{
  ramsey_t *rv = sequence_new ();

  if (one_indexed)
    rv->print = _sequence_print1;

  if (rv != NULL)
    while (size--)
      rv->append (rv, 0);
  return rv;
}

/* PROTOTYPE */
const ramsey_t *sequence_prototype ()
{
  static ramsey_t *rv;
  if (rv == NULL)
    rv = sequence_new ();
  return rv;
}


