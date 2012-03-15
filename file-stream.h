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

/*! \brief Create a new stream attached to a file.
 *
 *  \param [in]  filename  The name of the file to be used.
 *
 *  \return A newly-allocated stream, or NULL on failure.
 */
stream_t *file_stream_new (const char *filename);

/*! \brief Create a new write-only stream attached to stdout.
 *
 *  Creates a writable stream attached to stdout. The stream
 *  is permanently in write-mode, and the open/close methods
 *  are no-ops.
 *
 *  \return A newly-allocated stream, or NULL on failure.
 */
stream_t *stdout_stream_new ();

/*! \brief Create a new write-only stream attached to stderr.
 *
 *  Creates a writable stream attached to stderr. The stream
 *  is permanently in write-mode, and the open/close methods
 *  are no-ops.
 *
 *  \return A newly-allocated stream, or NULL on failure.
 */
stream_t *stderr_stream_new ();

/*! \brief Create a new read-only stream attached to stdin.
 *
 *  Creates a readable stream attached to stderr. The stream
 *  is permanently in read-mode, and the open/close methods
 *  are no-ops.
 *
 *  \return A newly-allocated stream, or NULL on failure.
 */
stream_t *stdin_stream_new  ();

#endif
