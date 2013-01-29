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

/*! \file coloring.h
 *  \brief Defines functions related to the coloring type.
 */

#ifndef COLORING_H
#define COLORING_H

#include "ramsey.h"

/*! \brief Create a new coloring.
 *
 *  The constructor uses the script variable n-colors to determine
 *  the number of colors to use. If this is unset, it will fail.
 *
 *  \param [in] vars  The table of script variables.
 *
 *  \return A newly allocated coloring, or NULL on failure.
 */
void *coloring_new (const setting_list_t *vars);

/*! \brief Create a new coloring directly.
 *
 *  \param [in] n_colors       The number of colors to use in the coloring.
 *  \param [in] base_sequence  The sequence to color. Defaults to [1 ...]
 *                             if NULL is passed in.
 *
 *  \return A newly allocated coloring, or NULL on failure.
 */
void *coloring_new_direct (int n_colors, const ramsey_t *base_sequence);

#endif
