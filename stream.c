
#include <stdlib.h>
#include <stdio.h>

#include "stream.h"

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

void stream_line_copy (Stream *output, Stream *input)
{
  char *buf;
  while ((buf = input->read_line (input)))
    {
      output->write_line (output, buf);
      free (buf);
    }
}

