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
#include <string.h>

#include "global.h"
#include "target/target.h"
#include "setting.h"
#include "stream.h"

struct _global_data *set_defaults (stream_t *in, stream_t *out, stream_t *err)
{
  struct _global_data *rv = malloc (sizeof *rv);
  if (rv)
    {
      rv->settings = setting_list_new ();
#define NEW_SET(s,t) rv->settings->add_setting (rv->settings, setting_new ((s), (t)))
      NEW_SET ("prune_tree",     "1");
      NEW_SET ("n_colors",       "3");
      NEW_SET ("random_length",  "10");
      NEW_SET ("dump_depth",     "400");
#undef NEW_SET
      rv->filters  = NULL;
      rv->dumps    = NULL;
      rv->interactive = 0;

      rv->in_stream  = in;
      rv->out_stream = out;
      rv->err_stream = err;

      /* Set max-length as a target by default.
       * We do this purely for historical reasons */
      rv->targets = malloc (sizeof *rv->targets);
      rv->targets->data = target_new ("max_length", rv->settings);
      rv->targets->next = NULL;
      /**/
    }
  return rv;
}

struct _global_data *clone_global_data (const struct _global_data *src)
{
  struct _global_data *rv = malloc (sizeof *rv);

  assert (src != NULL);

  if (rv == NULL)
    return NULL;

  memcpy (rv, src, sizeof *rv);
#define COPY(what, type)	\
  if (src->what)		\
    {				\
      type *srclist = src->what;	\
      type *dstlist = malloc (sizeof *dstlist);	\
      rv->what = dstlist;	\
      while (srclist)		\
        {			\
          dstlist->data = srclist->data->clone (srclist->data);	\
          dstlist->next = (srclist->next ? malloc (sizeof *dstlist) : NULL);	\
          srclist = srclist->next;     \
        }                      \
    }

  COPY (filters, filter_list);
  COPY (targets, dc_list);
  COPY (dumps, dc_list);
#undef COPY

  if (rv->seed)
    rv->seed = rv->seed->clone (rv->seed);

  /* Don't copy streams or settings, since they do not have copy constructors
   * yet, and shallow copies will inevitably cause Bad Things to happen. */
  rv->in_stream = rv->out_stream = rv->err_stream = NULL;
  rv->settings = NULL;

  return rv;
}

void absorb_global_data (struct _global_data *dst,
                         const struct _global_data *src)
{
  dc_list *dst_dlist, *src_dlist;

  if (dst->seed && src->seed)
    dst->seed->r_iterations += src->seed->r_iterations;

  dst_dlist = dst->targets;
  src_dlist = src->targets;
  while (dst_dlist && src_dlist)
    {
      dst_dlist->data->absorb (dst_dlist->data, src_dlist->data);
      dst_dlist = dst_dlist->next;
      src_dlist = src_dlist->next;
    }

  dst_dlist = dst->dumps;
  src_dlist = src->dumps;
  while (dst_dlist && src_dlist)
    {
      dst_dlist->data->absorb (dst_dlist->data, src_dlist->data);
      dst_dlist = dst_dlist->next;
      src_dlist = src_dlist->next;
    }
}

void destroy_global_data (struct _global_data *src)
{
  dc_list *dlist;

  filter_list *flist = src->filters;
  while (flist)
    {
      filter_list *tmp = flist;
      flist = flist->next;
      tmp->data->destroy (tmp->data);
      free (tmp);
    }
  dlist = src->targets;
  while (dlist)
    {
      dc_list *tmp = dlist;
      dlist = dlist->next;
      tmp->data->destroy (tmp->data);
      free (tmp);
    }
  dlist = src->dumps;
  while (dlist)
    {
      dc_list *tmp = dlist;
      dlist = dlist->next;
      tmp->data->destroy (tmp->data);
      free (tmp);
    }
  if (src->seed)
    src->seed->destroy (src->seed);
  free (src);
}


