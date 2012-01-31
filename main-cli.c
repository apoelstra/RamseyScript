/* RamseyScript
 * Written in 2012 by
 *   Andrew Poelstra <apoelstra@wpsoftware.net>
 *
 * To the extent possible under law, the author(s) have dedicated all
 * copyright and related and neighboring rights to this software to
 * the public domain worldwide. This software is distributed without
 * any warranty.
 *
 * You should have received a copy of the CC0 Public Domain Dedication
 * along with this software.
 * If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
 */


#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "global.h"
#include "file-stream.h"
#include "process.h"

struct _global_data global;

int main (int argc, char *argv[])
{
  srand (time (NULL));

  if (argc < 2)
    { 
      struct _global_data *defs = set_defaults (stdin_stream_new (),
                                                stdout_stream_new (),
                                                stderr_stream_new ());
      defs->interactive = 1;

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
          struct _global_data *defs = set_defaults (file_stream_new (argv[i]),
                                                    stdout_stream_new (),
                                                    stderr_stream_new ());
          if (!defs->in_stream->open (defs->in_stream, STREAM_READ))
            fprintf (stderr, "Failed to open script ``%s''\n", argv[i]);
          else
            process (defs);
          defs->in_stream->destroy (defs->in_stream);
          defs->out_stream->destroy (defs->out_stream);
          defs->err_stream->destroy (defs->err_stream);
        }
    }

  return 0;
}

