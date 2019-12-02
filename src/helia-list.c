/*
* Copyright 2019 Stepan Perun
* This program is free software.
*
* License: Gnu Lesser General Public License
* http://www.gnu.org/licenses/lgpl.html
*/


#include <helia-include.h>


GtkScrolledWindow * helia_create_scroll ( HeliaTreeview *tree_view, uint set_size )
{
	GtkScrolledWindow *scroll = (GtkScrolledWindow *)gtk_scrolled_window_new ( NULL, NULL );

	gtk_scrolled_window_set_policy ( scroll, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
	gtk_widget_set_size_request ( GTK_WIDGET ( scroll ), set_size, -1 );

	gtk_container_add ( GTK_CONTAINER ( scroll ), GTK_WIDGET ( tree_view ) );

	return scroll;
}

static void helia_panel_search_mp ( G_GNUC_UNUSED GtkButton *button, Helia *helia )
{
	if ( gtk_widget_get_visible ( GTK_WIDGET ( helia->search_entry_mp ) ) )
	{
		gtk_entry_set_text ( GTK_ENTRY ( helia->search_entry_mp ), "" );
		gtk_widget_hide ( GTK_WIDGET ( helia->search_entry_mp ) );
	}
	else
	{
		gtk_widget_show ( GTK_WIDGET ( helia->search_entry_mp ) );
		gtk_widget_grab_focus ( GTK_WIDGET ( helia->search_entry_mp ) );
	}
}

static void helia_panel_search_tv ( G_GNUC_UNUSED GtkButton *button, Helia *helia )
{
	if ( gtk_widget_get_visible ( GTK_WIDGET ( helia->search_entry_tv ) ) )
	{
		gtk_entry_set_text ( GTK_ENTRY ( helia->search_entry_tv ), "" );
		gtk_widget_hide ( GTK_WIDGET ( helia->search_entry_tv ) );
	}
	else
	{
		gtk_widget_show ( GTK_WIDGET ( helia->search_entry_tv ) );
		gtk_widget_grab_focus ( GTK_WIDGET ( helia->search_entry_tv ) );
	}
}

static void helia_panel_list_rept ( GtkButton *button, Helia *helia )
{
	helia->repeat = !helia->repeat;

	GdkPixbuf *pixbuf = gtk_icon_theme_load_icon ( gtk_icon_theme_get_default (), 
				helia->repeat ? "helia-set" : "helia-repeat", 16, GTK_ICON_LOOKUP_USE_BUILTIN, NULL );

	GtkImage *image = (GtkImage  *)gtk_image_new_from_pixbuf ( pixbuf );

	gtk_button_set_image ( button, GTK_WIDGET ( image ) );

	if ( pixbuf ) g_object_unref ( pixbuf );
}

static void helia_panel_list_scan ( G_GNUC_UNUSED GtkButton *button, Helia *helia )
{
	helia_scan_win_create ( helia );
}

static void helia_panel_list_save_mp ( G_GNUC_UNUSED GtkButton *button, Helia *helia )
{
	helia_treeview_win_save_mp ( helia->treeview_mp, helia->window );
}

static void helia_panel_list_save_tv ( G_GNUC_UNUSED GtkButton *button, Helia *helia )
{
	helia_treeview_win_save_tv ( helia->treeview_tv, helia->window );
}

static void helia_panel_list_close_mp ( G_GNUC_UNUSED GtkButton *button, Helia *helia )
{
	gtk_widget_hide ( GTK_WIDGET ( helia->list_mp ) );
}

static void helia_panel_list_close_tv ( G_GNUC_UNUSED GtkButton *button, Helia *helia )
{
	gtk_widget_hide ( GTK_WIDGET ( helia->list_tv ) );
}

static GtkBox * helia_panel_list_box ( Helia *helia, gboolean mp_tv )
{
	GtkBox *h_box  = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
	gtk_box_set_spacing ( h_box,  5 );

	gtk_widget_set_margin_start  ( GTK_WIDGET ( h_box ), 5 );
	gtk_widget_set_margin_end    ( GTK_WIDGET ( h_box ), 5 );
	gtk_widget_set_margin_top    ( GTK_WIDGET ( h_box ), 5 );
	/*if ( !mp_tv )*/ gtk_widget_set_margin_bottom ( GTK_WIDGET ( h_box ), 5 );

	const char *name = ( mp_tv ) ? "helia-repeat" : "helia-display";

	IconFunc IconFunc_n[] =
	{
		{ "helia-search", 	FALSE, ( mp_tv ) ? helia_panel_search_mp     : helia_panel_search_tv     },
		{ name, 			FALSE, ( mp_tv ) ? helia_panel_list_rept     : helia_panel_list_scan     },
		{ "helia-save", 	FALSE, ( mp_tv ) ? helia_panel_list_save_mp  : helia_panel_list_save_tv  },
		{ "helia-exit", 	FALSE, ( mp_tv ) ? helia_panel_list_close_mp : helia_panel_list_close_tv }
	};

	uint i = 0;
	for ( i = 0; i < G_N_ELEMENTS ( IconFunc_n ); i++ )
	{
		GtkButton *button = helia_set_image_button ( IconFunc_n[i].name, 16 );

		g_signal_connect ( button, "clicked", G_CALLBACK ( IconFunc_n[i].f ), helia );

		gtk_box_pack_start ( h_box, GTK_WIDGET ( button ), TRUE, TRUE, 0 );
	}

	return h_box;
}

static GtkBox * helia_panel_list_buf ( Helia *helia )
{
	GtkBox *h_box  = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );

	gtk_box_set_spacing ( h_box,  5 );
	gtk_widget_set_margin_top   ( GTK_WIDGET ( h_box ), 5 );
	gtk_widget_set_margin_start ( GTK_WIDGET ( h_box ), 5 );
	gtk_widget_set_margin_end   ( GTK_WIDGET ( h_box ), 5 );

	helia->label_buf = (GtkLabel *)gtk_label_new ( " ⇄ 0% " );
	gtk_widget_set_size_request ( GTK_WIDGET ( helia->label_buf ), 50, -1 );
	gtk_widget_set_halign ( GTK_WIDGET ( helia->label_buf ), GTK_ALIGN_START );
	gtk_box_pack_start ( h_box, GTK_WIDGET ( helia->label_buf ), TRUE, TRUE, 0 );

	helia->label_rec = (GtkLabel *)gtk_label_new ( " " );
	gtk_widget_set_size_request ( GTK_WIDGET ( helia->label_rec ), 50, -1 );
	gtk_widget_set_halign ( GTK_WIDGET ( helia->label_rec ), GTK_ALIGN_START );
	gtk_box_pack_start ( h_box, GTK_WIDGET ( helia->label_rec ), TRUE, TRUE, 0 );

	GtkLabel *label = (GtkLabel *)gtk_label_new ( " " );
	gtk_widget_set_size_request ( GTK_WIDGET ( label ), 105, -1 );
	gtk_box_pack_start ( h_box, GTK_WIDGET ( label ), TRUE, TRUE, 0 );

	return h_box;
}

static void helia_search_entry_changed ( G_GNUC_UNUSED GtkEntry *entry, GtkTreeView *tree_view )
{
	GtkTreeModel *model_filter = gtk_tree_view_get_model ( tree_view );

	gtk_tree_model_filter_refilter ( GTK_TREE_MODEL_FILTER ( model_filter ) );
}

static GtkEntry * helia_panel_list_search_entry ( GtkTreeView *tree_view )
{
	GtkEntry *search_entry = (GtkEntry *)gtk_search_entry_new ();

	gtk_widget_set_margin_start ( GTK_WIDGET ( search_entry ), 5 );
	gtk_widget_set_margin_end   ( GTK_WIDGET ( search_entry ), 5 );

	g_signal_connect ( search_entry, "changed", G_CALLBACK ( helia_search_entry_changed ), tree_view );

	return search_entry;
}

HeliaList * helia_list_new ( Helia *helia, gboolean mp_tv )
{
	GtkBox *vbox = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 );

	if ( mp_tv )
	{
		helia->search_entry_mp = helia_panel_list_search_entry ( helia->treeview_mp );
		gtk_box_pack_start ( vbox, GTK_WIDGET ( helia->search_entry_mp ), FALSE,  FALSE, 5 );
	}
	else
	{
		helia->search_entry_tv = helia_panel_list_search_entry ( helia->treeview_tv );
		gtk_box_pack_start ( vbox, GTK_WIDGET ( helia->search_entry_tv ), FALSE,  FALSE, 5 );
	}

	gtk_box_pack_start ( vbox, GTK_WIDGET ( helia_create_scroll ( ( mp_tv ) ? helia->treeview_mp : helia->treeview_tv, 220 ) ), TRUE, TRUE, 0 );

	gtk_box_pack_end ( vbox, GTK_WIDGET ( helia_panel_list_box ( helia, mp_tv ) ), FALSE, FALSE, 0 );

	gtk_box_pack_end ( vbox, GTK_WIDGET ( helia_panel_treeview_new ( ( mp_tv ) ? helia->treeview_mp : helia->treeview_tv ) ), FALSE, FALSE, 0 );

	if ( mp_tv )
		gtk_box_pack_end ( vbox, GTK_WIDGET ( helia_panel_list_buf ( helia ) ), FALSE, FALSE, 0 );
	else
		gtk_box_pack_end ( vbox, GTK_WIDGET ( helia->level ), FALSE, FALSE, 0 );

	return (HeliaList *)vbox;
}

