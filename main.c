
#include <time.h>
#include <stdlib.h>

#include "gtk-ui.h"

int main (int argc, char *argv[])
{
  srand (time (NULL));
  return run_gtk_ui (argc, argv);
}

