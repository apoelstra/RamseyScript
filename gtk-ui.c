
#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include "global.h"
#include "ramsey.h"
#include "stream.h"
#include "string-stream.h"
#include "gtk-text-buffer-stream.h"

static void new_callback ();
static void save_as_callback ();

struct _script
{
  const char *tab_title;
  char *filename;
  GtkWidget *page, *text_view;
  Stream *stream;
  gint page_index;
  gboolean modified;
  gboolean modify_holdoff;
};

struct _run
{
  const char *tab_title;
  char *filename;
  GtkWidget *page, *text_view;
  Stream *text_view_stream;
  Stream *input_stream;
  Stream *output_stream;
  gboolean modified;
  gboolean running;
};

struct {
  GtkWidget *window, *notebook;
  GList *script_list;
  GList *run_list;
  struct _script *active_script;
  struct _run *active_run;
} gui_data;

static void set_active_script_modified (gboolean modified)
{
  if (gui_data.active_script)
    {
      gui_data.active_script->modified = modified;
      gchar *title = g_strdup_printf ("%s%c",
                                      gui_data.active_script->tab_title,
                                      modified ? '*' : ' ');
      gtk_notebook_set_tab_label_text (GTK_NOTEBOOK (gui_data.notebook),
                                       gui_data.active_script->page,
                                       title);
      free (title);
      title = g_strdup_printf ("%s - RamseyScript",
        gtk_notebook_get_tab_label_text (GTK_NOTEBOOK (gui_data.notebook),
                                         gui_data.active_script->page));
      gtk_window_set_title (GTK_WINDOW (gui_data.window), title);
      free (title);
    }
}

static void destroy_page (struct _script *data)
{
  gui_data.script_list = g_list_remove (gui_data.script_list, data);
  free (data->filename);
  data->stream->destroy (data->stream);
  free (data);
}

static void *process_thread (void *r)
{
  struct _run *run = r;
  struct _global_data *defs = set_defaults ();

  /* Open streams */
  defs->out_stream = run->output_stream;
  defs->in_stream = run->input_stream;
  run->input_stream->open (run->input_stream, "r");
  run->output_stream->open (run->output_stream, "w");
  /* Go! */
  process (defs);
  /* Close streams */
  run->input_stream->destroy (run->input_stream);
  run->output_stream->destroy (run->output_stream);
  /* Cleanup */
  free (defs);
  return NULL;
}

/* SIGNAL CALLBACKS */
static void start_callback ()
{
  if (gui_data.active_script)
    {
      Stream *input_stream = string_stream_new ();
      Stream *output_stream = string_stream_new ();
      struct _run *new_run = malloc (sizeof *new_run);
      struct _script *script = gui_data.active_script;
      GtkTextBuffer *tb;
      gint page_index;

      /* Create run */
      new_run->modified = FALSE;
      new_run->filename = script->filename;
      new_run->input_stream = input_stream;
      new_run->output_stream = output_stream;

      /* Create gui */
      new_run->page = gtk_scrolled_window_new (NULL, NULL);
      gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (new_run->page),
                                      GTK_POLICY_AUTOMATIC,
                                      GTK_POLICY_AUTOMATIC);
      new_run->text_view = gtk_text_view_new ();
      tb = gtk_text_view_get_buffer (GTK_TEXT_VIEW (new_run->text_view));
      new_run->text_view_stream = text_buffer_stream_new (tb);
      gtk_container_add (GTK_CONTAINER (new_run->page),
                         new_run->text_view);
      page_index =
          gtk_notebook_insert_page (GTK_NOTEBOOK (gui_data.notebook),
                                    new_run->page,
                                    gtk_label_new (script->tab_title),
                                    gtk_notebook_get_n_pages (GTK_NOTEBOOK (gui_data.notebook)) - 1);

      gtk_text_view_set_editable (GTK_TEXT_VIEW (new_run->text_view), FALSE);
      gtk_widget_show_all (new_run->page);

      /* Install run */
      gui_data.run_list = g_list_append (gui_data.run_list, new_run);
      gui_data.active_run = new_run;
      gtk_notebook_set_current_page (GTK_NOTEBOOK (gui_data.notebook),
                                     page_index);

      /* Copy script text-buffer into input stream */
      script->stream->open (script->stream, "r");
      input_stream->open (input_stream, "w");
      stream_line_copy (input_stream, script->stream);

      /* Run */
      g_thread_create (process_thread, new_run, FALSE, NULL);
    }
}

static gboolean switch_page_callback (GtkNotebook *notebook, gpointer page,
                                      int page_index, gpointer data)
{
  GList *scan;
  (void) notebook;
  (void) page_index;
  (void) data;

  gui_data.active_script = NULL;
  gtk_window_set_title (GTK_WINDOW (gui_data.window), "RamseyScript");
  for (scan = gui_data.script_list; scan; scan = scan->next)
    {
      struct _script *data = scan->data;
      if (data->page == page)
        {
          gchar *title = g_strdup_printf ("%s - RamseyScript",
            gtk_notebook_get_tab_label_text (GTK_NOTEBOOK (gui_data.notebook),
                                             data->page));
          gui_data.active_script = data;
          gtk_window_set_title (GTK_WINDOW (gui_data.window), title);
          free (title);
        }
    }
  return TRUE;
}

static void page_edit_callback (GtkTextBuffer *buf, gpointer data)
{
  (void) buf;
  (void) data;
  if (gui_data.active_script->modify_holdoff == FALSE)
    set_active_script_modified (TRUE);
}
/* end SIGNAL CALLBACKS */

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
  GtkFileFilter *scripts_only, *all_files;

  dialog = gtk_file_chooser_dialog_new ("Open script",
                                        GTK_WINDOW (gui_data.window),
                                        GTK_FILE_CHOOSER_ACTION_OPEN,
                                        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                        GTK_STOCK_OK, GTK_RESPONSE_OK,
                                        NULL);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

  all_files = gtk_file_filter_new ();
  scripts_only = gtk_file_filter_new ();
  gtk_file_filter_set_name (scripts_only, "Ramsey Scripts");
  gtk_file_filter_add_pattern (scripts_only, "*.ramsey");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), scripts_only);
  gtk_file_filter_set_name (all_files, "All Files");
  gtk_file_filter_add_pattern (all_files, "*.*");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), all_files);

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK)
    {
      Stream *file_reader = file_stream_new ("r");
      gchar *result = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
      if (file_reader->open (file_reader, result))
        {
          new_callback ();
          gui_data.active_script->filename = result;
          gui_data.active_script->tab_title = _find_last_slash(result);
          gtk_notebook_set_tab_label_text (GTK_NOTEBOOK (gui_data.notebook),
                                            gui_data.active_script->page,
                                            gui_data.active_script->tab_title);

          gui_data.active_script->stream->open (gui_data.active_script->stream, "w");

          gui_data.active_script->modify_holdoff = TRUE;
          stream_line_copy (gui_data.active_script->stream, file_reader);
          gui_data.active_script->modify_holdoff = FALSE;
        }
      else
        fprintf (stderr, "Failed to open file ``%s''\n", result);
      file_reader->destroy (file_reader);
    }
  gtk_widget_destroy (dialog);
}

static void new_callback ()
{
  gint page_index;
  GtkTextBuffer *tb;
  struct _script *new_script = malloc (sizeof *new_script);

  new_script->modified = FALSE;
  new_script->modify_holdoff = FALSE;
  new_script->filename = NULL;
  new_script->tab_title = "New script";
  new_script->page = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (new_script->page),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);
  new_script->text_view = gtk_text_view_new ();
  tb = gtk_text_view_get_buffer (GTK_TEXT_VIEW (new_script->text_view));
  new_script->stream = text_buffer_stream_new (tb);
  gtk_container_add (GTK_CONTAINER (new_script->page),
                     new_script->text_view);
  page_index =
      gtk_notebook_insert_page (GTK_NOTEBOOK (gui_data.notebook),
                                new_script->page,
                                gtk_label_new ("New script"),
                                gtk_notebook_get_n_pages (GTK_NOTEBOOK (gui_data.notebook)) - 1);

  gtk_text_view_set_editable (GTK_TEXT_VIEW (new_script->text_view), TRUE);
  gtk_widget_show_all (new_script->page);

  gui_data.script_list = g_list_append (gui_data.script_list, new_script);
  gui_data.active_script = new_script;
  gtk_notebook_set_current_page (GTK_NOTEBOOK (gui_data.notebook),
                                 page_index);

  g_signal_connect (tb, "changed", G_CALLBACK (page_edit_callback), new_script);
}

static void save_callback ()
{
  if (gui_data.active_script)
    {
      if (gui_data.active_script->filename == NULL)
        save_as_callback ();
      else
        {
          Stream *file_writer = file_stream_new ("w");

          if (file_writer->open (file_writer, gui_data.active_script->filename))
            {
              gui_data.active_script->stream->open (gui_data.active_script->stream, "r");
              stream_line_copy (file_writer, gui_data.active_script->stream);
              file_writer->destroy (file_writer);
              set_active_script_modified (FALSE);
            }
          else
            fprintf (stderr, "Failed to save file ``%s''\n", gui_data.active_script->filename);
        }
    }
}

static void save_as_callback ()
{
  if (gui_data.active_script)
    {
      GtkWidget *dialog;
      gchar *title = g_strdup_printf ("Save %s as...",
          gtk_notebook_get_tab_label_text (GTK_NOTEBOOK (gui_data.notebook),
                                           gui_data.active_script->page));
      dialog = gtk_file_chooser_dialog_new (title,
                                            GTK_WINDOW (gui_data.window),
                                            GTK_FILE_CHOOSER_ACTION_SAVE,
                                            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                            GTK_STOCK_OK, GTK_RESPONSE_OK,
                                            NULL);
      gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog),
                                                    TRUE);
      gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

      if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK)
        {
          Stream *file_writer = file_stream_new ("w");
          gchar *result = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
          if (file_writer->open (file_writer, result))
            {
              gui_data.active_script->filename = result;
              gui_data.active_script->stream->open (gui_data.active_script->stream, "r");
              stream_line_copy (file_writer, gui_data.active_script->stream);
              set_active_script_modified (FALSE);
            }
          else
            fprintf (stderr, "Failed to save file ``%s''\n", result);
          file_writer->destroy (file_writer);
        }

      gtk_widget_destroy (dialog);
      g_free (title);
    }
}

static void close_callback ()
{
  if (gui_data.active_script != NULL)
    {
      struct _script *tmp;
      if (gui_data.active_script->modified)
        {
          GtkWidget *dialog;
          dialog = gtk_message_dialog_new (GTK_WINDOW (gui_data.window),
                                           GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                           GTK_MESSAGE_WARNING,
                                           GTK_BUTTONS_NONE, NULL);
          gtk_message_dialog_set_markup (GTK_MESSAGE_DIALOG (dialog),
            "There are unsaved changes. Are you sure you want to quit?");
          gtk_dialog_add_buttons (GTK_DIALOG (dialog),
                                  "Close _without saving", GTK_RESPONSE_NO,
                                  GTK_STOCK_CANCEL,        GTK_RESPONSE_CANCEL,
                                  GTK_STOCK_SAVE,          GTK_RESPONSE_YES,
                                  NULL);

          switch (gtk_dialog_run (GTK_DIALOG (dialog)))
            {
            case GTK_RESPONSE_YES:
              save_callback ();
              break;
            case GTK_RESPONSE_NO:
              break;
            case GTK_RESPONSE_CANCEL:
              gtk_widget_destroy (dialog);
              return;
            }
          gtk_widget_destroy (dialog);
        }
      tmp = gui_data.active_script;
      gtk_container_remove (GTK_CONTAINER (gui_data.notebook),
                            tmp->page);
      destroy_page (tmp);
      gui_data.active_script = NULL;
    }
}

static void quit_callback ()
{
  gboolean need_confirm = FALSE;
  GList *scan;

  for (scan = gui_data.script_list; scan; scan = scan->next)
    {
      struct _script *data = scan->data;
      if (data->modified)
        need_confirm = TRUE;
    }

  if (need_confirm)
    {
      GtkWidget *dialog;
      dialog = gtk_message_dialog_new (GTK_WINDOW (gui_data.window),
                                       GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                       GTK_MESSAGE_WARNING,
                                       GTK_BUTTONS_NONE, NULL);
      gtk_message_dialog_set_markup (GTK_MESSAGE_DIALOG (dialog),
        "There are unsaved changes. Are you sure you want to quit?");
      gtk_dialog_add_buttons (GTK_DIALOG (dialog),
                              "Close _without saving", GTK_RESPONSE_NO,
                              GTK_STOCK_CANCEL,        GTK_RESPONSE_CANCEL,
                              GTK_STOCK_SAVE,          GTK_RESPONSE_YES,
                              NULL);
      switch (gtk_dialog_run (GTK_DIALOG (dialog)))
        {
        case GTK_RESPONSE_YES:
          for (scan = gui_data.script_list; scan; scan = scan->next)
            {
              gui_data.active_script = scan->data;
              save_callback ();
            }
          break;
        case GTK_RESPONSE_NO:
          break;
        case GTK_RESPONSE_CANCEL:
          gtk_widget_destroy (dialog);
          return;
        }
      gtk_widget_destroy (dialog);
    }

  gtk_main_quit ();
}
/* end MENU CALLBACKS */

static GtkActionEntry entries[] = 
{
  { "file-menu-action", NULL, "_File", NULL, NULL, NULL },
  { "new-action", GTK_STOCK_NEW, "_New", "<control>N",
    "Create a new script", G_CALLBACK (new_callback) },
  { "open-action", GTK_STOCK_OPEN, "_Open", "<control>O",    
    "Open a script", G_CALLBACK (open_callback) },
  { "save-action", GTK_STOCK_SAVE, "_Save", "<control>S",
     "Save the script", G_CALLBACK (save_callback) },
  { "save-as-action", NULL, "Save _As...", "<control><shift>S",
     "Save the script...", G_CALLBACK (save_as_callback) },
  { "close-action", GTK_STOCK_CLOSE, "_Close", "<control>W",
    "Close the active script", G_CALLBACK (close_callback) },
  { "quit-action", GTK_STOCK_QUIT, "_Quit", "<control>Q",    
    "Quit", G_CALLBACK (quit_callback) },

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
  GtkWidget *start_btn;
  GtkActionGroup *action_group;

  gui_data.script_list = NULL;
  gui_data.run_list = NULL;
  gui_data.active_script = NULL;
  gui_data.active_run = NULL;
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
  gtk_rc_parse_string ("style \"mono\" { font_name = \"Monospace\" }\n"
                       "widget \"*.GtkTextView\" style \"mono\"");
  new_callback ();
  gtk_main ();
  gdk_threads_leave ();

  return 0;
}

