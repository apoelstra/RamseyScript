
#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "global.h"
#include "ramsey.h"
#include "sequence.h"
#include "coloring.h"
#include "filters.h"
#include "check.h"

#define strmatch(s, r) (!strcmp ((s), (r)))
#define MATCH_THEN_SET(tok, text)		\
  ((tok) != NULL && strmatch ((tok), #text))	\
    {						\
      char *new_tok = strtok (NULL, " \t\n");	\
      state->text = strtoul (new_tok, NULL, 0);	\
    }

struct _global_data *set_defaults ()
{
  struct _global_data *rv = malloc (sizeof *rv);
  if (rv)
    {
      rv->max_iterations = 0;
      rv->n_colors = 3;
      rv->ap_length = 3;
      rv->random_length = 10;
      rv->alphabet = sequence_new ();
      rv->alphabet->parse (rv->alphabet, "[1 2 3 4]");
      rv->gap_set = sequence_new ();
      rv->gap_set->parse (rv->gap_set, "[1 ... 1000]");
      rv->filter = cheap_check_sequence3;

      rv->dump_stream = file_stream_new ("w");
      rv->dump_stream->_data = stdout;
      rv->dump_iters = 0;
      rv->dump_depth = 400;
      rv->iters_data = NULL;

      rv->kill_now = 0;
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

      /* set <min_gap|max_gap|n_colors|ap_length|alphabet|gap_set> <N> */
      if (strmatch (tok, "set"))
        {
          tok = strtok (NULL, " \t\n");
          if MATCH_THEN_SET (tok, n_colors)
          else if MATCH_THEN_SET (tok, ap_length)
          else if MATCH_THEN_SET (tok, max_iterations)
          else if MATCH_THEN_SET (tok, dump_depth)
          else if MATCH_THEN_SET (tok, random_length)
          else if (strmatch (tok, "alphabet"))
            {
              tok = strtok (NULL, "\n");
              state->alphabet->destroy (state->alphabet);
              state->alphabet = new_from_parse (tok);
            }
          else if (strmatch (tok, "gap_set"))
            {
              tok = strtok (NULL, "\n");
              state->gap_set->destroy (state->gap_set);
              state->gap_set = new_from_parse (tok);
            }
          else if (strmatch (tok, "dump_file"))
            {
              tok = strtok (NULL, "\n");
              if (state->dump_stream->_data && state->dump_stream->_data != stdout)
                fclose (state->dump_stream->_data);
              if (strmatch (tok, "_"))
                state->dump_stream->_data = stdout;
              else
                {
                  state->dump_stream->_data = fopen (tok, "a");
                  if (state->dump_stream->_data == NULL)
                    {
                      fprintf (stderr, "Failed to open ``%s'' for writing. Using stdout instead.\n", tok);
                      state->dump_stream->_data = stdout;
                    }
                }
            }
        }
      /* filter <no-double-3-aps|no-additive-squares> */
      else if (strmatch (tok, "filter"))
        {
          tok = strtok (NULL, " \t\n");
          if (tok && strmatch (tok, "no_double_3_aps"))
            state->filter = cheap_check_sequence3;
          else if (tok && strmatch (tok, "no_additive_squares"))
            state->filter = cheap_check_additive_square;
          else
            fprintf (stderr, "Unknown filter ``%s''\n", tok);
	}
      /* dump <iterations-per-length> */
      else if (strmatch (tok, "dump"))
        {
          tok = strtok (NULL, " \t\n");
          if (strmatch (tok, "iterations_per_length"))
            state->dump_iters = 1;
          else
            fprintf (stderr, "Unknown dump format ``%s''\n", tok);
        }
      /* search <seqences|colorings|words> [seed] */
      else if (strmatch (tok, "search"))
        {
          ramsey_t *seed = NULL;

          if (state->dump_iters)
            {
              free (state->iters_data);
              state->iters_data = sequence_new_zeros (state->dump_depth, 1);
            }

          tok = strtok (NULL, " \t\n");
          if (tok && strmatch (tok, "sequences"))
            {
              seed = sequence_new ();
              if (seed == NULL)
                {
                  fprintf (stderr, "Failed to allocate sequence.");
                  exit (EXIT_FAILURE);
                }

              seed->add_filter  (seed, state->filter);
              seed->add_gap_set (seed, state->gap_set);

              tok = strtok (NULL, "\n");
              if (tok && *tok == '[')
                seed->parse (seed, tok);
              else
                seed->append (seed, 1);

              state->out_stream->write_line (state->out_stream, "#### Starting sequence search ####\n");
              if (state->max_iterations > 0)
                stream_printf (state->out_stream, "  Stop after: \t%ld iterations\n", state->max_iterations);
              stream_printf (state->out_stream,
                             "  AP length:\t%d\n"
                             "  Seed Seq.:\t",
                             state->ap_length);
              seed->print (seed, state->out_stream);
              state->out_stream->write_line (state->out_stream, "\n");
              stream_printf (state->out_stream, "  Gap set:\t"); state->gap_set->print (state->gap_set, state->out_stream);
              state->out_stream->write_line (state->out_stream, "\n");
            }
          else if (strmatch (tok, "colorings") ||
                   strmatch (tok, "partitions"))
            {
              seed = coloring_new (state->n_colors);
              if (seed == NULL)
                {
                  fprintf (stderr, "Failed to allocate coloring.");
                  exit (EXIT_FAILURE);
                }

              seed->add_filter  (seed, state->filter);
              seed->add_gap_set (seed, state->gap_set);

              tok = strtok (NULL, "\n");
              if (tok && *tok == '[')
                seed->parse (seed, tok);
              else if (tok && strmatch (tok, "random"))
                seed->randomize (seed, state->random_length);
              else
                seed->append (seed, 1);

              stream_printf (state->out_stream, "#### Starting coloring search ####\n");
              if (state->max_iterations > 0)
                stream_printf (state->out_stream, "  Stop after: \t%ld iterations\n", state->max_iterations);
              stream_printf (state->out_stream,
                             "  AP length:\t%d\n"
                             "  Seed Col.:\t",
                             state->ap_length);
              seed->print (seed, state->out_stream);
              state->out_stream->write_line (state->out_stream, "\n");
              stream_printf (state->out_stream, "  Gap set:\t"); state->gap_set->print (state->gap_set, state->out_stream);
              state->out_stream->write_line (state->out_stream, "\n");
            }
          else if (tok && strmatch (tok, "words"))
            {
              seed = sequence_new ();
              if (seed == NULL)
                {
                  fprintf (stderr, "Failed to allocate coloring.");
                  exit (EXIT_FAILURE);
                }

              seed->add_filter  (seed, state->filter);
              tok = strtok (NULL, "\n");
              if (tok && *tok == '[')
                seed->parse (seed, tok);
              else
                seed->append (seed, 1);

              stream_printf (state->out_stream, "#### Starting word search ####\n");
              if (state->max_iterations > 0)
                stream_printf (state->out_stream, "  Stop after: \t%ld iterations\n", state->max_iterations);
              stream_printf (state->out_stream, "  Alphabet:\t"); state->alphabet->print (state->alphabet, state->out_stream);
              stream_printf (state->out_stream, "\n  Seed Seq.:\t"); seed->print (seed, state->out_stream);
              stream_printf (state->out_stream, "\n");
            }
          else
            fprintf (stderr, "Unrecognized search space ``%s''\n", tok);

          if (seed != NULL)
            {
              time_t start = time (NULL);

              seed->recurse (seed, state);
              stream_printf (state->out_stream, "Done. Time taken: %ds. Iterations: %ld\n",
                             (int) (time (NULL) - start), seed->recurse_get_iterations (seed));
              seed->destroy (seed);
            }

          if (state->dump_iters)
            {
              state->iters_data->print (state->iters_data, state->dump_stream);
              fputs ("\n", state->dump_stream->_data);
            }
          state->out_stream->write_line (state->out_stream, "\n");
        }
      free (buf);
    }

  /* Cleanup */
  state->dump_stream->destroy (state->dump_stream);
}

