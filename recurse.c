
#include <stdio.h>

#include "global.h"
#include "sequence.h"
#include "coloring.h"
#include "recurse.h"
#include "filters.h"

static long int max_length;
static long int iterations;

void reset_max ()
{
  max_length = 0;
  iterations = 0;
}

long int get_iterations ()
{
  return iterations;
}

void recurse_sequence (Sequence *seed, const struct _global_data *state)
{
  int i;

  if (!state->filter (seed))
    return;
  if (state->iterations && iterations >= state->iterations)
    return;
  if (state->dump_iters && seed->length < state->dump_depth)
    ++state->iters_data->values[seed->length];
  ++iterations;

  if (seed->length > max_length)
    {
      printf ("Got new maximum length %d. Sequence: ", seed->length);
      sequence_print (seed);
      puts ("");
      fflush (stdout);
      max_length = seed->length;
    }

  for (i = 0; i < state->gap_set->length; ++i)
    {
      sequence_append (seed, sequence_max (seed) + state->gap_set->values[i]);
      recurse_sequence (seed, state);
      sequence_deappend (seed);
    }
}

void recurse_words (Sequence *seed, const struct _global_data *state)
{
  int i;

  if (!state->filter (seed))
    return;
  if (state->iterations && iterations >= state->iterations)
    return;
  if (state->dump_iters && seed->length < state->dump_depth)
    ++state->iters_data->values[seed->length];
  ++iterations;

  if (seed->length > max_length)
    {
      printf ("Got new maximum length %d. Word: ", seed->length);
      sequence_print (seed);
      puts ("");
      fflush (stdout);
      max_length = seed->length;
    }

  for (i = 0; i < state->alphabet->length; ++i)
    {
      sequence_append (seed, state->alphabet->values[i]);
      recurse_words (seed, state);
      sequence_deappend (seed);
    }
}

void recurse_colorings (Coloring *seed, int max_value, const struct _global_data *state)
{
  int length = 0;
  int i, j;

  for (i = 0; i < seed->n_colors; ++i)
    {
      if (!state->filter (seed->sequences[i]))
        return;
      length += seed->sequences[i]->length;
    }
  if (state->iterations && iterations >= state->iterations)
    return;
  if (state->dump_iters && length < state->dump_depth)
    ++state->iters_data->values[length];
  ++iterations;

  if (length > max_length)
    {
      printf ("Got new maximum length %d. Coloring: ", length);
      coloring_print (seed);
      puts ("");
      fflush (stdout);
      max_length = length;
    }

  for (j = 0; j < seed->n_colors; ++j)
    {
      if (seed->sequences[j]->length > 0)
        if ((state->max_gap && max_value + 1 - sequence_max (seed->sequences[j]) > state->max_gap) ||
            max_value + 1 - sequence_max (seed->sequences[j]) < state->min_gap)
          continue;

      coloring_append (seed, max_value + 1, j);
      recurse_colorings (seed, max_value + 1, state);
      coloring_deappend (seed, j);

      /* Only bother with one empty cell, since by symmetry they'll
       *  all behave the same. */
      if (seed->sequences[j]->length == 0)
        break;
    }

}



