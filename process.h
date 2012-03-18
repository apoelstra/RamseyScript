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

/*! \file process.h
 *  \brief Defines script-processing and parsing functions.
 */

#ifndef PROCESS_H
#define PROCESS_H

#include "stream.h"

/*! \brief Sets default variables before a script run.
 *
 *  \param [in] in   The input stream to use.
 *  \param [in] out  The output stream to use.
 *  \param [in] err  The error stream to use.
 *
 *  \return A newly-allocated global_data struct, or NULL on failure.
 */
struct _global_data *set_defaults (stream_t *in, stream_t *out, stream_t *err);

/*! \brief Reads and runs a script from the global state's input stream.
 *
 *  \param [in] state  The global state of the program.
 */
void process (struct _global_data *state);

/*! \brief Outputs a user-friendly usage message to a given stream.
 *
 * \param [in] out  The stream to output to.
 */
void ramsey_usage (stream_t *out);

#endif
