
#include <assert.h>

#include "ramsey.h"
#include "check.h"

/* Check for double 3-AP's involving the last element of the list
 * Returns 0 if it finds one, 1 if it doesn't.
 */
int cheap_check_sequence3 (ramsey_t *rt)
{
  int  len = rt->get_length (rt);
  int *val = rt->get_priv_data (rt);
  int i;

  assert (val != NULL);

  if (len >= 3)
    for (i = !(len % 2); i < len - 1; i += 2)
      if (2 * val[(i + len - 1)/2] == val[i] + val[len - 1])
        return 0;
  return 1;
}

/* Check for additive squares involving the last element of the list
 * Returns 0 if it finds one, 1 if it doesn't.
 */
int cheap_check_additive_square (ramsey_t *rt)
{
  int  len = rt->get_length (rt);
  int *val = rt->get_priv_data (rt);
  int i, sum1, sum2;

  for (i = 1; i <= len / 2; ++i)
    {
      int j;
      sum1 = sum2 = 0;
      for (j = 1; j <= i; ++j)
        {
          sum1 += val[len - j];
          sum2 += val[len - i - j];
        }
      if (sum1 == sum2)
        return 0;
    }
  return 1;
}

