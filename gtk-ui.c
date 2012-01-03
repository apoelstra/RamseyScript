
#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include "global.h"
#include "ramsey.h"
#include "stream.h"
#include "string-stream.h"
#include "gtk-text-buffer-stream.h"

struct _script
{
  char *filename;
  GtkWidget *page, *text_view;
  Stream *stream;
  gint page_index;
  gboolean modified;
  gboolean is_running;
};

struct {
  GtkWidget *window, *notebook, *output_view;
  GList *script_list;
  struct _script *active_script;
} gui_data;

static void *process_thread (void *is)
{
  Stream *input_stream = is;
  struct _global_data *defs = set_defaults ();
  Stream *output_stream = file_stream_new ("w");
  output_stream->_data = stdout;
  input_stream->open (input_stream, "r");

  defs->out_stream = output_stream;
  defs->in_stream = input_stream;
  process (defs);
  string_stream_delete (input_stream);
  file_stream_delete (output_stream);
  free (defs);
  return NULL;
}

/* SIGNAL CALLBACKS */
static void start_callback ()
{
  GtkTextBuffer *tb = gtk_text_buffer_new (NULL);
  gtk_text_view_set_buffer (GTK_TEXT_VIEW (gui_data.output_view),
                            tb);

  /* run script */
  if (gui_data.active_script)
    {
      Stream *input_stream = string_stream_new ();

      /* Copy text-buffer into thread-specific buffer */
      gui_data.active_script->stream->open (gui_data.active_script->stream, "r");
      input_stream->open (input_stream, "w");
      stream_line_copy (input_stream, gui_data.active_script->stream);
      /* Run */
      g_thread_create (process_thread, input_stream, FALSE, NULL);
    }

  g_object_unref (G_OBJECT (tb));
}

static gboolean switch_page_callback (GtkNotebook *notebook, gpointer page,
                                      int page_index, gpointer data)
{
  GList *scan;
  (void) notebook;
  (void) page;
  (void) data;

  gui_data.active_script = NULL;
  for (scan = gui_data.script_list; scan; scan = scan->next)
    {
      struct _script *data = scan->data;
      if (data->page_index == page_index)
        gui_data.active_script = data;
    }
  return TRUE;
}
/* end SIGNAL CALLBACKS */

static int check_save ()
{
  return 1;
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

/* MENU CALLBACKS */
static void open_callback ()
{
  GtkWidget *dialog;
  GtkFileFilter *scripts_only;

  dialog = gtk_file_chooser_dialog_new ("Open Ramsey Script",
                                        GTK_WINDOW (gui_data.window),
                                        GTK_FILE_CHOOSER_ACTION_OPEN,
                                        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                        GTK_STOCK_OK, GTK_RESPONSE_OK,
                                        NULL);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

  scripts_only = gtk_file_filter_new ();
  gtk_file_filter_set_name (scripts_only, "Ramsey Scripts");
  gtk_file_filter_add_pattern (scripts_only, "*.ramsey");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), scripts_only);

  if (check_save() && gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK)
    {
      Stream *file_reader = file_stream_new ("r");
      gchar *result = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
      if (file_reader->open (file_reader, result))
        {
          struct _script *new_script = malloc (sizeof *new_script);

          new_script->is_running = FALSE;
          new_script->modified = FALSE;
          new_script->filename = result;
          new_script->page = gtk_scrolled_window_new (NULL, NULL);
          gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (new_script->page),
                                          GTK_POLICY_AUTOMATIC,
                                          GTK_POLICY_AUTOMATIC);
          new_script->text_view = gtk_text_view_new ();
          new_script->stream = text_buffer_stream_new (gtk_text_view_get_buffer
                                               (GTK_TEXT_VIEW (new_script->text_view)));
          gtk_container_add (GTK_CONTAINER (new_script->page),
                             new_script->text_view);
          new_script->page_index =
              gtk_notebook_insert_page (GTK_NOTEBOOK (gui_data.notebook),
                                        new_script->page,
                                        gtk_label_new (_find_last_slash(result)),
                                        gtk_notebook_get_n_pages (GTK_NOTEBOOK (gui_data.notebook)) - 1);

          gtk_text_view_set_editable (GTK_TEXT_VIEW (new_script->text_view), TRUE);
          gtk_widget_show_all (new_script->page);

          gui_data.script_list = g_list_append (gui_data.script_list, new_script);
          gui_data.active_script = new_script;
          gtk_notebook_set_current_page (GTK_NOTEBOOK (gui_data.notebook),
                                         new_script->page_index);

          new_script->stream->open (new_script->stream, "w");
          stream_line_copy (new_script->stream, file_reader);
        }
      else
        fprintf (stderr, "Failed to open file ``%s''\n", result);
      file_stream_delete (file_reader);
    }
  gtk_widget_destroy (dialog);
}

static void save_callback ()
{
  if (gui_data.active_script)
    {
      Stream *file_writer = file_stream_new ("w");

      if (file_writer->open (file_writer, gui_data.active_script->filename))
        {
          gui_data.active_script->stream->open (gui_data.active_script->stream, "r");
          stream_line_copy (file_writer, gui_data.active_script->stream);
          file_stream_delete (file_writer);
        }
      else
        fprintf (stderr, "Failed to open file ``%s''\n", gui_data.active_script->filename);
    }
}
/* end MENU CALLBACKS */

static GtkActionEntry entries[] = 
{
  { "file-menu-action", NULL, "_File", NULL, NULL, NULL },
  { "new-action", NULL, "_New", NULL, NULL, NULL },
  { "open-action", GTK_STOCK_OPEN, "_Open", "<control>O",    
    "Open a script", G_CALLBACK (open_callback) },
  { "save-action", GTK_STOCK_SAVE, "_Save", "<control>S",
     "Save the script", G_CALLBACK (save_callback) },
  { "save-as-action", NULL, "Save _As...", NULL, NULL, NULL },
  { "close-action", GTK_STOCK_CLOSE, "_Close", NULL, NULL, NULL },
  { "quit-action", GTK_STOCK_QUIT, "_Quit", "<control>Q",    
    "Quit", G_CALLBACK (gtk_main_quit) },

  { "run-menu-action", NULL, "_Run", NULL, NULL, NULL },
  { "start-action", NULL, "_Start script", NULL, NULL, NULL },
  { "stop-action", NULL, "S_top script", NULL, NULL, NULL }
};
static guint n_entries = G_N_ELEMENTS (entries);

int run_gtk_ui (int argc, char *argv[])
{
  GError *failcode;
  GtkUIManager *ui;
  GtkWidget *menubar, *vbox, *hbox;
  GtkWidget *output_scroll;
  GtkWidget *start_btn;
  GtkActionGroup *action_group;

  gui_data.script_list = NULL;
  gui_data.active_script = NULL;
  g_thread_init (NULL);
  gdk_threads_init ();
  gtk_init (&argc, &argv);

  /* Build window and UI */
  gui_data.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (gui_data.window), "RamseyScript");
  g_signal_connect (gui_data.window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

  ui = gtk_ui_manager_new ();
  gtk_ui_manager_set_add_tearoffs (ui, TRUE);

  action_group = gtk_action_group_new ("MainActions");
  gtk_action_group_add_actions (action_group, entries, n_entries, gui_data.window);
  gtk_ui_manager_insert_action_group (ui, action_group, 0);

  if (!gtk_ui_manager_add_ui_from_file (ui, "interface.xml", &failcode)
        && failcode != NULL)
    {
      fprintf (stderr, "Failed to build menu: %s\n", failcode->message);
      g_error_free (failcode);
      return EXIT_FAILURE;
    }

  vbox = gtk_vbox_new (FALSE, 2);
  gtk_container_add (GTK_CONTAINER (gui_data.window), vbox);

  menubar = gtk_ui_manager_get_widget (ui, "/main-menu");

  /* Build notebook */
  gui_data.notebook = gtk_notebook_new ();
  g_signal_connect (gui_data.notebook, "switch_page",
                    G_CALLBACK (switch_page_callback), NULL);

  output_scroll = gtk_scrolled_window_new (NULL, NULL);
  gui_data.output_view = gtk_text_view_new ();

  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (output_scroll),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);
  gtk_text_view_set_editable (GTK_TEXT_VIEW (gui_data.output_view), FALSE);

  gtk_notebook_append_page (GTK_NOTEBOOK (gui_data.notebook), output_scroll,
                            gtk_label_new ("Output"));

  gtk_container_add (GTK_CONTAINER (output_scroll),
                     gui_data.output_view);

  /* Build start/stop buttons */
  hbox = gtk_hbox_new (FALSE, 0);
  start_btn = gtk_button_new_with_label ("Run Script");
  gtk_box_pack_end (GTK_BOX (hbox), start_btn, FALSE, FALSE, 2);
  g_signal_connect (start_btn, "clicked", G_CALLBACK (start_callback), NULL);

  /* Put it all together */
  gtk_box_pack_start (GTK_BOX (vbox), menubar, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), gui_data.notebook, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 2);

  gtk_window_add_accel_group (GTK_WINDOW (gui_data.window),
                              gtk_ui_manager_get_accel_group (ui));
  gtk_widget_show_all (gui_data.window);
  gdk_threads_enter ();
  gtk_main ();
  gdk_threads_leave ();

  return 0;
}

