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

/*! \file setting.h
 *  \brief Defines setting (i.e., script variable) class and related functions.
 */
#ifndef SETTING_H
#define SETTING_H

#include "global.h"
#include "stream.h"

/*! \brief Convienence typedef for struct _setting_t. */
typedef struct _setting_t setting_t;
/*! \brief Convienence typedef for struct _setting_list_t. */
typedef struct _setting_list_t setting_list_t;

/*! \brief Script variable type.
 *
 *  When a script says "set <var> <value>", this structure stores that
 *  setting, determining the type of the given value from a best-guess
 *  (mainly, how many '['s it starts with), and controls access to the
 *  value to safely catch type mismatches.
 */
struct _setting_t {
  /*! \brief Type of variable value. */
  e_setting_type type;

  /*! \brief Returns a string describing the variable. */
  const char *(*get_type) (const setting_t *);
  /*! \brief Returns the variable's name. */
  const char *(*get_name) (const setting_t *);
  /*! \brief Returns the raw text used to set the variable. */
  const char *(*get_text) (const setting_t *);

  /*! \brief Print the value of the variable. */
  void (*print) (const setting_t *set, stream_t *out);

  /*! \brief Get the value of an integer variable. */
  long      (*get_int_value)    (const setting_t *);
  /*! \brief Get the value of an Ramsey object variable. */
  ramsey_t *(*get_ramsey_value) (const setting_t *);

  /*! \brief Destroys a variable and frees its associated variables. */
  void  (*destroy)       (setting_t *);
};

/*! \brief Hash table of script variables. */
struct _setting_list_t {
  /*! \brief Print the contents of the table. */
  void (*print) (const setting_list_t *slist, stream_t *out);

  /*! \brief Add a script variable to the table. */
  setting_t *(*add_setting) (setting_list_t *, setting_t *);
  /*! \brief Fetch a script variable of a given name. */
  const setting_t *(*get_setting) (const setting_list_t *, const char *name);
  /*! \brief Remove a script variable of a given name. */
  int  (*remove_setting) (setting_list_t *, const char *name);
  /*! \brief Destroys the table and frees its associated variables. */
  void (*destroy) (setting_list_t *);
};

/*! \brief Constructor for script variables.
 *
 *  \param [in] name  The case-sensitive name that the varibale
 *                    should be accessed by.
 *  \param [in] text  The value of the variable.
 *
 *  \return A newly-accolcated variable, or NULL on error.
 */
setting_t *setting_new (const char *name, const char *text);

/*! \brief Constructor for variable hash table
 *  \return A newly-accolcated variable, or NULL on error.
 */
setting_list_t *setting_list_new ();

#endif
