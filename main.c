
#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include "global.h"
#include "ramsey.h"
#include "stream.h"
#include "gtk-ui.h"

struct _global_data global;

int main (int argc, char *argv[])
{

#if 0
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

  set_defaults ();

  process (stm);

  stm->close (stm);
  file_stream_delete (stm);
#endif

  return run_gtk_ui (argc, argv);
}

