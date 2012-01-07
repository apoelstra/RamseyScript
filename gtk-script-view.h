#ifndef GHID_SCRIPT_VIEW_H
#define GHID_SCRIPT_VIEW_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define GTK_SCRIPT_VIEW_TYPE            (gtk_script_view_get_type ())
#define GTK_SCRIPT_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_SCRIPT_VIEW_TYPE, GtkScriptView))
#define GTK_SCRIPT_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_SCRIPT_VIEW_TYPE, GtkScriptViewClass))
#define IS_GTK_SCRIPT_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_SCRIPT_VIEW_TYPE))
#define IS_GTK_SCRIPT_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_SCRIPT_VIEW_TYPE))

typedef struct _GtkScriptView       GtkScriptView;
typedef struct _GtkScriptViewClass  GtkScriptViewClass;

GType gtk_script_view_get_type (void);
GtkWidget* gtk_script_view_new (const gchar *label);
GtkWidget *gtk_script_view_new_from_file (const gchar *filename);

GtkWidget *gtk_script_view_get_label (GtkScriptView *view);
const gchar *gtk_script_view_get_filename (GtkScriptView *view);
void gtk_script_view_set_filename (GtkScriptView *view, const gchar *filename);
const gchar *gtk_script_view_get_title (GtkScriptView *view);
gboolean gtk_script_view_get_modified (GtkScriptView *view);
gboolean gtk_script_view_save (GtkScriptView *view, gboolean force_save_as);
GtkTextBuffer *gtk_script_view_get_buffer (GtkScriptView *view);
void gtk_script_view_destroy (GtkScriptView *view, gboolean confirm);

G_END_DECLS 
#endif
