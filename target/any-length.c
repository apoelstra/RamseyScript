
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "target.h"

struct _target_priv {
  data_collector_t parent;

  stream_t *out;
};

static const char *_target_get_type (const data_collector_t *dc)
{
  (void) dc;
  return "any-length";
}

static void _target_record (data_collector_t *dc, ramsey_t *ram)
{
  long len = ram->get_length (ram);
  struct _target_priv *priv = (struct _target_priv *) dc;

  assert (dc != NULL);
  assert (ram != NULL);

  stream_printf (priv->out, "Found %s (length %3d): ",
                 ram->get_type (ram), len);
  ram->print (ram, priv->out);
  stream_printf (priv->out, "\n");
}

static void _target_reset (data_collector_t *dc)
{
  (void) dc;
}

static void _target_output  (const data_collector_t *dc)
{
  (void) dc;
}

static void _target_destroy (data_collector_t *dc)
{
  free (dc);
}

void *target_any_length_new (const global_data_t *state)
{
  struct _target_priv *priv = malloc (sizeof *priv);
  data_collector_t *rv = (data_collector_t *) priv;

  if (rv != NULL)
    {
      rv->reset   = _target_reset;
      rv->output  = _target_output;
      rv->destroy = _target_destroy;

      priv->out = state->out_stream;
      rv->get_type = _target_get_type;
      rv->record   = _target_record;
    }
  return rv;
}

