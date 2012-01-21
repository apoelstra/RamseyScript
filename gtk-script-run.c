
#include <stdlib.h>

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include "global.h"
#include "ramsey.h"
#include "stream.h"
#include "string-stream.h"
#include "gtk-text-buffer-stream.h"
#include "gtk-script-view.h"
#include "gtk-script-run.h"

struct _GtkScriptRun
{
  GtkScrolledWindow parent;

  GtkWidget *text_view, *label;
  GtkTextBuffer *buffer;
  stream_t *view_stream;
  stream_t *script_stream;
  stream_t *output_stream;

  GThread *thread;
  struct _global_data *tdata;
  guint timeout_id;
  gboolean running;
};

struct _GtkScriptRunClass
{
  GtkScrolledWindowClass parent_class;
};

/* THREAD BODY */
static void *_thread_body (gpointer r)
{
  GtkScriptRun *run = r;
  run->tdata = set_defaults (run->script_stream, run->output_stream, run->output_stream); 

  /* Open streams */
  run->output_stream->open (run->output_stream, STREAM_WRITE);
  run->script_stream->open (run->script_stream, STREAM_READ);

  /* Go! */
  process (run->tdata);

  /* Cleanup */
  run->running = FALSE;
  free (run->tdata);
  return NULL;
}

static gboolean _update_view (gpointer r)
{
  GtkScriptRun *run = r;
  stream_line_copy (run->view_stream, run->output_stream);
  return run->running;
}


/* CONSTRUCTOR */
static void gtk_script_run_init (GtkScriptRun *run)
{
  (void) run;
}

static void
gtk_script_run_class_init (GtkScriptRunClass *klass)
{
  (void) klass;
}

/* PUBLIC FUNCTIONS */
GType gtk_script_run_get_type (void)
{
  static GType obj_type = 0;

  if (!obj_type)
    {
      const GTypeInfo obj_info =
      {
	sizeof (GtkScriptRunClass),
	NULL, /* base_init */
	NULL, /* base_finalize */
	(GClassInitFunc) gtk_script_run_class_init,
	NULL, /* class_finalize */
	NULL, /* class_data */
	sizeof (GtkScriptRun),
	0,    /* n_preallocs */
	(GInstanceInitFunc) gtk_script_run_init,
        NULL
      };

      obj_type = g_type_register_static (GTK_TYPE_SCROLLED_WINDOW,
                                        "GtkScriptRun",
                                        &obj_info,
                                        0);
    }

  return obj_type;
}

GtkWidget *gtk_script_run_new (GtkScriptView *view)
{ 
  GtkScriptRun *run = g_object_new (GTK_SCRIPT_RUN_TYPE, NULL);
  stream_t *script_stream;
  gchar *label_text = g_strdup_printf ("Output: %s",
                                       gtk_script_view_get_title (view));

  run->label = gtk_label_new (label_text);
  run->text_view = gtk_text_view_new ();
  run->buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (run->text_view));
  run->running = FALSE;
  run->thread = NULL;
  run->timeout_id = -1;

  run->script_stream = string_stream_new ();
  run->view_stream = text_buffer_stream_new (run->buffer);

  gtk_text_view_set_editable (GTK_TEXT_VIEW (run->text_view), FALSE);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (run),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);

  gtk_container_add (GTK_CONTAINER (run), run->text_view);
  g_object_ref (G_OBJECT (run->label));

  /* Copy data from scriptview */
  script_stream = text_buffer_stream_new (gtk_script_view_get_buffer (view));
  script_stream->open (script_stream, STREAM_READ);
  run->script_stream->open (run->script_stream, STREAM_WRITE);
  stream_line_copy (run->script_stream, script_stream);
  script_stream->destroy (script_stream);

  free (label_text);
  return GTK_WIDGET (run);
}

void gtk_script_run_start (GtkScriptRun *run)
{
  if (run->running == FALSE)
    {
      run->output_stream = string_stream_new ();
      run->output_stream->open (run->output_stream, STREAM_READ);
      run->running = TRUE;
      run->thread = g_thread_create (_thread_body, run, TRUE, NULL);
      if (run->thread != NULL)
        run->timeout_id = g_timeout_add (500, _update_view, run);
    }
}

const gchar *gtk_script_run_get_title (GtkScriptRun *run)
{
  return gtk_label_get_text (GTK_LABEL (run->label));
}

GtkWidget *gtk_script_run_get_label (GtkScriptRun *run)
{
  return run->label;
}

void gtk_script_run_stop (GtkScriptRun *run)
{
  run->output_stream->write (run->output_stream, "ABORT\n");
  run->tdata->kill_now = 1;  /* shoot out the thread */
  g_thread_join (run->thread);
}

void gtk_script_run_destroy (GtkScriptRun *run, gboolean confirm)
{
  if (confirm && run->running)
    {
      GtkWidget *dialog;
      GtkWidget *window = gtk_widget_get_toplevel (GTK_WIDGET (run));
      dialog = gtk_message_dialog_new (GTK_WINDOW (window),
                                       GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                       GTK_MESSAGE_WARNING,
                                       GTK_BUTTONS_NONE, NULL);
      gtk_message_dialog_set_markup (GTK_MESSAGE_DIALOG (dialog),
        "Are you sure you want to close a running process?");
      gtk_dialog_add_buttons (GTK_DIALOG (dialog),
                              GTK_STOCK_NO,  GTK_RESPONSE_CANCEL,
                              GTK_STOCK_YES, GTK_RESPONSE_YES,
                              NULL);

      switch (gtk_dialog_run (GTK_DIALOG (dialog)))
        {
        case GTK_RESPONSE_YES:
          break;
        case GTK_RESPONSE_CANCEL:
          gtk_widget_destroy (dialog);
          return;
        }
      gtk_widget_destroy (dialog);
    }
  gtk_script_run_stop (run);
  g_source_remove (run->timeout_id);
  run->view_stream->destroy (run->view_stream);
  run->script_stream->destroy (run->script_stream);
  run->output_stream->destroy (run->output_stream);
  g_object_unref (G_OBJECT (run->label));
  gtk_widget_destroy (run->text_view);
  gtk_widget_destroy (GTK_WIDGET (run));
}



