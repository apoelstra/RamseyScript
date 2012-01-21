
#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "global.h"
#include "coloring.h"
#include "dump.h"
#include "file-stream.h"
#include "filter.h"
#include "permutation.h"
#include "ramsey.h"
#include "recurse.h"
#include "setting.h"
#include "sequence.h"
#include "word.h"

#define strmatch(s, r) (!strcmp ((s), (r)))

struct _global_data *set_defaults ()
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
      rv->kill_now = 0;
      rv->interactive = 0;
    }
  return rv;
}

ramsey_t *new_from_parse (const char *data)
{
  ramsey_t *rv = NULL;
  const char *scan = data;
  int lb_count  = 0;
  while (*scan && (isspace (*scan) || *scan == '['))
    {
      if (*scan == '[')
        ++lb_count;
      ++scan;
    }

  switch (lb_count)
    {
    case 1:
      rv = sequence_new ();
      break;
    case 2:
      lb_count = 0;
      scan = data;
      while (*scan)
        {
          if (*scan == '[')
            ++lb_count;
          ++scan;
        }
      rv = coloring_new (lb_count - 1);
      break;
    default:
      fprintf (stderr, "Failed to parse: %s\n", data);
      break;
    }

  if (rv)
    rv->parse (rv, data);

  return rv;
}

void process (struct _global_data *state)
{
  char *buf;
  int i;

  /* Parse */
  if (state->interactive)
    {
      printf ("ramsey> ");
      fflush (stdout);
    }
  while ((buf = state->in_stream->read_line (state->in_stream)))
    {
      char *tok;
      /* Convert all - signs to _ so lispers can feel at home */
      for (i = 0; buf[i]; ++i)
        if (buf[i] == '-')
          buf[i] = '_';
        else if (isalpha (buf[i]))
          buf[i] = tolower (buf[i]);

      tok = strtok (buf, " \t\n");

      /* skip comments and blank lines */
      if (tok == NULL || *tok == '#')
        {
          free (buf);
          continue;
        }

      /* set <variable> <value> */
      if (strmatch (tok, "set"))
        {
          const char *name = strtok (NULL, " #\t\n");
          const char *text = strtok (NULL, "#\n");
          setting_t *new_set = setting_new (name, text);
          if (state->settings->add_setting (state->settings, new_set))
            {
              if (state->interactive)
                new_set->print (new_set, state->out_stream);
            }
          else
            fprintf (stderr, "Failed to add setting ``%s''.\n", name);
        }
      /* get <variable> */
      else if (strmatch (tok, "get"))
        {
          tok = strtok (NULL, " #\t\n");
          if (tok && *tok)
            {
              const setting_t *set = SETTING (tok);
              if (set)
                set->print (set, state->out_stream);
              else
                fprintf (stderr, "No such variable ``%s''.\n", tok);
            }
          else
            state->settings->print (state->settings, state->out_stream);
        }
      /* unset <variable> */
      else if (strmatch (tok, "unset"))
        {
          const char *name = strtok (NULL, " #\t\n");
          if (state->settings->remove_setting (state->settings, name))
            if (state->interactive)
              stream_printf (state->out_stream, "Removed ``%s''.\n", name);
        }
      /* filter <no-double-3-aps|no-additive-squares> */
      else if (strmatch (tok, "filter"))
        {
          tok = strtok (NULL, " #\t\n");
          /* Delete all filters */
          if ((tok != NULL) && strmatch (tok, "clear"))
            {
              filter_list *flist = state->filters;
              /* Apply filters */
              while (flist)
                {
                  filter_list *tmp = flist;
                  flist = flist->next;
                  if (state->interactive)
                    stream_printf (state->out_stream, "Removed filter ``%s''.\n",
                                   tmp->data->get_type (tmp->data));
                  tmp->data->destroy (tmp->data);
                  free (tmp);
                }
              state->filters = NULL;
            }
          /* Add a new filter */
          else if (tok != NULL)
            {
              filter_t *new_filter = filter_new (tok);
              if (new_filter != NULL)
                {
                  filter_list *new_cell = malloc (sizeof *new_cell);
                  new_cell->next = state->filters;
                  new_cell->data = new_filter;
                  state->filters = new_cell;
                  stream_printf (state->out_stream, "Added filter ``%s''.\n",
                                 new_filter->get_type (new_filter));
                }
            }
	}
      /* dump <iterations-per-length> */
      else if (strmatch (tok, "dump"))
        {
          tok = strtok (NULL, " #\t\n");
          /* Delete all dumps */
          if (tok != NULL && strmatch (tok, "clear"))
            {
              dc_list *dlist = state->dumps;
              /* Apply filters */
              while (dlist)
                {
                  dc_list *tmp = dlist;
                  dlist = dlist->next;
                  if (state->interactive)
                    stream_printf (state->out_stream, "Removed dump ``%s''.\n",
                                   tmp->data->get_type (tmp->data));
                  tmp->data->destroy (tmp->data);
                  free (tmp);
                }
              state->dumps = NULL;
            }
          /* Add a new dump */
          else if (tok != NULL)
            {
              data_collector_t *new_dump;
              stream_t *dump_stream;
              const setting_t *dump_depth_set = SETTING ("dump_depth");
              const setting_t *dump_file_set  = SETTING ("dump_file");
              int dump_depth = 0;

              if (dump_depth_set)
                dump_depth = dump_depth_set->get_int_value (dump_depth_set);
              if (dump_file_set && strcmp (dump_file_set->get_text (dump_file_set), "-"))
                dump_stream = file_stream_new (dump_file_set->get_text (dump_file_set));
              else
                dump_stream = stdout_stream_new ();

              new_dump = dump_new (tok, dump_depth, dump_stream);
              if (new_dump != NULL)
                {
                  dc_list *new_cell = malloc (sizeof *new_cell);
                  new_cell->next = state->dumps;
                  new_cell->data = new_dump;
                  state->dumps = new_cell;
                  stream_printf (state->out_stream, "Added dump ``%s''.\n",
                                 new_dump->get_type (new_dump));
                }
            }
        }
      /* search <seqences|colorings|words> [seed] */
      else if (strmatch (tok, "search"))
        {
          ramsey_t *seed = NULL;

          tok = strtok (NULL, " #\t\n");
          if (tok && strmatch (tok, "sequences"))
            seed = sequence_new ();
          else if (tok && (strmatch (tok, "colorings") ||
                           strmatch (tok, "partitions")))
            {
              const setting_t *n_colors_set = SETTING ("n_colors");
              seed = coloring_new (n_colors_set->get_int_value (n_colors_set));
            }
          else if (tok && strmatch (tok, "words"))
            seed = word_new ();
          else if (tok && (strmatch (tok, "permutations") ||
                           strmatch (tok, "perms")))
            seed = permutation_new ();
          else
            fprintf (stderr, "Unrecognized search space ``%s''\n", tok);

          if (seed == NULL)
            fprintf (stderr, "No seed. Bad search space or allocation failure.\n");
          else
            {
              filter_list *flist = state->filters;
              dc_list     *dlist = state->dumps;
              const setting_t *max_iters_set = SETTING ("max_iterations");
              const setting_t *max_depth_set = SETTING ("max_depth");
              const setting_t *alphabet_set  = SETTING ("alphabet");
              const setting_t *gap_set_set   = SETTING ("gap_set");
              const setting_t *rand_len_set  = SETTING ("random_length");
              time_t start = time (NULL);

              /* Apply filters */
              while (flist)
                {
                  seed->add_filter (seed, flist->data->clone (flist->data));
                  flist = flist->next;
                }
              /* Reset dump data */
              while (dlist)
                {
                  dlist->data->reset (dlist->data);
                  dlist = dlist->next;
                }

              /* Parse seed */
              tok = strtok (NULL, "\n");
              if (tok && *tok == '[')
                seed->parse (seed, tok);
              else if (tok && strmatch (tok, "random"))
                seed->randomize (seed, rand_len_set->get_int_value (rand_len_set));
              else
                seed->append (seed, 1);

              /* Output header */
              stream_printf (state->out_stream, "#### Starting %s search ####\n",
                             seed->get_type (seed));
              if (max_iters_set)
                stream_printf (state->out_stream, "  Stop after: \t%ld iterations\n",
                               max_iters_set->get_int_value (max_iters_set));
              if (max_depth_set)
                stream_printf (state->out_stream, "  Max. depth: \t%ld\n",
                               max_depth_set->get_int_value (max_depth_set));
              if (alphabet_set && alphabet_set->type == TYPE_RAMSEY)
                {
                  const ramsey_t *alphabet = alphabet_set->get_ramsey_value (alphabet_set);
                  stream_printf (state->out_stream, "  Alphabet: \t");
                  alphabet->print (alphabet, state->out_stream);
                  stream_printf (state->out_stream, "\n");
                }
              if (gap_set_set && gap_set_set->type == TYPE_RAMSEY)
                stream_printf (state->out_stream, "  Gap set: \t%s\n", gap_set_set->get_text (gap_set_set));

              stream_printf (state->out_stream, "  Filters: \t");
              for (flist = state->filters; flist; flist = flist->next)
                stream_printf (state->out_stream, "%s ", flist->data->get_type (flist->data));
              stream_printf (state->out_stream, "\n");
              stream_printf (state->out_stream, "  Dump data: \t");
              for (dlist = state->dumps; dlist; dlist = dlist->next)
                stream_printf (state->out_stream, "%s ", dlist->data->get_type (dlist->data));
              stream_printf (state->out_stream, "\n");
                
              stream_printf (state->out_stream, "  Seed:\t\t");
              seed->print (seed, state->out_stream);
              stream_printf (state->out_stream, "\n");

              /* Do recursion */
              recursion_reset (seed, state);
              seed->recurse (seed, state);
              stream_printf (state->out_stream, "Done. Time taken: %ds. Iterations: %ld\n\n",
                             (int) (time (NULL) - start), seed->r_iterations);

              /* Output dump data */
              for (dlist = state->dumps; dlist; dlist = dlist->next)
                dlist->data->output (dlist->data);
              /* Cleanup */
              seed->destroy (seed);
            }
        }
      /* Interactive mode commands */
      else if (state->interactive)
        {
          if (strmatch (tok, "quit") || strmatch (tok, "exit"))
            return;
          else 
            fprintf (stderr, "Unrecognized command ``%s''. Type 'quit' to quit.\n"
                             "See the README file for a full language specification.\n", tok);
        }
      else
        fprintf (stderr, "Unrecognized command ``%s''.\n", tok);

      free (buf);
      if (state->interactive)
        {
          printf ("ramsey> ");
          fflush (stdout);
        }
    }
}

