#include "netlistwin.h"
#include <string.h>

enum cols
{
	COL_IDSAT,
	COL_NOMBRE,
	COL_DESC,
	NUM_COLS
};

enum toolbtns
{
	T_NUEVO,
	T_ABRIR,
	T_GUARDAR,
	T_SEP,
	T_AGREGAR,
	T_QUITAR,
	T_COUNT
};

enum menus
{
	M_ARCHIVO,
	M_COUNT
};

enum submenusArchivo
{
	SMA_NUEVO,
	SMA_ABRIR,
	SMA_GUARDAR,
	SMA_GUARDAR_COMO,
	SMA_CERRAR,
	SMA_SEP,
	SMA_SALIR,
	SMA_COUNT
};

static GtkListStore *_netList = NULL;
static GtkWidget *_tv = NULL;
static GtkWidget *window = NULL;
static GtkWidget *barra = NULL;
static GtkWidget *menuBar = NULL;
static GString *file = NULL;
static gboolean saved = TRUE;

/** Other Functions **/

void
_warn(GError *e) {
	GtkWidget *msgDlg = NULL;
	GString *eMsg;

	eMsg = g_string_new ("Error:\n");
	g_string_append(eMsg, e->message);
	msgDlg = gtk_message_dialog_new(GTK_WINDOW(window),
                             		GTK_DIALOG_MODAL,
	                                GTK_MESSAGE_ERROR,
	                                GTK_BUTTONS_OK,
	                                "%s",
	                            	eMsg->str);
	gtk_dialog_run(GTK_DIALOG(msgDlg));
	gtk_widget_destroy(msgDlg);
	g_string_free(eMsg, TRUE);
	g_error_free(e);
}

gint
_confirm(){
	GtkWidget *dlg;
	GtkVBox *cont;
	GtkHBox *hbox = NULL;
	GtkWidget *lbl;
	gint ret;

	dlg = gtk_dialog_new_with_buttons("Aviso",
	                                  GTK_WINDOW(window),
	                                  GTK_DIALOG_MODAL | 
	                                  GTK_DIALOG_DESTROY_WITH_PARENT,
	                                  "Sí", GTK_RESPONSE_YES,
	                                  "No", GTK_RESPONSE_NO,
	                                  "Cancelar", GTK_RESPONSE_CANCEL,
	                                  NULL);
    hbox = GTK_HBOX(gtk_hbox_new (FALSE, 0));
    cont = GTK_VBOX(gtk_dialog_get_content_area (GTK_DIALOG(dlg)));
	gtk_box_pack_start(GTK_BOX(cont),
	                   GTK_WIDGET(hbox),
	                   FALSE,
	                   TRUE,
	                   0);
	lbl = gtk_label_new("El archivo no se ha guardado.\n"
						"¿Desea guardar antes de continuar?");
	gtk_box_pack_start(GTK_BOX(hbox),
	                   gtk_image_new_from_stock (GTK_STOCK_DIALOG_QUESTION,
	                                             GTK_ICON_SIZE_DIALOG),
	                   FALSE,
	                   TRUE,
	                   0);
	gtk_box_pack_start(GTK_BOX(hbox),
	                   lbl,
	                   FALSE,
	                   TRUE,
	                   0);
	gtk_widget_show_all(GTK_WIDGET(hbox));
	g_signal_connect(dlg,
	                 "close",
	                 G_CALLBACK(gtk_widget_destroy),
	                 NULL);

	ret = gtk_dialog_run(GTK_DIALOG(dlg));
	gtk_widget_destroy(dlg);
	return ret;
}

gboolean
_save()
{
	GtkWidget *dlg = NULL;
	GFile *nl = NULL;
	GFileOutputStream *strm = NULL;
	GList *l = NULL;
	GtkTreeIter iter;
	gchar *satid = NULL, *nom = NULL, *desc = NULL;
	GString *out = NULL;
	GError *err = NULL;

	if (saved) return TRUE;

	if (!file) {
		dlg = gtk_file_chooser_dialog_new("Guardar Lista de Red",
			                              GTK_WINDOW(window),
			                              GTK_FILE_CHOOSER_ACTION_SAVE,
			                              "Guardar",
			                              GTK_RESPONSE_ACCEPT,
			                              "Cancelar",
			                              GTK_RESPONSE_CANCEL,
			                              NULL);
		gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dlg),
		                                                TRUE);
		/*GtkFileFilter *filtro;

		filtro = gtk_file_filter_new();
		gtk_file_filter_set_name (filtro, "Listas de Red");
		gtk_file_filter_add_pattern(filtro, "*.nl");
		gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(dlg),filtro);
		g_object_unref(filtro);*/
		g_signal_connect(dlg, "close",
		                 G_CALLBACK(gtk_widget_destroy), NULL);
		if(gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_ACCEPT) {
			g_string_free(file, TRUE);
			file = g_string_sized_new (255);
			g_string_assign (file, 
			                 gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg)));
			gtk_widget_destroy (GTK_WIDGET(dlg));
		} else {
			gtk_widget_destroy (GTK_WIDGET(dlg));
			return FALSE;
		}
	} 
	nl = g_file_new_for_path (file->str);

	strm = g_file_replace (nl, NULL, FALSE,
	                       G_FILE_CREATE_REPLACE_DESTINATION, NULL, &err);
	/* TODO: Handle File Save Errors */
	if (!strm) {
		_warn(err);
		return FALSE;
	}

	gtk_tree_model_get_iter_first (GTK_TREE_MODEL(_netList), &iter);
	do {
		if (out) g_string_free(out, TRUE);
		out = g_string_sized_new (255);
		while(TRUE) {
			if (!gtk_list_store_iter_is_valid (_netList, &iter)) {
				break;
			}
			gtk_tree_model_get(GTK_TREE_MODEL(_netList),
				               &iter,
				               COL_IDSAT,
				               &satid,
				               COL_NOMBRE,
				               &nom,
				               COL_DESC,
				               &desc,
				               -1);
			if (!satid) {
				gtk_list_store_remove(_netList, &iter);
			} else {
				break;
			}
		}
		if (!gtk_list_store_iter_is_valid (_netList, &iter)) {
			break;
		}
			
		g_string_printf(out, "%s:%s%s%s%c",
		                satid,
		                nom ? nom : "",
		                (desc && strlen(desc)) ? " " : "",
		                desc ? desc : "",
		                0x0A);
		if (nom) g_free ((gpointer)nom);
		if (desc) g_free ((gpointer)desc);
		if (satid) g_free ((gpointer)satid);		
		g_output_stream_write(G_OUTPUT_STREAM(strm),
		                      out->str,
		                      out->len,
		                      NULL,
		                      &err);
	} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(_netList), &iter));
	g_list_free(l);
	g_output_stream_close (G_OUTPUT_STREAM(strm), NULL, NULL);
	g_object_unref((gpointer)strm);	

	GtkWidget *menuArc = NULL, *submenuArc = NULL;
	GList *ls = NULL;

	ls = gtk_container_get_children (GTK_CONTAINER(menuBar));
	menuArc = GTK_WIDGET(g_list_nth (ls, M_ARCHIVO)->data);
	g_list_free(ls);
	submenuArc = gtk_menu_item_get_submenu (GTK_MENU_ITEM(menuArc));
	ls = gtk_container_get_children (GTK_CONTAINER(submenuArc));
	gtk_widget_set_sensitive(GTK_WIDGET(g_list_nth(ls, SMA_GUARDAR)->data), FALSE);
	g_list_free(ls);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_toolbar_get_nth_item(GTK_TOOLBAR(barra),
	                                              T_GUARDAR)),
	                     FALSE);
	saved = TRUE;
	return TRUE;
}

gboolean
_close()
{
	if (!saved){
		/* Confirm opening new file */
		switch(_confirm()){
			case GTK_RESPONSE_YES:
				if(!_save()) return FALSE;
				break;
			case GTK_RESPONSE_CANCEL:
				return FALSE;
			case GTK_RESPONSE_NO:
				break;
			default:
				break;
		}
	}

	gtk_list_store_clear (_netList);
	if (file) {
		g_string_free(file, TRUE);
		file = NULL;
	}

	GtkWidget *menuArc = NULL, *submenuArc = NULL;
	GList *ls = NULL;

	ls = gtk_container_get_children (GTK_CONTAINER(menuBar));
	menuArc = GTK_WIDGET(g_list_nth (ls, M_ARCHIVO)->data);
	g_list_free(ls);
	submenuArc = gtk_menu_item_get_submenu (GTK_MENU_ITEM(menuArc));
	ls = gtk_container_get_children (GTK_CONTAINER(submenuArc));
	gtk_widget_set_sensitive(GTK_WIDGET(g_list_nth(ls, SMA_GUARDAR)->data), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(g_list_nth(ls, SMA_CERRAR)->data), FALSE);
	/*gtk_widget_set_sensitive(GTK_WIDGET(g_list_nth(ls, SMA_GUARDAR_COMO)->data), FALSE);*/
	g_list_free(ls);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_toolbar_get_nth_item(GTK_TOOLBAR(barra),
	                                                             T_GUARDAR)),
	                         FALSE);
	saved = TRUE;
	return TRUE;
}

void
_open()
{
	GtkWidget *dlg = NULL;
	GFile *nl = NULL;
	GFileInputStream *strm = NULL;
	GtkTreeIter iter;
	gchar *satid = NULL, *nom = NULL, *desc = NULL;
	GError *err = NULL;
	
	if (!_close()) return;

	if (!file) {
		file = g_string_sized_new (255);
	}

	dlg = gtk_file_chooser_dialog_new("Abrir Lista de Red",
		                              GTK_WINDOW(window),
		                              GTK_FILE_CHOOSER_ACTION_OPEN,
		                              "Abrir",
		                              GTK_RESPONSE_ACCEPT,
		                              "Cancelar",
		                              GTK_RESPONSE_CANCEL,
		                              NULL
		                              );
	GtkFileFilter *filtro = NULL;

 	filtro = gtk_file_filter_new();
	gtk_file_filter_set_name (filtro, "Listas de Red");
	gtk_file_filter_add_pattern(filtro, "*.nl");
	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(dlg),filtro);
	//g_object_unref(filtro);
	                            
	g_signal_connect(dlg,
	                 "close",
	                 G_CALLBACK(gtk_widget_destroy),
	                 NULL);
	if(gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_ACCEPT) {	
		g_string_assign(file,
		                gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg)));
		gtk_widget_destroy(dlg);
	} else {
		gtk_widget_destroy(dlg);
		return;
	}

	nl = g_file_new_for_path (file->str);
	
	strm = g_file_read(nl, NULL, &err);
	/* TODO: Handle File Open Errors */
	if (!strm) {
		_warn(err);
		return;
	}

	GFileInfo *finfo = NULL;
	guint64 fsize;
	gsize read;
	gchar *fcont;
	gboolean t;

	finfo = g_file_input_stream_query_info (strm,
	                                		G_FILE_ATTRIBUTE_STANDARD_SIZE,
	                                		NULL,
	                                		&err);
	if (!finfo) {
		_warn(err);
		return;
	}
	fsize = g_file_info_get_attribute_uint64 (finfo,
	                                  G_FILE_ATTRIBUTE_STANDARD_SIZE);

	fcont = (gchar*)g_malloc((gsize)fsize + 1);
	fcont[fsize] = '\0';

	t = g_input_stream_read_all(G_INPUT_STREAM(strm),
	                        	fcont,
	                        	(gsize)fsize,
	                        	&read,
	                        	NULL,
	                        	&err);
	if (!t) {
		_warn(err);
		return;
	}
	GRegex *regex;
	GMatchInfo *match_info;	

	regex = g_regex_new("^([0-9A-F]{8}):([\\S]*) ?(.*)$",
	                    G_REGEX_MULTILINE,
	                    0,
	                    NULL);
	g_regex_match_full (regex, fcont, fsize, 0, 0, &match_info, &err);
  	while (g_match_info_matches (match_info))
    {
		satid = g_match_info_fetch (match_info, 1);
		nom = g_match_info_fetch (match_info, 2);
		desc = g_match_info_fetch (match_info, 3);

		gtk_list_store_append(_netList, &iter);
		gtk_list_store_set(_netList, &iter,
	                   COL_IDSAT, satid,
	                   COL_NOMBRE, nom,
	                   COL_DESC, desc,
	                   -1);                     
		
        g_free (satid);
        g_free (nom);
        g_free (desc);
        g_match_info_next (match_info, &err);
    }
	g_match_info_free (match_info);
	g_regex_unref (regex);
	g_free(fcont);
	
	GtkWidget *menuArc = NULL, *submenuArc = NULL;
	GList *ls = NULL;

	ls = gtk_container_get_children (GTK_CONTAINER(menuBar));
	menuArc = GTK_WIDGET(g_list_nth (ls, M_ARCHIVO)->data);
	g_list_free(ls);
	submenuArc = gtk_menu_item_get_submenu (GTK_MENU_ITEM(menuArc));
	ls = gtk_container_get_children (GTK_CONTAINER(submenuArc));
	gtk_widget_set_sensitive(GTK_WIDGET(g_list_nth(ls, SMA_CERRAR)->data), TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(g_list_nth(ls, SMA_GUARDAR_COMO)->data), TRUE);
	g_list_free(ls);

	saved = TRUE;
}

/** FIN Other Functions **/

/** Callbacks **/

void
_onOpen(GtkWidget *self, gpointer user_data)
{
	_open();
}

void
_onSave(GtkWidget *self, gpointer user_data)
{
	_save();
}

void
_onSaveAs(GtkWidget *self, gpointer user_data)
{
	GtkWidget *dlg = NULL;
	
	dlg = gtk_file_chooser_dialog_new("Guardar Lista de Red",
		                              GTK_WINDOW(window),
		                              GTK_FILE_CHOOSER_ACTION_SAVE,
		                              "Guardar",
		                              GTK_RESPONSE_ACCEPT,
		                              "Cancelar",
		                              GTK_RESPONSE_CANCEL,
		                              NULL);
	gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dlg),
	                                                TRUE);
	/*GtkFileFilter *filtro;

	filtro = gtk_file_filter_new();
	gtk_file_filter_set_name (filtro, "Listas de Red");
	gtk_file_filter_add_pattern(filtro, "*.nl");
	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(dlg),filtro);
	g_object_unref(filtro);*/
	g_signal_connect(dlg, "close",
	                 G_CALLBACK(gtk_widget_destroy), NULL);
	if(gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_ACCEPT) {
		g_string_free(file, TRUE);
		file = g_string_sized_new (255);
		g_string_assign (file, 
		                 gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg)));
		gtk_widget_destroy (GTK_WIDGET(dlg));
	} else {
		gtk_widget_destroy (GTK_WIDGET(dlg));
		return;
	}		
	_save();
}

void
_onNew(GtkWidget *self, gpointer user_data)
{
	_close();
}

void
_onSalir(GtkWidget *self, gpointer user_data)
{
	if(_close()) {
		gtk_widget_destroy(window);
	}
}

void
_onDestroy(GtkWidget* wid, gpointer user_data)
{
	gtk_list_store_clear (_netList);
	g_object_unref(_netList);
	if (file) g_string_free(file, TRUE);
}
	

void
_onAgregar (GtkToolButton *self, gpointer user_data)
{
	GtkTreeIter iter;

	gtk_list_store_append(_netList, &iter);
	/*gtk_list_store_set(_netList, &iter,
	                   COL_IDSAT, "<id>",
	                   COL_NOMBRE, "<nombre>",
	                   COL_DESC, "<desc>",
	                   -1);*/
	if (saved) {
		GtkWidget *menuArc = NULL, *submenuArc = NULL;
		GList *l = NULL;

		l = gtk_container_get_children (GTK_CONTAINER(menuBar));
		menuArc = GTK_WIDGET(g_list_nth (l, M_ARCHIVO)->data);
		g_list_free(l);
		submenuArc = gtk_menu_item_get_submenu (GTK_MENU_ITEM(menuArc));
		l = gtk_container_get_children (GTK_CONTAINER(submenuArc));
		gtk_widget_set_sensitive(GTK_WIDGET(g_list_nth(l, SMA_GUARDAR)->data), TRUE);
		g_list_free(l);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_toolbar_get_nth_item(GTK_TOOLBAR(barra),
	                                                  T_GUARDAR)),
	                         TRUE);
		saved = FALSE;
	}
	
}

void
_onQuitar (GtkToolButton *self, gpointer user_data)
{
	GtkTreeIter iter;
  	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(_tv));
	
	if (gtk_tree_selection_get_selected (selection, NULL, &iter))
	{
		gtk_list_store_remove (_netList, &iter);
		if (saved) {
			GtkWidget *menuArc = NULL, *submenuArc = NULL;
			GList *l = NULL;

			l = gtk_container_get_children (GTK_CONTAINER(menuBar));
			menuArc = GTK_WIDGET(g_list_nth (l, M_ARCHIVO)->data);
			g_list_free(l);
			submenuArc = gtk_menu_item_get_submenu (GTK_MENU_ITEM(menuArc));
			l = gtk_container_get_children (GTK_CONTAINER(submenuArc));
			gtk_widget_set_sensitive(GTK_WIDGET(g_list_nth(l, SMA_GUARDAR)->data), TRUE);
			g_list_free(l);
			gtk_widget_set_sensitive(GTK_WIDGET(gtk_toolbar_get_nth_item(GTK_TOOLBAR(barra),
			                                              T_GUARDAR)),
			                     TRUE);
			saved = FALSE;
		}
	}
}

void 
_onCellEdited (GtkCellRendererText *cell, gchar *path_string, gchar *new_text,
         	 gpointer user_data)
{
	GtkTreeIter iter;
	GtkTreePath *path = NULL;
	gchar *old_text;
	gint t = -1;

	t = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(cell),
	                      "colnum"));

	path = gtk_tree_path_new_from_string(path_string);
	
    gtk_tree_model_get_iter(GTK_TREE_MODEL(_netList),
                            &iter,
                            path);
	
	gtk_tree_model_get (GTK_TREE_MODEL(_netList), &iter, t, &old_text, -1);
	if (old_text) g_free(old_text);
	gtk_list_store_set(_netList, &iter,
                       t, new_text,
	                   -1);
	gtk_tree_path_free(path);
	if (saved) {
		GtkWidget *menuArc = NULL, *submenuArc = NULL;
		GList *l = NULL;

		l = gtk_container_get_children (GTK_CONTAINER(menuBar));
		menuArc = GTK_WIDGET(g_list_nth (l, M_ARCHIVO)->data);
		g_list_free(l);
		submenuArc = gtk_menu_item_get_submenu (GTK_MENU_ITEM(menuArc));
		l = gtk_container_get_children (GTK_CONTAINER(submenuArc));
		gtk_widget_set_sensitive(GTK_WIDGET(g_list_nth(l, SMA_GUARDAR)->data), TRUE);
		g_list_free(l);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_toolbar_get_nth_item(GTK_TOOLBAR(barra),
		                                              T_GUARDAR)),
		                     TRUE);
		saved = FALSE;
	}
}

gboolean
_onDeleteEvent(GtkWidget *widget,
               GdkEvent  *event,
               gpointer   user_data)
{
	return !_close();
}

/** FIN Callbacks **/

/* For testing propose use the local (not installed) ui file */
/* #define UI_FILE PACKAGE_DATA_DIR"/pmdn_dds/ui/pmdn_dds.ui" */
#define UI_FILE "src/netlistwin.ui"

GtkWidget* net_list_win_new()
{
	GtkBuilder *builder;
	GError* error = NULL;

	GtkCellRenderer *renderer = NULL;
	
	builder = gtk_builder_new ();
	if (!gtk_builder_add_from_file (builder, UI_FILE, &error))
	{
		g_warning ("No se pudo cargar el archivo de builder: %s", error->message);
		g_error_free (error);
	}

	/* This is important */
	gtk_builder_connect_signals (builder, NULL);
	window = GTK_WIDGET (gtk_builder_get_object (builder, "window"));
	_tv = GTK_WIDGET (gtk_builder_get_object (builder, "_tv"));
	barra = GTK_WIDGET (gtk_builder_get_object (builder, "barra"));
	menuBar = GTK_WIDGET (gtk_builder_get_object (builder, "menuBar"));
	
	gtk_window_set_title(GTK_WINDOW(window), "Listas de Red");
	gtk_window_set_default_size(GTK_WINDOW(window), 640, 480);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_modal(GTK_WINDOW(window), TRUE);
	g_signal_connect(window,
	                 "destroy",
	                 G_CALLBACK(_onDestroy),
	                 NULL);
	g_signal_connect(window,
	                 "delete-event",
	                 G_CALLBACK(_onDeleteEvent),
	                 NULL);

	g_object_unref (builder);


	/* TreeView y Modelo */
	_netList = gtk_list_store_new(NUM_COLS,
	                              GTK_TYPE_STRING,
	                              GTK_TYPE_STRING,
	                              GTK_TYPE_STRING);
	
	gtk_tree_view_set_model(GTK_TREE_VIEW(_tv), GTK_TREE_MODEL(_netList));

	/* Columna Id Satelital */
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(G_OBJECT(renderer),
	             "editable", TRUE,
	             NULL);
	g_object_set_data(G_OBJECT(renderer),
	                  "colnum", GINT_TO_POINTER(COL_IDSAT));
	g_signal_connect(renderer,
	                 "edited",
	                 G_CALLBACK(_onCellEdited),
	                 NULL);
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW (_tv), -1,
	                                            "Id Satelital", renderer,
	                                            "text", COL_IDSAT,
	                                            NULL);
	gtk_tree_view_column_set_expand(gtk_tree_view_get_column(
	                                                         GTK_TREE_VIEW(_tv),
	                                                         COL_IDSAT),
	                                TRUE);
	gtk_tree_view_column_set_sizing(gtk_tree_view_get_column(
	                                                         GTK_TREE_VIEW(_tv),
	                                                         COL_IDSAT),
	                                GTK_TREE_VIEW_COLUMN_AUTOSIZE);

	/* Columna Nombre */
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(G_OBJECT(renderer),
	             "editable", TRUE,
	             NULL);
	g_object_set_data(G_OBJECT(renderer),
	                  "colnum", GINT_TO_POINTER(COL_NOMBRE));
	g_signal_connect(renderer,
	                 "edited",
	                 G_CALLBACK(_onCellEdited),
	                 NULL);
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW (_tv), -1,
	                                            "Nombre", renderer,
	                                            "text", COL_NOMBRE,
	                                            NULL);
	gtk_tree_view_column_set_expand(gtk_tree_view_get_column(
	                                                         GTK_TREE_VIEW(_tv),
	                                                         COL_NOMBRE),
	                                TRUE);
	gtk_tree_view_column_set_sizing(gtk_tree_view_get_column(
	                                                         GTK_TREE_VIEW(_tv),
	                                                         COL_NOMBRE),
	                                GTK_TREE_VIEW_COLUMN_AUTOSIZE);	                 

	/* Columna Descripción */
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(G_OBJECT(renderer),
	             "editable", TRUE,
	             NULL);
	g_object_set_data(G_OBJECT(renderer),
	                  "colnum", GINT_TO_POINTER(COL_DESC));
	g_signal_connect(renderer,
	                 "edited",
	                 G_CALLBACK(_onCellEdited),
	                 NULL);
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW (_tv), -1,
	                                            "Descripción", renderer,
	                                            "text", COL_DESC,
	                                            NULL);
	gtk_tree_view_column_set_expand(gtk_tree_view_get_column(
	                                                         GTK_TREE_VIEW(_tv),
	                                                         COL_DESC),
	                                TRUE);
	gtk_tree_view_column_set_sizing(gtk_tree_view_get_column(
	                                                         GTK_TREE_VIEW(_tv),
	                                                         COL_DESC),
	                                GTK_TREE_VIEW_COLUMN_AUTOSIZE);

	return window;
}
