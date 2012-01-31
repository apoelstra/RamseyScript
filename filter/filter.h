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

#ifndef FILTER_H
#define FILTER_H

#include "../global.h"
#include "../ramsey/ramsey.h"

typedef enum _e_filter_mode {
  MODE_FULL,
  MODE_LAST_ONLY
} e_filter_mode;

struct _filter_t {
  e_filter_mode mode;

  const char *(*get_type) (const filter_t *);

  bool (*run)      (const filter_t *, const ramsey_t *);

  bool (*supports) (const filter_t *, e_ramsey_type);
  bool (*set_mode) (filter_t *, e_filter_mode);
  filter_t *(*clone) (const filter_t *);
  void (*destroy)  (filter_t *);
};

filter_t *filter_new (const char *, const global_data_t *);
filter_t *filter_new_custom (const char *name,
                             bool (*run) (const filter_t *f, const ramsey_t *));
filter_t *filter_new_generic (void);
void filter_usage (stream_t *out);

#endif
