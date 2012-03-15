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

/*! \file stream.h
 *  \brief Defines stream class and related functions.
 */

#ifndef STREAM_H
#define STREAM_H

#include <stddef.h>

/*! \brief What state the I/O stream is in.
 *
 *  Used when opening a stream, to describe its initial state,
 *  and internally to the stream class. These may be bitmasked
 *  together.
 */
enum e_stream_mode {
  STREAM_CLOSED = 0,  /*!< stream is closed. */
  STREAM_READ = 1,    /*!< stream is open and readable */
  STREAM_WRITE = 2,   /*!< stream is open and writable */
  STREAM_APPEND = 4   /*!< stream is open and writable in append mode */
};

/*! \brief Convienience typedef for struct stream_t */
typedef struct _stream_t stream_t;

/*! \brief Generic stream I/O type */
struct _stream_t {
  /*! \brief Returns a string describing the stream */
  const char *(*get_type) (const stream_t *);

  /*! \brief Opens a stream in a given mode */
  int   (*open)      (stream_t *, enum e_stream_mode);
  /*! \brief Closes a stream */
  void  (*close)     (stream_t *);
  /*! \brief Reads a single line, including trailing newline, from a stream */
  char *(*read_line) (stream_t *);
  /*! \brief Writes the given text to a stream */
  int   (*write)     (stream_t *, const char *);
  /*! \brief Closes a stream and frees its associated resources */
  void  (*destroy)   (stream_t *);
};

/*! \brief Copies the contents of one stream into another, line by line
 *
 *  \param [in] output  The stream to write to
 *  \param [in] input   The stream to read from
 */
void stream_line_copy (stream_t *output, stream_t *input);

/*! \brief Prints formatted text to a stream
 *
 *  \param [in] output  The stream to write to
 *  \param [in] fmt     A format stream (printf style) describing the
 *                      remaining arguments
 */
void stream_printf (stream_t *output, const char *fmt, ...);

#endif
