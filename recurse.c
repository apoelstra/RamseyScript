
#include <stdio.h>

#include "global.h"
#include "recurse.h"

/* Preamble that doesn't return 0 if filters fail (though it requires
 * the filters to pass to increment recursion counts) */
int recursion_preamble (ramsey_t *rt, global_data_t *state)
{
  bool filter_success = rt->run_filters (rt);

  if (state->kill_now)
    return 0;
  if (state->prune_tree && !filter_success)
    return 0;
  if (state->max_iterations &&
      rt->r_iterations >= state->max_iterations)
    return 0;
  if (state->max_depth &&
      rt->r_depth >= state->max_depth)
    return 0;

  ++rt->r_depth;

  if (filter_success)
    {
      if (state->dump_iters && rt->get_length (rt) < state->dump_depth)
        {
          int *dump_val = state->iters_data->get_priv_data (state->iters_data);
          ++dump_val[rt->get_length (rt)];
        }
      ++rt->r_iterations;

      if (rt->get_length (rt) > rt->r_max_found)
        {
          stream_printf (state->out_stream,
                         "New maximal %s (length %3d): ",
                         rt->get_type (rt), rt->get_length (rt));
          rt->print (rt, state->out_stream);
          stream_printf (state->out_stream, "\n");
          rt->r_max_found = rt->get_length (rt);
        }
    }
  return 1;
}


void recursion_postamble (ramsey_t *rt)
{
  --rt->r_depth;
}

void recursion_reset (ramsey_t *rt)
{
  rt->r_iterations = rt->r_depth = rt->r_max_found = 0;
}

