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
#include <stdio.h>
#include <stdlib.h>

#include "target.h"

struct _target_priv {
  data_collector_t parent;

  stream_t *out;
  long max_recorded;
};

static const char *_target_get_type (const data_collector_t *dc)
{
  (void) dc;
  return "max-length";
}

static int _target_record (data_collector_t *dc, ramsey_t *ram)
{
  long len = ram->get_length (ram);
  struct _target_priv *priv = (struct _target_priv *) dc;

  assert (dc != NULL);
  assert (ram != NULL);

  if (len >= priv->max_recorded)
    {
      stream_printf (priv->out, "New maximal %s (length %3d): ",
                     ram->get_type (ram), len);
      ram->print (ram, priv->out);
      stream_printf (priv->out, "\n");
      priv->max_recorded = len;
      return 1;
    }
  return 0;
}

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

/* CONSTRUCTOR / DESTRUCTOR */
void *target_max_length_new (const global_data_t *state)
{
  struct _target_priv *priv = malloc (sizeof *priv);
  data_collector_t *rv = (data_collector_t *) priv;

  if (rv != NULL)
    {
      rv->reset   = _target_reset;
      rv->output  = _target_output;
      rv->destroy = _target_destroy;

      priv->out = state->out_stream;
      priv->max_recorded = 0;
      rv->get_type = _target_get_type;
      rv->record   = _target_record;
    }
  return rv;
}

