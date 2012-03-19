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

#ifndef DUMP_H
#define DUMP_H

#include "../global.h"
#include "../setting.h"
#include "../stream.h"

data_collector_t *dump_new (const char *, const setting_list_t *);
void dump_usage (stream_t *out);

#endif
