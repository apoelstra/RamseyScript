
#include <stdio.h>
#include <stdlib.h>

#include "global.h"
#include "ramsey.h"
#include "stream.h"

struct _global_data global;

int main (int argc, char *argv[])
{
  struct _global_data *defs = set_defaults ();
  Stream *stm = file_stream_new ("r");

  if (argc > 1)
    {
      if (stm->open (stm, argv[1]) == NULL)
        {
          fprintf (stderr, "Failed to open script ``%s''\n", argv[1]);
          exit (EXIT_FAILURE);
        }
    }
  else stm->_data = stdin;

  defs->in_stream = stm;
  defs->out_stream = file_stream_new ("w");
  defs->out_stream->_data = stdout;

  puts ("Welcome to RamseyScript CLI " VERSION);
  puts ("All code used in this project is public domain.");
  puts ("");
  process (defs);

  return 0;
}

