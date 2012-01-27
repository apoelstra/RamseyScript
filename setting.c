
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "global.h"
#include "ramsey/ramsey.h"
#include "setting.h"

#define HASH_TABLE_SIZE	1009

struct _setting_priv {
  setting_t parent;

  char *name;
  char *text;

  long int_data;
  ramsey_t *ramsey_data;

  struct _setting_priv *next;
};

struct _setting_list_priv {
  setting_list_t parent;
  struct _setting_priv *setting[HASH_TABLE_SIZE];
};

/* HELPERS */
/* djb2, lifted from http://www.cse.yorku.ca/~oz/hash.html */
static unsigned _hash (const char *text)
{
  unsigned rv = 5381;
  while (*text)
    {
      rv = 33 * rv + *text;
      ++text;
    }

  return rv % HASH_TABLE_SIZE;
}

static char *_strip_strdup (const char *text)
{
  int n;
  char *rv;

  /* Skip starting whitespace */
  while (*text && isspace (*text))
    ++text;
  /* Drop ending whitespace */
  n = strlen (text) - 1;
  while (n && isspace (text[n]))
    --n;

  rv = malloc (n + 2);
  if (rv != NULL)
    {
      strncpy (rv, text, n + 1);
      rv[n + 1] = '\0';
    }
  return rv;
}

/** setting_list_t  **/
static setting_t *_setting_list_add_setting (setting_list_t *slist, setting_t *set)
{
  struct _setting_list_priv *priv = (struct _setting_list_priv *) slist;
  struct _setting_priv *setting_priv;
  unsigned hash;

  if (set == NULL)
    return NULL;
  hash = _hash (set->get_name (set));
  setting_priv = priv->setting[hash];

  if (setting_priv == NULL)
    priv->setting[hash] = (struct _setting_priv *) set;
  else if (!strcmp (set->get_name (set), setting_priv->name))
    {
      priv->setting[hash] = (struct _setting_priv *) set;
      ((setting_t *) setting_priv)->destroy ((setting_t *) setting_priv);
    }
  else
    {
      while (setting_priv->next)
        {
          if (!strcmp (set->get_name (set), setting_priv->next->name))
            {
              ((setting_t *) setting_priv->next)->destroy ((setting_t *) setting_priv->next);
              setting_priv->next = (struct _setting_priv *) set;
            }
            
          setting_priv = setting_priv->next;
        }
    }

  return set;
}

static const setting_t *_setting_list_get_setting (const setting_list_t *slist, const char *name)
{
  const struct _setting_list_priv *priv = (struct _setting_list_priv *) slist;
  unsigned hash = _hash (name);
  const struct _setting_priv *setting_priv = priv->setting[hash];

  while (setting_priv)
    {
      if (!strcmp (name, setting_priv->name))
        return (setting_t *) setting_priv;
      setting_priv = setting_priv->next;
    }
  return NULL;
}

static int _setting_list_remove_setting (setting_list_t *slist, const char *name)
{
  struct _setting_list_priv *priv = (struct _setting_list_priv *) slist;
  unsigned hash = _hash (name);
  struct _setting_priv *setting_priv = priv->setting[hash];

  if (setting_priv)
    {
      if (!strcmp (name, setting_priv->name))
        {
          setting_t *tmp = (setting_t *) priv->setting[hash];
          priv->setting[hash] = setting_priv->next;
          tmp->destroy (tmp);
          return 1;
        }

      while (setting_priv->next)
        {
          if (!strcmp (name, setting_priv->next->name))
            {
              setting_t *tmp = (setting_t *) setting_priv->next;
              setting_priv->next = setting_priv->next->next;
              tmp->destroy (tmp);
              return 1;
            }
          setting_priv = setting_priv->next;
        }
    }
  return 0;
}

static void _setting_list_print (const setting_list_t *slist, stream_t *out)
{
  struct _setting_list_priv *priv = (struct _setting_list_priv *) slist;
  int i;
  for (i = 0; i < HASH_TABLE_SIZE; ++i)
    {
      struct _setting_priv *set = priv->setting[i];
      while (set)
        {
          setting_t *s = (setting_t *) set;
          s->print (s, out);
          set = set->next;
        }
    }
}

static void _setting_list_destroy (setting_list_t *slist)
{
  if (slist != NULL)
    {
      struct _setting_list_priv *priv = (struct _setting_list_priv *) slist;
      int i;
      for (i = 0; i < HASH_TABLE_SIZE; ++i)
        while (priv->setting[i])
          {
            struct _setting_priv *tmp = priv->setting[i];
            priv->setting[i] = tmp->next;
            free (tmp);
          }
    }
  free (slist);
}

setting_list_t *setting_list_new ()
{
  struct _setting_list_priv *priv = malloc (sizeof *priv);
  setting_list_t *rv = (setting_list_t *) priv;
  int i;

  if (rv != NULL)
    {
      rv->add_setting = _setting_list_add_setting;
      rv->get_setting = _setting_list_get_setting;
      rv->remove_setting = _setting_list_remove_setting;
      rv->print       = _setting_list_print;
      rv->destroy     = _setting_list_destroy;

      for (i = 0; i < HASH_TABLE_SIZE; ++i)
        priv->setting[i] = NULL;
    }

  return rv;
}

/** setting_t  **/

/* ACCESSORS */
static const char *_setting_get_type (const setting_t *set)
{
  const struct _setting_priv *priv = (const struct _setting_priv *) set;
  switch (set->type)
    {
    case TYPE_STRING:  return "string";
    case TYPE_INTEGER: return "integer";
    case TYPE_RAMSEY:  return priv->ramsey_data->get_type (priv->ramsey_data);
    }
  return "unknown";
}

static const char *_setting_get_name (const setting_t *set)
{
  const struct _setting_priv *priv = (const struct _setting_priv *) set;
  return priv->name;
}

static const char *_setting_get_text (const setting_t *set)
{
  const struct _setting_priv *priv = (const struct _setting_priv *) set;
  return priv->text;
}

static ramsey_t *_setting_get_ramsey_value (const setting_t *set)
{
  const struct _setting_priv *priv = (const struct _setting_priv *) set;
  if (set->type == TYPE_RAMSEY)
    return priv->ramsey_data;
  fprintf (stderr, "Warning: tried to read ``%s'' as a Ramsey object, but it is a ``%s''!\n",
           set->get_name (set), set->get_type (set));
  return NULL;
}

static long _setting_get_int_value (const setting_t *set)
{
  const struct _setting_priv *priv = (const struct _setting_priv *) set;
  if (set->type == TYPE_INTEGER)
    return priv->int_data;
  fprintf (stderr, "Warning: tried to read ``%s'' as an integer, but it is a ``%s''!\n",
           set->get_name (set), set->get_type (set));
  return 0;
}

static void _setting_print (const setting_t *set, stream_t *out)
{
  const struct _setting_priv *priv = (const struct _setting_priv *) set;
  stream_printf (out, "%s = ", priv->name);
  switch (set->type)
    {
    case TYPE_STRING:
      stream_printf (out, "(string) %s\n", priv->text);
      break;
    case TYPE_INTEGER:
      stream_printf (out, "(integer) %ld\n", priv->int_data);
      break;
    case TYPE_RAMSEY:
      {
        const ramsey_t *rt = priv->ramsey_data;
        stream_printf (out, "(%s): ", rt->get_type (rt));
        rt->print (rt, out);
        out->write (out, "\n");
      }
      break;
    }
}

/* CONSTRUCTOR / DESTRUCTOR */
static void _setting_destroy (setting_t *set)
{
  struct _setting_priv *priv = (struct _setting_priv *) set;
  free (priv->name);
  free (priv->text);
  if (set->type == TYPE_RAMSEY)
    priv->ramsey_data->destroy (priv->ramsey_data);
  free (set);
}

setting_t *setting_new (const char *name, const char *text)
{
  struct _setting_priv *priv;
  setting_t *rv;

  if (name == NULL || text == NULL)
    return NULL;

  priv = malloc (sizeof *priv);
  rv = (setting_t *) priv;
  if (rv != NULL)
    {
      const char *scan = text;

      rv->type = TYPE_STRING;
      rv->get_type = _setting_get_type;
      rv->get_name = _setting_get_name;
      rv->get_text = _setting_get_text;
      rv->get_ramsey_value = _setting_get_ramsey_value;
      rv->get_int_value = _setting_get_int_value;
      rv->print         = _setting_print;
      rv->destroy       = _setting_destroy;

      priv->name = _strip_strdup (name);
      priv->text = _strip_strdup (text);

      priv->ramsey_data = NULL;
      priv->int_data    = 0;

      priv->next = NULL;

      while (*scan && isspace (*scan))
        ++scan;
      if (isdigit (*scan) || *scan == '-')
        {
          rv->type = TYPE_INTEGER;
          priv->int_data = strtol (text, NULL, 0);
        }
      else if (*scan == '[')
        {
          priv->ramsey_data = ramsey_new_from_parse (scan);
          rv->type = TYPE_RAMSEY;
        }
    }

  return rv;
}

