#ifndef GHID_SCRIPT_RUN_H
#define GHID_SCRIPT_RUN_H

#include <glib.h>
#include <glib-object.h>

#include "gtk-script-view.h"

G_BEGIN_DECLS

#define GTK_SCRIPT_RUN_TYPE            (gtk_script_run_get_type ())
#define GTK_SCRIPT_RUN(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_SCRIPT_RUN_TYPE, GtkScriptRun))
#define GTK_SCRIPT_RUN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_SCRIPT_RUN_TYPE, GtkScriptRunClass))
#define IS_GTK_SCRIPT_RUN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_SCRIPT_RUN_TYPE))
#define IS_GTK_SCRIPT_RUN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_SCRIPT_RUN_TYPE))

typedef struct _GtkScriptRun       GtkScriptRun;
typedef struct _GtkScriptRunClass  GtkScriptRunClass;

GType gtk_script_run_get_type (void);
GtkWidget* gtk_script_run_new (GtkScriptView *view);

void gtk_script_run_start (GtkScriptRun *run);
void gtk_script_run_stop (GtkScriptRun *run);
GtkWidget *gtk_script_run_get_label (GtkScriptRun *run);
const gchar *gtk_script_run_get_title (GtkScriptRun *run);
void gtk_script_run_destroy (GtkScriptRun *run, gboolean confirm);

G_END_DECLS 
#endif
