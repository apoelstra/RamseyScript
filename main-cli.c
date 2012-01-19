
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
  srand (time (NULL));

  if (argc < 2)
    { 
      struct _global_data *defs = set_defaults ();
      defs->out_stream = stdout_stream_new ();
      defs->in_stream = stdin_stream_new ();

      puts ("Welcome to RamseyScript CLI " VERSION);
      puts ("All code used in this project is public domain.");
      puts ("");
      puts ("Interactive mode.");

      process (defs);
    }
  else
    {
      int i;
      for (i = 1; argv[i]; ++i)
        {
          struct _global_data *defs = set_defaults ();
          defs->out_stream = stdout_stream_new ();
          defs->in_stream = file_stream_new (argv[i]);
          if (!defs->in_stream->open (defs->in_stream, STREAM_READ))
            fprintf (stderr, "Failed to open script ``%s''\n", argv[i]);
          else
            process (defs);
          defs->in_stream->destroy (defs->in_stream);
          defs->out_stream->destroy (defs->out_stream);
        }
    }

  return 0;
}

