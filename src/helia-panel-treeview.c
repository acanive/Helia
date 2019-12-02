/*
* Copyright 2019 Stepan Perun
* This program is free software.
*
* License: Gnu Lesser General Public License
* http://www.gnu.org/licenses/lgpl.html
*/


#include <helia-include.h>


static void helia_panel_treeview_window_clear ( GtkWindow *win_base, GtkTreeView *tree_view )
{
	GtkTreeModel *model_filter = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );
	GtkTreeModel *model = gtk_tree_model_filter_get_model ( GTK_TREE_MODEL_FILTER ( model_filter ) );

	uint ind = gtk_tree_model_iter_n_children ( model, NULL );

	if ( ind == 0 ) return;

	GtkWindow *window = helia_create_window_top ( win_base, "", "helia-clear", GTK_WIN_POS_CENTER_ON_PARENT, TRUE );
	gtk_widget_set_size_request ( GTK_WIDGET ( window ), 400, 150 );

	GtkBox *m_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_VERTICAL,   0 );
	GtkBox *i_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
	GtkBox *h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );

	GdkPixbuf *pixbuf = gtk_icon_theme_load_icon ( gtk_icon_theme_get_default (), 
						"helia-warning", 48, GTK_ICON_LOOKUP_USE_BUILTIN, NULL );

	GtkImage *image   = (GtkImage  *)gtk_image_new_from_pixbuf ( pixbuf );

	if ( pixbuf ) g_object_unref ( pixbuf );

	gtk_box_pack_start ( i_box, GTK_WIDGET ( image ), TRUE, TRUE, 0 );

	GtkLabel *label = (GtkLabel *)gtk_label_new ( "" );

	char *text = g_strdup_printf ( "%d", ind );

		gtk_label_set_text ( label, text );

	g_free  ( text );

	gtk_box_pack_start ( i_box, GTK_WIDGET ( label ), TRUE, TRUE, 10 );

	pixbuf = gtk_icon_theme_load_icon ( gtk_icon_theme_get_default (), 
			"helia-clear", 48, GTK_ICON_LOOKUP_USE_BUILTIN, NULL );

	image = (GtkImage  *)gtk_image_new_from_pixbuf ( pixbuf );

	if ( pixbuf ) g_object_unref ( pixbuf );

	gtk_box_pack_start ( i_box, GTK_WIDGET ( image ), TRUE, TRUE, 0 );

	gtk_box_pack_start ( m_box, GTK_WIDGET ( i_box ), TRUE, TRUE, 5 );

	GtkButton *button_clear = helia_set_image_button ( "helia-ok", 16 );
	g_signal_connect_swapped ( button_clear, "clicked", G_CALLBACK ( gtk_list_store_clear ), GTK_LIST_STORE ( model ) );
	g_signal_connect_swapped ( button_clear, "clicked", G_CALLBACK ( gtk_widget_destroy ), window );

	gtk_box_pack_end ( h_box, GTK_WIDGET ( button_clear ), TRUE, TRUE, 5 );

	GtkButton *button_close = helia_set_image_button ( "helia-exit", 16 );
	g_signal_connect_swapped ( button_close, "clicked", G_CALLBACK ( gtk_widget_destroy ), window );

	gtk_box_pack_end ( h_box, GTK_WIDGET ( button_close ), TRUE, TRUE, 5 );

	gtk_box_pack_start ( m_box, GTK_WIDGET ( h_box ), FALSE, FALSE, 5 );

	gtk_container_set_border_width ( GTK_CONTAINER ( m_box ), 5 );
	gtk_container_add ( GTK_CONTAINER ( window ), GTK_WIDGET ( m_box ) );

	gtk_widget_show_all ( GTK_WIDGET ( window ) );
	gtk_widget_set_opacity ( GTK_WIDGET ( window ), gtk_widget_get_opacity ( GTK_WIDGET ( win_base ) ) );
}

static void helia_panel_treeview_reread ( GtkTreeView *tree_view )
{
	GtkTreeIter iter;

	GtkTreeModel *model_filter = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );
	GtkTreeModel *model = gtk_tree_model_filter_get_model ( GTK_TREE_MODEL_FILTER ( model_filter ) );

	int row_count = 1;
	gboolean valid;

	for ( valid = gtk_tree_model_get_iter_first ( model, &iter ); valid;
		  valid = gtk_tree_model_iter_next ( model, &iter ) )
	{
		gtk_list_store_set ( GTK_LIST_STORE ( model ), &iter, COL_NUM, row_count++, -1 );
	}
}

static void helia_panel_treeview_up_down ( GtkTreeView *tree_view, gboolean up_dw )
{
	GtkTreeModel *model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );

	int ind = gtk_tree_model_iter_n_children ( model, NULL );

	if ( ind < 2 ) return;

	GtkTreeIter child_iter, child_iter_c, filt_iter, filt_iter_c;

	GtkTreeModel *filter_model = NULL;

	if ( gtk_tree_selection_get_selected ( gtk_tree_view_get_selection ( tree_view ), &filter_model, &filt_iter ) )
	{
		gtk_tree_selection_get_selected ( gtk_tree_view_get_selection ( tree_view ), NULL, &filt_iter_c );

		gtk_tree_model_filter_convert_iter_to_child_iter ( GTK_TREE_MODEL_FILTER ( filter_model ), &child_iter, &filt_iter );
		gtk_tree_model_filter_convert_iter_to_child_iter ( GTK_TREE_MODEL_FILTER ( filter_model ), &child_iter_c, &filt_iter_c );

		GtkTreeModel *model = gtk_tree_model_filter_get_model ( GTK_TREE_MODEL_FILTER ( filter_model ) );

		if ( up_dw )
		if ( gtk_tree_model_iter_previous ( model, &child_iter ) )
			gtk_list_store_move_before ( GTK_LIST_STORE ( model ), &child_iter_c, &child_iter );

		if ( !up_dw )
		if ( gtk_tree_model_iter_next ( model, &child_iter ) )
			gtk_list_store_move_after ( GTK_LIST_STORE ( model ), &child_iter_c, &child_iter );

		helia_panel_treeview_reread ( tree_view );
	}
}

static void helia_panel_treeview_remove ( GtkTreeView *tree_view )
{
	GtkTreeIter child_iter, filt_iter;

	GtkTreeModel *filter_model = NULL;

	if ( gtk_tree_selection_get_selected ( gtk_tree_view_get_selection ( tree_view ), &filter_model, &filt_iter ) )
	{
		gtk_tree_model_filter_convert_iter_to_child_iter ( GTK_TREE_MODEL_FILTER ( filter_model ), &child_iter, &filt_iter );

		gtk_list_store_remove ( GTK_LIST_STORE ( gtk_tree_model_filter_get_model ( GTK_TREE_MODEL_FILTER ( filter_model ) ) ), &child_iter );

		helia_panel_treeview_reread ( tree_view );
	}
}

static void helia_panel_treeview_goup ( G_GNUC_UNUSED GtkButton *button, GtkTreeView *tree_view )
{
	helia_panel_treeview_up_down ( tree_view, TRUE  );
}

static void helia_panel_treeview_down ( G_GNUC_UNUSED GtkButton *button, GtkTreeView *tree_view )
{
	helia_panel_treeview_up_down ( tree_view, FALSE );
}

static void helia_panel_treeview_remv ( G_GNUC_UNUSED GtkButton *button, GtkTreeView *tree_view )
{
	helia_panel_treeview_remove ( tree_view );
}

static void helia_panel_treeview_clear ( G_GNUC_UNUSED GtkButton *button, GtkTreeView *tree_view )
{
	GtkWindow *window = GTK_WINDOW ( gtk_widget_get_toplevel ( GTK_WIDGET ( tree_view ) ) );

	helia_panel_treeview_window_clear ( window, tree_view );
}

GtkBox * helia_panel_treeview_new ( GtkTreeView *tree_view )
{
	GtkBox *h_box  = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
	gtk_box_set_spacing ( h_box,  5 );

	gtk_widget_set_margin_start  ( GTK_WIDGET ( h_box ), 5 );
	gtk_widget_set_margin_end    ( GTK_WIDGET ( h_box ), 5 );
	gtk_widget_set_margin_top    ( GTK_WIDGET ( h_box ), 5 );
	//gtk_widget_set_margin_bottom ( GTK_WIDGET ( h_box ), 5 );

	IconFunc IconFunc_n[] =
	{
		{ "helia-up", 	 	FALSE, helia_panel_treeview_goup  },
		{ "helia-down", 	FALSE, helia_panel_treeview_down  },
		{ "helia-remove", 	FALSE, helia_panel_treeview_remv  },
		{ "helia-clear", 	FALSE, helia_panel_treeview_clear },
	};

	uint i = 0;
	for ( i = 0; i < G_N_ELEMENTS ( IconFunc_n ); i++ )
	{
		GtkButton *button = helia_set_image_button ( IconFunc_n[i].name, 16 );

		g_signal_connect ( button, "clicked", G_CALLBACK ( IconFunc_n[i].f ), tree_view );

		gtk_box_pack_start ( h_box, GTK_WIDGET ( button ), TRUE, TRUE, 0 );
	}

	return h_box;
}

