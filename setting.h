#ifndef SETTING_H
#define SETTING_H

#include "global.h"
#include "stream.h"

typedef struct _setting_t setting_t;
typedef struct _setting_list_t setting_list_t;

struct _setting_t {
  e_setting_type type;

  const char *(*get_type) (const setting_t *);
  const char *(*get_name) (const setting_t *);
  const char *(*get_text) (const setting_t *);

  void (*print) (const setting_t *set, stream_t *out);

  long      (*get_int_value)    (const setting_t *);
  ramsey_t *(*get_ramsey_value) (const setting_t *);
  void  (*destroy)       (setting_t *);
};

struct _setting_list_t {
  void (*print) (const setting_list_t *slist, stream_t *out);

  setting_t *(*add_setting) (setting_list_t *, setting_t *);
  setting_t *(*get_setting) (const setting_list_t *, const char *name);
  int  (*remove_setting) (setting_list_t *, const char *name);
  void (*destroy) (setting_list_t *);
};

setting_t *setting_new (const char *name, const char *text);
setting_list_t *setting_list_new ();

#endif
