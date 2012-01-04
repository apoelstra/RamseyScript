
#include <stdlib.h>
#include <stdio.h>

#include <gtk/gtk.h>

#include "stream.h"

struct priv_data
{
  GtkTextBuffer *tb;
  GtkTextIter startln;
  GtkTextIter endln;
  gboolean alive;
};

/* START file stream functions */
static void *_text_buffer_open (Stream *obj, void *data)
{
  struct priv_data *pd = obj->_data;
  gtk_text_buffer_get_start_iter (pd->tb, &pd->startln);
  gtk_text_buffer_get_start_iter (pd->tb, &pd->endln);
  pd->alive = TRUE;

  switch (((char *)data) [0])
    {
    case 'r': obj->type = STREAM_READ;  break;
    case 'w': obj->type = STREAM_WRITE; break;
    }

  return obj->_data;
}

static void _text_buffer_close (Stream *obj)
{
  struct priv_data *pd = obj->_data;
  pd->alive = FALSE;
}

static char *_text_buffer_read_line (Stream *obj)
{
  struct priv_data *pd = obj->_data;
  char *rv = NULL;
  if (pd->alive)
    {
      gtk_text_iter_forward_line (&pd->startln);
      rv = gtk_text_iter_get_text (&pd->startln, &pd->endln);
      if (!gtk_text_iter_forward_line (&pd->endln))
        pd->alive = FALSE;
    }

  return rv;
}

static int _text_buffer_write_line (Stream *obj, const char *line)
{
  GtkTextIter end;
  struct priv_data *pd = obj->_data;

  gtk_text_buffer_get_end_iter (pd->tb, &end);
  gtk_text_buffer_insert (pd->tb, &end, line, -1);
  return 0;
}


static int _text_buffer_eof (Stream *obj)
{
  struct priv_data *pd = obj->_data;
  return pd->alive;
}

void _text_buffer_destroy (Stream *stream)
{
  struct priv_data *pd = stream->_data;
  if (pd)
    g_object_unref (G_OBJECT (pd->tb));
  free (stream->_data);
  free (stream);
}

/* END text stream functions */


Stream *text_buffer_stream_new (GtkTextBuffer *buff)
{
  Stream *rv = malloc (sizeof *rv);
  struct priv_data *pd = malloc (sizeof *pd);

  if (buff == NULL)
    {
      fputs ("text_buffer_stream_new: was given NULL buffer!\n", stderr);
      return NULL;
    }

  if (rv == NULL || pd == NULL)
    {
      fputs ("text_buffer_stream_new: out of memory!\n", stderr);
      return NULL;
    }

  rv->open = _text_buffer_open;
  rv->close = _text_buffer_close;
  rv->read_line = _text_buffer_read_line;
  rv->write_line = _text_buffer_write_line;
  rv->eof = _text_buffer_eof;
  rv->destroy = _text_buffer_destroy;
  rv->_data = pd;

  pd->tb = buff;
  pd->alive = FALSE;

  g_object_ref (G_OBJECT (buff));

  return rv;
}
