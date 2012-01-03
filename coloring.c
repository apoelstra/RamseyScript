
#include <stdio.h>
#include <stdlib.h>

#include "sequence.h"
#include "coloring.h"
#include "stream.h"

Coloring *coloring_new (int n_colors)
{
  Coloring *c = malloc (sizeof *c);
  if (c != NULL)
    {
      c->n_colors = n_colors;
      c->sequences = malloc (c->n_colors * sizeof *c->sequences);
      if (c->sequences == NULL)
        {
          free (c);
          c = NULL;
        }
      else
        {
          int i;
          for (i = 0; i < c->n_colors; ++i)
            c->sequences[i] = sequence_new ();
        }
    }
  return c;
}

int coloring_append (Coloring *c, int value, int position)
{
  return sequence_append (c->sequences[position], value);
}

void coloring_deappend (Coloring *c, int position)
{
  sequence_deappend (c->sequences[position]);
}

void coloring_print (Coloring *c, Stream *out)
{
  int i;
  out->write_line (out, "[");
  sequence_print (c->sequences[0], out);
  for (i = 1; i < c->n_colors; ++i)
    {
      out->write_line (out, " ");
      sequence_print (c->sequences[i], out);
    }
  out->write_line (out, "]");
}

void coloring_delete (Coloring *c)
{
  int i;
  if (c)
    {
      for (i = 0; i < c->n_colors; ++i)
        sequence_delete (c->sequences[i]);
      free (c->sequences);
      free(c);
    }
}

