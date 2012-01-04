
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include <gtk/gtk.h>

#include "stream.h"
#include "string-stream.h"

#define STRING_DEFAULT_LEN 1000

struct _string_priv_data
{
  char *data;
  char *read_idx;
  char *write_idx;
  int max_len;
  GMutex *mutex;
};

static void *_string_stream_open (Stream *obj, void *data)
{
  struct _string_priv_data *pd = obj->_data;
  char mode = ((char *) data)[0];

  g_mutex_lock (pd->mutex);
  switch (mode)
    {
    case 'r':
      obj->type |= STREAM_READ;
      pd->read_idx = pd->data;
      break;
    case 'w':
      obj->type |= STREAM_WRITE | STREAM_READ;
      free (pd->data);
      pd->max_len = STRING_DEFAULT_LEN;
      pd->data = malloc (pd->max_len);
      pd->read_idx = pd->write_idx = pd->data;
      break;
    }
  g_mutex_unlock (pd->mutex);

  return pd->data;
}

static void _string_stream_close (Stream *obj)
{
  struct _string_priv_data *pd = obj->_data;
  g_mutex_lock (pd->mutex);
  free (pd->data);
  pd->read_idx = pd->write_idx = pd->data = NULL;
  g_mutex_unlock (pd->mutex);
}

static char *_string_stream_read_line (Stream *obj)
{
  if (obj->type & STREAM_READ)
    {
      int copy_len;
      struct _string_priv_data *pd = obj->_data;
      char *rv;

      g_mutex_lock (pd->mutex);
      if (*pd->read_idx == '\0')
        {
          g_mutex_unlock (pd->mutex);
          return NULL;
        }

      copy_len = 1;
      while (pd->read_idx[copy_len - 1] &&
             pd->read_idx[copy_len - 1] != '\n')
        ++copy_len;

      rv = malloc (copy_len + 1);
      strncpy (rv, pd->read_idx, copy_len);
      rv[copy_len] = '\0';
      pd->read_idx += copy_len;

      /* Skip over \n, but not the terminator */
      if (pd->read_idx[-1] == '\0')
        --pd->read_idx;
      g_mutex_unlock (pd->mutex);
      return rv;
    }
  return NULL;
}
 
static int _string_stream_write_line (Stream *obj, const char *line)
{
  if (obj->type & STREAM_WRITE)
    {
      struct _string_priv_data *pd = obj->_data;
      int windex = pd->write_idx - pd->data;
      int rindex = pd->read_idx - pd->data;
      int add_len;

      g_mutex_lock (pd->mutex);
      add_len = strlen (line);
      while (pd->max_len < windex + add_len + 1)
        {
          void *new_mem = realloc (pd->data, pd->max_len * 2);
          if (new_mem == NULL)
            {
              g_mutex_unlock (pd->mutex);
              return 0;
            }
          pd->data = new_mem;
          pd->write_idx = pd->data + windex;
          pd->read_idx  = pd->data + rindex;
          pd->max_len *= 2;
        }

      strcpy (pd->write_idx, line);
      pd->write_idx += add_len;
      g_mutex_unlock (pd->mutex);
      return add_len;
    }
  return 0;
}

static int _string_stream_eof (Stream *obj)
{
  struct _string_priv_data *pd = obj->_data;
  return (pd->data == '\0');
}

void _string_stream_destroy (Stream *stream)
{
  struct _string_priv_data *pd = stream->_data;
  g_mutex_free (pd->mutex);
  free (pd->data);
  free (pd);
  free (stream);
}

/* START string stream con/destructors */
Stream *string_stream_new ()
{
  Stream *rv = malloc (sizeof *rv);
  struct _string_priv_data *pd = malloc (sizeof *pd);

  if (rv == NULL || pd == NULL)
    return NULL;

  rv->type = 0;
  rv->_data = pd;
  pd->read_idx = pd->write_idx = pd->data = NULL;
  pd->mutex = g_mutex_new ();

  rv->open = _string_stream_open;
  rv->close = _string_stream_close;
  rv->read_line = _string_stream_read_line;
  rv->write_line = _string_stream_write_line;
  rv->eof = _string_stream_eof;
  rv->destroy = _string_stream_destroy;

  return rv;
}
/* END string stream con/destructors */

