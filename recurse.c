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

/* Preamble that doesn't return 0 if filters fail (though it requires
 * the filters to pass to increment recursion counts) */
int recursion_preamble (const global_data_t *state)
{
  dc_list *dlist;

  if (state->seed->r_prune_tree && !state->seed->run_filters (state->seed))
    return 0;
  if (state->seed->r_max_iterations && state->seed->r_iterations >= state->seed->r_max_iterations)
    return 0;
  if (state->seed->r_max_depth && state->seed->r_depth >= state->seed->r_max_depth)
    return 0;
  if (state->seed->r_stall_after &&
      (state->seed->r_iterations - state->seed->r_stall_index > state->seed->r_stall_after))
    return 0;
  if (state->seed->r_max_run_time && state->seed->r_iterations % 1000 == 0 &&
      (time (NULL) - state->seed->r_start_time) > state->seed->r_max_run_time)
    return 0;

  ++state->seed->r_iterations;
  ++state->seed->r_depth;

  for (dlist = state->dumps; dlist; dlist = dlist->next)
    if (dlist->data->record (dlist->data, state->seed, state->out_stream))
      state->seed->r_stall_index = state->seed->r_iterations;
  for (dlist = state->targets; dlist; dlist = dlist->next)
    if (dlist->data->record (dlist->data, state->seed, state->out_stream))
      state->seed->r_stall_index = state->seed->r_iterations;

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
  rt->r_max_iterations =
  rt->r_stall_after =
  rt->r_stall_index =
  rt->r_max_depth =
  rt->r_max_run_time =
  rt->r_prune_tree = 0;

}

void recursion_reset (global_data_t *state)
{
  const setting_t *max_iters_set = SETTING ("max_iterations");
  const setting_t *max_depth_set = SETTING ("max_depth");
  const setting_t *stall_after_set = SETTING ("stall_after");
  const setting_t *prune_tree_set  = SETTING ("prune_tree");
  const setting_t *max_run_time_set = SETTING ("max_run_time");

  recursion_init (state->seed);

  if (max_iters_set)
    state->seed->r_max_iterations = max_iters_set->get_int_value (max_iters_set);
  if (max_depth_set)
    state->seed->r_max_depth = max_depth_set->get_int_value (max_depth_set);
  if (stall_after_set)
    state->seed->r_stall_after = stall_after_set->get_int_value (stall_after_set);
  if (prune_tree_set)
    state->seed->r_prune_tree = prune_tree_set->get_int_value (prune_tree_set);
  if (max_run_time_set)
    state->seed->r_max_run_time = max_run_time_set->get_int_value (max_run_time_set);

  state->seed->r_start_time = time (NULL);
}

int recursion_thread_spawn (pthread_t *thread, const global_data_t *parent,
                            void *(*thread_main)(void *))
{
  global_data_t *thread_state = clone_global_data (parent);
  if (thread_state == NULL)
    {
      fputs ("OOM creating thread.\n", stderr); 
      return 0;
    }

  thread_state->seed->r_iterations = 0;
  if (pthread_create (thread, NULL, thread_main, thread_state))
    {
      fputs ("Failed creating thread.\n", stderr); 
      destroy_global_data (thread_state);
      return 0;
    }

  return 1;
}

void recursion_thread_join (pthread_t thread, global_data_t *parent)
{
  void *res;

  if (pthread_join (thread, &res))
    fputs ("Failed to catch thread. Data has been lost.\n", stderr);
  else
    {
      global_data_t *child = res;
      absorb_global_data (parent, child);
      destroy_global_data (child);
    }
}



