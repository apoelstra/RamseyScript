
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "sequence.h"
#include "recurse.h"

#define strmatch(s, r) (!strcasecmp ((s), (r)))
#define MATCH_THEN_SET(tok, text)		\
  ((tok) != NULL && strmatch ((tok), #text))	\
    {						\
      char *new_tok = strtok (NULL, " \t\n");	\
      (text) = strtoul (new_tok, NULL, 0);	\
    }

int main (int argc, char *argv[])
{
  int min_gap = 1;
  int max_gap = 1;
  int n_colors = 3;
  int ap_length = 3;

  char buf[1024];
  FILE *fh;
  /* Open script */
  if (argc > 1)
    {
      fh = fopen (argv[1], "r");
      if (fh == NULL)
        {
          fprintf (stderr, "Failed to open script ``%s''\n", argv[1]);
          exit (EXIT_FAILURE);
        }
    }
  else fh = stdin;

  /* Parse */
  while (fgets (buf, sizeof buf, fh))
    {
      char *tok = strtok (buf, " \t\n");
      /* skip comments and blank lines */
      if (tok == NULL || *tok == '#')
        continue;

      /* set <min_gap|max_gap|n_colors|ap_length> <N> */
      if (strmatch (tok, "set"))
        {
          tok = strtok (NULL, " \t\n");
          if MATCH_THEN_SET (tok, min_gap)
          else if MATCH_THEN_SET (tok, max_gap)
          else if MATCH_THEN_SET (tok, n_colors)
          else if MATCH_THEN_SET (tok, ap_length)
        }
      /* search <seqences|colorings> [seed] */
      else if (strmatch (tok, "search"))
        {
          tok = strtok (NULL, " \t\n");
          if (tok && strmatch (tok, "sequences"))
            {
              Sequence *seek = sequence_new ();

              tok = strtok (NULL, " \t\n");
              if (tok && *tok != '#')
                fputs ("Warning: using seed values not yet supported.\n", stderr);

              puts ("#### Starting sequence search ####");
              printf ("  Minimum gap:\t%d\n", min_gap);
              printf ("  Maximum gap:\t%d\n", max_gap);
              printf ("  AP length:\t%d\n", ap_length);
              printf ("  Seed Seq.:\t[1 2]\n\n");

              if (seek == NULL)
                {
                  fprintf (stderr, "Failed to allocate sequence.");
                  exit (EXIT_FAILURE);
                }
              sequence_append (seek, 1);
              sequence_append (seek, 2);

              recurse_sequence3 (seek, min_gap, max_gap);

              sequence_delete (seek);
            }
          else if (tok && strmatch (tok, "colorings"))
            {
              puts ("Sorry, coloring search not yet supported.");
            }
        }
    }

  /* Cleanup */
  if (fh != NULL && fh != stdin)
    fclose (fh);
  return 0;
}

