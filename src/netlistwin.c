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


void
_rowInsEdit(GtkTreeModel *tree_model, GtkTreePath  *path,
            GtkTreeIter  *iter, gpointer      user_data)
{
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_toolbar_get_nth_item(GTK_TOOLBAR(barra),
	                                                  TGUARDAR)),
	                         TRUE);
}

void
_rowDel(GtkTreeModel *tree_model, GtkTreePath  *path,
        gpointer      user_data)
{
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_toolbar_get_nth_item(GTK_TOOLBAR(barra),
	                                                  TGUARDAR)),
	                         TRUE);
}

void
_save(GtkToolButton *self, gpointer user_data)
{
	GtkWidget *dlg = NULL;
	GFile *nl = NULL;
	GFileOutputStream *strm = NULL;
	GList *l = NULL;
	GtkTreeIter iter;
	gchar *satid = NULL, *nom = NULL, *desc = NULL;
	GString *out = NULL;
	GError *err = NULL;

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
		if(gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_ACCEPT) {
			/*Save code here*/
			saved = TRUE;
			gtk_widget_set_sensitive (GTK_WIDGET(self),
				                      FALSE);
			g_string_free(file, TRUE);
			file = g_string_sized_new (255);
			g_string_assign (file, 
			                 gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg)));
			nl = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dlg));
		}
		gtk_widget_destroy (GTK_WIDGET(dlg));
	} else {
		gtk_widget_set_sensitive (GTK_WIDGET(self),
				                      FALSE);
		nl = g_file_new_for_path (file->str);
	}
	strm = g_file_replace (nl, NULL, FALSE,
	                       G_FILE_CREATE_REPLACE_DESTINATION, NULL, &err);

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
		g_output_stream_write(strm, out->str, out->len, NULL, &err);
	} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(_netList), &iter));
	g_list_free(l);
	g_output_stream_close (strm, NULL, NULL);
	g_object_unref(strm);
}

void
_destroy(GtkWidget* wid, gpointer user_data)
{
	gtk_list_store_clear (_netList);
	g_object_unref(_netList);
}
	

void
_agregar (GtkToolButton *self, gpointer user_data)
{
	GtkTreeIter iter;
	
	gtk_list_store_append(_netList, &iter);
	/*gtk_list_store_set(_netList, &iter,
	                   COL_IDSAT, "<id>",
	                   COL_NOMBRE, "<nombre>",
	                   COL_DESC, "<desc>",
	                   -1);*/
}

void
_quitar (GtkToolButton *self, gpointer user_data)
{
	GtkTreeIter iter;
  	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(_tv));
	
    if (gtk_tree_selection_get_selected (selection, NULL, &iter))
	{
		gtk_list_store_remove (_netList, &iter);
	}
}

void 
_cellEdited (GtkCellRendererText *cell, gchar *path_string, gchar *new_text,
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
	saved = FALSE;
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_toolbar_get_nth_item(GTK_TOOLBAR(barra),
	                                                  TGUARDAR)),
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
	                    G_CALLBACK(_save),
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
	                 G_CALLBACK(_agregar),
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
	                    G_CALLBACK(_quitar),
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

	/*** TODO: Ligar menús a sus respectivas señales ***/
	/*g_signal_connect(subitem,
	                 "activate",
	                 G_CALLBACK(),
	                 NULL);*/

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
	g_signal_connect(_netList,
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
	                 NULL);
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
	                 G_CALLBACK(_cellEdited),
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
	                 G_CALLBACK(_cellEdited),
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
	                 G_CALLBACK(_cellEdited),
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
