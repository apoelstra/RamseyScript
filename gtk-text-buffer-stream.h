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

/*! \file gtk-text-buffer-stream.h
 *  \brief Defines GtkTextBuffer-based streams and related functions.
 */

#ifndef GTK_TEXT_BUFFER_STREAM_H
#define GTK_TEXT_BUFFER_STREAM_H

#include "stream.h"

/*! \brief Create a new GtkTextBuffer-based stream.
 *
 *  This stream type lets GtkTextBuffer interface with the rest of
 *  the code. For example, you can run a script directly from the
 *  buffer of a text view, and capture the output to another text
 *  view.
 *
 *  However, it is recommended to use a string_stream_t as an intermediary,
 *  in order to get thread safety (if this is a concern), and because
 *  running scripts takes a long time, so a user will be able to modify a
 *  GtkTextBuffer stream while it is running, potentially invalidating
 *  iterators and causing EOF to be read prematurely.
 *
 *  Calling open() always resets the read pointer, regardless of what mode
 *  is passed, and writing is always done to the end of the buffer. 
 *
 *  \param [in] buff  The GtkTextBuffer that the stream should be based on.
 *                    The stream will keep a ref to this object.
 *
 *  \return A newly-allocated stream, or NULL on failure.
 */
stream_t *text_buffer_stream_new (GtkTextBuffer *buff);

#endif
