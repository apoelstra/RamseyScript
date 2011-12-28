
#include <stdio.h>

#include "sequence.h"
#include "recurse.h"
#include "check.h"

void recurse_sequence3 (Sequence *seed, int min, int max)
{
  static int max_length;
  int i;

  if (seed->length >= 3 && !cheap_check_sequence3 (seed))
    return;

  if (seed->length > max_length)
    {
      printf ("Got new maximum length %d. Sequence: ", seed->length);
      sequence_print (seed);
      max_length = seed->length;
    }

  for (i = sequence_max (seed) + min; i <= sequence_max(seed) + max; ++i)
    {
      sequence_append (seed, i);
      recurse_sequence3 (seed, min, max);
      sequence_deappend (seed);
    }
}

