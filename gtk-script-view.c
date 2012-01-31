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


#include <string.h>

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include "file-stream.h"
#include "stream.h"
#include "gtk-text-buffer-stream.h"
#include "gtk-script-view.h"

struct _GtkScriptView
{
  GtkScrolledWindow parent;

  const gchar *filename;
  GtkWidget *text_view, *label;
  GtkTextBuffer *buffer;

  gboolean modified;
  gboolean modify_holdoff;
};

struct _GtkScriptViewClass
{
  GtkScrolledWindowClass parent_class;
};

/* HELPERS */
static void _set_modified (GtkScriptView *view, gboolean modified)
{
  if (view->modified == modified)
    return;
  if (modified == TRUE)
    {
      const gchar *label = gtk_label_get_text (GTK_LABEL (view->label));
      gchar *new_label = g_strdup_printf ("%s*", label);
      gtk_label_set_text (GTK_LABEL (view->label), new_label);
      g_free (new_label);
    }
  else
    {
      const gchar *label = gtk_label_get_text (GTK_LABEL (view->label));
      gchar *new_label = g_strdup (label);
      new_label[strlen (new_label) - 1] = '\0';  /* erase '*' */
      gtk_label_set_text (GTK_LABEL (view->label), new_label);
      g_free (new_label);
    }
  view->modified = modified;
}

static gboolean _show_save_as_dialog (GtkScriptView *view)
{
  gboolean success = FALSE;
  GtkWidget *dialog;
  GtkWidget *window = gtk_widget_get_toplevel (GTK_WIDGET (view));
  gchar *title = g_strdup_printf ("Save %s as...",
                                  gtk_label_get_text (GTK_LABEL (view->label)));
  dialog = gtk_file_chooser_dialog_new (title,
                                        GTK_WINDOW (window),
                                        GTK_FILE_CHOOSER_ACTION_SAVE,
                                        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                        GTK_STOCK_OK, GTK_RESPONSE_OK,
                                        NULL);
  gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog),
                                                  TRUE);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK)
    {
      gchar *result = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
      gtk_script_view_set_filename (view, result);
      success = TRUE;
    }

  gtk_widget_destroy (dialog);
  g_free (title);
  return success;
}

static const char *_find_last_slash (const char *inp)
{
  const char *rv = inp;
  const char *scan = inp;
  while (*scan)
    if (*scan++ == '/')
      rv = scan;
  return rv;
}


/* SIGNAL HANDLERS */
static void _buffer_changed_cb (GtkTextBuffer *buf, gpointer data)
{
  GtkScriptView *view = data;
  if (!view->modify_holdoff)
    _set_modified (view, TRUE);

  (void) buf;
}

/* CONSTRUCTOR */
static void gtk_script_view_init (GtkScriptView *view)
{
  (void) view;
}

static void
gtk_script_view_class_init (GtkScriptViewClass *klass)
{
  (void) klass;
}

/* PUBLIC FUNCTIONS */
GType gtk_script_view_get_type (void)
{
  static GType obj_type = 0;

  if (!obj_type)
    {
      const GTypeInfo obj_info =
      {
	sizeof (GtkScriptViewClass),
	NULL, /* base_init */
	NULL, /* base_finalize */
	(GClassInitFunc) gtk_script_view_class_init,
	NULL, /* class_finalize */
	NULL, /* class_data */
	sizeof (GtkScriptView),
	0,    /* n_preallocs */
	(GInstanceInitFunc) gtk_script_view_init,
        NULL
      };

      obj_type = g_type_register_static (GTK_TYPE_SCROLLED_WINDOW,
                                        "GtkScriptView",
                                        &obj_info,
                                        0);
    }

  return obj_type;
}

GtkWidget *gtk_script_view_new (const gchar *label)
{
  GtkScriptView *view = g_object_new (GTK_SCRIPT_VIEW_TYPE, NULL);

  view->filename = NULL;
  view->label = gtk_label_new (label);
  view->text_view = gtk_text_view_new ();
  view->buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view->text_view));

  view->modified = FALSE;
  view->modify_holdoff = FALSE;

  gtk_text_view_set_editable (GTK_TEXT_VIEW (view->text_view), TRUE);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (view),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (view), view->text_view);

  g_signal_connect (view->buffer, "changed",
                    G_CALLBACK (_buffer_changed_cb), view);

  g_object_ref (G_OBJECT (view->label));

  return GTK_WIDGET (view);
}

GtkWidget *gtk_script_view_new_from_file (const gchar *filename)
{
  GtkScriptView *view = GTK_SCRIPT_VIEW (gtk_script_view_new (""));
  stream_t *file_stream = file_stream_new (filename);
  stream_t *view_stream = text_buffer_stream_new (view->buffer);

  if (file_stream->open (file_stream, STREAM_READ))
    {
      view_stream->open (view_stream, STREAM_WRITE);
      view->modify_holdoff = TRUE;
      stream_line_copy (view_stream, file_stream);
      view->modify_holdoff = FALSE;
      gtk_script_view_set_filename (view, filename);
    }
  else
    {
      fprintf (stderr, "Failed to open file ``%s''\n", filename);
      gtk_widget_destroy (GTK_WIDGET (view));
      view = NULL;
    }
  file_stream->destroy (file_stream);
  view_stream->destroy (view_stream);
  return GTK_WIDGET (view);
}

gboolean gtk_script_view_save (GtkScriptView *view, gboolean force_save_as)
{
  gboolean success = FALSE;
  stream_t *file_writer, *view_reader;

  if (view->filename == NULL || force_save_as)
    if (!_show_save_as_dialog (view))
      return FALSE;

  file_writer = file_stream_new (view->filename);
  view_reader = text_buffer_stream_new (view->buffer);
  if (file_writer->open (file_writer, STREAM_WRITE))
    {
      view_reader->open (view_reader, STREAM_READ);
      stream_line_copy (file_writer, view_reader);
      _set_modified (view, FALSE);
      success = TRUE;
    }
  else
    fprintf (stderr, "gtk_script_view: failed to save file ``%s''\n", view->filename);

  view_reader->destroy (view_reader);
  file_writer->destroy (file_writer);
  return success;
}

void gtk_script_view_set_filename (GtkScriptView *view, const gchar *filename)
{
  view->filename = filename;
  gtk_label_set_text (GTK_LABEL (view->label), _find_last_slash (filename));
}

const gchar *gtk_script_view_get_filename (GtkScriptView *view)
{
  return view->filename;
}

const gchar *gtk_script_view_get_title (GtkScriptView *view)
{
  return gtk_label_get_text (GTK_LABEL (view->label));
}

GtkWidget *gtk_script_view_get_label (GtkScriptView *view)
{
  return view->label;
}

gboolean gtk_script_view_get_modified (GtkScriptView *view)
{
  return view->modified;
}

GtkTextBuffer *gtk_script_view_get_buffer (GtkScriptView *view)
{
  return view->buffer;
}

void gtk_script_view_destroy (GtkScriptView *view, gboolean confirm)
{
  if (confirm && view->modified)
    {
      GtkWidget *dialog;
      GtkWidget *window = gtk_widget_get_toplevel (GTK_WIDGET (view));
      dialog = gtk_message_dialog_new (GTK_WINDOW (window),
                                       GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                       GTK_MESSAGE_WARNING,
                                       GTK_BUTTONS_NONE, NULL);
      gtk_message_dialog_set_markup (GTK_MESSAGE_DIALOG (dialog),
        "There are unsaved changes. Are you sure you want to close this script?");
      gtk_dialog_add_buttons (GTK_DIALOG (dialog),
                              "Close _without saving", GTK_RESPONSE_NO,
                              GTK_STOCK_CANCEL,        GTK_RESPONSE_CANCEL,
                              GTK_STOCK_SAVE,          GTK_RESPONSE_YES,
                              NULL);

      switch (gtk_dialog_run (GTK_DIALOG (dialog)))
        {
        case GTK_RESPONSE_YES:
          gtk_script_view_save (view, FALSE);
          break;
        case GTK_RESPONSE_NO:
          break;
        case GTK_RESPONSE_CANCEL:
          gtk_widget_destroy (dialog);
          return;
        }
      gtk_widget_destroy (dialog);
    }
  g_object_unref (G_OBJECT (view->label));
  gtk_widget_destroy (view->text_view);
  gtk_widget_destroy (GTK_WIDGET (view));
}

