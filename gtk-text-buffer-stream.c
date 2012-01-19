
#include <stdlib.h>
#include <stdio.h>

#include <gtk/gtk.h>

#include "stream.h"

struct _tb_stream
{
  stream_t parent;

  GtkTextBuffer *tb;
  GtkTextIter startln;
  GtkTextIter endln;
  gboolean alive;
};

/* START file stream functions */
static int _text_buffer_open (stream_t *obj, enum e_stream_mode mode)
{
  struct _tb_stream *priv = (struct _tb_stream *) obj;
  gtk_text_buffer_get_start_iter (priv->tb, &priv->startln);
  gtk_text_buffer_get_start_iter (priv->tb, &priv->endln);
  priv->alive = TRUE;
  (void) mode;
  return 1;
}

static void _text_buffer_close (stream_t *obj)
{
  struct _tb_stream *priv = (struct _tb_stream *) obj;
  priv->alive = FALSE;
}

static char *_text_buffer_read_line (stream_t *obj)
{
  struct _tb_stream *priv = (struct _tb_stream *) obj;
  char *rv = NULL;
  if (priv->alive)
    {
      gtk_text_iter_forward_line (&priv->startln);
      rv = gtk_text_iter_get_text (&priv->startln, &priv->endln);
      if (!gtk_text_iter_forward_line (&priv->endln))
        priv->alive = FALSE;
    }

  return rv;
}

static int _text_buffer_write (stream_t *obj, const char *line)
{
  GtkTextIter end;
  struct _tb_stream *priv = (struct _tb_stream *) obj;

  gtk_text_buffer_get_end_iter (priv->tb, &end);
  gtk_text_buffer_insert (priv->tb, &end, line, -1);
  return 0;
}

void _text_buffer_destroy (stream_t *stream)
{
  struct _tb_stream *priv = (struct _tb_stream *) stream;
  if (stream)
    g_object_unref (G_OBJECT (priv->tb));
  free (stream);
}

/* END text stream functions */


stream_t *text_buffer_stream_new (GtkTextBuffer *buff)
{
  struct _tb_stream *priv = malloc (sizeof *priv);
  stream_t *rv = (stream_t *) priv;

  if (buff == NULL)
    {
      fputs ("text_buffer_stream_new: was given NULL buffer!\n", stderr);
      return NULL;
    }

  if (rv == NULL)
    {
      fputs ("text_buffer_stream_new: out of memory!\n", stderr);
      return NULL;
    }

  rv->open = _text_buffer_open;
  rv->close = _text_buffer_close;
  rv->read_line = _text_buffer_read_line;
  rv->write = _text_buffer_write;
  rv->destroy = _text_buffer_destroy;

  priv->tb = buff;
  priv->alive = FALSE;

  g_object_ref (G_OBJECT (buff));

  return rv;
}
