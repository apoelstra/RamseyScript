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

/*! \file sequence.h
 *  \brief Defines functions related to the sequence type.
 */

#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "ramsey.h"

/*! \brief Create a new sequence.
 *
 *  The constructor determines what gap sizes are allowable for
 *  the sequence by the 'gap-set' script variable. If this is
 *  unset, the sequence will be unable to be recursed.
 *
 *  \param [in] vars  The table of script variables.
 *
 *  \return A newly allocated coloring, or NULL on failure.
 */
void *sequence_new (const setting_list_t *vars);

/*! \brief Create a new sequence directly.
 *
 *  The created sequence will have no gap set associated with it,
 *  so it will not be recursable by default. It is used by other
 *  objects, such as words or permutations, which recurse in their
 *  own way.
 */
void *sequence_new_direct (void);

/*! \brief Return a singleton prototype of the sequence object.
 *
 *  This is used to copy functionality from the sequence object
 *  into other ramsey_t objects, without worrying about their
 *  underlying implementation.
 *
 *  \return Sequence prototype.
 */
const ramsey_t *sequence_prototype (void);

#endif
