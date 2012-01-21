
#include <stdio.h>

#include "global.h"
#include "ramsey.h"
#include "recurse.h"

/* Preamble that doesn't return 0 if filters fail (though it requires
 * the filters to pass to increment recursion counts) */
int recursion_preamble (ramsey_t *rt, global_data_t *state)
{
  bool filter_success = rt->run_filters (rt);

  if (state->kill_now)
    return 0;
  if (rt->r_prune_tree && !filter_success)
    return 0;
  if (rt->r_max_iterations && rt->r_iterations >= rt->r_max_iterations)
    return 0;
  if (rt->r_max_depth && rt->r_depth >= rt->r_max_depth)
    return 0;

  ++rt->r_depth;

  if (filter_success)
    {
      dc_list *dlist;
      for (dlist = state->dumps; dlist; dlist = dlist->next)
        dlist->data->record (dlist->data, rt);
      for (dlist = state->targets; dlist; dlist = dlist->next)
        dlist->data->record (dlist->data, rt);

      ++rt->r_iterations;
    }
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
  rt->r_max_depth =
  rt->r_max_found = 
  rt->r_prune_tree = 0;
  rt->r_gap_set = NULL;
  rt->r_alphabet = NULL;
}

void recursion_reset (ramsey_t *rt, global_data_t *state)
{
  const setting_t *max_iters_set = SETTING ("max_iterations");
  const setting_t *max_depth_set = SETTING ("max_depth");
  const setting_t *prune_tree_set = SETTING ("prune_tree");
  const setting_t *gap_set_set   = SETTING ("gap_set");
  const setting_t *alphabet_set  = SETTING ("alphabet");

  recursion_init (rt);

  if (max_iters_set)
    rt->r_max_iterations = max_iters_set->get_int_value (max_iters_set);
  if (max_depth_set)
    rt->r_max_depth = max_depth_set->get_int_value (max_depth_set);
  if (prune_tree_set)
    rt->r_prune_tree = prune_tree_set->get_int_value (prune_tree_set);
  if (gap_set_set)
    rt->add_gap_set (rt, gap_set_set->get_ramsey_value (gap_set_set));
  if (alphabet_set)
    rt->r_alphabet = alphabet_set->get_ramsey_value (alphabet_set);
}

