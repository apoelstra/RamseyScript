
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "global.h"
#include "file-stream.h"
#include "ramsey.h"
#include "stream.h"

struct _global_data global;

int main (int argc, char *argv[])
{
  struct _global_data *defs = set_defaults ();

  if (argc > 1)
    {
      defs->in_stream = file_stream_new (argv[1]);
      if (!defs->in_stream->open (defs->in_stream, STREAM_READ))
        {
          fprintf (stderr, "Failed to open script ``%s''\n", argv[1]);
          exit (EXIT_FAILURE);
        }
    }
  else defs->in_stream = stdin_stream_new ();
  defs->out_stream = stdout_stream_new ();

  puts ("Welcome to RamseyScript CLI " VERSION);
  puts ("All code used in this project is public domain.");
  puts ("");

  srand (time (NULL));
  process (defs);

  return 0;
}

