/*
* Copyright 2019 Stepan Perun
* This program is free software.
*
* License: Gnu Lesser General Public License
* http://www.gnu.org/licenses/lgpl.html
*/


#include <helia-include.h>


/* Returns a newly-allocated string holding the result. Free with free() */
char * helia_uri_get_path ( const char *uri )
{
	char *path = NULL;

	GFile *file = g_file_new_for_uri ( uri );

		path = g_file_get_path ( file );

	g_object_unref ( file );

	return path;
}

void helia_treeview_next_play ( Helia *helia )
{
	if ( helia->repeat )
	{
		gst_element_set_state ( helia->player, GST_STATE_NULL    );
		gst_element_set_state ( helia->player, GST_STATE_PLAYING );

		return;
	}

	helia_player_stop ( helia );

	GtkTreeModel *model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( helia->treeview_mp ) );
	int indx = gtk_tree_model_iter_n_children ( model, NULL );

	if ( indx < 2 ) { helia_player_stop ( helia ); return; }

	GtkTreeIter iter;
	gboolean valid, break_f = FALSE;

	for ( valid = gtk_tree_model_get_iter_first ( model, &iter ); valid;
		valid = gtk_tree_model_iter_next ( model, &iter ) )
	{
		char *data;
		gtk_tree_model_get ( model, &iter, COL_DATA,  &data, -1 );

		if ( g_str_has_suffix ( helia->file_ch, data ) )
		{
			if ( gtk_tree_model_iter_next ( model, &iter ) )
			{
				char *data2;
				gtk_tree_model_get ( model, &iter, COL_DATA, &data2, -1 );

				helia_player_stop_set_play ( helia, data2 );

				gtk_tree_selection_select_iter ( gtk_tree_view_get_selection ( helia->treeview_mp ), &iter );

				g_free ( data2 );
			}

			break_f = TRUE;
		}

		g_free ( data );
		if ( break_f ) break;
	}
}

static void helia_treeview_to_file ( GtkTreeView *tree_view, const char *filename, gboolean mp_tv )
{
	GString *gstring = g_string_new ( ( mp_tv ) ? "#EXTM3U \n" : "# Gtv-Dvb channel format \n" );

	GtkTreeIter iter;
	GtkTreeModel *model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );

	gboolean valid;
	for ( valid = gtk_tree_model_get_iter_first ( model, &iter ); valid;
		  valid = gtk_tree_model_iter_next ( model, &iter ) )
	{
		char *data = NULL;
		char *name = NULL;

		gtk_tree_model_get ( model, &iter, COL_DATA,  &data, -1 );
		gtk_tree_model_get ( model, &iter, COL_FL_CH, &name, -1 );

		if ( mp_tv ) g_string_append_printf ( gstring, "#EXTINF:-1,%s\n", name );

		g_string_append_printf ( gstring, "%s\n", data );

		g_free ( name );
		g_free ( data );
	}

	GError *err = NULL;

	if ( !g_file_set_contents ( filename, gstring->str, -1, &err ) )
	{
		g_critical ( "%s: %s ", __func__, err->message );

		g_error_free ( err );
	}

	g_string_free ( gstring, TRUE );
}

static void helia_dialod_add_filter ( GtkFileChooserDialog *dialog, const char *name, const char *filter_set )
{
	GtkFileFilter *filter = gtk_file_filter_new ();

	gtk_file_filter_set_name ( filter, name );
	gtk_file_filter_add_pattern ( filter, filter_set );
	gtk_file_chooser_add_filter ( GTK_FILE_CHOOSER ( dialog ), filter );
}

static void helia_treeview_save ( GtkTreeView *tree_view, const char *dir, const char *file, GtkWindow *window, gboolean mp_tv )
{
	GtkTreeModel *model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );
	int ind = gtk_tree_model_iter_n_children ( model, NULL );

	if ( ind == 0 ) return;

	GtkFileChooserDialog *dialog = ( GtkFileChooserDialog *)gtk_file_chooser_dialog_new (
					" ", window,   GTK_FILE_CHOOSER_ACTION_SAVE,
					"gtk-cancel",  GTK_RESPONSE_CANCEL,
					"gtk-save",    GTK_RESPONSE_ACCEPT,
					NULL );

	if ( mp_tv )
		helia_dialod_add_filter ( dialog, "m3u",  "*.m3u"  );
	else
		helia_dialod_add_filter ( dialog, "conf", "*.conf" );

	gtk_window_set_icon_name ( GTK_WINDOW ( dialog ), "document-save" );

	gtk_file_chooser_set_current_folder ( GTK_FILE_CHOOSER ( dialog ), dir );
	gtk_file_chooser_set_do_overwrite_confirmation ( GTK_FILE_CHOOSER ( dialog ), TRUE );
	gtk_file_chooser_set_current_name   ( GTK_FILE_CHOOSER ( dialog ), file );

	if ( gtk_dialog_run ( GTK_DIALOG ( dialog ) ) == GTK_RESPONSE_ACCEPT )
	{
		char *filename = gtk_file_chooser_get_filename ( GTK_FILE_CHOOSER ( dialog ) );

			helia_treeview_to_file ( tree_view, filename, mp_tv );

		g_free ( filename );
	}

	gtk_widget_destroy ( GTK_WIDGET ( dialog ) );
}

void helia_treeview_win_save_mp ( GtkTreeView *tree_view, GtkWindow *window )
{
	helia_treeview_save ( tree_view, g_get_home_dir (), "playlist-001.m3u", window, TRUE );
}

void helia_treeview_win_save_tv ( GtkTreeView *tree_view, GtkWindow *window )
{
	helia_treeview_save ( tree_view, g_get_home_dir (), "gtv-channel.conf", window, FALSE );
}

void helia_treeview_auto_save_tv ( GtkTreeView *tree_view, const char *filename )
{
	helia_treeview_to_file ( tree_view, filename, FALSE );
}


static void helia_treeview_append ( Helia *helia, HeliaTreeview *treeview, const char *name, const char *data, gboolean play )
{
	GtkTreeIter iter;

	GtkTreeModel *model_filter = gtk_tree_view_get_model ( GTK_TREE_VIEW ( treeview ) );
	GtkTreeModel *model = gtk_tree_model_filter_get_model ( GTK_TREE_MODEL_FILTER ( model_filter ) );

	uint ind = gtk_tree_model_iter_n_children ( model, NULL );

	gtk_list_store_append ( GTK_LIST_STORE ( model ), &iter );
	gtk_list_store_set    ( GTK_LIST_STORE ( model ), &iter,
				COL_NUM, ind + 1,
				COL_FL_CH, name,
				COL_DATA,  data,
				-1 );

	if ( play )
	{
		if ( helia->pipeline_rec == NULL && helia->player && GST_ELEMENT_CAST ( helia->player )->current_state == GST_STATE_NULL )
		{
			helia_player_stop_set_play ( helia, data );

			GtkTreePath *path = gtk_tree_model_get_path ( model, &iter );
			gtk_tree_selection_select_path ( gtk_tree_view_get_selection ( GTK_TREE_VIEW ( treeview ) ), path );
			gtk_tree_path_free ( path );
		}
	}
}

static void helia_treeview_add_m3u ( Helia *helia, const char *file )
{
	char  *contents = NULL;
	GError *err     = NULL;

	if ( g_file_get_contents ( file, &contents, 0, &err ) )
	{
		char **lines = g_strsplit ( contents, "\n", 0 );

		uint i = 0; for ( i = 0; lines[i] != NULL; i++ )
		//for ( i = 0; lines[i] != NULL && *lines[i]; i++ )
		{
			if ( g_str_has_prefix ( lines[i], "#EXTM3U" ) || g_str_has_prefix ( lines[i], " " ) || strlen ( lines[i] ) < 4 ) continue;

			if ( g_str_has_prefix ( lines[i], "#EXTINF" ) )
			{
				char **lines_info = g_strsplit ( lines[i], ",", 0 );

					if ( g_str_has_prefix ( lines[i+1], "#EXTGRP" ) ) i++;

					helia_treeview_append ( helia, helia->treeview_mp, g_strstrip ( lines_info[1] ), g_strstrip ( lines[i+1] ), FALSE );

				g_strfreev ( lines_info );
				i++;
			}
			else
			{
				if ( g_str_has_prefix ( lines[i], "#" ) || g_str_has_prefix ( lines[i], " " ) || strlen ( lines[i] ) < 4 ) continue;

				char *name = g_path_get_basename ( lines[i] );

					helia_treeview_append ( helia, helia->treeview_mp, g_strstrip ( name ), g_strstrip ( lines[i] ), FALSE );

				g_free ( name );
			}
		}

		g_strfreev ( lines );
		free ( contents );
	}
	else
	{
		g_debug ( "%s:: ERROR: %s ", __func__, err->message );

		g_error_free ( err );
	}
}

static gboolean helia_treeview_media_filter ( const char *file_name )
{
	gboolean res  = FALSE;
	GError *error = NULL;

	GFile *file = g_file_new_for_path ( file_name );
	GFileInfo *file_info = g_file_query_info ( file, "standard::*", 0, NULL, &error );

	const char *content_type = g_file_info_get_content_type ( file_info );

	if ( g_str_has_prefix ( content_type, "audio" ) || g_str_has_prefix ( content_type, "video" ) ) res =  TRUE;

	g_object_unref ( file_info );
	g_object_unref ( file );

	return res;
}

void helia_treeview_add_file ( Helia *helia, const char *path, gboolean play )
{
	char *name_down = g_utf8_strdown ( path, -1 );

	if ( g_str_has_suffix ( name_down, "m3u" ) )
	{
		helia_treeview_add_m3u ( helia, path );
	}
	else
	{
		char *basename = g_path_get_basename ( path );

			helia_treeview_append ( helia, helia->treeview_mp, basename, path, play );

		free ( basename );
	}

	free ( name_down );
}

static void helia_treeview_add_arg ( GFile **files, int n_files, Helia *helia )
{
	int i = 0; for ( i = 0; i < n_files; i++ )
	{
		char *path = g_file_get_path ( files[i] );

		if ( path && g_file_test ( path, G_FILE_TEST_IS_DIR ) )
			helia_treeview_add_dir ( helia, path );

		if ( path && g_file_test ( path, G_FILE_TEST_IS_REGULAR ) )
			helia_treeview_add_file ( helia, path, ( i == 0 ) ? TRUE : FALSE );

		free ( path );
	}
}

static int _sort_func_list ( gconstpointer a, gconstpointer b )
{
	return g_utf8_collate ( a, b );
}

static void helia_treeview_slist_sort ( GList *list, Helia *helia )
{
	GList *list_sort = g_list_sort ( list, _sort_func_list );
	uint i = 0;

	while ( list_sort != NULL )
	{
		helia_treeview_add_file ( helia, (char *)list_sort->data, ( i == 0 ) ? TRUE : FALSE );

		list_sort = list_sort->next;
		i++;
	}

	g_list_free_full ( list_sort, (GDestroyNotify) g_free );
}

void helia_treeview_add_dir ( Helia *helia, const char *dir_path )
{
	GDir *dir = g_dir_open ( dir_path, 0, NULL );

	GList *list = NULL;

	if ( dir )
	{
		const char *name = NULL;

		while ( ( name = g_dir_read_name ( dir ) ) != NULL )
		{
			char *path_name = g_strconcat ( dir_path, "/", name, NULL );

			if ( g_file_test ( path_name, G_FILE_TEST_IS_DIR ) )
				helia_treeview_add_dir ( helia, path_name ); // Recursion!

			if ( g_file_test ( path_name, G_FILE_TEST_IS_REGULAR ) )
				if ( helia_treeview_media_filter ( path_name ) )
					list = g_list_append ( list, g_strdup ( path_name ) );

			g_free ( path_name );
		}

		g_dir_close ( dir );
	}
	else
	{
		g_critical ( "%s: opening directory %s failed.", __func__, dir_path );
	}

	helia_treeview_slist_sort ( list, helia );

	g_list_free_full ( list, (GDestroyNotify) g_free );
}

void helia_treeview_add_channels ( Helia *helia, const char *file )
{
	char  *contents = NULL;
	GError *err     = NULL;

	if ( g_file_get_contents ( file, &contents, 0, &err ) )
	{
		char **lines = g_strsplit ( contents, "\n", 0 );

		uint i = 0; for ( i = 0; lines[i] != NULL; i++ )
		// for ( i = 0; lines[i] != NULL && *lines[i]; i++ )
		{
			if ( g_str_has_prefix ( lines[i], "#" ) || strlen ( lines[i] ) < 2 ) continue;

			char **data = g_strsplit ( lines[i], ":", 0 );

				helia_treeview_append ( helia, helia->treeview_tv, data[0], lines[i], FALSE );

			g_strfreev ( data );
		}

		g_strfreev ( lines );
		free ( contents );
	}
	else
	{
		g_debug ( "%s:: ERROR: %s ", __func__, err->message );

		g_error_free ( err );
	}
}

static void helia_treeview_start_channel ( const char *channel, Helia *helia )
{
	GtkTreeModel *model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( helia->treeview_tv ) );
	int indx = gtk_tree_model_iter_n_children ( model, NULL );

	if ( indx == 0 ) return;

	GtkTreeIter iter;
	gboolean valid, break_f = FALSE;

	for ( valid = gtk_tree_model_get_iter_first ( model, &iter ); valid;
		valid = gtk_tree_model_iter_next ( model, &iter ) )
	{
		char *data;
		gtk_tree_model_get ( model, &iter, COL_DATA,  &data, -1 );

		if ( g_str_has_prefix ( data, channel ) )
		{
			helia_dtv_stop_set_play ( helia, data );

			break_f = TRUE;
		}

		g_free ( data );
		if ( break_f ) break;
	}
}

void helia_treeview_add_start ( GFile **files, int n_files, Helia *helia )
{
	if ( g_file_test ( helia->ch_conf, G_FILE_TEST_EXISTS ) )
		helia_treeview_add_channels ( helia, helia->ch_conf );

	if ( n_files == 0 ) return;

	char *ch_prop = g_file_get_basename ( files[0] );

	if ( ch_prop && g_str_has_prefix ( ch_prop, "channel" ) )
	{
		helia_window_set_win_tv ( NULL, helia );

		if ( n_files == 2 )
		{
			char *channel = g_file_get_basename ( files[1] );

			if ( channel ) helia_treeview_start_channel ( channel, helia );

			g_debug ( "%s:: %s = %s ", __func__, ch_prop, channel );

			g_free ( channel );
		}
	}
	else
	{
		helia_window_set_win_mp ( NULL, helia );
		helia_treeview_add_arg ( files, n_files, helia );
	}

	g_free ( ch_prop );
}

static void helia_treeview_row_activated_mp ( GtkTreeView *tree_view, GtkTreePath *path, G_GNUC_UNUSED GtkTreeViewColumn *column, Helia *helia )
{
	GtkTreeIter iter;
	GtkTreeModel *model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );

	if ( gtk_tree_model_get_iter ( model, &iter, path ) )
	{
		char *data = NULL;

		gtk_tree_model_get ( model, &iter, COL_DATA, &data, -1 );

		helia_player_stop_set_play ( helia, data );

		free ( data );
	}
}
static void helia_treeview_row_activated_tv ( GtkTreeView *tree_view, GtkTreePath *path, G_GNUC_UNUSED GtkTreeViewColumn *column, Helia *helia )
{
	GtkTreeIter iter;
	GtkTreeModel *model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );

	if ( gtk_tree_model_get_iter ( model, &iter, path ) )
	{
		char *data = NULL;

		gtk_tree_model_get ( model, &iter, COL_DATA, &data, -1 );

		helia_dtv_stop_set_play ( helia, data );

		free ( data );
	}
}

static void helia_treeview_create_columns ( GtkTreeView *tree_view, const char *name, int column_id, gboolean col_vis )
{
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new ();

	GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes ( name, renderer, "text", column_id, NULL );

	gtk_tree_view_append_column ( tree_view, column );
	gtk_tree_view_column_set_visible ( column, col_vis );
}

static void helia_treeview_add_columns ( GtkTreeView *tree_view, const char *title )
{
	struct col_title_list { const char *title; gboolean vis; } col_title_list_n[] = 
	{
		{ " № ",  TRUE  },
		{ title,  TRUE  },
		{ "Data", FALSE }
	};

	uint c = 0; for ( c = 0; c < NUM_COLS; c++ )
	{
		helia_treeview_create_columns ( tree_view, col_title_list_n[c].title, c, col_title_list_n[c].vis );
	}
}

static gboolean search_entry_row_strdown_visible ( const char *name, const char *text )
{
	gboolean visible = FALSE;

	char *name_strdown = g_utf8_strdown ( name, -1 );
	char *text_strdown = g_utf8_strdown ( text, -1 );

	if ( /*name_strdown && text_strdown &&*/ g_strrstr ( name_strdown, text_strdown ) ) visible = TRUE;

	g_free ( name_strdown );
	g_free ( text_strdown );

	return visible;
}
static gboolean search_entry_row_all_visible ( GtkTreeModel *model, GtkTreeIter *iter, const char *text )
{
	gboolean visible = FALSE;

	char *name = NULL;

	gtk_tree_model_get ( model, iter, COL_FL_CH, &name, -1 );

	if ( name && g_strrstr ( name, text ) ) visible = TRUE;

	if ( name && visible == FALSE ) visible = search_entry_row_strdown_visible ( name, text );

	g_free ( name );

	return visible;
}
static gboolean search_entry_row_mp_visible ( GtkTreeModel *model, GtkTreeIter *iter, Helia *helia )
{
	gboolean visible = FALSE;

	const char *text = gtk_entry_get_text ( helia->search_entry_mp );

	if ( gtk_entry_get_text_length ( helia->search_entry_mp ) > 0 )
		visible = search_entry_row_all_visible ( model, iter, text );
	else
		visible = TRUE;

	return visible;
}
static gboolean search_entry_row_tv_visible ( GtkTreeModel *model, GtkTreeIter *iter, Helia *helia )
{
	gboolean visible = FALSE;

	const char *text = gtk_entry_get_text ( helia->search_entry_tv );

	if ( gtk_entry_get_text_length ( helia->search_entry_tv ) > 0 )
		visible = search_entry_row_all_visible ( model, iter, text );
	else
		visible = TRUE;

	return visible;
}

HeliaTreeview * helia_treeview_new ( Helia *helia, gboolean mp_tv )
{
	GtkListStore *store = (GtkListStore *)gtk_list_store_new ( 3, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_STRING );

	GtkTreeModelFilter *filtered = GTK_TREE_MODEL_FILTER ( gtk_tree_model_filter_new ( GTK_TREE_MODEL ( store ), NULL ) );

	if ( mp_tv )
		gtk_tree_model_filter_set_visible_func ( filtered, (GtkTreeModelFilterVisibleFunc)search_entry_row_mp_visible, helia, NULL );
	else
		gtk_tree_model_filter_set_visible_func ( filtered, (GtkTreeModelFilterVisibleFunc)search_entry_row_tv_visible, helia, NULL );

	GtkTreeView *treeview = (GtkTreeView *)gtk_tree_view_new_with_model ( GTK_TREE_MODEL ( filtered ) );

	gtk_tree_view_set_search_column ( treeview, COL_FL_CH );

	helia_treeview_add_columns ( treeview, ( mp_tv ) ? _i18n_ ( "Files" ) : _i18n_ ( "Channels" ) );

	if ( mp_tv )
		g_signal_connect ( treeview, "row-activated", G_CALLBACK ( helia_treeview_row_activated_mp ), helia );
	else
		g_signal_connect ( treeview, "row-activated", G_CALLBACK ( helia_treeview_row_activated_tv ), helia );

    return treeview;
}
