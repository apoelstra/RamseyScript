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


#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include "global.h"
#include "file-stream.h"
#include "process.h"
#include "stream.h"
#include "string-stream.h"
#include "gtk-text-buffer-stream.h"
#include "gtk-script-view.h"
#include "gtk-script-run.h"

static void new_callback ();
static void save_as_callback ();

struct {
  GtkWidget *window, *notebook, *start_btn;
  GList *script_list;
  GList *run_list;
  GtkScriptView *active_script;
  GtkScriptRun *active_run;
} gui_data;


static void open_readme ()
{
  GtkWidget *scrolled_page = gtk_scrolled_window_new (NULL, NULL);
  GtkWidget *text_view = gtk_text_view_new ();
  GtkTextBuffer *tb;
  stream_t *tb_stream, *readme_stream;
  GtkTextIter iter;

  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_page),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (scrolled_page), text_view);

  gtk_notebook_insert_page (GTK_NOTEBOOK (gui_data.notebook),
                            scrolled_page, gtk_label_new ("README"), -1);
  gtk_text_view_set_editable (GTK_TEXT_VIEW (text_view), FALSE);
  gtk_widget_show_all (scrolled_page);

  tb = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));
  tb_stream = text_buffer_stream_new (tb);
  readme_stream = file_stream_new ("README");
  tb_stream->open (tb_stream, STREAM_WRITE);

  if (readme_stream->open (readme_stream, STREAM_READ))
    stream_line_copy (tb_stream, readme_stream);

  gtk_text_buffer_get_start_iter (tb, &iter);
  gtk_text_buffer_place_cursor (tb, &iter);

  readme_stream->destroy (readme_stream);
  tb_stream->destroy (tb_stream);
}

/* SIGNAL CALLBACKS */
static void start_callback ()
{
  if (gui_data.active_script)
    {
      GtkScriptRun *new_run = GTK_SCRIPT_RUN
                                (gtk_script_run_new (gui_data.active_script));
      gint page_index =
          gtk_notebook_append_page (GTK_NOTEBOOK (gui_data.notebook),
                                    GTK_WIDGET (new_run),
                                    gtk_script_run_get_label (new_run));

      gtk_widget_show_all (GTK_WIDGET (new_run));
      gui_data.run_list = g_list_append (gui_data.run_list, new_run);
      gtk_notebook_set_current_page (GTK_NOTEBOOK (gui_data.notebook),
                                     page_index);

      gtk_script_run_start (new_run);
    }
}

static void stop_callback ()
{
  if (gui_data.active_run)
    gtk_script_run_stop (gui_data.active_run);
}

static gboolean switch_page_callback (GtkNotebook *notebook, gpointer page,
                                      int page_index, gpointer data)
{
  GList *scan;
  (void) notebook;
  (void) page_index;
  (void) data;

  gui_data.active_run = NULL;
  gui_data.active_script = NULL;
  gtk_widget_set_sensitive (gui_data.start_btn, FALSE);
  gtk_window_set_title (GTK_WINDOW (gui_data.window), "RamseyScript " VERSION);
  /* Scan through scripts */
  for (scan = gui_data.script_list; scan; scan = scan->next)
    {
      GtkScriptView *data = scan->data;
      if (data == page)
        {
          gchar *title = g_strdup_printf ("%s - RamseyScript " VERSION,
                                          gtk_script_view_get_title (data));
          gui_data.active_script = data;
          gtk_window_set_title (GTK_WINDOW (gui_data.window), title);
          gtk_widget_set_sensitive (gui_data.start_btn, TRUE);
          g_free (title);
        }
    }
  /* Scan through runs */
  for (scan = gui_data.run_list; scan; scan = scan->next)
    {
      GtkScriptRun *data = scan->data;
      if (data == page)
        {
          gchar *title = g_strdup_printf ("%s - RamseyScript " VERSION,
                                          gtk_script_run_get_title (data));
          gui_data.active_run = data;
          gtk_window_set_title (GTK_WINDOW (gui_data.window), title);
          gtk_widget_set_sensitive (gui_data.start_btn, FALSE);
          g_free (title);
        }
    }
  return TRUE;
}
/* end SIGNAL CALLBACKS */

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
      gchar *result = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
      GtkScriptView *new_script = GTK_SCRIPT_VIEW
                                    (gtk_script_view_new_from_file (result));
      if (new_script != NULL)
        {
          gint page_index =
               gtk_notebook_append_page (GTK_NOTEBOOK (gui_data.notebook),
                                         GTK_WIDGET (new_script),
                                         gtk_script_view_get_label (new_script));

          gtk_widget_show_all (GTK_WIDGET (new_script));

          gui_data.script_list = g_list_append (gui_data.script_list, new_script);
          gtk_notebook_set_current_page (GTK_NOTEBOOK (gui_data.notebook),
                                         page_index);
        }
    }
  gtk_widget_destroy (dialog);
}

static void new_callback ()
{
  GtkScriptView *new_script = GTK_SCRIPT_VIEW (gtk_script_view_new ("New script"));
  gint page_index =
       gtk_notebook_append_page (GTK_NOTEBOOK (gui_data.notebook),
                                 GTK_WIDGET (new_script),
                                 gtk_script_view_get_label (new_script));

  gtk_widget_show_all (GTK_WIDGET (new_script));
  gui_data.script_list = g_list_append (gui_data.script_list, new_script);
  gtk_notebook_set_current_page (GTK_NOTEBOOK (gui_data.notebook),
                                 page_index);
}

static void save_callback ()
{
  if (gui_data.active_script)
    gtk_script_view_save (GTK_SCRIPT_VIEW (gui_data.active_script), FALSE);
}

static void save_as_callback ()
{
  if (gui_data.active_script)
    gtk_script_view_save (GTK_SCRIPT_VIEW (gui_data.active_script), TRUE);
}

static void close_callback ()
{
  /* Kill active script */
  if (gui_data.active_script != NULL)
    {
      gui_data.script_list = g_list_remove (gui_data.script_list,
                                            gui_data.active_script);
      gtk_script_view_destroy (gui_data.active_script, TRUE);
    }
  /* Kill active run */
  else if (gui_data.active_run != NULL)
    {
      gui_data.run_list = g_list_remove (gui_data.run_list,
                                         gui_data.active_run);
      gtk_script_run_destroy (gui_data.active_run, TRUE);
    }
}

static void quit_callback ()
{
  gboolean need_confirm = FALSE;
  GList *scan;

  for (scan = gui_data.script_list; scan; scan = scan->next)
    if (gtk_script_view_get_modified (scan->data))
      need_confirm = TRUE;

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
            gtk_script_view_save (scan->data, FALSE);
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
  { "start-action", NULL, "_Start script", "F5",
    "Run a script", G_CALLBACK (start_callback) },
  { "stop-action", NULL, "S_top script", "F6",
    "Stop a script", G_CALLBACK (stop_callback) }
};
static guint n_entries = G_N_ELEMENTS (entries);

int run_gtk_ui (int argc, char *argv[])
{
  GError *failcode;
  GtkUIManager *ui;
  GtkWidget *menubar, *vbox, *hbox;
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
  gtk_window_set_title (GTK_WINDOW (gui_data.window), "RamseyScript " VERSION);
  g_signal_connect (gui_data.window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
  /* Start maximized (bad style, I know..) */
  gtk_window_maximize (GTK_WINDOW (gui_data.window));

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
  gui_data.start_btn = gtk_button_new_with_label ("Run Script");
  gtk_box_pack_end (GTK_BOX (hbox), gui_data.start_btn, FALSE, FALSE, 2);
  g_signal_connect (gui_data.start_btn, "clicked", G_CALLBACK (start_callback), NULL);

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
  open_readme ();
  gtk_main ();
  gdk_threads_leave ();

  return 0;
}

