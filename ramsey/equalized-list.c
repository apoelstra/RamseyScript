/* RamseyScript
 * Written in 2013 by
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

/*! \file equalized-list.c
 *  \brief Implementation of the equalized-list type.
 *
 *  An equalized list is a sequence with two additional functions
 *  to increment/decrement a counter associated to each member of
 *  the sequence. Then get_priv_data_const will always return a
 *  sorted array of values, sorted on these counters.
 *
 *  Basically, this type will only be used for gap sets. It cannot
 *  be recursed on.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <ctype.h>

#include "ramsey.h"
#include "sequence.h"
#include "equalized-list.h"

/*! \brief Default allocation size for equalized lists. */
#define DEFAULT_MAX_LENGTH	100

/*! \brief Private data for the equalized-list type. */
struct _qlist {
  /*! \brief parent struct */
  ramsey_t parent;

  /*! \brief List values */
  int *value;
  /*! \brief Counter values */
  int *count;
  /*! \brief Allocated list/counter size */
  int max_size;
  /*! \brief Actual list/counter size */
  int size;
};

static const char *_qlist_get_type (const ramsey_t *rt)
{
  assert (rt && rt->type == TYPE_EQUALIZED_LIST);
  return "equalized-list";
}

static const ramsey_t *_qlist_find_value (const ramsey_t *rt, int value)
{
  int i;
  struct _qlist *ql = (struct _qlist *) rt;
  assert (rt && rt->type == TYPE_EQUALIZED_LIST);
  for (i = 0; i < ql->size; ++i)
    if (ql->value[i] == value)
      return rt;
  return NULL;
}

/* FILTERS */
static int _qlist_run_filters (const ramsey_t *rt)
{
  (void) rt;
  fputs ("run_filters: unimplemented for equalized list.\n", stderr);
  return 0;
}

static int _qlist_add_filter (ramsey_t *rt, filter_t *f)
{
  (void) rt;
  (void) f;
  fputs ("add_filter: unimplemented for equalized list.\n", stderr);
  return 0;
}

/* RECURSION */
static void _qlist_recurse (ramsey_t *rt, global_data_t *state)
{
  (void) rt;
  (void) state;
  fputs ("recurse: unimplemented for equalized list.\n", stderr);
}

/* PRINT / PARSE */
static void _qlist_print (const ramsey_t *rt, stream_t *out)
{
  struct _qlist *ql = (struct _qlist *) rt;
  int i;
  assert (rt && rt->type == TYPE_EQUALIZED_LIST);

  stream_printf (out, "[ ");
  if (ql->size)
    stream_printf (out, "%2d", ql->value[0]);
  for (i = 1; i < ql->size; ++i)
    stream_printf (out, ", %2d", ql->value[i]);
  stream_printf (out, " ]");
}

/* Helper to make a qlist from a sequence */
static void _qlist_rob_sequence (struct _qlist *ql, ramsey_t *seq)
{
  int i;
  const int *data = seq->get_priv_data_const (seq);

  assert (seq != NULL);
  assert (ql != NULL);

  if (data == NULL)
    {
      fputs ("Warning: tried to qlistify a nil list, doing nothing.\n", stderr);
      return;
    }

  /* Clean out any existing data in the ql */
  if (ql->value) free (ql->value);
  if (ql->count) free (ql->count);
  ql->max_size = seq->get_length (seq);
  ql->value = malloc (ql->max_size * sizeof *ql->value);
  ql->count = malloc (ql->max_size * sizeof *ql->count);

  /* Allocate memory */
  if (ql->value == NULL || ql->count == NULL)
    {
      fputs ("OOM when qlistifying sequence, returning nothing.\n", stderr);
      free (ql->value);
      free (ql->count);
      ql->max_size = ql->size = 0;
      return;
    }

  /* Steal the data from the sequence */
  for (i = 0; i < ql->max_size; ++i)
    {
      ql->value[i] = data[i];
      ql->count[i] = 0;
    }
  ql->size = ql->max_size;
}

static const char *_qlist_parse (ramsey_t *rt, const char *data)
{
  ramsey_t *temp = sequence_new_direct ();
  struct _qlist *ql = (struct _qlist *) rt;

  assert (rt && rt->type == TYPE_EQUALIZED_LIST);

  if (temp == NULL)
    return data;

  /* Parse a sequence, steal its data */
  data = temp->parse (temp, data);
  _qlist_rob_sequence (ql, temp);
  temp->destroy (temp);

  return data;
}

static void _qlist_randomize (ramsey_t *rt, int n)
{
  ramsey_t *temp = sequence_new_direct ();
  struct _qlist *ql = (struct _qlist *) rt;

  assert (rt && rt->type == TYPE_EQUALIZED_LIST);

  /* Use a sequence, steal its data */
  temp->randomize (temp, n);
  _qlist_rob_sequence (ql, temp);
  temp->destroy (temp);
}

/* ACCESSORS */
static int _qlist_get_length (const ramsey_t *rt)
{
  struct _qlist *ql = (struct _qlist *) rt;
  assert (rt && rt->type == TYPE_EQUALIZED_LIST);
  return ql->size;
}

static int _qlist_get_maximum (const ramsey_t *rt)
{
  struct _qlist *ql = (struct _qlist *) rt;
  int i, max = INT_MIN;

  assert (rt && rt->type == TYPE_EQUALIZED_LIST);
  for (i = 0; i < ql->size; ++i)
    if (ql->value[i] > max)
      max = ql->value[i];

  if (max == INT_MIN)
    max = 0;

  return max;
}

static int _qlist_get_n_cells (const ramsey_t *rt)
{
  (void) rt;
  return 1;
}

static void *_qlist_get_priv_data (ramsey_t *rt)
{
  (void) rt;
  fputs ("Warning: tried to get non-const private data from an "
         "equalized list. Returning NULL because you cannot safely "
         "modify this. Use get_priv_data_const instead.\n", stderr);
  return NULL;
}

static const void *_qlist_get_priv_data_const (const ramsey_t *rt)
{
  const struct _qlist *ql = (struct _qlist *) rt;
  assert (rt && rt->type == TYPE_EQUALIZED_LIST);
  return ql->value;
}

/* APPEND / DEAPPEND */
static int _qlist_append (ramsey_t *rt, int value)
{
  struct _qlist *ql = (struct _qlist *) rt;
  assert (rt && rt->type == TYPE_EQUALIZED_LIST);

  if (ql->size >= ql->max_size)
    {
      void *re1 = realloc (ql->value, 2 * ql->max_size * sizeof *ql->value);
      void *re2 = realloc (ql->count, 2 * ql->max_size * sizeof *ql->count);
      if (re1 == NULL || re2 == NULL)
        {
          free (re1);
          free (re2);
          return 0;
        }
      ql->max_size *= 2;
      ql->value = re1;
      ql->count = re2;
    }

  ql->value[ql->size++] = value;
  ql->count[ql->size++] = 0;
  return 1;
}

static int _qlist_deappend (ramsey_t *rt)
{
  struct _qlist *ql = (struct _qlist *) rt;
  assert (rt && rt->type == TYPE_EQUALIZED_LIST);
  if (ql->size == 0)
    return 0;
  --ql->size;
  return 1;
}

static int _qlist_cell_append (ramsey_t *rt, int value, int cell)
{
  (void) rt;
  (void) value;
  (void) cell;
  return 0;
}

static int _qlist_cell_deappend (ramsey_t *rt, int cell)
{
  (void) rt;
  (void) cell;
  return 0;
}


/* PUBLIC INCREMENT/DECREMENT FUNCTIONS */
void equalized_list_increment (ramsey_t *rt, int index)
{
  struct _qlist *ql = (struct _qlist *) rt;
  assert (rt && rt->type == TYPE_EQUALIZED_LIST);

  if (index < 0 || index > ql->size)
    return;

  /* Increment the index */
  ++ql->count[index];
  /* Bubble it into the right place */
  while (index < (ql->size - 1) &&
         ql->count[index] > ql->count[index + 1])
    {
      int t = ql->count[index];
      ql->count[index] = ql->count[index + 1];
      ql->count[index + 1] = t;
      ++index;
    }
}

void equalized_list_decrement (ramsey_t *rt, int index)
{
  struct _qlist *ql = (struct _qlist *) rt;
  assert (rt && rt->type == TYPE_EQUALIZED_LIST);

  if (index < 0 || index > ql->size)
    return;

  /* Decrement the index */
  --ql->count[index];
  /* Bubble it into the right place */
  while (index > 0 &&
         ql->count[index] > ql->count[index - 1])
    {
      int t = ql->count[index];
      ql->count[index] = ql->count[index - 1];
      ql->count[index - 1] = t;
      --index;
    }
}

/* CONSTRUCTOR / DESTRUCTOR */
static void _qlist_empty (ramsey_t *rt)
{
  struct _qlist *ql = (struct _qlist *) rt;
  assert (rt && rt->type == TYPE_EQUALIZED_LIST);
  ql->size = 0;
}

static void _qlist_reset (ramsey_t *rt)
{
  _qlist_empty (rt);
}

static ramsey_t *_qlist_clone (const ramsey_t *rt)
{
  const struct _qlist *old_ql = (struct _qlist *) rt;
  struct _qlist *ql = malloc (sizeof *ql);

  assert (rt && rt->type == TYPE_EQUALIZED_LIST);
  if (ql == NULL)
    return NULL;

  memcpy (ql, old_ql, sizeof *ql);
  ql->value = malloc (ql->max_size * sizeof *ql->value);
  ql->count = malloc (ql->max_size * sizeof *ql->count);
  if (ql->value == NULL || ql->count == NULL)
    {
      free (ql->value);
      free (ql->count);
      free (ql);
      return NULL;
    }
  memcpy (ql->value, old_ql->value, ql->max_size * sizeof *ql->value);
  memcpy (ql->count, old_ql->count, ql->max_size * sizeof *ql->count);

  return (ramsey_t *) ql;
}

static void _qlist_destroy (ramsey_t *rt)
{
  struct _qlist *ql = (struct _qlist *) rt;
  assert (rt && rt->type == TYPE_EQUALIZED_LIST);

  free (ql->value);
  free (ql->count);
  free (ql);
}

void *equalized_list_new (const setting_list_t *vars)
{
  struct _qlist *ql = malloc (sizeof *ql);
  ramsey_t *rv = (ramsey_t *) ql;
  (void) vars;

  if (ql == NULL)
    {
      fputs ("OOM creating equalized list!\n", stderr);
      return NULL;
    }

  rv->type = TYPE_EQUALIZED_LIST;
  rv->get_type = _qlist_get_type;

  rv->print   = _qlist_print;
  rv->parse   = _qlist_parse;
  rv->empty   = _qlist_empty;
  rv->reset   = _qlist_reset;
  rv->clone   = _qlist_clone;
  rv->destroy = _qlist_destroy;
  rv->randomize = _qlist_randomize;
  rv->recurse = _qlist_recurse;

  rv->find_value  = _qlist_find_value;
  rv->get_length  = _qlist_get_length;
  rv->get_maximum = _qlist_get_maximum;
  rv->get_n_cells = _qlist_get_n_cells;
  rv->append      = _qlist_append;
  rv->cell_append = _qlist_cell_append;
  rv->deappend    = _qlist_deappend;
  rv->cell_deappend = _qlist_cell_deappend;
  rv->get_priv_data       = _qlist_get_priv_data;
  rv->get_priv_data_const = _qlist_get_priv_data_const;

  rv->add_filter  = _qlist_add_filter;
  rv->run_filters = _qlist_run_filters;

  ql->size = 0;
  ql->max_size = DEFAULT_MAX_LENGTH;
  ql->value = malloc (ql->max_size * sizeof *ql->value);
  ql->count = malloc (ql->max_size * sizeof *ql->count);

  if (ql->value == NULL || ql->count == NULL)
    {
      fputs ("OOM creating equalized list!\n", stderr);
      free (ql->value);
      free (ql->count);
      free (ql);
      rv = NULL;
    }
  return rv;
}

