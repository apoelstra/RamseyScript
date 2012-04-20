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

/*! \file recurse.h
 *  \brief Defines generic recursion-handling functions.
 */

#ifndef RECURSE_H
#define RECURSE_H

#include "global.h"

#include <pthread.h>

/*! \brief Recursion checks to run before ramsey_t->recurse().
 *
 *  \param [in] state The global state of the program.
 *
 *  \return 1 if recursion should be done, 0 if it should be stopped.
 */
int recursion_preamble (const global_data_t *state);


/*! \brief Recursion checks to run before subthread recursion call
 *
 *  \param [in] rt    The Ramsey object that is being recursed on.
 *
 *  \return 1 if recursion should be done, 0 if it should be stopped.
 */
int recursion_preamble_statefree (ramsey_t *rt);

/*! \brief Recursion checks to run after ramsey_t->recurse().
 *
 *  \param [in] rt    The Ramsey object that is being recursed on.
 */
void recursion_postamble (ramsey_t *rt);

/*! \brief Zero out recursion-related variables of a ramsey_t.
 *
 *  \param [in] rt    The Ramsey object that is being recursed on.
 */
void recursion_init (ramsey_t *rt);

/*! \brief Set recursion-related variables of a ramsey_t from script variables.
 *
 *  \param [in] state The global state of the program.
 */
void recursion_reset (global_data_t *state);


int recursion_thread_spawn (pthread_t *thread, const ramsey_t *parent,
                            void *(*thread_main)(void *));

void recursion_thread_join (pthread_t thread, ramsey_t *parent);

#endif
