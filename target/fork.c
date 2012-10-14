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

  long fork_depth;
};

static const char *_target_get_type (const data_collector_t *dc)
{
  (void) dc;
  return "fork";
}

static int _target_record (data_collector_t *dc, const ramsey_t *ram, stream_t *out)
{
  long depth = ram->get_length (ram);
  struct _target_priv *priv = (struct _target_priv *) dc;

  (void) out;

  assert (dc != NULL);
  assert (ram != NULL);

  if (out && depth == priv->fork_depth)
    {
      stream_printf (out, "search %ss ", ram->get_type (ram));
      ram->print (ram, out);
      stream_printf (out, "\n");
      return 1;
    }
  return 0;
}

static void _target_reset (data_collector_t *dc)
{
  (void) dc;
}

static void _target_output  (const data_collector_t *dc, stream_t *out)
{
  (void) dc;
  (void) out;
}

static void _target_destroy (data_collector_t *dc)
{
  free (dc);
}

/* CONSTRUCTOR / DESTRUCTOR */
void *target_fork_new (const setting_list_t *vars)
{
  struct _target_priv *priv = malloc (sizeof *priv);
  data_collector_t *rv = (data_collector_t *) priv;

  const setting_t *fork_depth_set = vars->get_setting (vars, "fork_depth");
  if (fork_depth_set == NULL)
    {
      fprintf (stderr, "Error: target requires variable ``fork_depth'' set.\n");
      free (rv);
      return NULL;
    }

  (void) vars;
  if (rv != NULL)
    {
      rv->reset   = _target_reset;
      rv->output  = _target_output;
      rv->destroy = _target_destroy;
      rv->get_type = _target_get_type;
      rv->record   = _target_record;

      priv->fork_depth = fork_depth_set->get_int_value (fork_depth_set);
    }
  return rv;
}

