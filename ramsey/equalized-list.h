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

/*! \brief Increment a counter for an equalized list member.
 *
 *  If incrementing this counter causes the member to have a
 *  higher count than anyone after him, he will be moved toward
 *  the end of the list.
 *
 *  The net result will be that get_priv_data_const will always
 *  have the least popular members first.
 */
void equalized_list_increment (ramsey_t *rt, int index);

/*! \brief Decrement a counter for an equalized list member.
 *
 *  If decrementing this counter causes the member to have a
 *  lower count than anyone after him, he will be moved toward
 *  the beginning of the list.
 *
 *  The net result will be that get_priv_data_const will always
 *  have the least popular members first.
 */
void equalized_list_decrement (ramsey_t *rt, int index);

#endif
