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

/*! \file filter.h
 *  \brief Defines the interfaces that filters must support.
 */
#ifndef FILTER_H
#define FILTER_H

#include "../global.h"
#include "../ramsey/ramsey.h"

/*! \brief How well the filter needs to do its job.
 *
 *  As an example of how this enum is used, consider a filter
 *  that checks for 3-term arithmetic progressions; i.e., solutions
 *  to x + y = 2z. If set to MODE_FULL, it needs to check an entire
 *  coloring for solutions.
 *
 *  If set to MODE_LAST_ONLY, it only checks for solutions which
 *  involve the most recently added element. This gives a speedup
 *  when recursively building colorings.
 */
typedef enum _e_filter_mode {
  MODE_FULL,      /*!< Check the entire object. */
  MODE_LAST_ONLY  /*!< Only check the recently-changed part of the robot. */
} e_filter_mode;

/*! \brief The main filter type.
 *
 *  Filters are used to restrict recursion to only search objects
 *  which satisfy some constraints (having no solutions to x + y = z,
 *  for example).
 */
struct _filter_t {
  /*! \brief The mode the filter operates in. */
  e_filter_mode mode;

  /*! \brief Returns a string description of the filter. */
  const char *(*get_type) (const filter_t *);

  /*! \brief Runs the filter on a ramsey object.
   *
   *  Returns 1 if the object is okay, 0 if it's not.
   */
  bool (*run)      (const filter_t *, const ramsey_t *);

  /*! \brief Whether the filter affects to all colors the same way (for colorings). */
  bool (*get_symmetry) (const filter_t *);
  /*! \brief Checks whether the filter applies to a given type of object. */
  bool (*supports) (const filter_t *, e_ramsey_type);
  /*! \brief Sets the filter's mode. */
  bool (*set_mode) (filter_t *, e_filter_mode);
  /*! \brief Copies the filter. */
  filter_t *(*clone) (const filter_t *);
  /*! \brief Destroy the filter and free its associated resources. */
  void (*destroy)  (filter_t *);
};

/*! \brief Creates a new filter.
 *
 *  \param [in]  data The name of the filter (must match an installed filter).
 *  \param [in]  vars The table of script variables.
 *
 *  \return A newly-allocated filter, or NULL on failure.
 */
filter_t *filter_new (const char *data, const setting_list_t *vars);

/*! \brief Create a simple single-function filter.
 *
 *  \param [in]  name The user-visible name of the filter.
 *  \param [in]  run  The filter function to run on objects.
 *
 *  \return A newly-allocated filter, or NULL on failure.
 */
filter_t *filter_new_custom (const char *name,
                             bool (*run) (const filter_t *f, const ramsey_t *));

/*! \brief Create a blank filter.
 *
 * This is used by other filters, and wraps up some of the boilerplate
 * junk.
 *
 * \return A newly-allocated filter, or NULL on failure.
 */
filter_t *filter_new_generic (void);

/*! \brief Output the list of installed filters.
 *
 *  \param [in] out  The stream to output to.
 *
 *  Dumps a list of filters and their help prompts.
 */
void filter_usage (stream_t *out);

#endif
