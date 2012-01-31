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

#ifndef STREAM_H
#define STREAM_H

#include <stddef.h>

enum e_stream_mode {
  STREAM_CLOSED = 0,
  STREAM_READ = 1,
  STREAM_WRITE = 2,
  STREAM_APPEND = 4
};

typedef struct _stream_t stream_t;

struct _stream_t {
  const char *(*get_type) (const stream_t *);

  int   (*open)      (stream_t *, enum e_stream_mode);
  void  (*close)     (stream_t *);
  char *(*read_line) (stream_t *);
  int   (*write)     (stream_t *, const char *);
  void  (*destroy)   (stream_t *);
};

void stream_line_copy (stream_t *output, stream_t *input);
void stream_printf (stream_t *output, const char *fmt, ...);

#endif
