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

/*! \file string-stream.h
 *  \brief Defines string-based streams and related functions.
 */

#ifndef STRING_STREAM_H
#define STRING_STREAM_H

/*! \brief Creates a new string-based stream.
 *
 *  String-based streams are weird, in short, and are only used for
 *  interfacing with the GUI. (In fact, they depend on GLib to be
 *  thread-safe, so they can NOT be used in code that should compile
 *  without it.)
 *
 *  They are the only thread-safe stream type, so they should be used
 *  to interface with any threaded UI.
 *
 *  String streams can be opened arbitrarily many times. Opening in
 *  STREAM_READ mode resets the read-pointer to the beginning of the
 *  string. Opening in any other mode resets both the read-pointer
 *  and the write-pointer to the beginning of the string.
 *
 *  It should not be possible to cause a buffer overflow when using
 *  a string-based stream. Memory management is done internally to
 *  the object.
 *
 *  \return A newly-allocated stream, or NULL on failure.
 */
stream_t *string_stream_new ();

#endif
