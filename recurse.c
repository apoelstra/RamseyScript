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


#include <stdio.h>

#include "global.h"
#include "ramsey/ramsey.h"
#include "recurse.h"

int recursion_preamble_statefree (ramsey_t *rt)
{
  bool filter_success = rt->run_filters (rt);

  if (rt->r_prune_tree && !filter_success)
    return 0;
  if (rt->r_max_iterations && rt->r_iterations >= rt->r_max_iterations)
    return 0;
  if (rt->r_max_depth && rt->r_depth >= rt->r_max_depth)
    return 0;
  if (rt->r_stall_after &&
      (rt->r_iterations - rt->r_stall_index > rt->r_stall_after))
    return 0;
  if (rt->r_max_run_time && rt->r_iterations % 1000 == 0 &&
      (time (NULL) - rt->r_start_time) > rt->r_max_run_time)
    return 0;

  if (filter_success)
    {
      ++rt->r_iterations;
      if (rt->r_depth > rt->r_max_thread_depth)
        rt->r_max_thread_depth = rt->r_depth;
    }

  ++rt->r_depth;
  return 1;
}

/* Preamble that doesn't return 0 if filters fail (though it requires
 * the filters to pass to increment recursion counts) */
int recursion_preamble (ramsey_t *rt, const global_data_t *state)
{
  dc_list *dlist;

  if (!recursion_preamble_statefree (rt))
    return 0;
    
  for (dlist = state->dumps; dlist; dlist = dlist->next)
    if (dlist->data->record (dlist->data, rt, state->out_stream))
      rt->r_stall_index = rt->r_iterations;
  for (dlist = state->targets; dlist; dlist = dlist->next)
    if (dlist->data->record (dlist->data, rt, state->out_stream))
      rt->r_stall_index = rt->r_iterations;

  return 1;
}


void recursion_postamble (ramsey_t *rt)
{
  --rt->r_depth;
}

void recursion_init (ramsey_t *rt)
{
  rt->r_iterations =
  rt->r_depth =
  rt->r_max_thread_depth =
  rt->r_max_iterations =
  rt->r_stall_after =
  rt->r_stall_index =
  rt->r_max_depth =
  rt->r_max_run_time =
  rt->r_prune_tree = 0;

}

void recursion_reset (ramsey_t *rt, global_data_t *state)
{
  const setting_t *max_iters_set = SETTING ("max_iterations");
  const setting_t *max_depth_set = SETTING ("max_depth");
  const setting_t *stall_after_set = SETTING ("stall_after");
  const setting_t *prune_tree_set  = SETTING ("prune_tree");
  const setting_t *max_run_time_set = SETTING ("max_run_time");

  recursion_init (rt);

  if (max_iters_set)
    rt->r_max_iterations = max_iters_set->get_int_value (max_iters_set);
  if (max_depth_set)
    rt->r_max_depth = max_depth_set->get_int_value (max_depth_set);
  if (stall_after_set)
    rt->r_stall_after = stall_after_set->get_int_value (stall_after_set);
  if (prune_tree_set)
    rt->r_prune_tree = prune_tree_set->get_int_value (prune_tree_set);
  if (max_run_time_set)
    rt->r_max_run_time = max_run_time_set->get_int_value (max_run_time_set);

  rt->r_start_time = time (NULL);
}

