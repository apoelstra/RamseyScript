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

/*! \file ramsey.h
 *  \brief Defines the main Ramsey object type and its functions.
 */

#ifndef RAMSEY_H
#define RAMSEY_H

#include <time.h>

#include "../global.h"
#include "../stream.h"
#include "../recurse.h"
#include "../setting.h"

/*! \brief The main Ramsey type.
 *
 * This is subclassed by all Ramsey objects (colorings, sequences,
 * permutations, etc.) and defines all the functionality required
 * of such an object. In this way, the core code can function without
 * making any assumptions about what specific type of object it is
 * maniputating, and conversely, Ramsey problems can be changed and
 * added without needing to modify or understand the core logic.
 *
 * There are a number of public recursion-related variables in this
 * class, which maybe shouldn't be there, but they make the code much
 * simpler.
 */
struct _ramsey_t {
  /*! \brief A class ID specifying what type of Ramsey object this is.
   *         Generally, filters and data collectors should be the only
   *         things that care about this.
   */
  e_ramsey_type type;

  /* Recursion state variables */
  /*! \brief Depth we have recursed to to get this object. */
  int r_depth;
  /*! \brief Maximum depth we have attained so far. */
  int r_max_thread_depth;
  /*! \brief Number of recursion calls done to get this object. */
  long r_iterations;
  /*! \brief Maximum allowable recursion depth (may be 0 for no max). */
  int r_max_depth;
  /*! \brief Maximum allowable iteration count (may be 0 for no max). */
  long r_max_iterations;
  /*! \brief Whether to prune the recursion tree (see prune-tree section of README) */
  int r_prune_tree;
  /*! \brief Maximum number of iterations with no progress (may be 0 for no max). */
  long r_stall_after;
  /*! \brief Current number of iterations with no progress. */
  long r_stall_index;
  /*! \brief The time (in seconds) that the recursion was started */
  time_t r_start_time;
  /*! \brief Maximum allowable runtime (in seconds) */
  long r_max_run_time;

  /* vtable */
  /*! \brief Returns a string describing the object. */
  const char *(*get_type) (const ramsey_t *);

  /*! \brief Populate the object from a stringized description. Returns a
   *         a pointer into the string just past the last character used. */
  const char *(*parse) (ramsey_t *, const char *data);
  /*! \brief Populate the object randomly, given a target length. */
  void (*randomize)    (ramsey_t *, int);
  /*! \brief Output a text representation of the object to the given stream. */
  void (*print)        (const ramsey_t *, stream_t *);
  /*! \brief Covert the object into a nil object (i.e., empty sequence,
   *         empty coloring, etc.). */
  void (*empty)        (ramsey_t *);
  /*! \brief Empty the object, as well as remove all filters and other data. */
  void (*reset)        (ramsey_t *);
  /*! \brief Make an exact copy of the object.  */
  ramsey_t *(*clone)   (const ramsey_t *);
  /*! \brief Free the object and its associated resources. */
  void (*destroy)      (ramsey_t *);

  /* WARNING: when a filter is added to a ramsey_t, the
   *          ramsey_t assumes ownership of the filter
   *          and may modify or free it at its discretion. */
  /*! \brief Attach a filter to the object.
   *
   * Note that when a filter is attached, the Ramsey object assumes ownership
   * of it, and may free it at its discretion. Also, if you destroy the filter
   * except by destroying the Ramsey object, the behavior is undefined.
   */
  int (*add_filter)  (ramsey_t *, filter_t *);
  /*! \brief Run all attached filters on the object. Returns 1 for pass, 0 for fail. */
  int (*run_filters) (const ramsey_t *);

  /*! \brief Recursively search a space of objects, using the given object
   *         as a seed. */
  void (*recurse)       (ramsey_t *, const global_data_t *);

  /*! \brief Returns the length of the object. */
  int (*get_length)  (const ramsey_t *);
  /*! \brief Returns the maximum number in the object. */
  int (*get_maximum) (const ramsey_t *);
  /*! \brief Returns the number of cells for cell-based objects (i.e., colorings). */
  int (*get_n_cells) (const ramsey_t *);
  /*! \brief Add a new value to the object. */
  int (*append)      (ramsey_t *, int value);
  /*! \brief Add a new value to a given cell in the object. */
  int (*cell_append) (ramsey_t *, int value, int cell);
  /*! \brief Remove the last-added value from the object. */
  int (*deappend)    (ramsey_t *);
  /*! \brief Remove the last-added value from a given cell in the object. */
  int (*cell_deappend) (ramsey_t *, int cell);

  /*! \brief Finds a value in the object. Returns the cell in which it
    *        was found, or NULL if the number does not exist. */
  const ramsey_t *(*find_value) (const ramsey_t *, int);

  /*! \brief Returns private data for the object. (Used for filters.)
   * 
   *  There are necessarily no safety checks on this, so make
   *  sure you check the object's type before using its private
   *  data. */
  void *(*get_priv_data) (ramsey_t *);
  /*! \brief Returns a const-ified version of the object's private data.
   * 
   *  For sanity's sake, this should be used whenever possible as an alternate
   *  to get_priv_data(). Optimally, I'd like to get rid of the non-const
   *  version entirely, but that does not appear to be feasible.
   */
  const void *(*get_priv_data_const) (const ramsey_t *);
  /*! \brief Return an alternate representation for the object's private data.
   * 
   *  Right now this is only implemented for the coloring object, to return
   *  its colors as a word on the alphabet { 0,1,...,(r-1) }, where r is the
   *  number of colors. get_priv_data() in this case returns an array of cells,
   *  which is not useful for some filters.
   */
  const void *(*get_alt_priv_data_const) (const ramsey_t *);
};

/*! \brief Constructor for a Ramsey object
 *
 *  \param [in] data   The name of the object's type (i.e., "sequence" or "coloring").
 *  \param [in] vars   The table of script variables.
 *
 *  \return A newly-allocated Ramsey object, or NULL on failure.
 */
ramsey_t *ramsey_new (const char *data, const setting_list_t *vars);

/*! \brief Constructor for a Ramsey object
 *
 *  Accepts a string to parse (i.e., "[ 1 2 3 ]") and infers the
 *  correct type from this.
 *
 *  \param [in] data   The string to parse.
 *
 *  \return A newly-allocated Ramsey object, or NULL on failure.
 */
ramsey_t *ramsey_new_from_parse (const char *data);

#endif
