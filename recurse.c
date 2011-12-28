
#include <stdio.h>

#include "sequence.h"
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

