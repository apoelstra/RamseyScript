
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "stream.h"
#include "sequence.h"

#define DEFAULT_MAX_LENGTH	400
#define DEFAULT_MAX_FILTERS	20

struct _sequence {
  ramsey_t parent;
  int *value;
  int length;
  int max_length;

  const ramsey_t *gap_set;
  filter_func *filter;
  int n_filters;
  int max_filters;

  /* Recursion state variables */
  int r_iterations;
  int r_max_found;
};

static const ramsey_t *_sequence_find_value (const ramsey_t *rt, int value)
{
  struct _sequence *s = (struct _sequence *) rt;
  int i;
  assert (rt && rt->type == TYPE_SEQUENCE);

  for (i = 0; i < s->length; ++i)
    if (s->value[i] == value)
      return rt;
  return NULL;
}

/* FILTERS */
static int _sequence_run_filters (ramsey_t *rt)
{
  struct _sequence *s = (struct _sequence *) rt;
  int i;
  assert (rt && rt->type == TYPE_SEQUENCE);

  for (i = 0; i < s->n_filters; ++i)
    if (!s->filter[i] (rt))
      return 0;
  return 1;
}

static int _sequence_add_filter (ramsey_t *rt, filter_func f)
{
  struct _sequence *s = (struct _sequence *) rt;
  assert (rt && rt->type == TYPE_SEQUENCE);

  if (s->n_filters == s->max_filters)
    {
      void *new_alloc = realloc (s->filter, 2 * s->max_filters);
      if (new_alloc == NULL)
        return 0;
      s->filter = new_alloc;
      s->max_filters *= 2;
    }

  s->filter[s->n_filters++] = f;
  return 1;
}

static int _cheap_gap_set_filter (ramsey_t *rt)
{
  struct _sequence *s = (struct _sequence *) rt;
  return (s->length < 2 || s->gap_set->find_value
             (s->gap_set, s->value[s->length - 1] - s->value[s->length - 2]));
}

static int _sequence_add_gap_set (ramsey_t *rt, const ramsey_t *gap_set)
{
  struct _sequence *s = (struct _sequence *) rt;
  assert (rt && rt->type == TYPE_SEQUENCE);

  if (gap_set->type != TYPE_SEQUENCE)
    {
      fprintf (stderr, "Bad gap set type %d for sequence search, sorry.\n", gap_set->type);
      return 0;
    }

  s->gap_set = gap_set;
  return rt->add_filter (rt, _cheap_gap_set_filter);
}

/* RECURSION */
static void _sequence_recurse_reset (ramsey_t *rt)
{
  struct _sequence *s = (struct _sequence *) rt;
  assert (rt && rt->type == TYPE_SEQUENCE);
  s->r_iterations = 0;
  s->r_max_found = 0;
}

static int _sequence_recurse_get_iterations (const ramsey_t *rt)
{
  struct _sequence *s = (struct _sequence *) rt;
  assert (rt && rt->type == TYPE_SEQUENCE);
  return s->r_iterations;
}

static int _sequence_recurse_get_max_found (const ramsey_t *rt)
{
  struct _sequence *s = (struct _sequence *) rt;
  assert (rt && rt->type == TYPE_SEQUENCE);
  return s->r_max_found;
}

static void _sequence_recurse (ramsey_t *rt, global_data_t *state)
{
  int i;
  int *gap_set;
  int gap_set_len;
  struct _sequence *s = (struct _sequence *) rt;
  assert (rt && rt->type == TYPE_SEQUENCE);

  if (!rt->run_filters (rt) || state->kill_now)
    return;
  if (state->max_iterations && s->r_iterations >= state->max_iterations)
    return;

  if (state->dump_iters && s->length < state->dump_depth)
    {
      int *dump_val = state->iters_data->get_priv_data (state->iters_data);
      ++dump_val[s->length];
    }
  ++s->r_iterations;

  if (s->length > s->r_max_found)
    {
      stream_printf (state->out_stream,
                     "Got new maximum length %d. Sequence: ", s->length);
      rt->print (rt, state->out_stream);
      stream_printf (state->out_stream, "\n");
      s->r_max_found = s->length;
    }

  gap_set = state->gap_set->get_priv_data (state->gap_set);
  gap_set_len = state->gap_set->get_length (state->gap_set);
  for (i = 0; i < gap_set_len; ++i)
    {
      rt->append (rt, rt->get_maximum (rt) + gap_set[i]);
      rt->recurse (rt, state);
      rt->deappend (rt);
    }
}

/* PRINT / PARSE */
static void _sequence_print_real (const ramsey_t *rt, int start, Stream *out)
{
  struct _sequence *s = (struct _sequence *) rt;
  int i;

  assert (rt && rt->type == TYPE_SEQUENCE);
  assert (start >= 0);

  out->write_line (out, "[");
  if (start < s->length)
    stream_printf (out, "%d", s->value[start]);
  for (i = start + 1; i < s->length; ++i)
    stream_printf (out, ", %d", s->value[i]);
  out->write_line (out, "]");
}

static void _sequence_print (const ramsey_t *rt, Stream *out)
{
  _sequence_print_real (rt, 0, out);
}

static void _sequence_print1 (const ramsey_t *rt, Stream *out)
{
  _sequence_print_real (rt, 1, out);
}

static const char *_sequence_parse (ramsey_t *rt, const char *data)
{
  int value;

  assert (rt && rt->type == TYPE_SEQUENCE);

  while (*data && *data != '[')
    ++data;
  if (*data == '[')
    ++data;

  while ((value = strtoul (data, (char **) &data, 0)))
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
          if ((next_value = strtoul (data, (char **) &data, 0)) && next_value > value)
            while (value++ < next_value)
              rt->append(rt, value);
          /* Read decending run */
          if ((next_value = strtoul (data, (char **) &data, 0)) && next_value < value)
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

/* ACCESSORS */
static int _sequence_get_length (const ramsey_t *rt)
{
  struct _sequence *s = (struct _sequence *) rt;
  assert (rt && rt->type == TYPE_SEQUENCE);
  return s->length;
}

static int _sequence_get_maximum (const ramsey_t *rt)
{
  struct _sequence *s = (struct _sequence *) rt;
  assert (rt && rt->type == TYPE_SEQUENCE);
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
  assert (rt && rt->type == TYPE_SEQUENCE);
  return s->value;
}

static const void *_sequence_get_priv_data_const (const ramsey_t *rt)
{
  const struct _sequence *s = (const struct _sequence *) rt;
  assert (rt && rt->type == TYPE_SEQUENCE);
  return s->value;
}

/* APPEND / DEAPPEND */
int _sequence_append (ramsey_t *rt, int value)
{
  struct _sequence *s = (struct _sequence *) rt;
  assert (rt && rt->type == TYPE_SEQUENCE);
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

int _sequence_deappend (ramsey_t *rt)
{
  struct _sequence *s = (struct _sequence *) rt;
  assert (rt && rt->type == TYPE_SEQUENCE);
  if (s->length)
    --s->length;
  return 1;
}

int _sequence_cell_append (ramsey_t *rt, int value, int cell)
{
  (void) rt;
  (void) value;
  (void) cell;
  return 0;
}

int _sequence_cell_deappend (ramsey_t *rt, int cell)
{
  (void) rt;
  (void) cell;
  return 0;
}


/* CONSTRUCTOR / DESTRUCTOR */
static void _sequence_empty (ramsey_t *rt)
{
  struct _sequence *s = (struct _sequence *) rt;
  assert (rt && rt->type == TYPE_SEQUENCE);

  s->length = 0;
  s->n_filters = 0;
  rt->recurse_reset (rt);
}

static void _sequence_destroy (ramsey_t *rt)
{
  struct _sequence *s = (struct _sequence *) rt;
  assert (rt && rt->type == TYPE_SEQUENCE);

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

      rv->print   = _sequence_print;
      rv->parse   = _sequence_parse;
      rv->empty   = _sequence_empty;
      rv->destroy = _sequence_destroy;

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

      rv->recurse       = _sequence_recurse;
      rv->recurse_reset = _sequence_recurse_reset;
      rv->recurse_get_iterations = _sequence_recurse_get_iterations;
      rv->recurse_get_max_length = _sequence_recurse_get_max_found;
      _sequence_recurse_reset (rv);

      s->gap_set   = NULL;
      s->length    = 0;
      s->n_filters = 0;
      s->max_length = DEFAULT_MAX_LENGTH;
      s->value = malloc (s->max_length * sizeof *s->value);
      s->max_filters = DEFAULT_MAX_FILTERS;
      s->filter = malloc (s->max_filters * sizeof *s->filter);

      if (s->value == NULL)
        {
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

