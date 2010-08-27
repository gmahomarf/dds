#include "searchcriteriawindow.h"

static GtkWidget *window = NULL;

void
_toggle(GtkWidget *wid, gpointer data)
{
	gtk_widget_set_visible(GTK_WIDGET(data),
	                       !gtk_widget_get_visible (GTK_WIDGET(data)));
}

GtkWidget*
search_criteria_window_new()
{
	/** TODO: DECLS **/

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "Criterios de Búsqueda");
	gtk_window_set_default_size(GTK_WINDOW(window), 640, 480);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_modal(GTK_WINDOW(window), TRUE);

	

	return window;
}