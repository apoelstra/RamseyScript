
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "global.h"
#include "ramsey.h"
#include "stream.h"
#include "gtk-ui.h"

struct _global_data global;

int main (int argc, char *argv[])
{
  srand (time (NULL));
  return run_gtk_ui (argc, argv);
}

