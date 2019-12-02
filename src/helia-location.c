/*
* Copyright 2019 Stepan Perun
* This program is free software.
*
* License: Gnu Lesser General Public License
* http://www.gnu.org/licenses/lgpl.html
*/


#include <helia-include.h>


static void helia_open_net_play ( const char *file, Helia *helia )
{
	if ( file && strlen ( file ) > 0 )
		helia_treeview_add_file ( helia, file, TRUE );
}
static void helia_open_net_entry_activate ( GtkEntry *entry, Helia *helia )
{
	helia_open_net_play ( gtk_entry_get_text ( entry ), helia );
}
static void helia_open_net_button_activate ( G_GNUC_UNUSED GtkButton *button, Helia *helia )
{
	helia_open_net_play ( gtk_entry_get_text ( helia->net_entry ), helia );
}

static void helia_open_net_clear ( GtkEntry *entry, G_GNUC_UNUSED GtkEntryIconPosition icon_pos, G_GNUC_UNUSED GdkEvent *event, G_GNUC_UNUSED gpointer data )
{
	gtk_entry_set_text ( GTK_ENTRY ( entry ), "" );
}

void helia_open_net ( Helia *helia )
{
	GtkWindow *window = helia_create_window_top ( helia->window, "", "helia-net", GTK_WIN_POS_CENTER_ON_PARENT, TRUE );

	GtkBox *m_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_VERTICAL,   0 );

	GtkBox *h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
	gtk_box_set_spacing ( h_box, 10 );

	GtkImage *image = helia_create_image ( "helia-net", 24 );
	gtk_widget_set_halign ( GTK_WIDGET ( image ), GTK_ALIGN_START );

	GtkEntry *entry = (GtkEntry *)gtk_entry_new ();
	gtk_entry_set_icon_from_icon_name ( entry, GTK_ENTRY_ICON_SECONDARY, "helia-clear" );
	g_signal_connect ( entry, "icon-press", G_CALLBACK ( helia_open_net_clear ), NULL );
	g_signal_connect ( entry, "activate", G_CALLBACK ( helia_open_net_entry_activate ), helia );
	g_signal_connect_swapped ( entry, "activate", G_CALLBACK ( gtk_widget_destroy ), window );

	gtk_widget_set_size_request ( GTK_WIDGET (entry), 400, -1 );

	helia->net_entry = entry;

	GtkButton *button_activate = helia_set_image_button ( "helia-ok", 16 );
	g_signal_connect ( button_activate, "clicked", G_CALLBACK ( helia_open_net_button_activate ), helia );
	g_signal_connect_swapped ( button_activate, "clicked", G_CALLBACK ( gtk_widget_destroy ), window );

	gtk_box_pack_start ( h_box, GTK_WIDGET ( image ), FALSE, FALSE, 0 );
	gtk_box_pack_start ( GTK_BOX ( h_box ), GTK_WIDGET ( entry ), TRUE, TRUE, 0 );
	gtk_box_pack_start ( h_box, GTK_WIDGET ( button_activate ), TRUE, TRUE, 0 );

	GtkButton *button_close = helia_set_image_button ( "helia-exit", 16 );
	g_signal_connect_swapped ( button_close, "clicked", G_CALLBACK ( gtk_widget_destroy ), window );

	gtk_box_pack_start ( h_box, GTK_WIDGET ( button_close ), TRUE, TRUE, 0 );

	gtk_box_pack_end ( m_box, GTK_WIDGET ( h_box ), FALSE, FALSE, 5 );

	gtk_container_set_border_width ( GTK_CONTAINER ( m_box ), 10 );
	gtk_container_add   ( GTK_CONTAINER ( window ), GTK_WIDGET ( m_box ) );
	gtk_widget_show_all ( GTK_WIDGET ( window ) );

	gtk_widget_set_opacity ( GTK_WIDGET ( window ), helia->opacity_window );
}

/* Returns a newly-allocated string holding the result. Free with free() */
char * helia_open_file ( Helia *helia, const char *path )
{
	GtkFileChooserDialog *dialog = ( GtkFileChooserDialog *)gtk_file_chooser_dialog_new (
					" ",  helia->window, GTK_FILE_CHOOSER_ACTION_OPEN,
					"gtk-cancel", GTK_RESPONSE_CANCEL,
					"gtk-open",   GTK_RESPONSE_ACCEPT,
					NULL );

	gtk_window_set_icon_name ( GTK_WINDOW ( dialog ), "document-open" );

	gtk_file_chooser_set_current_folder  ( GTK_FILE_CHOOSER ( dialog ), path );
	gtk_file_chooser_set_select_multiple ( GTK_FILE_CHOOSER ( dialog ), FALSE );

	char *filename = NULL;

	if ( gtk_dialog_run ( GTK_DIALOG ( dialog ) ) == GTK_RESPONSE_ACCEPT )
		filename = gtk_file_chooser_get_filename ( GTK_FILE_CHOOSER ( dialog ) );

	gtk_widget_destroy ( GTK_WIDGET ( dialog ) );

	return filename;
}

/* Returns a GSList containing the filenames. Free the returned list with g_slist_free(), and the filenames with g_free(). */
static GSList * helia_open_files ( Helia *helia, const char *path )
{
	GtkFileChooserDialog *dialog = ( GtkFileChooserDialog *)gtk_file_chooser_dialog_new (
					" ",  helia->window, GTK_FILE_CHOOSER_ACTION_OPEN,
					"gtk-cancel", GTK_RESPONSE_CANCEL,
					"gtk-open",   GTK_RESPONSE_ACCEPT,
					NULL );

	gtk_window_set_icon_name ( GTK_WINDOW ( dialog ), "document-open" );

	gtk_file_chooser_set_current_folder  ( GTK_FILE_CHOOSER ( dialog ), path );
	gtk_file_chooser_set_select_multiple ( GTK_FILE_CHOOSER ( dialog ), TRUE );

	GSList *files = NULL;

	if ( gtk_dialog_run ( GTK_DIALOG ( dialog ) ) == GTK_RESPONSE_ACCEPT )
		files = gtk_file_chooser_get_filenames ( GTK_FILE_CHOOSER ( dialog ) );

	gtk_widget_destroy ( GTK_WIDGET ( dialog ) );

	return files;
}

/* Returns a newly-allocated string holding the result. Free with free() */
char * helia_open_dir ( Helia *helia, const char *path )
{
	GtkFileChooserDialog *dialog = ( GtkFileChooserDialog *)gtk_file_chooser_dialog_new (
					" ",  helia->window, GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
					"gtk-cancel", GTK_RESPONSE_CANCEL,
					"gtk-apply",  GTK_RESPONSE_ACCEPT,
					NULL );

	gtk_window_set_icon_name ( GTK_WINDOW ( dialog ), "folder-open" );

	gtk_file_chooser_set_current_folder ( GTK_FILE_CHOOSER ( dialog ), path );

	char *dirname = NULL;

	if ( gtk_dialog_run ( GTK_DIALOG ( dialog ) ) == GTK_RESPONSE_ACCEPT )
		dirname = gtk_file_chooser_get_filename ( GTK_FILE_CHOOSER ( dialog ) );

	gtk_widget_destroy ( GTK_WIDGET ( dialog ) );

	return dirname;
}

void helia_dialog_open_dir ( Helia *helia )
{
	char *path = helia_open_dir ( helia, g_get_home_dir () );

		if ( path == NULL ) return;

		helia_treeview_add_dir ( helia, path );

	g_free ( path );
}

void helia_dialog_open_files ( Helia *helia )
{
	GSList *files = helia_open_files ( helia, g_get_home_dir () );
	uint i = 0;

	if ( files == NULL ) return;

	while ( files != NULL )
	{
		helia_treeview_add_file ( helia, files->data, ( i == 0 ) ? TRUE : FALSE );

		files = files->next;
		i++;
	}

	g_slist_free_full ( files, (GDestroyNotify) g_free );
}
