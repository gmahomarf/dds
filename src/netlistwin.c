#include "netlistwin.h"

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

/** Callbacks **/


/*void
_rowInsEdit(GtkTreeModel *tree_model, GtkTreePath  *path,
            GtkTreeIter  *iter, gpointer      user_data)
{
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

void
_rowDel(GtkTreeModel *tree_model, GtkTreePath  *path,
        gpointer      user_data)
{
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
}*/

gint
_confirm(){
	GtkWidget *dlg;
	gint ret;

	dlg = gtk_message_dialog_new(GTK_WINDOW(window),
	                             GTK_DIALOG_MODAL,
	                             GTK_MESSAGE_QUESTION,
	                             GTK_BUTTONS_YES_NO | GTK_BUTTONS_CANCEL,
	                             "El archivo no se ha guardado.\n"
	                             "¿Desea guardar antes de continuar?");
	g_signal_connect(dlg,
	                 "close",
	                 G_CALLBACK(gtk_widget_destroy),
	                 NULL);

	ret = gtk_dialog_run(GTK_DIALOG(dlg));
	gtk_widget_destroy(dlg);
	return ret;
}

void _save()
{
	GtkWidget *dlg = NULL;
	GFile *nl = NULL;
	GFileOutputStream *strm = NULL;
	GList *l = NULL;
	GtkTreeIter iter;
	gchar *satid = NULL, *nom = NULL, *desc = NULL;
	GString *out = NULL;
	GError *err = NULL;
	GtkFileFilter *filtro;

	if (saved) return;

	if (!file) {
		dlg = gtk_file_chooser_dialog_new("Guardar Lista de Red",
			                              GTK_WINDOW(window),
			                              GTK_FILE_CHOOSER_ACTION_SAVE,
			                              "Guardar",
			                              GTK_RESPONSE_ACCEPT,
			                              "Cancelar",
			                              GTK_RESPONSE_CANCEL,
			                              NULL
			                              );
		gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dlg),
		                                                TRUE);
		filtro = gtk_file_filter_new();
		gtk_file_filter_set_name (filtro, "Listas de Red");
		gtk_file_filter_add_pattern(filtro, "*.nl");
		gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(dlg),filtro);
		g_object_unref(filtro);
		g_signal_connect(dlg, "close",
		                 G_CALLBACK(gtk_widget_destroy), NULL);
		if(gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_ACCEPT) {
			g_string_free(file, TRUE);
			file = g_string_sized_new (255);
			g_string_assign (file, 
			                 gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg)));
			nl = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dlg));
			gtk_widget_destroy (GTK_WIDGET(dlg));
		} else {
			gtk_widget_destroy (GTK_WIDGET(dlg));
			return;
		}
	} else {
		nl = g_file_new_for_path (file->str);
	}

	strm = g_file_replace (nl, NULL, FALSE,
	                       G_FILE_CREATE_REPLACE_DESTINATION, NULL, &err);
	/* TODO: Handle File Save Errors */

	gtk_tree_model_get_iter_first (GTK_TREE_MODEL(_netList), &iter);
	do {
		if (out) g_string_free(out, TRUE);
		out = g_string_sized_new (255);
		gtk_tree_model_get(GTK_TREE_MODEL(_netList),
		                   &iter,
		                   COL_IDSAT,
		                   &satid,
		                   COL_NOMBRE,
		                   &nom,
		                   COL_DESC,
		                   &desc,
		                   -1);
		g_string_printf(out, "%s:%s:%s%c",
		                satid ? satid : "\0",
		                nom ? nom : "\0",
		                desc ? desc : "u\0",
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
	ls = gtk_container_get_children (GTK_CONTAINER(menuArc));
	submenuArc = GTK_WIDGET(g_list_nth (ls, 0));
	g_list_free(ls);
	ls = gtk_container_get_children (GTK_CONTAINER(submenuArc));
	gtk_widget_set_sensitive(GTK_WIDGET(g_list_nth(ls, SMA_GUARDAR)->data), FALSE);
	g_list_free(ls);
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_toolbar_get_nth_item(GTK_TOOLBAR(barra),
	                                                             T_GUARDAR)),
	                         FALSE);
	saved = TRUE;
	
}

gboolean
_close()
{
	if (!saved){
		/* Confirm opening new file */
		switch(_confirm()){
			case GTK_RESPONSE_YES:
				_save();
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

	GtkWidget *menuArc = NULL, *submenuArc = NULL;
	GList *ls = NULL;

	ls = gtk_container_get_children (GTK_CONTAINER(menuBar));
	menuArc = GTK_WIDGET(g_list_nth (ls, M_ARCHIVO)->data);
	g_list_free(ls);
	ls = gtk_container_get_children (GTK_CONTAINER(menuArc));
	submenuArc = GTK_WIDGET(g_list_nth (ls, 0));
	g_list_free(ls);
	ls = gtk_container_get_children (GTK_CONTAINER(submenuArc));
	gtk_widget_set_sensitive(GTK_WIDGET(g_list_nth(ls, SMA_GUARDAR)->data), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(g_list_nth(ls, SMA_CERRAR)->data), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(g_list_nth(ls, SMA_GUARDAR_COMO)->data), FALSE);
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
	GList *l = NULL;
	GtkTreeIter iter;
	gchar *satid = NULL, *nom = NULL, *desc = NULL;
	GString *ins = NULL;
	GError *err = NULL;
	GtkFileFilter *filtro = NULL;
	
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
	filtro = gtk_file_filter_new();
	gtk_file_filter_set_name (filtro, "Listas de Red");
	gtk_file_filter_add_pattern(filtro, "*.nl");
	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(dlg),filtro);
	g_object_unref(filtro);
	                            
	g_signal_connect(dlg,
	                 "close",
	                 G_CALLBACK(gtk_widget_destroy),
	                 NULL);
	if(gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_ACCEPT) {	
		g_string_assign(file,
		                gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg)));
		nl = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dlg));
		strm = g_file_read(nl, NULL, &err);
		/* TODO: Handle File Open Errors */

		
		
		gtk_widget_destroy(dlg);
	} else {
		gtk_widget_destroy(dlg);
		return;
	}

	
	
	GtkWidget *menuArc = NULL, *submenuArc = NULL;
	GList *ls = NULL;

	ls = gtk_container_get_children (GTK_CONTAINER(menuBar));
	menuArc = GTK_WIDGET(g_list_nth (ls, M_ARCHIVO)->data);
	g_list_free(ls);
	ls = gtk_container_get_children (GTK_CONTAINER(menuArc));
	submenuArc = GTK_WIDGET(g_list_nth (ls, 0));
	g_list_free(ls);
	ls = gtk_container_get_children (GTK_CONTAINER(submenuArc));
	gtk_widget_set_sensitive(GTK_WIDGET(g_list_nth(ls, SMA_CERRAR)->data), TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(g_list_nth(ls, SMA_GUARDAR_COMO)->data), TRUE);
	g_list_free(ls);

	saved = TRUE;
}

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
	GtkWidget *menuArc = NULL, *submenuArc = NULL;
	GList *l = NULL;

	l = gtk_container_get_children (GTK_CONTAINER(menuBar));
	menuArc = GTK_WIDGET(g_list_nth (l, M_ARCHIVO)->data);
	g_list_free(l);
	submenuArc = gtk_menu_item_get_submenu (GTK_MENU_ITEM(menuArc));
	l = gtk_container_get_children (GTK_CONTAINER(submenuArc));
	gtk_widget_set_sensitive(GTK_WIDGET(g_list_nth(l, SMA_GUARDAR)->data), TRUE);
	g_list_free(l);
	
	gtk_list_store_append(_netList, &iter);
	/*gtk_list_store_set(_netList, &iter,
	                   COL_IDSAT, "<id>",
	                   COL_NOMBRE, "<nombre>",
	                   COL_DESC, "<desc>",
	                   -1);*/
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_toolbar_get_nth_item(GTK_TOOLBAR(barra),
	                                                  T_GUARDAR)),
	                         TRUE);
	saved = FALSE;
}

void
_onQuitar (GtkToolButton *self, gpointer user_data)
{
	GtkTreeIter iter;
  	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(_tv));
	
	if (gtk_tree_selection_get_selected (selection, NULL, &iter))
	{
		GtkWidget *menuArc = NULL, *submenuArc = NULL;
		GList *l = NULL;

		l = gtk_container_get_children (GTK_CONTAINER(menuBar));
		menuArc = GTK_WIDGET(g_list_nth (l, M_ARCHIVO)->data);
		g_list_free(l);
		submenuArc = gtk_menu_item_get_submenu (GTK_MENU_ITEM(menuArc));
		l = gtk_container_get_children (GTK_CONTAINER(submenuArc));
		gtk_widget_set_sensitive(GTK_WIDGET(g_list_nth(l, SMA_GUARDAR)->data), TRUE);
		g_list_free(l);
		gtk_list_store_remove (_netList, &iter);
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_toolbar_get_nth_item(GTK_TOOLBAR(barra),
	                                                  T_GUARDAR)),
	                         TRUE);
		saved = FALSE;
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
	GtkWidget *menuArc = NULL, *submenuArc = NULL;
	GList *l = NULL;

	l = gtk_container_get_children (GTK_CONTAINER(menuBar));
	menuArc = GTK_WIDGET(g_list_nth (l, M_ARCHIVO));
	g_list_free(l);
	l = gtk_container_get_children (GTK_CONTAINER(menuArc));
	submenuArc = GTK_WIDGET(g_list_nth (l, 0));
	g_list_free(l);
	l = gtk_container_get_children (GTK_CONTAINER(submenuArc));
	gtk_widget_set_sensitive(GTK_WIDGET(g_list_nth(l, SMA_GUARDAR)), TRUE);
	g_list_free(l);
	
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
	saved = FALSE;
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_toolbar_get_nth_item(GTK_TOOLBAR(barra),
	                                                  T_GUARDAR)),
	                         TRUE);
}

/** FIN Callbacks **/

GtkWidget* _barraHerramientas()
{
	GtkWidget *toolBtn = NULL;

	/* Crear barra */
	barra = gtk_toolbar_new();

	/*** Crear Botones ***/
	/* Crear botón 'Nueva' */
	toolBtn = GTK_WIDGET(
		      	gtk_tool_button_new
	                     (gtk_image_new_from_stock(GTK_STOCK_NEW,
	                                               GTK_ICON_SIZE_SMALL_TOOLBAR),
	                     "Nueva"));

	/*** TODO: Ligar botones de barra de herramientas a respectivas señales ***/
	/* g_signal_connect(toolBtn,
	                    "clicked",
	                    G_CALLBACK(),
						NULL);*/
	
	/* Agregar botón */
	gtk_toolbar_insert(GTK_TOOLBAR(barra),
	                   GTK_TOOL_ITEM(toolBtn),
	                   -1);

	/* Crear botón 'Abrir' */
	toolBtn = GTK_WIDGET(
		      	gtk_tool_button_new
	                     (gtk_image_new_from_stock(GTK_STOCK_OPEN,
	                                               GTK_ICON_SIZE_SMALL_TOOLBAR),
	                     "Abrir"));

	/*** TODO: Ligar botones de barra de herramientas a respectivas señales ***/
	/* g_signal_connect(toolBtn,
	                    "clicked",
	                    G_CALLBACK(),
						NULL);*/
	
	/* Agregar botón */
	gtk_toolbar_insert(GTK_TOOLBAR(barra),
	                   GTK_TOOL_ITEM(toolBtn),
	                   -1);

	/* Crear botón 'Guardar' */
	toolBtn = GTK_WIDGET(
		      	gtk_tool_button_new
	                     (gtk_image_new_from_stock(GTK_STOCK_SAVE,
	                                               GTK_ICON_SIZE_SMALL_TOOLBAR),
	                     "Guardar"));
	gtk_widget_set_sensitive(toolBtn, FALSE);

	/* Conectar señal */
	g_signal_connect(toolBtn,
	                    "clicked",
	                    G_CALLBACK(_onSave),
						NULL);
	
	/* Agregar botón */
	gtk_toolbar_insert(GTK_TOOLBAR(barra),
	                   GTK_TOOL_ITEM(toolBtn),
	                   -1);

	/* Crear separador */
	toolBtn = GTK_WIDGET(gtk_separator_tool_item_new ());

	/* Agregar separador */
	gtk_toolbar_insert(GTK_TOOLBAR(barra),
	                   GTK_TOOL_ITEM(toolBtn),
	                   -1);

	/* Crear botón 'Agregar' */
	toolBtn = GTK_WIDGET(
		      	gtk_tool_button_new
	                     (gtk_image_new_from_stock(GTK_STOCK_ADD,
	                                               GTK_ICON_SIZE_SMALL_TOOLBAR),
	                     "Agregar"));

	/*** Conectar señal ***/
	g_signal_connect(toolBtn,
	                 "clicked",
	                 G_CALLBACK(_onAgregar),
	                 NULL);
	
	/* Agregar botón */
	gtk_toolbar_insert(GTK_TOOLBAR(barra),
	                   GTK_TOOL_ITEM(toolBtn),
	                   -1);

	/* Crear botón 'Quitar' */
	toolBtn = GTK_WIDGET(
		      	gtk_tool_button_new
	                     (gtk_image_new_from_stock(GTK_STOCK_REMOVE,
	                                               GTK_ICON_SIZE_SMALL_TOOLBAR),
	                     "Quitar"));

	/* Conectar Señal */
	g_signal_connect(toolBtn,
	                    "clicked",
	                    G_CALLBACK(_onQuitar),
						NULL);
	
	/* Agregar botón */
	gtk_toolbar_insert(GTK_TOOLBAR(barra),
	                   GTK_TOOL_ITEM(toolBtn),
	                   -1);

	return barra;
}

GtkWidget* _menuPrincipal()
{
	
	GtkWidget *menu = NULL;
	GtkWidget *submenu = NULL;
	GtkWidget *subitem = NULL;

	/* Crear barra de menú */
	menuBar = gtk_menu_bar_new ();

	/** Menú 'Archivo' **/
	/* Crear menú */
	menu = gtk_menu_item_new_with_mnemonic ("_Archivo");

	/* Crear submenú */
	submenu = gtk_menu_new();

	/* Crear subitem 'Nueva' */
	subitem = gtk_image_menu_item_new_with_mnemonic ("_Nueva");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(subitem),
	                              gtk_image_new_from_stock (GTK_STOCK_NEW,
	                                                        GTK_ICON_SIZE_MENU));

	/*** TODO: Ligar menús a sus respectivas señales ***/
	/*g_signal_connect(subitem,
	                 "activate",
	                 G_CALLBACK(),
	                 NULL);*/

	/* Agregar subitem a submenú */
	gtk_menu_shell_append(GTK_MENU_SHELL(submenu), subitem);
	
	/* Crear subitem 'Abrir' */
	subitem = gtk_image_menu_item_new_with_mnemonic ("_Abrir...");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(subitem),
	                              gtk_image_new_from_stock (GTK_STOCK_OPEN,
	                                                        GTK_ICON_SIZE_MENU));

	/*** TODO: Ligar menús a sus respectivas señales ***/
	/*g_signal_connect(subitem,
	                 "activate",
	                 G_CALLBACK(),
	                 NULL);*/

	/* Agregar subitem a submenú */
	gtk_menu_shell_append(GTK_MENU_SHELL(submenu), subitem);
	
	/* Crear subitem 'Guardar' */
	subitem = gtk_image_menu_item_new_with_mnemonic ("_Guardar");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(subitem),
	                              gtk_image_new_from_stock (GTK_STOCK_SAVE,
	                                                        GTK_ICON_SIZE_MENU));
	gtk_widget_set_sensitive (subitem, FALSE);

	/* Conectar señal */
	g_signal_connect(subitem,
	                 "activate",
	                 G_CALLBACK(_onSave),
	                 NULL);

	/* Agregar subitem a submenú */
	gtk_menu_shell_append(GTK_MENU_SHELL(submenu), subitem);
	
	/* Crear subitem 'Guardar como' */
	subitem = gtk_image_menu_item_new_with_mnemonic ("Guardar como...");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(subitem),
	                              gtk_image_new_from_stock (GTK_STOCK_SAVE_AS,
	                                                        GTK_ICON_SIZE_MENU));

	/*** TODO: Ligar menús a sus respectivas señales ***/
	/*g_signal_connect(subitem,
	                 "activate",
	                 G_CALLBACK(),
	                 NULL);*/

	/* Agregar subitem a submenú */
	gtk_menu_shell_append(GTK_MENU_SHELL(submenu), subitem);
	
	/* Crear subitem 'Cerrar' */
	subitem = gtk_image_menu_item_new_with_mnemonic ("_Cerrar");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(subitem),
	                              gtk_image_new_from_stock (GTK_STOCK_CLOSE,
	                                                        GTK_ICON_SIZE_MENU));

	/*** TODO: Ligar menús a sus respectivas señales ***/
	/*g_signal_connect(subitem,
	                 "activate",
	                 G_CALLBACK(),
	                 NULL);*/

	/* Agregar subitem a submenú */
	gtk_menu_shell_append(GTK_MENU_SHELL(submenu), subitem);
	
	/* Crear separador */
	subitem = gtk_separator_menu_item_new();

	/* Agregar separador a submenú */
	gtk_menu_shell_append(GTK_MENU_SHELL(submenu), subitem);
	
	/* Crear subitem 'Salir' */
	subitem = gtk_image_menu_item_new_with_mnemonic ("_Salir");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(subitem),
	                              gtk_image_new_from_stock (GTK_STOCK_QUIT,
	                                                        GTK_ICON_SIZE_MENU));

	/*** TODO: Ligar menús a sus respectivas señales ***/
	/*g_signal_connect(subitem,
	                 "activate",
	                 G_CALLBACK(),
	                 NULL);*/

	/* Agregar subitem a submenú */
	gtk_menu_shell_append(GTK_MENU_SHELL(submenu), subitem);
	
	/* Agregar submenú a menú*/
	gtk_menu_item_set_submenu (GTK_MENU_ITEM(menu), submenu);

	/* Agregar menú a barra */
	gtk_menu_shell_append(GTK_MENU_SHELL(menuBar), menu);

	return menuBar;
}

GtkWidget* _listaRed()
{
	GtkCellRenderer *renderer = NULL;
	//GtkWidget *tv = NULL;
	
	/* TreeView y Modelo */
	_netList = gtk_list_store_new(NUM_COLS,
	                              GTK_TYPE_STRING,
	                              GTK_TYPE_STRING,
	                              GTK_TYPE_STRING,
	                              GTK_TYPE_BOOL);
	/*g_signal_connect(_netList,
	                 "row-inserted",
	                 G_CALLBACK(_rowInsEdit),
	                 NULL);
    g_signal_connect(_netList,
	                 "row-edited",
	                 G_CALLBACK(_rowInsEdit),
	                 NULL);
    g_signal_connect(_netList,
	                 "row-deleted",
	                 G_CALLBACK(_rowDel),
	                 NULL);*/
    _tv = gtk_tree_view_new_with_model (GTK_TREE_MODEL(_netList));
	gtk_tree_view_set_headers_clickable (GTK_TREE_VIEW(_tv), FALSE);
	gtk_tree_view_set_grid_lines (GTK_TREE_VIEW(_tv),
	                              GTK_TREE_VIEW_GRID_LINES_BOTH);

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

	return _tv;
}

GtkWidget* net_list_win_new()
{
	GtkWidget *vbox1 = NULL;
	GtkWidget *sw = NULL;

	/* Crear la ventana */
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "Listas de Red");
	gtk_window_set_default_size(GTK_WINDOW(window), 640, 480);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_modal(GTK_WINDOW(window), TRUE);
	g_signal_connect(window,
	                 "destroy",
	                 G_CALLBACK(_onDestroy),
	                 NULL);

	/* Crear vbox */
	vbox1 = gtk_vbox_new (FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(vbox1));

	/* Menú principal */
	gtk_box_pack_start(GTK_BOX(vbox1),
	                   _menuPrincipal(),
	                   FALSE,
	                   TRUE,
	                   0);

	/* Barra de Herramientas */
	gtk_box_pack_start(GTK_BOX(vbox1),
	                   _barraHerramientas(),
	                   FALSE,
	                   TRUE,
	                   0);

	/* Lista de red */
	sw = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(sw),
	                                GTK_POLICY_AUTOMATIC,
	                                GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(sw),
	                  _listaRed());
	gtk_box_pack_start (GTK_BOX(vbox1),
	                    sw,
	                    TRUE,
	                    TRUE,
	                    0);
	
	return window;
}
