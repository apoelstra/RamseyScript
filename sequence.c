
#include <stdio.h>
#include <stdlib.h>

#include "sequence.h"

Sequence *sequence_new ()
{
  Sequence *s = malloc (sizeof *s);
  if (s != NULL)
    {
      s->length = 0;
      s->_max_length = DEFAULT_MAX_LENGTH;
      s->values = malloc (s->_max_length * sizeof *s->values);
      if (s->values == NULL)
        {
          free (s);
          s = NULL;
        }
    }
  return s;
}

Sequence *sequence_parse (const char *data)
{
  int value;
  Sequence *rv = sequence_new ();

  if (rv == NULL)
    return NULL;

  while (*data && *data != '[')
    ++data;
  ++data;

  while ((value = strtoul (data, (char **) &data, 0)))
    if (!sequence_append(rv, value))
      {
        sequence_delete (rv);
        return NULL;
      }

  return rv;
}

int sequence_append (Sequence *s, int value)
{
  if (s->length == s->_max_length)
    {
      void *tmp = realloc (s->values, 2 * s->_max_length * sizeof *s->values);
      if (tmp == NULL)
        return 0;
      s->_max_length *= 2;
      s->values = tmp;
    }
  s->values[s->length++] = value;
  return 1;
}

void sequence_deappend (Sequence *s)
{
  if (s->length)
    --s->length;
}

void sequence_print (Sequence *s)
{
  int i;
  printf ("[");
  if (s->length)
    printf ("%d", s->values[0]);
  for (i = 1; i < s->length; ++i)
    printf (", %d", s->values[i]);
  printf ("]\n");
}

void sequence_delete (Sequence *s)
{
  if (s)
    free (s->values);
  free(s);
}

