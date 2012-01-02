
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
#include "recurse.h"
#include "filters.h"
#include "check.h"

#define strmatch(s, r) (!strcmp ((s), (r)))
#define MATCH_THEN_SET(tok, text)		\
  ((tok) != NULL && strmatch ((tok), #text))	\
    {						\
      char *new_tok = strtok (NULL, " \t\n");	\
      global.text = strtoul (new_tok, NULL, 0);	\
    }

void set_defaults ()
{
  global.iterations = 0;
  global.min_gap = 1;
  global.max_gap = 0;
  global.n_colors = 3;
  global.ap_length = 3;
  global.alphabet = sequence_parse ("[1 2 3 4]");
  global.gap_set = NULL;
  global.filter = cheap_check_sequence3;

  global.dump_fh = stdout;
  global.dump_iters = 0;
  global.dump_depth = 400;
  global.iters_data = NULL;
}

void process (Stream *stm)
{
  char *buf;
  int i;

  /* Parse */
  while ((buf = stm->read_line (stm)))
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
          if MATCH_THEN_SET (tok, min_gap)
          else if MATCH_THEN_SET (tok, max_gap)
          else if MATCH_THEN_SET (tok, n_colors)
          else if MATCH_THEN_SET (tok, ap_length)
          else if MATCH_THEN_SET (tok, iterations)
          else if MATCH_THEN_SET (tok, dump_depth)
          else if (strmatch (tok, "alphabet"))
            {
              tok = strtok (NULL, "\n");
              sequence_delete (global.alphabet);
              global.alphabet = sequence_parse (tok);
            }
          else if (strmatch (tok, "gap_set"))
            {
              tok = strtok (NULL, "\n");
              global.gap_set = sequence_parse (tok);
            }
          else if (strmatch (tok, "dump_file"))
            {
              tok = strtok (NULL, "\n");
              if (global.dump_fh && global.dump_fh != stdout)
                fclose (global.dump_fh);
              if (strmatch (tok, "_"))
                global.dump_fh = stdout;
              else
                {
                  global.dump_fh = fopen (tok, "a");
                  if (global.dump_fh == NULL)
                    {
                      fprintf (stderr, "Failed to open ``%s'' for writing. Using stdout instead.\n", tok);
                      global.dump_fh = stdout;
                    }
                }
            }
        }
      /* filter <no-double-3-aps|no-additive-squares> */
      else if (strmatch (tok, "filter"))
        {
          tok = strtok (NULL, " \t\n");
          if (tok && strmatch (tok, "no_double_3_aps"))
            global.filter = cheap_check_sequence3;
          else if (tok && strmatch (tok, "no_additive_squares"))
            global.filter = cheap_check_additive_square;
          else
            fprintf (stderr, "Unknown filter ``%s''\n", tok);
	}
      /* dump <iterations-per-length> */
      else if (strmatch (tok, "dump"))
        {
          tok = strtok (NULL, " \t\n");
          if (strmatch (tok, "iterations_per_length"))
            global.dump_iters = 1;
          else
            fprintf (stderr, "Unknown dump format ``%s''\n", tok);
        }
      /* search <seqences|colorings|words> [seed] */
      else if (strmatch (tok, "search"))
        {
          time_t start = time (NULL);
          reset_max ();

          if (global.dump_iters)
            {
              free (global.iters_data);
              global.iters_data = sequence_new_zeros (global.dump_depth);
            }

          tok = strtok (NULL, " \t\n");
          if (tok && strmatch (tok, "sequences"))
            {
              Sequence *seek;

              tok = strtok (NULL, " \t\n");
              if (tok && *tok == '[')
                seek = sequence_parse (tok);
              else
                {
                  seek = sequence_new ();
                  sequence_append (seek, 1);
                }

              puts ("#### Starting sequence search ####");
              if (global.iterations > 0)
                printf ("  Stop after: \t%ld iterations\n", global.iterations);
              printf ("  Minimum gap:\t%d\n", global.min_gap);
              printf ("  Maximum gap:\t%d\n", global.max_gap);
              printf ("  AP length:\t%d\n", global.ap_length);
              printf ("  Seed Seq.:\t"); sequence_print (seek);
              puts("\n");

              if (seek == NULL)
                {
                  fprintf (stderr, "Failed to allocate sequence.");
                  exit (EXIT_FAILURE);
                }

              if (global.gap_set == NULL)
                {
                  global.gap_set = sequence_new ();
                  for (i = global.min_gap; i <= global.max_gap; ++i)
                    sequence_append (global.gap_set, i);
                }
              recurse_sequence (seek);
              sequence_delete (seek);
            }
          else if (strmatch (tok, "colorings") ||
                   strmatch (tok, "partitions"))
            {
              Coloring *seek;

/*
              tok = strtok (NULL, " \t\n");
              if (tok && *tok == '[')
                seek = sequence_parse (tok);
              else
*/
                {
                  seek = coloring_new (global.n_colors);
                  coloring_append (seek, 1, 0);
                }

              puts ("#### Starting coloring search ####");
              if (global.iterations > 0)
                printf ("  Stop after: \t%ld iterations\n", global.iterations);
              printf ("  Minimum gap:\t%d\n", global.min_gap);
              printf ("  Maximum gap:\t%d\n", global.max_gap);
              printf ("  AP length:\t%d\n", global.ap_length);
              printf ("  Seed Col.:\t"); coloring_print (seek);
              puts("");

              recurse_colorings (seek, 1);

              coloring_delete (seek);
            }
          else if (tok && strmatch (tok, "words"))
            {
              Sequence *seek = sequence_new ();

              puts ("#### Starting word search ####");
              if (global.iterations > 0)
                printf ("  Stop after: \t%ld iterations\n", global.iterations);
              printf ("  Alphabet:\t"); sequence_print (global.alphabet);
              printf ("\n  Seed Seq.:\t"); sequence_print (seek);
              puts("\n");

              recurse_words (seek);

              sequence_delete (global.alphabet);
              sequence_delete (seek);
            }
          else
            fprintf (stderr, "Unrecognized search space ``%s''\n", tok);
          printf ("Done. Time taken: %ds. Iterations: %ld\n", (int) (time (NULL) - start), get_iterations());
          if (global.dump_iters)
            {
              sequence_print_real (global.iters_data, 1, global.dump_fh);
              fputs ("\n", global.dump_fh);
            }
          puts("\n");
        }
      free (buf);
    }

  /* Cleanup */
  if (global.dump_fh && global.dump_fh != stdout)
    fclose (global.dump_fh);
}

