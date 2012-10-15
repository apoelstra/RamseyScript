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

/*! \file global.h
 *  \brief Definitions needed by all parts of the codebase.
 */
#ifndef GLOBAL_H
#define GLOBAL_H

/*! \brief Convienence typedef for parser object. */
typedef struct _parser_t parser_t;
/*! \brief Convienence typedef for Ramsey object. */
typedef struct _ramsey_t ramsey_t;
/*! \brief Convienence typedef for Ramsey filter. */
typedef struct _filter_t filter_t;
/*! \brief Convienence typedef for targets and dumps. */
typedef struct _data_collector_t data_collector_t;
/*! \brief Convienence typedef for global context. */
typedef struct _global_data global_data_t;
/*! \brief C boolean ;) */
typedef int bool;

/*! \brief Type ID's for Ramsey objects. */
typedef enum _e_ramsey_type {
  TYPE_SEQUENCE,      /*!< Sequence -- list of integers. */
  TYPE_COLORING,      /*!< Coloring -- partition of [1, N]
                           into sequences. */
  TYPE_WORD,          /*!< Word -- sequence on fixed alphabet. */
  TYPE_PERMUTATION,   /*!< Permutation -- rearrangement of [1, N]. */
  TYPE_LATTICE        /*!< Lattice -- 2D array of integers */
} e_ramsey_type;

/*! \brief Type ID's for script variables. */
typedef enum _e_setting_type {
  TYPE_INTEGER,       /*!< Integer -- long integer. */
  TYPE_RAMSEY,        /*!< Ramsey -- a Ramsey object, see _e_ramsey_type. */
  TYPE_STRING         /*!< String -- anything else. */
} e_setting_type;

#include "dump/dump.h"
#include "filter/filter.h"
#include "setting.h"
#include "stream.h"

/*! \brief The version that is displayed in the hello message. */
#define VERSION "0.5"
/*! \brief Convienence macro to access script variables. */
#define SETTING(s) (state->settings->get_setting (state->settings, (s)))

/*! \brief Word that the parser understands. */
struct _parser_t {
  /*! \brief The actual word. */
  const char *name;
  /*! \brief User-visible description of the word. */
  const char *help;
  /*! \brief Constructor for whatever the word represents. */
  void *(*construct) (const setting_list_t *);
};

/*! \brief Object class for targets and data dumps. */
struct _data_collector_t {
  /*! \brief Returns a string describing the data collector */
  const char *(*get_type) (const data_collector_t *);

  /*! \brief Reset any internal counters and erase recorded data. */
  void (*reset)   (data_collector_t *);
  /*! \brief Record collector-specific data from the given ramsey_t. */
  int  (*record)  (data_collector_t *, const ramsey_t *, stream_t *);
  /*! \brief Print collector state. */
  void (*output)  (const data_collector_t *, stream_t *);
  /*! \brief Destroy collector and release associated resources. */
  void (*destroy) (data_collector_t *);
};

/*! \brief Linked list of currently set filters. */
typedef struct _filter_list {
  /*! */
  filter_t *data;
  /*! */
  struct _filter_list *next;
} filter_list;

/*! \brief Linked list of currently set targets and data dumps. */
typedef struct _dc_list {
  /*! */
  data_collector_t *data;
  /*! */
  struct _dc_list *next;
} dc_list;

/*! \brief Global program state. */
struct _global_data {
  /*! \brief Script variable list. */
  setting_list_t *settings;

  /*! \brief Script filter list. */
  filter_list *filters;
  /*! \brief Script data-dump list. */
  dc_list *dumps;
  /*! \brief Script target list. */
  dc_list *targets;

  /*! \brief Repress metadata output on 'search' command */
  bool quiet;
  /*! \brief If we are reading from stdin, output friendlier messages. */
  bool interactive;
  /*! \brief Hook for 'Stop' command from threads or signals. */
  bool kill_now;

  /*! \brief Abstraction of stdout. */
  stream_t *out_stream;
  /*! \brief Abstraction of stdin. */
  stream_t *in_stream;
  /*! \brief Abstraction of stderr. */
  stream_t *err_stream;
};

#endif
