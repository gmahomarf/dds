#include "searchcriteriawindow.h"

static GtkWidget *window = NULL;

/* For testing propose use the local (not installed) ui file */
/* #define UI_FILE PACKAGE_DATA_DIR"/pmdn_dds/ui/searchcriteria.ui" */
#define UI_FILE "src/searchcriteria.ui"

GtkWidget*
search_criteria_window_new()
{
	/** TODO: DECLS **/

	GtkBuilder *builder;
	GError* error = NULL;

	builder = gtk_builder_new ();
	if (!gtk_builder_add_from_file (builder, UI_FILE, &error))
	{
		g_warning ("No se pudo cargar el archivo de builder: %s", error->message);
		g_error_free (error);
	}

	window = GTK_WIDGET (gtk_builder_get_object (builder, "window"));

	/* This is important */
	g_object_unref (builder);

	return window;
}
