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
#include <string.h>

#include "target.h"

static const char *_target_get_type (const data_collector_t *dc)
{
  (void) dc;
  return "any-length";
}

static int _target_record (data_collector_t *dc, const ramsey_t *ram, stream_t *out)
{
  long len = ram->get_length (ram);

  assert (dc != NULL);
  assert (ram != NULL);

  if (out)
    {
      stream_printf (out, "Found %s (length %3d): ",
                     ram->get_type (ram), len);
      ram->print (ram, out);
      stream_printf (out, "\n");
    }
  return 1;
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

static data_collector_t *_target_clone (const data_collector_t *src)
{
  data_collector_t *rv = malloc (sizeof *rv);
  if (rv)
    memcpy (rv, src, sizeof *rv);
  return rv;
}

static void _target_destroy (data_collector_t *dc)
{
  free (dc);
}

void *target_any_length_new (const setting_list_t *vars)
{
  data_collector_t *rv = malloc (sizeof *rv);

  (void) vars;
  if (rv != NULL)
    {
      rv->reset   = _target_reset;
      rv->output  = _target_output;
      rv->clone   = _target_clone;
      rv->destroy = _target_destroy;

      rv->get_type = _target_get_type;
      rv->record   = _target_record;
    }
  return rv;
}

