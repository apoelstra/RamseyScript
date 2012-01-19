
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "global.h"
#include "file-stream.h"

#define DEFAULT_READ_LEN	1000

struct _file_stream {
  stream_t parent;

  char *filename;
  enum e_stream_mode mode;
  FILE *fh;
};

/* OPEN / CLOSE */
static int _file_stream_open (stream_t *s, enum e_stream_mode mode)
{
  struct _file_stream *priv = (struct _file_stream *) s;
  if (priv->mode != STREAM_CLOSED)
    {
      fprintf (stderr, "Tried to open stream that was already open!\n");
      return 0;
    }

  priv->fh = NULL;
  switch ((int) mode)
    {
    case STREAM_READ:
      priv->fh = fopen (priv->filename, "r");
      break;
    case STREAM_WRITE:
      priv->fh = fopen (priv->filename, "w");
      break;
    case STREAM_APPEND:
      priv->fh = fopen (priv->filename, "a");
      break;
    case STREAM_READ | STREAM_WRITE:
      priv->fh = fopen (priv->filename, "r+");
      break;
    case STREAM_READ | STREAM_APPEND:
      priv->fh = fopen (priv->filename, "a+");
      break;
    default:
      fprintf (stderr, "Invalid mode. ");
    }

  if (priv->fh == NULL)
    {
      fprintf (stderr, "Failed to open ``%s''.", priv->filename);
      return 0;
    }
  priv->mode = mode;
  return 1;
}

static void _file_stream_close (stream_t *s)
{
  struct _file_stream *priv = (struct _file_stream *) s;
  if (priv->mode == STREAM_CLOSED)
    return;
  priv->mode = STREAM_CLOSED;
  fclose (priv->fh);
  priv->fh = NULL;
}

/* READ / WRITE */
static char *_file_stream_read_line (stream_t *s)
{
  char *rv = NULL;
  struct _file_stream *priv = (struct _file_stream *) s;

  if (priv->mode & STREAM_READ)
    {
      int len = DEFAULT_READ_LEN;
      do
        {
          char *mem = realloc (rv, len);
          if (mem == NULL)
            {
              free (rv);
              return NULL;
            }

          rv = mem;
          rv[len - 1] = '\n';
          if (!fgets (rv, len, priv->fh))
            {
              free (rv);
              return NULL;
            }
          len *= 2;
        }
      while (rv[len/2 - 1] != '\n');
    }

  return rv;
}

static int _file_stream_write (stream_t *s, const char *line)
{
  struct _file_stream *priv = (struct _file_stream *) s;

  if (priv->mode & (STREAM_WRITE | STREAM_APPEND))
    return fputs (line, priv->fh);
  return 0;
}

/* DESTRUCTOR */
static void _file_stream_destroy (stream_t *s)
{
  struct _file_stream *priv = (struct _file_stream *) s;
  free (priv->filename);
  s->close (s);
  free (s);
}

/* noop functions for special streams */
static int _special_stream_open (stream_t *s, enum e_stream_mode mode)
{
  (void) s;
  (void) mode;
  return 1;
}

static void _special_stream_close (stream_t *s)
{
  (void) s;
}

static void _special_stream_destroy (stream_t *s)
{
  free (s);
}

/* CONSTRUCTORS */
static stream_t *_file_stream_new_real ()
{
  struct _file_stream *priv = malloc (sizeof *priv);
  stream_t *rv = (stream_t *) priv;
  if (rv)
    {
      rv->open    = _file_stream_open;
      rv->close   = _file_stream_close;
      rv->read_line = _file_stream_read_line;
      rv->write   = _file_stream_write;
      rv->destroy = _file_stream_destroy;

      priv->filename = NULL;
      priv->mode = STREAM_CLOSED;
      priv->fh = NULL;
    }
  return rv;
}

/* special cases */
stream_t *file_stream_new (const char *filename)
{
  stream_t *rv = NULL;

  if (filename)
    {
      struct _file_stream *priv;
      rv = _file_stream_new_real ();
      priv = (struct _file_stream *) rv;
      priv->filename = malloc (strlen (filename) + 1);
      if (priv->filename)
        strcpy (priv->filename, filename);
    }
  return rv;
}

stream_t *stdout_stream_new ()
{
  stream_t *rv = _file_stream_new_real ();
  struct _file_stream *priv = (struct _file_stream *) rv;

  if (rv)
    {
      priv->fh = stdout;
      priv->mode = STREAM_WRITE;
      rv->destroy = _special_stream_destroy;
      rv->close = _special_stream_close;
      rv->open = _special_stream_open;
    }
  return rv;
}

stream_t *stderr_stream_new ()
{
  stream_t *rv = _file_stream_new_real ();
  struct _file_stream *priv = (struct _file_stream *) rv;

  if (rv)
    {
      priv->fh = stderr;
      priv->mode = STREAM_WRITE;
      rv->destroy = _special_stream_destroy;
      rv->close = _special_stream_close;
      rv->open = _special_stream_open;
    }
  return rv;
}

stream_t *stdin_stream_new  ()
{
  stream_t *rv = _file_stream_new_real ();
  struct _file_stream *priv = (struct _file_stream *) rv;

  if (rv)
    {
      priv->fh = stdin;
      priv->mode = STREAM_READ;
      rv->destroy = _special_stream_destroy;
      rv->close = _special_stream_close;
      rv->open = _special_stream_open;
    }
  return rv;
}

