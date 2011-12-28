
#include "sequence.h"
#include "check.h"

/* Check for double 3-AP's involving the last element of the list
 * Returns 0 if it finds one, 1 if it doesn't.
 */
int cheap_check_sequence3 (Sequence *s)
{
  int i;
  if (s->length >= 3)
    for (i = !(s->length % 2); i < s->length - 1; i += 2)
      if (2 * s->values[(i + s->length - 1)/2] == s->values[i] + s->values[s->length - 1])
        return 0;
  return 1;
}

/* Check for additive squares involving the last element of the list
 * Returns 0 if it finds one, 1 if it doesn't.
 */
int cheap_check_additive_square (Sequence *s)
{
  int i;
  int sum1, sum2;

  for (i = 1; i <= s->length / 2; ++i)
    {
      int j;
      sum1 = sum2 = 0;
      for (j = 1; j <= i; ++j)
        {
          sum1 += s->values[s->length - j];
          sum2 += s->values[s->length - i - j];
        }
      if (sum1 == sum2)
        return 0;
    }
  return 1;
}

