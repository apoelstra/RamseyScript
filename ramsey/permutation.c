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

/*! \file permutation.c
 *  \brief Implementation of the permutation type.
 *
 *  A permutation is a sequence whose contents are a rearrangement
 *  of the integers in the interval [1, N], where N is maximised
 *  recursively given some constraints.
 *
 *  The recursion does not proceed by appending elements to the
 *  end of the permutation, but rather by inserting them in various
 *  positions in the middle. Therefore, filters are used in MODE_FULL,
 *  and for most problems, prune-tree will need to be set by
 *  the user to 0.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "ramsey.h"
#include "permutation.h"
#include "sequence.h"

static const char *_permutation_get_type (const ramsey_t *rt)
{
  assert (rt && rt->type == TYPE_PERMUTATION);
  return "permutation";
}

static int _permutation_add_filter (ramsey_t *rt, filter_t *f)
{
  if (sequence_prototype()->add_filter (rt, f))
    {
      f->set_mode (f, MODE_FULL);
      return 1;
    }
  return 0;
}

/* RECURSION */
static void _permutation_recurse (ramsey_t *rt, const global_data_t *state)
{
  int i;
  int *val;
  assert (rt && rt->type == TYPE_PERMUTATION);

  if (!recursion_preamble (rt, state))
    return;

  /* Initial iteration -- append next number */
  rt->append (rt, rt->get_length (rt) + 1);
  rt->recurse (rt, state);
  /* Remaining iterations */
  val = rt->get_priv_data (rt);
  for (i = rt->get_length (rt) - 1; i > 0; --i)
    {
      /* shuffle number into each positition */
      int t = val[i];
      val[i] = val[i - 1];
      val[i - 1] = t;
      /* Recurse */
      rt->recurse (rt, state);
    }
  /* Remove new number, which is now at the start of the array */
  memmove (&val[0], &val[1], rt->get_length (rt) * sizeof *val);
  rt->deappend (rt);

  recursion_postamble (rt);
}

void *permutation_new (const setting_list_t *vars)
{
  ramsey_t *rv = sequence_new (vars);

  if (rv != NULL)
    {
      rv->type = TYPE_PERMUTATION;
      rv->get_type = _permutation_get_type;
      rv->recurse  = _permutation_recurse;
      rv->add_filter = _permutation_add_filter;
    }
  return rv;
}

