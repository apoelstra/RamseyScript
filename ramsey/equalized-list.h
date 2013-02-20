/* RamseyScript
 * Written in 2013 by
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

/*! \file equalized-list.h
 *  \brief Defines functions related to the equalized list type.
 */

#ifndef QLIST_H
#define QLIST_H

#include "ramsey.h"

/*! \brief Create a new equalized list
 *
 *  \param [in] vars  The script variable table.
 *
 *  \return A newly allocated equalized list, or NULL on failure.
 */
void *equalized_list_new (const setting_list_t *vars);

#endif
