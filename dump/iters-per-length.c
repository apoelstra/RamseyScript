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


#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "dump.h"
#include "../file-stream.h"

struct _dump_priv {
  data_collector_t parent;

  int size;
  long int *data;
};

static const char *_dump_get_type (const data_collector_t *dc)
{
  (void) dc;
  return "iterations-per-length";
}

static int _dump_record (data_collector_t *dc, const ramsey_t *ram, stream_t *out)
{
  int idx = ram->get_length (ram);
  struct _dump_priv *priv = (struct _dump_priv *) dc;
  (void) out;
  if (idx >= 0 && idx <= priv->size)
    {
      ++priv->data[idx];
      return 1;
    }
  return 0;
}

static void _dump_reset (data_collector_t *dc)
{
  const struct _dump_priv *priv = (struct _dump_priv *) dc;
  int i;
  for (i = 0; i <= priv->size; ++i)
    priv->data[i] = 0;
}

static void _dump_output  (const data_collector_t *dc, stream_t *out)
{
  const struct _dump_priv *priv = (struct _dump_priv *) dc;
  int i;
  if (out)
    {
      out->open (out, STREAM_APPEND);
      stream_printf (out, "[ %ld", priv->data[1]);
      for (i = 2; i < priv->size; ++i)
        stream_printf (out, ", %ld", priv->data[i]);
      stream_printf (out, " ]\n");
      out->close (out);
    }
}

static void _dump_destroy (data_collector_t *dc)
{
  struct _dump_priv *priv = (struct _dump_priv *) dc;
  if (dc)
    free (priv->data);
  free (dc);
}

void *dump_iters_per_length_new (const global_data_t *state)
{
  struct _dump_priv *priv;
  data_collector_t *rv;
  const setting_t *dump_depth_set = SETTING ("dump_depth");
  const setting_t *dump_file_set  = SETTING ("dump_file");
  stream_t *dump_stream;
  int dump_depth = 0;

  /* Parse variablse */
  if (dump_depth_set == NULL)
    {
      fprintf (stderr, "Error: need variable ``dump-depth'' to be set.\n");
      return NULL;
    }
  if (dump_file_set == NULL)
    dump_stream = stdout_stream_new ();
  else
    {
      dump_stream = file_stream_new (dump_file_set->get_text (dump_file_set));
      if (dump_stream == NULL)
        {
          fprintf (stderr, "Warning: could not dump to ``%s''. Using stdout instead.\n",
                   dump_file_set->get_text (dump_file_set));
          dump_stream = stdout_stream_new ();
        }
    }
  dump_depth = dump_depth_set->get_int_value (dump_depth_set);

  /* Allocate memory */
  priv = malloc (sizeof *priv);
  rv = (data_collector_t *) priv;
  if (priv == NULL)
    {
      fprintf (stderr, "Out of memory creating dump!\n");
      return NULL;
    }
  priv->size = dump_depth;
  priv->data = malloc ((1 + dump_depth) * sizeof *priv->data);
  if (priv->data == NULL)
    {
      fprintf (stderr, "Out of memory creating dump!\n");
      rv->destroy (rv);
      return NULL;
    }

  /* Actually setup object */
  rv->reset   = _dump_reset;
  rv->output  = _dump_output;
  rv->destroy = _dump_destroy;
  rv->get_type = _dump_get_type;
  rv->record   = _dump_record;

  return rv;
}

