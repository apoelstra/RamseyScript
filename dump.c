
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "dump.h"
#include "ramsey.h"
#include "stream.h"

struct _dump_priv {
  dump_t parent;

  stream_t *out;
  int size;
  int *data;
};

/* RECORD FUNCTIONS */
static const char *_iters_per_length_get_type (const dump_t *dump)
{
  (void) dump;
  return "iterations-per-length";
}

static void _iters_per_length_record (dump_t *dump, ramsey_t *ram)
{
  int idx = ram->get_length (ram);
  struct _dump_priv *priv = (struct _dump_priv *) dump;
  if (idx >= 0 && idx <= priv->size)
    ++priv->data[idx];
}

/* GENERIC DUMP FUNCTIONS */
static void _dump_reset (dump_t *dump)
{
  const struct _dump_priv *priv = (struct _dump_priv *) dump;
  int i;
  for (i = 0; i <= priv->size; ++i)
    priv->data[i] = 0;
}

static void _dump_output  (const dump_t *dump)
{
  const struct _dump_priv *priv = (struct _dump_priv *) dump;
  int i;
  priv->out->open (priv->out, STREAM_APPEND);
  stream_printf (priv->out, "[ %d", priv->data[1]);
  for (i = 2; i < priv->size; ++i)
    stream_printf (priv->out, ", %d", priv->data[i]);
  stream_printf (priv->out, " ]\n");
  priv->out->close (priv->out);
}

static void _dump_destroy (dump_t *dump)
{
  struct _dump_priv *priv = (struct _dump_priv *) dump;
  if (dump)
    free (priv->data);
  free (dump);
}

dump_t *dump_new (const char *data, size_t size, stream_t *out)
{
  struct _dump_priv *priv = malloc (sizeof *priv);
  dump_t *rv = (dump_t *) priv;

  assert (data != NULL);

  if (rv != NULL)
    {
      rv->reset   = _dump_reset;
      rv->output  = _dump_output;
      rv->destroy = _dump_destroy;

      priv->out  = out;
      priv->size = size;
      priv->data = malloc ((1 + size) * sizeof *priv->data);
      if (priv->data == NULL)
        {
          rv->destroy (rv);
          return NULL;
        }

      if (!strcmp (data, "iterations_per_length"))
        {
          rv->get_type = _iters_per_length_get_type;
          rv->record   = _iters_per_length_record;
        }
      else
        {
          fprintf (stderr, "Warning: unknown dump type ``%s''.\n",
                   data);
          rv->destroy (rv);
          return NULL;
        }
    }
  return rv;
}

