
#include <check.h>

#include <stdio.h>

#include "../file-stream.h"

START_TEST (stdio_test);
{
  int i;
  stream_t *stdo = stdout_stream_new ();
  stream_t *stde = stderr_stream_new ();
  stream_t *stdi = stdin_stream_new ();

  for (i = 0; i < 2; ++i)
    {
      if (stdo->write (stdo, "Testing stdout.\n") == EOF)
        fail ("Failed testing stdout output.");
      if (stdo->write (stde, "Testing stderr.\n") == EOF)
        fail ("Failed testing stderr output.");
      if (stdo->write (stdi, "Testing stdin.\n") != EOF)
        fail ("Failed testing stdin output.");

      if (stdo->read_line (stdo) != NULL)
        fail ("Failed testing stdout input.");
      if (stde->read_line (stde) != NULL)
        fail ("Failed testing stderr input.");

      /* These should have no effect. */
      stdo->close (stdo);
      stde->close (stde);
      stdi->close (stdi);
    }

  stdo->destroy (stdo);
  stde->destroy (stde);
  stdi->destroy (stdi);
}
END_TEST

