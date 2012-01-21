
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "word.h"
#include "ramsey.h"
#include "recurse.h"
#include "stream.h"
#include "sequence.h"

static const char *_word_get_type (const ramsey_t *rt)
{
  assert (rt && rt->type == TYPE_WORD);
  return "word";
}

/* RECURSION */
static void _word_recurse (ramsey_t *rt, global_data_t *state)
{
  int i;
  const int *alphabet;
  int alphabet_len;
  assert (rt && rt->type == TYPE_WORD);

  if (!rt->r_alphabet)
    {
      fprintf (stderr, "Cannot recurse on words without an alphabet!\n");
      return;
    }

  if (!recursion_preamble (rt, state))
    return;

  alphabet = rt->r_alphabet->get_priv_data_const (rt->r_alphabet);
  alphabet_len = rt->r_alphabet->get_length (rt->r_alphabet);
  for (i = 0; i < alphabet_len; ++i)
    {
      rt->append (rt, alphabet[i]);
      rt->recurse (rt, state);
      rt->deappend (rt);
    }

  recursion_postamble (rt);
}

/* CONSTRUCTOR */
ramsey_t *word_new ()
{
  ramsey_t *rv = sequence_new ();

  if (rv != NULL)
    {
      rv->type = TYPE_WORD;
      rv->get_type = _word_get_type;
      rv->recurse  = _word_recurse;
    }
  return rv;
}

/* PROTOTYPE */
const ramsey_t *word_prototype ()
{
  static ramsey_t *rv;
  if (rv == NULL)
    rv = word_new ();
  return rv;
}

