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

#ifndef RECURSE_H
#define RECURSE_H

#include "global.h"

int recursion_preamble (ramsey_t *rt, global_data_t *state);
void recursion_postamble (ramsey_t *rt);
void recursion_init (ramsey_t *rt);
void recursion_reset (ramsey_t *rt, global_data_t *state);

#endif
