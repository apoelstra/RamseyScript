
#include <stdio.h>
#include <stdlib.h>

#include "global.h"
#include "ramsey.h"

struct _global_data global;

int main (int argc, char *argv[])
{
  FILE *fh;

  if (argc > 1)
    {
      fh = fopen (argv[1], "r");
      if (fh == NULL)
        {
          fprintf (stderr, "Failed to open script ``%s''\n", argv[1]);
          exit (EXIT_FAILURE);
        }
    }
  else fh = stdin;

  set_defaults ();
  process (fh);

  if (fh != NULL && fh != stdin)
    fclose (fh);

  return 0;
}

