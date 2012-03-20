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

/*! \file lattice.h
 *  \brief Defines functions related to the lattice type.
 */

#ifndef LATTICE_H
#define LATTICE_H

#include "ramsey.h"

/*! \brief Create a new lattice.
 *
 *  The constructor uses the script variable n-colors to determine
 *  the number of colors to use, and n-columns to determine the
 *  width of the lattice.
 *
 *  \param [in] vars  The script variable table.
 *
 *  \return A newly allocated lattice, or NULL on failure.
 */
void *lattice_new (const setting_list_t *vars);

#endif
