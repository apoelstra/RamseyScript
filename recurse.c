
#include <stdio.h>

#include "sequence.h"
#include "coloring.h"
#include "recurse.h"
#include "filters.h"

void recurse_sequence (Sequence *seed, int min, int max, filter_func filter)
{
  static int max_length;
  int i;

  if (!filter (seed))
    return;

  if (seed->length > max_length)
    {
      printf ("Got new maximum length %d. Sequence: ", seed->length);
      sequence_print (seed);
      max_length = seed->length;
    }

  for (i = sequence_max (seed) + min; max == 0 || i <= sequence_max(seed) + max; ++i)
    {
      sequence_append (seed, i);
      recurse_sequence (seed, min, max, filter);
      sequence_deappend (seed);
    }
}

void recurse_words (Sequence *seed, Sequence *alphabet, filter_func filter)
{
  static int max_length;
  int i;

  if (!filter (seed))
    return;

  if (seed->length > max_length)
    {
      printf ("Got new maximum length %d. Word: ", seed->length);
      sequence_print (seed);
      max_length = seed->length;
    }

  for (i = 0; i < alphabet->length; ++i)
    {
      sequence_append (seed, alphabet->values[i]);
      recurse_words (seed, alphabet, filter);
      sequence_deappend (seed);
    }
}

void recurse_colorings (Coloring *seed, int max_value, int min,
                        int max, filter_func filter)
{
  static int max_length;
  int length = 0;
  int i, j;

  for (i = 0; i < seed->n_colors; ++i)
    {
      if (!filter (seed->sequences[i]))
        return;
      length += seed->sequences[i]->length;
    }

  if (length > max_length)
    {
      printf ("Got new maximum length %d. Coloring: ", length);
      coloring_print (seed);
      max_length = length;
    }

  for (j = 0; j < seed->n_colors; ++j)
    {
      if (max)
        if (seed->sequences[j]->length > 0)
          if (max_value + 1 - sequence_max (seed->sequences[j]) > max ||
              max_value + 1 - sequence_max (seed->sequences[j]) < min)
            continue;

      coloring_append (seed, max_value + 1, j);
      recurse_colorings (seed, max_value + 1, min, max, filter);
      coloring_deappend (seed, j);

      /* Only bother with one empty cell, since by symmetry they'll
       *  all behave the same. */
      if (seed->sequences[j]->length == 0)
        break;
    }

}



