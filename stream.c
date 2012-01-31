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


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "stream.h"

#define PRINT_BUFSIZ	4000
void stream_printf (stream_t *output, const char *fmt, ...)
{
  va_list args;
  char buf[PRINT_BUFSIZ];

  va_start(args, fmt);
  vsnprintf (buf, PRINT_BUFSIZ, fmt, args);  
  va_end(args);
  output->write (output, buf);
}

void stream_line_copy (stream_t *output, stream_t *input)
{
  char *buf;
  while ((buf = input->read_line (input)))
    {
      output->write (output, buf);
      free (buf);
    }
}

