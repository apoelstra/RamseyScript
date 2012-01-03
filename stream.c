
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "stream.h"

#define STRING_DEFAULT_LEN 1000

struct _string_priv_data
{
  char *data;
  char *rw_idx;
  int max_len;
};

/* START file stream functions */
static void *_file_stream_open (Stream *obj, void *data)
{
  switch (obj->type)
    {
    case STREAM_READ:   obj->_data = fopen (data, "r"); break;
    case STREAM_WRITE:  obj->_data = fopen (data, "w"); break;
    case STREAM_APPEND: obj->_data = fopen (data, "a"); break;
    }
  return obj->_data;
}

static void _file_stream_close (Stream *obj)
{
  if (obj->_data != NULL)
    fclose (obj->_data);
  obj->_data = NULL;
}

static char *_file_stream_read_line (Stream *obj)
{
  /* TODO: fix this */
  if (obj->type == STREAM_READ)
    {
      char *rv = malloc (1000);
      if (fgets (rv, 1000, obj->_data))
        return rv;
    }
  return NULL;
}

static int _file_stream_write_line (Stream *obj, char *line)
{
  if (obj->type == STREAM_WRITE)
    return fputs (line, obj->_data);
  return 0;
}

static int _file_stream_eof (Stream *obj)
{
  return feof (obj->_data);
}
/* END file stream functions */

/* START string stream functions */
static void *_string_stream_open (Stream *obj, void *data)
{
  struct _string_priv_data *pd = obj->_data;
  char mode = ((char *) data)[0];

  switch (mode)
    {
    case 'r':
      obj->type = STREAM_READ;
      pd->rw_idx = pd->data;
      break;
    case 'w':
      obj->type = STREAM_WRITE;
      free (pd->data);
      pd->max_len = STRING_DEFAULT_LEN;
      pd->data = malloc (pd->max_len);
      pd->rw_idx = pd->data;
      break;
    }

  return pd->data;
}

static void _string_stream_close (Stream *obj)
{
  struct _string_priv_data *pd = obj->_data;
  free (pd->data);
  pd->data = NULL;
}

static char *_string_stream_read_line (Stream *obj)
{
  if (obj->type == STREAM_READ)
    {
      int copy_len;
      struct _string_priv_data *pd = obj->_data;
      char *rv;

      if (*pd->rw_idx == '\0')
        return NULL;

      copy_len = 1;
      while (pd->rw_idx[copy_len - 1] && pd->rw_idx[copy_len - 1] != '\n')
        ++copy_len;

      rv = malloc (copy_len + 1);
      strncpy (rv, pd->rw_idx, copy_len);
      rv[copy_len] = '\0';
      pd->rw_idx += copy_len;

      /* Skip over \n, but not the terminator */
      if (pd->rw_idx[-1] == '\0')
        --pd->rw_idx;
      return rv;
    }
  return NULL;
}
 
static int _string_stream_write_line (Stream *obj, char *line)
{
  if (obj->type == STREAM_WRITE)
    {
      struct _string_priv_data *pd = obj->_data;
      int add_len = strlen (line);

      while (pd->max_len < pd->rw_idx - pd->data + add_len)
        {
          void *new_mem = realloc (pd->data, pd->max_len * 2);
          if (new_mem == NULL)
            return 0;
          pd->data = new_mem;
          pd->max_len *= 2;
        }

      strcpy (pd->rw_idx, line);
      pd->rw_idx += add_len;
      return add_len;
    }
  return 0;
}

static int _string_stream_eof (Stream *obj)
{
  struct _string_priv_data *pd = obj->_data;
  return (pd->data == '\0');
}
/* END string stream functions */

/* START file stream con/destructors */
Stream *file_stream_new (const char *mode)
{
  Stream *rv;

  if (mode[1] != 0)
    {
      fprintf (stderr, "Unknown mode ``%s'' for file_stream_new().\n", mode);
      return NULL;
    }

  rv = malloc (sizeof *rv);
  if (rv == NULL)
    return NULL;

  switch (mode[0])
    {
    case 'r': rv->type = STREAM_READ; break;
    case 'w': rv->type = STREAM_WRITE; break;
    case 'a': rv->type = STREAM_APPEND; break;
    default:
      fprintf (stderr, "Unknown mode ``%s'' for file_stream_new().\n", mode);
      return NULL;
    }

  rv->open = _file_stream_open;
  rv->close = _file_stream_close;
  rv->read_line = _file_stream_read_line;
  rv->write_line = _file_stream_write_line;
  rv->eof = _file_stream_eof;
  rv->_data = NULL;

  return rv;
}

void file_stream_delete (Stream *stream)
{
  stream->close (stream);
  free (stream);
}
/* END file stream con/destructors */

/* START string stream con/destructors */
Stream *string_stream_new ()
{
  Stream *rv = malloc (sizeof *rv);
  struct _string_priv_data *pd = malloc (sizeof *pd);

  if (rv == NULL || pd == NULL)
    return NULL;

  rv->_data = pd;
  pd->rw_idx = pd->data = NULL;

  rv->open = _string_stream_open;
  rv->close = _string_stream_close;
  rv->read_line = _string_stream_read_line;
  rv->write_line = _string_stream_write_line;
  rv->eof = _string_stream_eof;

  return rv;
}

void string_stream_delete (Stream *stream)
{
  struct _string_priv_data *pd = stream->_data;
  free (pd->data);
  free (pd);
  free (stream);
}
/* END string stream con/destructors */


void stream_line_copy (Stream *output, Stream *input)
{
  char *buf;
  while ((buf = input->read_line (input)))
    {
      output->write_line (output, buf);
      free (buf);
    }
}

