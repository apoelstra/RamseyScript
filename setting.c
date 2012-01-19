
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "global.h"
#include "coloring.h"
#include "sequence.h"
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
  setting_priv = (struct _setting_priv *) priv->setting[hash];

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

static setting_t *_setting_list_get_setting (const setting_list_t *slist, const char *name)
{
  struct _setting_list_priv *priv = (struct _setting_list_priv *) slist;
  unsigned hash = _hash (name);
  const struct _setting_priv *setting_priv = (struct _setting_priv *) priv->setting[hash];

  while (setting_priv)
    {
      if (!strcmp (name, setting_priv->name))
        return (setting_t *) setting_priv;
      setting_priv = setting_priv->next;
    }
  return NULL;
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
  struct _setting_priv *priv = malloc (sizeof *priv);
  setting_t *rv = (setting_t *) priv;

  if (rv != NULL)
    {
      const char *scan = text;

      rv->type = TYPE_STRING;
      rv->get_type = _setting_get_type;
      rv->get_name = _setting_get_name;
      rv->get_text = _setting_get_text;
      rv->get_ramsey_value = _setting_get_ramsey_value;
      rv->get_int_value = _setting_get_int_value;
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
          const char *scan2 = scan + 1;
          while (*scan2 && isspace (*scan2))
            ++scan2;
          if (isdigit (*scan2) || *scan2 == '-')
            priv->ramsey_data = sequence_new ();
          else if (*scan2 == '[')
            {
              int n_colors = 1;
              while (*scan2)
                {
                  if (*scan2 == '[')
                    ++n_colors;
                  ++scan2;
                }
              priv->ramsey_data = coloring_new (n_colors);
            }
          else 
            {
              fprintf (stderr, "Error: ``%s'' looks like a list but is not.\n",
                       priv->name);
              rv->destroy (rv);
              return NULL;
            }
          if (priv->ramsey_data)
            priv->ramsey_data->parse (priv->ramsey_data, scan);
          rv->type = TYPE_RAMSEY;
        }
    }

  return rv;
}

