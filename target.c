
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "dump.h"
#include "ramsey.h"
#include "stream.h"

struct _target_priv {
  data_collector_t parent;

  stream_t *out;
  long max_recorded;
};

/* RECORD FUNCTIONS */
static const char *_max_length_get_type (const data_collector_t *dc)
{
  (void) dc;
  return "max-length";
}

static void _max_length_record (data_collector_t *dc, ramsey_t *ram)
{
  long len = ram->get_length (ram);
  struct _target_priv *priv = (struct _target_priv *) dc;

  if (len > priv->max_recorded)
    {
      stream_printf (priv->out, "New maximal %s (length %3d): ",
                     ram->get_type (ram), len);
      ram->print (ram, priv->out);
      stream_printf (priv->out, "\n");
      priv->max_recorded = len;
    }
}

/* GENERIC TARGET FUNCTIONS */
static void _target_reset (data_collector_t *dc)
{
  struct _target_priv *priv = (struct _target_priv *) dc;
  priv->max_recorded = 0;
}

static void _target_output  (const data_collector_t *dc)
{
  const struct _target_priv *priv = (struct _target_priv *) dc;
  stream_printf (priv->out, "Maximum length %ld\n", priv->max_recorded);
}

static void _target_destroy (data_collector_t *dc)
{
  free (dc);
}

data_collector_t *target_new (const char *data, stream_t *out)
{
  struct _target_priv *priv = malloc (sizeof *priv);
  data_collector_t *rv = (data_collector_t *) priv;

  assert (data != NULL);

  if (rv != NULL)
    {
      rv->reset   = _target_reset;
      rv->output  = _target_output;
      rv->destroy = _target_destroy;

      priv->out = out;
      priv->max_recorded = 0;

      if (!strcmp (data, "max_length"))
        {
          rv->get_type = _max_length_get_type;
          rv->record   = _max_length_record;
        }
      else
        {
          fprintf (stderr, "Warning: unknown target ``%s''.\n", data);
          rv->destroy (rv);
          return NULL;
        }
    }
  return rv;
}

