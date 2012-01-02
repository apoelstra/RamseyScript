
#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include "global.h"
#include "ramsey.h"

struct {
  GtkWidget *window, *script_view, *output_view;
} gui_data;

static void start_callback ()
{
  GtkTextBuffer *tb = gtk_text_buffer_new (NULL);

  gtk_text_view_set_buffer (GTK_TEXT_VIEW (gui_data.output_view),
                            tb);
  g_object_unref (G_OBJECT (tb));
}

static int check_save ()
{
  return 1;
}

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
      gchar *result = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
      FILE *fh = fopen (result, "r");
      if (fh != NULL)
        {
          char buf[1000];
          GtkTextBuffer *tb = gtk_text_buffer_new (NULL);
          while (fgets (buf, sizeof buf, fh))
            {
              gtk_text_buffer_insert_at_cursor (tb, buf, -1);
            }
          gtk_text_view_set_buffer (GTK_TEXT_VIEW (gui_data.script_view),
                                    tb);
          g_object_unref (G_OBJECT (tb));
          fclose (fh);
        }
    }
  gtk_widget_destroy (dialog);
}

static GtkActionEntry entries[] = 
{
  { "file-menu-action", NULL, "_File", NULL, NULL, NULL },
  { "open-action", GTK_STOCK_OPEN, "_Open", "<control>O",    
    "Open a script", G_CALLBACK (open_callback) },
  { "quit-action", GTK_STOCK_QUIT, "_Quit", "<control>Q",    
    "Quit", G_CALLBACK (gtk_main_quit) }
};
static guint n_entries = G_N_ELEMENTS (entries);

int run_gtk_ui (int argc, char *argv[])
{
  GError *failcode;
  GtkUIManager *ui;
  GtkWidget *menubar, *vbox, *notebook, *hbox;
  GtkWidget *script_scroll, *output_scroll;
  GtkWidget *start_btn;
  GtkActionGroup *action_group;

  gtk_init (&argc, &argv);

  /* Build window and UI */
  gui_data.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
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
  notebook = gtk_notebook_new ();
  script_scroll = gtk_scrolled_window_new (NULL, NULL);
  output_scroll = gtk_scrolled_window_new (NULL, NULL);
  gui_data.script_view = gtk_text_view_new ();
  gui_data.output_view = gtk_text_view_new ();

  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (script_scroll),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (output_scroll),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);
  gtk_text_view_set_editable (GTK_TEXT_VIEW (gui_data.script_view), TRUE);
  gtk_text_view_set_editable (GTK_TEXT_VIEW (gui_data.output_view), FALSE);

  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), script_scroll,
                            gtk_label_new ("Script"));
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), output_scroll,
                            gtk_label_new ("Output"));

  gtk_container_add (GTK_CONTAINER (script_scroll),
                     gui_data.script_view);
  gtk_container_add (GTK_CONTAINER (output_scroll),
                     gui_data.output_view);

  /* Build start/stop buttons */
  hbox = gtk_hbox_new (FALSE, 0);
  start_btn = gtk_button_new_with_label ("Run Script");
  gtk_box_pack_end (GTK_BOX (hbox), start_btn, FALSE, FALSE, 2);
  g_signal_connect (start_btn, "clicked", G_CALLBACK (start_callback), NULL);

  /* Put it all together */
  gtk_box_pack_start (GTK_BOX (vbox), menubar, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), notebook, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 2);

  gtk_window_add_accel_group (GTK_WINDOW (gui_data.window),
                              gtk_ui_manager_get_accel_group (ui));
  gtk_widget_show_all (gui_data.window);
  gtk_main ();

  return 0;
}

