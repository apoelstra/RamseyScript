
#include "sequence.h"
#include "check.h"

/* Check for double 3-AP's involving the last element of the list
 * Returns 0 if it finds one, 1 if it doesn't.
 */
int cheap_check_sequence3 (Sequence *s)
{
  int i;
  for (i = !(s->length % 2); i < s->length - 1; i += 2)
    if (2 * s->values[(i + s->length - 1)/2] == s->values[i] + s->values[s->length - 1])
      return 0;
  return 1;
}

