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

#ifndef FILE_STREAM_H
#define FILE_STREAM_H

#include "stream.h"

stream_t *file_stream_new (const char *filename);
stream_t *stdout_stream_new ();
stream_t *stderr_stream_new ();
stream_t *stdin_stream_new  ();

#endif
