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


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include <gtk/gtk.h>

#include "stream.h"
#include "string-stream.h"

/*! \file string-stream.c 
 *  \brief Implementation of a string-based stream. */

/*! \brief The initial size of the internal string buffer of a string. */
#define STRING_DEFAULT_LEN 1000

/*! \brief Private data for the string stream type. */
struct _string_stream
{
  /*! \brief parent struct. */
  stream_t parent;

  /*! \brief Read/write state of the stream. */
  enum e_stream_mode mode;

  /*! \brief Low-level string data. */
  char *data;
  /*! \brief Read cursor pointing into the string. */
  char *read_idx;
  /*! \brief Write cursor pointing into the string. */
  char *write_idx;
  /*! \brief Amount of memory currently allocated to the string. */
  int max_len;
  /*! \brief for thread-safety. */
  GMutex *mutex;
};

static int _string_stream_open (stream_t *obj, enum e_stream_mode mode)
{
  struct _string_stream *priv = (struct _string_stream *) obj;

  g_mutex_lock (priv->mutex);
  switch ((int) mode)
    {
    case STREAM_READ:
      priv->mode = STREAM_READ;
      priv->read_idx = priv->data;
      break;
    case STREAM_WRITE:
    case (STREAM_READ | STREAM_WRITE):
      priv->mode = STREAM_WRITE | STREAM_READ;
      free (priv->data);
      priv->max_len = STRING_DEFAULT_LEN;
      priv->data = malloc (priv->max_len);
      priv->read_idx = priv->write_idx = priv->data;
      if (priv->data == NULL)
        return 0;
      break;
    }
  g_mutex_unlock (priv->mutex);

  return 1;
}

static void _string_stream_close (stream_t *obj)
{
  struct _string_stream *priv = (struct _string_stream *) obj;
  g_mutex_lock (priv->mutex);
  free (priv->data);
  priv->read_idx = priv->write_idx = priv->data = NULL;
  g_mutex_unlock (priv->mutex);
}

static char *_string_stream_read_line (stream_t *obj)
{
  struct _string_stream *priv = (struct _string_stream *) obj;
  g_mutex_lock (priv->mutex);
  if (priv->mode & STREAM_READ)
    {
      int copy_len;
      char *rv;

      if (*priv->read_idx == '\0')
        {
          g_mutex_unlock (priv->mutex);
          return NULL;
        }

      copy_len = 1;
      while (priv->read_idx[copy_len - 1] &&
             priv->read_idx[copy_len - 1] != '\n')
        ++copy_len;

      rv = malloc (copy_len + 1);
      strncpy (rv, priv->read_idx, copy_len);
      rv[copy_len] = '\0';
      priv->read_idx += copy_len;

      /* Skip over \n, but not the terminator */
      if (priv->read_idx[-1] == '\0')
        --priv->read_idx;
      g_mutex_unlock (priv->mutex);
      return rv;
    }
  g_mutex_unlock (priv->mutex);
  return NULL;
}
 
static int _string_stream_write (stream_t *obj, const char *line)
{
  struct _string_stream *priv = (struct _string_stream *) obj;
  g_mutex_lock (priv->mutex);
  if (priv->mode & STREAM_READ)
    {
      int windex = priv->write_idx - priv->data;
      int rindex = priv->read_idx - priv->data;
      int add_len;

      add_len = strlen (line);
      while (priv->max_len < windex + add_len + 1)
        {
          void *new_mem = realloc (priv->data, priv->max_len * 2);
          if (new_mem == NULL)
            {
              g_mutex_unlock (priv->mutex);
              return 0;
            }
          priv->data = new_mem;
          priv->write_idx = priv->data + windex;
          priv->read_idx  = priv->data + rindex;
          priv->max_len *= 2;
        }

      strcpy (priv->write_idx, line);
      priv->write_idx += add_len;
      g_mutex_unlock (priv->mutex);
      return add_len;
    }
  g_mutex_unlock (priv->mutex);
  return 0;
}

static void _string_stream_destroy (stream_t *stream)
{
  struct _string_stream *priv = (struct _string_stream *) stream;
  g_mutex_free (priv->mutex);
  free (stream);
}

/* START string stream con/destructors */
stream_t *string_stream_new ()
{
  struct _string_stream *priv = malloc (sizeof *priv);
  stream_t *rv = (stream_t *) priv;

  if (rv == NULL)
    return NULL;

  priv->mode = STREAM_CLOSED;
  priv->read_idx = priv->write_idx = priv->data = NULL;
  priv->mutex = g_mutex_new ();

  rv->open = _string_stream_open;
  rv->close = _string_stream_close;
  rv->read_line = _string_stream_read_line;
  rv->write = _string_stream_write;
  rv->destroy = _string_stream_destroy;

  return rv;
}
/* END string stream con/destructors */

