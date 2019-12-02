/*
* Copyright 2019 Stepan Perun
* This program is free software.
*
* License: Gnu Lesser General Public License
* http://www.gnu.org/licenses/lgpl.html
*/


#include <base.h>

#include "tree-view.h"
#include "lang.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


static char * pref_get_prop ( const char *prop );


void about_win ( GtkWindow *window )
{
	GtkAboutDialog *dialog = (GtkAboutDialog *)gtk_about_dialog_new ();
	gtk_window_set_transient_for ( (GtkWindow *)dialog, window );

	const char *authors[]   = { "Stepan Perun",   " ", NULL };
	const char *artists[]   = { "Itzik Gur",      " ", NULL };
	const char *translators = "Anton Midyukov \nHeimen Stoffels \nPavel Fric \nMartin Gansser \n";
	const char *license     = "This program is free software. \n\nGNU Lesser General Public License \nwww.gnu.org/licenses/lgpl.html";

	gtk_about_dialog_set_program_name ( dialog, "Helia" );
	gtk_about_dialog_set_version ( dialog, "9.9" );
	gtk_about_dialog_set_license ( dialog, license );
	gtk_about_dialog_set_authors ( dialog, authors );
	gtk_about_dialog_set_artists ( dialog, artists );
	gtk_about_dialog_set_translator_credits ( dialog, translators );
	gtk_about_dialog_set_website ( dialog,   "https://www.opencode.net/vl-nix/helia" );
	gtk_about_dialog_set_copyright ( dialog, "Copyright 2019 Helia" );
	gtk_about_dialog_set_comments  ( dialog, "Media Player & IPTV & Digital TV \nDVB-T2/S2/C, ATSC, DTMB, ISDB" );

	gtk_about_dialog_set_logo_icon_name ( dialog, "helia-logo" );

	gtk_dialog_run ( GTK_DIALOG (dialog) );

	gtk_widget_destroy ( GTK_WIDGET (dialog) );
}

static void win_open_net_play ( const char *file, Base *base )
{
	if ( file && strlen ( file ) > 0 )
		treeview_add_file ( base, file, FALSE, TRUE );
}
static void win_open_net_entry_activate ( GtkEntry *entry, Base *base )
{
	win_open_net_play ( gtk_entry_get_text ( entry ), base );
}
static void win_open_net_button_activate ( G_GNUC_UNUSED GtkButton *button, Base *base )
{
	win_open_net_play ( gtk_entry_get_text ( base->player->net_entry ), base );
}

static void win_open_net_clear ( GtkEntry *entry, G_GNUC_UNUSED GtkEntryIconPosition icon_pos, G_GNUC_UNUSED GdkEvent *event, G_GNUC_UNUSED gpointer data )
{
	gtk_entry_set_text ( GTK_ENTRY ( entry ), "" );
}

void win_open_net ( Base *base )
{
	GtkWindow *window =      (GtkWindow *)gtk_window_new ( GTK_WINDOW_TOPLEVEL );
	gtk_window_set_transient_for ( window, base->window );
	gtk_window_set_modal     ( window, TRUE );
	gtk_window_set_position  ( window, GTK_WIN_POS_CENTER_ON_PARENT );
	gtk_window_set_title     ( window, "" );

	GdkPixbuf *pixbuf = gtk_icon_theme_load_icon ( gtk_icon_theme_get_default (), 
					    "helia-net", 48, GTK_ICON_LOOKUP_USE_BUILTIN, NULL );

	gtk_window_set_icon ( window, pixbuf );

	if ( pixbuf ) g_object_unref ( pixbuf );

	GtkBox *m_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_VERTICAL,   0 );

	GtkBox *h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
	gtk_box_set_spacing ( h_box, 10 );

	GtkEntry *entry = (GtkEntry *)gtk_entry_new ();
	gtk_entry_set_icon_from_icon_name ( entry, GTK_ENTRY_ICON_SECONDARY, "helia-clear" );
	g_signal_connect ( entry, "icon-press", G_CALLBACK ( win_open_net_clear ), NULL );
	g_signal_connect ( entry, "activate", G_CALLBACK ( win_open_net_entry_activate ), base );
	g_signal_connect_swapped ( entry, "activate", G_CALLBACK ( gtk_widget_destroy ), window );

	gtk_widget_set_size_request ( GTK_WIDGET (entry), 400, -1 );

	base->player->net_entry = entry;

	GtkButton *button_activate = (GtkButton *)gtk_button_new_from_icon_name ( "helia-ok", GTK_ICON_SIZE_BUTTON );
	g_signal_connect ( button_activate, "clicked", G_CALLBACK ( win_open_net_button_activate ), base );
	g_signal_connect_swapped ( button_activate, "clicked", G_CALLBACK ( gtk_widget_destroy ), window );

	gtk_box_pack_start ( GTK_BOX ( h_box ), GTK_WIDGET ( entry ), TRUE, TRUE, 0 );
	gtk_box_pack_start ( h_box, GTK_WIDGET ( button_activate ), TRUE, TRUE, 0 );

	GtkButton *button_close = (GtkButton *)gtk_button_new_from_icon_name ( "helia-exit", GTK_ICON_SIZE_BUTTON );
	g_signal_connect_swapped ( button_close, "clicked", G_CALLBACK ( gtk_widget_destroy ), window );

	gtk_box_pack_start ( h_box, GTK_WIDGET ( button_close ), TRUE, TRUE, 0 );

	gtk_box_pack_end ( m_box, GTK_WIDGET ( h_box ), FALSE, FALSE, 5 );

	gtk_container_set_border_width ( GTK_CONTAINER ( m_box ), 10 );
	gtk_container_add   ( GTK_CONTAINER ( window ), GTK_WIDGET ( m_box ) );
	gtk_widget_show_all ( GTK_WIDGET ( window ) );

	gtk_widget_set_opacity ( GTK_WIDGET ( window ), base->opacity_win );
}

/* Returns a newly-allocated string holding the result. Free with free() */
char * pref_open_file ( Base *base, const char *path )
{
	GtkFileChooserDialog *dialog = ( GtkFileChooserDialog *)gtk_file_chooser_dialog_new (
									" ",  base->window, GTK_FILE_CHOOSER_ACTION_OPEN,
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
GSList * pref_open_files ( Base *base, const char *path )
{
	GtkFileChooserDialog *dialog = ( GtkFileChooserDialog *)gtk_file_chooser_dialog_new (
									" ",  base->window, GTK_FILE_CHOOSER_ACTION_OPEN,
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
char * pref_open_dir ( Base *base, const char *path )
{
	GtkFileChooserDialog *dialog = ( GtkFileChooserDialog *)gtk_file_chooser_dialog_new (
									" ",  base->window, GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
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

void add_arg ( GFile **files, int n_files, Base *base )
{
	int i = 0;

	for ( i = 0; i < n_files; i++ )
	{
		char *path = g_file_get_path ( files[i] );

		if ( g_file_test ( path, G_FILE_TEST_IS_DIR ) )
		{
			treeview_add_dir ( base, path );
		}

		if ( g_file_test ( path, G_FILE_TEST_IS_REGULAR ) )
		{
			treeview_add_file ( base, path, TRUE, ( i == 0 ) ? TRUE : FALSE );
		}

		g_free ( path );
	}	
}

void dialog_open_dir ( Base *base )
{
	char *path = pref_open_dir ( base, g_get_home_dir () );

		if ( path == NULL ) return;

		treeview_add_dir ( base, path );

	g_free ( path );
}

void dialog_open_files ( Base *base )
{
	GSList *files = pref_open_files ( base, g_get_home_dir () );
	uint i = 0;

	if ( files == NULL ) return;

	while ( files != NULL )
	{
		treeview_add_file ( base, files->data, FALSE, ( i == 0 ) ? TRUE : FALSE );

		files = files->next;
		i++;
	}

	g_slist_free_full ( files, (GDestroyNotify) g_free );
}

void pref_read_config ( Base *base )
{
	uint n = 0;
	char *contents;

	GError *err = NULL;

	if ( g_file_get_contents ( base->helia_conf, &contents, 0, &err ) )
	{
		char **lines = g_strsplit ( contents, "\n", 0 );

		for ( n = 0; lines[n] != NULL; n++ )
		{
			if ( !g_strrstr ( lines[n], "=" ) ) continue;

			char **key_val = g_strsplit ( lines[n], "=", 0 );

			if ( g_strrstr ( lines[n], "gtk-theme-name" ) )
				g_object_set ( gtk_settings_get_default (), key_val[0], key_val[1], NULL );

			if ( g_strrstr ( lines[n], "record-dir" ) )
			{
				if ( base->rec_dir ) g_free ( base->rec_dir );
				base->rec_dir = g_strdup ( key_val[1] );
			}

			if ( g_strrstr ( lines[n], "opacity-control" ) )
				base->opacity_panel = (double)( atoi ( key_val[1] ) ) / 100;

			if ( g_strrstr ( lines[n], "opacity-equalizer" ) )
				base->opacity_eq = (double)( atoi ( key_val[1] ) ) / 100;

			if ( g_strrstr ( lines[n], "opacity-window" ) )
				base->opacity_win = (double)( atoi ( key_val[1] ) ) / 100;

			if ( g_strrstr ( lines[n], "resize-icon" ) )
				base->size_icon = atoi ( key_val[1] );

			if ( g_strrstr ( lines[n], "pause-mouse" ) )
				base->pause_mouse = ( atoi ( key_val[1] ) ) ? TRUE : FALSE;

			if ( g_strrstr ( lines[n], "window-width" ) )
				base->win_width = ( atoi ( key_val[1] ) );

			if ( g_strrstr ( lines[n], "window-height" ) )
				base->win_height = ( atoi ( key_val[1] ) );

			if ( g_strrstr ( lines[n], "dark-theme" ) )
			{
				base->dark_theme = ( atoi ( key_val[1] ) ) ? TRUE : FALSE;
				g_object_set ( gtk_settings_get_default(), "gtk-application-prefer-dark-theme", base->dark_theme, NULL );
			}

			// g_print ( "%s:: Set %s -> %s \n", __func__, key_val[0], key_val[1] );

			g_strfreev ( key_val );
		}

		g_strfreev ( lines );
		g_free ( contents );
	}
	else
	{
		g_critical ( "%s:: %s\n", __func__, err->message );
		g_error_free ( err );
	}
}

void pref_save_config ( Base *base )
{
	char *conf_t = pref_get_prop ( "gtk-theme-name" );

	GString *gstring = g_string_new ( "# Gtv-Dvb conf \n" );

	g_string_append_printf ( gstring, "record-dir=%s\n",          base->rec_dir  );
	g_string_append_printf ( gstring, "gtk-theme-name=%s\n",      conf_t  );
	g_string_append_printf ( gstring, "opacity-control=%d\n",     (int)( base->opacity_panel * 100 ) );
	g_string_append_printf ( gstring, "opacity-equalizer=%d\n",   (int)( base->opacity_eq  * 100 ) );
	g_string_append_printf ( gstring, "opacity-window=%d\n",      (int)( base->opacity_win * 100 ) );
	g_string_append_printf ( gstring, "resize-icon=%d\n",         base->size_icon );
	g_string_append_printf ( gstring, "pause-mouse=%d\n",         base->pause_mouse ? 1 : 0 );
	g_string_append_printf ( gstring, "window-width=%d\n",        base->win_width  );
	g_string_append_printf ( gstring, "window-height=%d\n",       base->win_height );
	g_string_append_printf ( gstring, "dark-theme=%d\n",          base->dark_theme ? 1 : 0 );

	GError *err = NULL;

	if ( !g_file_set_contents ( base->helia_conf, gstring->str, -1, &err ) )
	{
		g_critical ( "%s:: %s\n", __func__, err->message );
		g_error_free ( err );
	}

	g_string_free ( gstring, TRUE );

	g_free ( conf_t );
}

static GdkRectangle pref_monitor_get_geometry ()
{
	GdkDisplay *display = gdk_display_get_default ();
    GdkMonitor *monitor = gdk_display_get_primary_monitor ( display );

    GdkRectangle geom;
    gdk_monitor_get_geometry ( monitor, &geom );

	g_debug ( "%s: width %d | height %d ", __func__, geom.width, geom.height );

	return geom;
}
static uint pref_screen_get_width ()
{
	GdkRectangle geom = pref_monitor_get_geometry ();

	return geom.width;
}
static uint pref_screen_get_height ()
{
	GdkRectangle geom = pref_monitor_get_geometry ();

	return geom.height;
}

static char * pref_get_prop ( const char *prop )
{
	char *name = NULL;

	g_object_get ( gtk_settings_get_default (), prop, &name, NULL );

	return name;
}
static void pref_set_prop ( const char *prop, char *path )
{
	char *i_file = g_strconcat ( path, "/index.theme", NULL );

	if ( g_file_test ( i_file, G_FILE_TEST_EXISTS ) )
	{
		char *name = g_path_get_basename ( path );

			g_object_set ( gtk_settings_get_default (), prop, name, NULL );

		g_free ( name );
	}

	g_free ( i_file );
}

static void pref_set_theme ( GtkEntry *entry, G_GNUC_UNUSED GtkEntryIconPosition icon_pos, G_GNUC_UNUSED GdkEvent *event, Base *base )
{
	char *path = pref_open_dir ( base, "/usr/share/themes" );

	if ( path == NULL ) return;

	pref_set_prop ( "gtk-theme-name", path );

	char *name = g_path_get_basename ( path );

		gtk_entry_set_text ( entry, name );

	g_free ( name );

	g_free ( path );
}

static void pref_set_rec_dir ( GtkEntry *entry, G_GNUC_UNUSED GtkEntryIconPosition icon_pos, G_GNUC_UNUSED GdkEvent *event, Base *base )
{
	char *path = pref_open_dir ( base, g_get_home_dir () );

	if ( path == NULL ) return;

	if ( base->rec_dir ) g_free ( base->rec_dir );

	base->rec_dir = path;

	gtk_entry_set_text ( entry, base->rec_dir );
}

static void pref_changed_opacity_panel ( GtkRange *range, Base *base )
{
	base->opacity_panel = gtk_range_get_value ( range );
}
static void pref_changed_opacity_eq ( GtkRange *range, Base *base )
{
	base->opacity_eq = gtk_range_get_value ( range );
}
static void pref_changed_opacity_win ( GtkRange *range, Base *base )
{
	base->opacity_win = gtk_range_get_value ( range );
	gtk_widget_set_opacity ( GTK_WIDGET ( base->window ), base->opacity_win );
}
static void pref_changed_opacity_base_win ( GtkRange *range, GtkWindow *window )
{
	gdouble opacity = gtk_range_get_value ( range );
	gtk_widget_set_opacity ( GTK_WIDGET ( window ), opacity );
}
static void pref_changed_resize_icon ( GtkRange *range, Base *base )
{
	base->size_icon = (guint)gtk_range_get_value ( range );
}

static void pref_set_label_sw_rec ( Base *base )
{
	if ( !base->dtv->enable_rec )
	{
		gtk_label_set_text ( base->pref_rec, "  ◉  " );

		return;
	}

	const char *format = "<span foreground=\"#ff0000\">  ◉  </span>";

	char *markup = g_markup_printf_escaped ( format, "  ◉  " );

		gtk_label_set_markup ( base->pref_rec, markup );

	g_free ( markup );
}

static void pref_changed_sw_rec ( GObject *gobject, G_GNUC_UNUSED GParamSpec *pspec, Base *base )
{
	base->dtv->enable_rec = gtk_switch_get_state ( GTK_SWITCH ( gobject ) );

	pref_set_label_sw_rec ( base );

	if ( !base->dtv->enable_rec) gtk_switch_set_state ( base->rec_enc.gswitch, base->dtv->enable_rec );

	gtk_widget_set_sensitive ( GTK_WIDGET ( base->rec_enc.gswitch ), base->dtv->enable_rec );
}

static GtkSwitch * pref_create_switch_rec ( Base *base )
{
	GtkSwitch *gswitch = (GtkSwitch *)gtk_switch_new ();
	gtk_switch_set_state ( gswitch, base->dtv->enable_rec );
	g_signal_connect ( gswitch, "notify::active", G_CALLBACK ( pref_changed_sw_rec ), base );

	return gswitch;
}

static void pref_changed_sw_dark ( GObject *gobject, G_GNUC_UNUSED GParamSpec *pspec, Base *base )
{
	base->dark_theme = gtk_switch_get_state ( GTK_SWITCH ( gobject ) );
	g_object_set ( gtk_settings_get_default(), "gtk-application-prefer-dark-theme", base->dark_theme, NULL );
}

static GtkSwitch * pref_create_switch_dark ( Base *base )
{
	GtkSwitch *gswitch = (GtkSwitch *)gtk_switch_new ();
	gtk_switch_set_state ( gswitch, base->dark_theme );
	g_signal_connect ( gswitch, "notify::active", G_CALLBACK ( pref_changed_sw_dark ), base );

	return gswitch;
}

static void pref_create_entry ( Base *base, const char *text, const char *set_text, void (*f)(), GtkBox *h_box, 
								gboolean swe, GtkWidget *label_swe, GtkSwitch * (*fs)(Base *) )
{
	GtkImage *image = base_create_image ( text, 32 );
	gtk_widget_set_halign ( GTK_WIDGET ( image ), GTK_ALIGN_START );
	gtk_box_pack_start ( h_box, GTK_WIDGET ( image ), FALSE, FALSE, 0 );

	GtkLabel *label = (GtkLabel *)gtk_label_new ( "" );
	gtk_widget_set_size_request ( GTK_WIDGET ( label ), 50, -1 );
	gtk_box_pack_start ( h_box, GTK_WIDGET ( label ), FALSE, FALSE, 0 );

	GtkEntry *entry = (GtkEntry *)gtk_entry_new ();
	gtk_entry_set_text ( entry, set_text );

	g_object_set ( entry, "editable", FALSE, NULL );
	gtk_entry_set_icon_from_icon_name ( entry, GTK_ENTRY_ICON_SECONDARY, "folder" );
	g_signal_connect ( entry, "icon-press", G_CALLBACK ( f ), base );

	gtk_box_pack_start ( h_box, GTK_WIDGET ( entry ), TRUE, TRUE, 0 );

	if ( swe )
	{
		gtk_box_pack_start ( h_box, GTK_WIDGET ( label_swe   ), FALSE, FALSE, 0 );
		gtk_box_pack_start ( h_box, GTK_WIDGET ( fs ( base ) ), TRUE,  TRUE,  0 );
	}
}


static gboolean pref_element_factory_check_enc ( GtkEntry *entry, const char *type )
{
    const gchar *text = gtk_entry_get_text ( entry );

    gtk_entry_set_icon_from_icon_name ( entry, GTK_ENTRY_ICON_PRIMARY, "helia-info" );
	gtk_entry_set_icon_tooltip_text ( GTK_ENTRY ( entry ), GTK_ENTRY_ICON_PRIMARY, type );

    GstElementFactory *element_find = NULL;

    gboolean ret = FALSE;

    if ( gtk_entry_get_text_length ( entry ) > 0 )
    {
        element_find = gst_element_factory_find ( text );

		if ( element_find )
		{
			const char *metadata = gst_element_factory_get_metadata ( element_find, GST_ELEMENT_METADATA_KLASS );

			// g_print ( "%s: %s | metadata %s | type %s \n", __func__, gtk_entry_get_text ( entry ), metadata, type );

			if ( g_strrstr ( metadata, type ) ) ret = TRUE;

			if ( ret )
				gtk_entry_set_icon_from_icon_name ( entry, GTK_ENTRY_ICON_SECONDARY, "helia-ok" );
			else
				gtk_entry_set_icon_from_icon_name ( entry, GTK_ENTRY_ICON_SECONDARY, "helia-warning" );
		}
		else
			gtk_entry_set_icon_from_icon_name ( entry, GTK_ENTRY_ICON_SECONDARY, "helia-warning" );
    }

    return ret;
}
static void pref_changed_entry_rec_enc_audio ( GtkEntry *entry, Base *base )
{
	if ( pref_element_factory_check_enc ( entry, "Encoder/Audio" ) ) // GST_ELEMENT_FACTORY_TYPE_AUDIO_ENCODER
	{
		const char *text = gtk_entry_get_text ( entry );

		g_free ( base->rec_enc.str_audio_enc );

		base->rec_enc.str_audio_enc = g_strdup ( text );
	}
}
static void pref_changed_entry_rec_enc_video ( GtkEntry *entry, Base *base )
{
	if ( pref_element_factory_check_enc ( entry, "Encoder/Video" ) ) // GST_ELEMENT_FACTORY_TYPE_VIDEO_ENCODER
	{
		const char *text = gtk_entry_get_text ( entry );

		g_free ( base->rec_enc.str_video_enc );

		base->rec_enc.str_video_enc = g_strdup ( text );
	}
}
static void pref_changed_entry_rec_enc_muxer ( GtkEntry *entry, Base *base )
{
	if ( pref_element_factory_check_enc ( entry, "Muxer" ) ) // GST_ELEMENT_FACTORY_TYPE_MUXER
	{
		const char *text = gtk_entry_get_text ( entry );

		g_free ( base->rec_enc.str_muxer_enc );

		base->rec_enc.str_muxer_enc = g_strdup ( text );
	}
}


static gboolean pref_gst_element_find_property ( GstElement *element, const char *prop )
{
	gboolean find_prop = FALSE;

	if ( element && g_object_class_find_property ( G_OBJECT_GET_CLASS ( element ), prop ) )
		find_prop = TRUE;
	else
		find_prop = FALSE;

	return find_prop;
}
static gboolean pref_check_prop_gst_element ( const char *prop, GtkEntry *entry )
{
	gboolean ret = FALSE;

	GstElementFactory *factory = gst_element_factory_find ( gtk_entry_get_text ( entry ) );

	if ( factory == NULL ) return ret;

	GstElement *element = gst_element_factory_create ( factory, NULL );

	if ( element )
	{
		if ( pref_gst_element_find_property ( element, prop ) ) ret = TRUE;

		gst_object_unref ( element );
	}

    return ret;
}
static void pref_entry_rec_check_prop_set_icon ( GtkEntry *entry, GtkEntry *entry_enc )
{
	uint res = 0;

	gtk_entry_set_icon_from_icon_name ( entry, GTK_ENTRY_ICON_PRIMARY, "helia-info" );

	gtk_entry_set_icon_tooltip_text ( GTK_ENTRY ( entry ), GTK_ENTRY_ICON_PRIMARY, 
		"Type=Property=Value[space]\n    Type[int|uint|float|bool|char]\nExample:\n    uint=bitrate=1000 float=quality=0,5 bool=vbr=true char=option-string=..." );

	if ( gtk_entry_get_text_length ( entry ) > 0 )
	{
		const char *text = gtk_entry_get_text ( entry );

		if ( g_strrstr ( text, "=" ) )
		{
			char **fields = g_strsplit ( text, " ", 0 );
			uint j = 0, numfields = g_strv_length ( fields );

			for ( j = 0; j < numfields; j++ )
			{
				char **splits = g_strsplit ( fields[j], "=", 0 );
				uint  numsplits = g_strv_length ( splits );

				if ( numsplits > 1 && pref_check_prop_gst_element ( splits[1], entry_enc ) )
					g_debug ( "%s: prop  %s  Ok ", __func__, splits[1] );
				else
					{ res++; g_warning ( "%s:: Error: prop  %s ", __func__, splits[1] ); }

				g_strfreev ( splits );
			}

			g_strfreev ( fields );
		}
	}

	if ( res == 0 )
		gtk_entry_set_icon_from_icon_name ( entry, GTK_ENTRY_ICON_SECONDARY, "helia-ok" );
	else
		gtk_entry_set_icon_from_icon_name ( entry, GTK_ENTRY_ICON_SECONDARY, "helia-warning" );
}
static void pref_changed_entry_rec_prop_audio ( GtkEntry *entry, Base *base )
{
	const char *text = gtk_entry_get_text ( entry );

	if ( gtk_entry_get_text_length ( entry ) > 0 )
	{
		pref_entry_rec_check_prop_set_icon ( entry, base->rec_enc.audio_enc );

		g_free ( base->rec_enc.str_audio_prop );

		base->rec_enc.str_audio_prop = g_strdup ( text );
	}
}
static void pref_changed_entry_rec_prop_video ( GtkEntry *entry, Base *base )
{
	const char *text = gtk_entry_get_text ( entry );

	if ( gtk_entry_get_text_length ( entry ) > 0 )
	{
		pref_entry_rec_check_prop_set_icon ( entry, base->rec_enc.video_enc );

		g_free ( base->rec_enc.str_video_prop );

		base->rec_enc.str_video_prop = g_strdup ( text );
	}
}
static GtkEntry * pref_create_entry_rec_enc_prop ( Base *base, const char *set_text, void (*f)(), gboolean enc_prop, const char *type, GtkEntry *entry_enc )
{
	GtkEntry *entry = (GtkEntry *)gtk_entry_new ();
	gtk_entry_set_text ( entry, set_text );
	g_signal_connect ( entry, "changed", G_CALLBACK ( f ), base );

	if ( enc_prop ) pref_element_factory_check_enc ( entry, type ); else pref_entry_rec_check_prop_set_icon ( entry, entry_enc );

	return entry;
}
static void pref_set_visible_rec_enc_prop ( Base *base, GtkWindow *window )
{
	gtk_widget_set_visible ( GTK_WIDGET ( base->rec_enc.audio_enc  ), base->rec_enc.rec_enc_prop );
	gtk_widget_set_visible ( GTK_WIDGET ( base->rec_enc.audio_prop ), base->rec_enc.rec_enc_prop );
	gtk_widget_set_visible ( GTK_WIDGET ( base->rec_enc.video_enc  ), base->rec_enc.rec_enc_prop );
	gtk_widget_set_visible ( GTK_WIDGET ( base->rec_enc.video_prop ), base->rec_enc.rec_enc_prop );
	gtk_widget_set_visible ( GTK_WIDGET ( base->rec_enc.muxer_enc  ), base->rec_enc.rec_enc_prop );

	gtk_window_resize ( window, 200, 200 );
}
static void pref_changed_sw_rec_enc_prop ( GObject *gobject, G_GNUC_UNUSED GParamSpec *pspec, Base *base )
{
	base->rec_enc.rec_enc_prop = gtk_switch_get_state ( GTK_SWITCH ( gobject ) );

	GtkWindow *window = GTK_WINDOW ( gtk_widget_get_toplevel ( GTK_WIDGET ( base->rec_enc.gswitch ) ) );
	pref_set_visible_rec_enc_prop ( base, window );
}

static GtkSwitch * pref_create_switch_rec_enc_prop ( Base *base )
{
	GtkSwitch *gswitch = (GtkSwitch *)gtk_switch_new ();
	gtk_switch_set_state ( gswitch, base->rec_enc.rec_enc_prop );
	g_signal_connect ( gswitch, "notify::active", G_CALLBACK ( pref_changed_sw_rec_enc_prop ), base );

	return gswitch;
}
static void pref_create_rec_enc_prop ( Base *base, GtkBox *v_box )
{
	GtkBox *h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
	gtk_box_set_spacing ( h_box, 5 );

	GtkLabel *label = (GtkLabel *)gtk_label_new ( " TS / Encoder " );
	gtk_widget_set_halign ( GTK_WIDGET ( label ), GTK_ALIGN_START );
	gtk_box_pack_start ( h_box, GTK_WIDGET ( label ), FALSE,  FALSE, 0 );

	base->rec_enc.gswitch = pref_create_switch_rec_enc_prop ( base );
	gtk_box_pack_start ( h_box, GTK_WIDGET ( base->rec_enc.gswitch ), TRUE,  TRUE,  0 );

	gtk_box_pack_start ( v_box, GTK_WIDGET ( h_box ), TRUE, TRUE, 0 );
	h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
	gtk_box_set_spacing ( h_box, 5 );

	base->rec_enc.audio_enc = pref_create_entry_rec_enc_prop ( base, base->rec_enc.str_audio_enc, pref_changed_entry_rec_enc_audio, TRUE, "Encoder/Audio", NULL );
	gtk_box_pack_start ( h_box, GTK_WIDGET ( base->rec_enc.audio_enc ), TRUE, TRUE, 0 );

	base->rec_enc.audio_prop = pref_create_entry_rec_enc_prop ( base, base->rec_enc.str_audio_prop, pref_changed_entry_rec_prop_audio, FALSE, "Encoder/Audio", base->rec_enc.audio_enc );
	gtk_box_pack_start ( h_box, GTK_WIDGET ( base->rec_enc.audio_prop ), TRUE, TRUE, 0 );

	gtk_box_pack_start ( v_box, GTK_WIDGET ( h_box ), TRUE, TRUE, 0 );
	h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
	gtk_box_set_spacing ( h_box, 5 );

	base->rec_enc.video_enc = pref_create_entry_rec_enc_prop ( base, base->rec_enc.str_video_enc, pref_changed_entry_rec_enc_video, TRUE, "Encoder/Video", NULL );
	gtk_box_pack_start ( h_box, GTK_WIDGET ( base->rec_enc.video_enc ), TRUE, TRUE, 0 );

	base->rec_enc.video_prop = pref_create_entry_rec_enc_prop ( base, base->rec_enc.str_video_prop, pref_changed_entry_rec_prop_video, FALSE, "Encoder/Video", base->rec_enc.video_enc );
	gtk_box_pack_start ( h_box, GTK_WIDGET ( base->rec_enc.video_prop ), TRUE, TRUE, 0 );

	gtk_box_pack_start ( v_box, GTK_WIDGET ( h_box ), TRUE, TRUE, 0 );
	h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );

	base->rec_enc.muxer_enc = pref_create_entry_rec_enc_prop ( base, base->rec_enc.str_muxer_enc, pref_changed_entry_rec_enc_muxer, TRUE, "Muxer", NULL );
	gtk_box_pack_start ( h_box, GTK_WIDGET ( base->rec_enc.muxer_enc ), TRUE, TRUE, 0 );

	gtk_box_pack_start ( v_box, GTK_WIDGET ( h_box ), TRUE, TRUE, 0 );

	gtk_widget_set_sensitive ( GTK_WIDGET ( base->rec_enc.gswitch ), base->dtv->enable_rec );
}


static void pref_create_scale ( Base *base, const char *action, const char *element, double val, double min, double max, double step, 
									void (*f)(), GtkBox *h_box, GtkWindow *window )
{
	GtkImage *image = base_create_image ( action, 32 );
	gtk_widget_set_halign ( GTK_WIDGET ( image ), GTK_ALIGN_START );
	gtk_box_pack_start ( h_box, GTK_WIDGET ( image ), FALSE, FALSE, 0 );

	image = base_create_image ( element, 32 );
	gtk_widget_set_halign ( GTK_WIDGET ( image ), GTK_ALIGN_START );
	gtk_box_pack_start ( h_box, GTK_WIDGET ( image ), FALSE, FALSE, 5 );

	GtkScale *scale = (GtkScale *)gtk_scale_new_with_range ( GTK_ORIENTATION_HORIZONTAL, min, max, step );
	gtk_range_set_value ( GTK_RANGE ( scale ), val );
	g_signal_connect ( scale, "value-changed", G_CALLBACK ( f ), base  );
	if ( window ) g_signal_connect ( scale, "value-changed", G_CALLBACK ( pref_changed_opacity_base_win ), window );

	gtk_box_pack_start ( h_box, GTK_WIDGET ( scale ), TRUE, TRUE, 0 );
}

static void pref_changed_combo_lang ( GtkComboBoxText *combo_box, Base *base )
{
	base->num_lang = gtk_combo_box_get_active ( GTK_COMBO_BOX ( combo_box ) );

	GtkTreeViewColumn *column = gtk_tree_view_get_column ( base->player->treeview, COL_FL_CH );
	gtk_tree_view_column_set_title ( column, _i18n_ ( base, "Files" ) );

	column = gtk_tree_view_get_column ( base->dtv->treeview, COL_FL_CH );
	gtk_tree_view_column_set_title ( column, _i18n_ ( base, "Channels" ) );
}

static void pref_create_combo ( Base *base, const char *text, void (*f)(), GtkBox *h_box )
{
	GtkImage *image = base_create_image ( text, 32 );
	gtk_widget_set_halign ( GTK_WIDGET ( image ), GTK_ALIGN_START );
	gtk_box_pack_start ( h_box, GTK_WIDGET ( image ), FALSE, FALSE, 0 );

	GtkLabel *label = (GtkLabel *)gtk_label_new ( "" );
	gtk_widget_set_size_request ( GTK_WIDGET ( label ), 50, -1 );
	gtk_box_pack_start ( h_box, GTK_WIDGET ( label ), FALSE, FALSE, 0 );

	GtkComboBoxText *combo = (GtkComboBoxText *)gtk_combo_box_text_new ();
	
	lang_add_combo ( combo );
	
	gtk_combo_box_set_active ( GTK_COMBO_BOX ( combo ), base->num_lang );
	g_signal_connect ( combo, "changed", G_CALLBACK ( f ), base );
	gtk_box_pack_start ( h_box, GTK_WIDGET ( combo ), TRUE, TRUE, 0 );
}

static void pref_changed_resize_win_width ( GtkSpinButton *spin, Base *base )
{
	base->win_width = gtk_spin_button_get_value_as_int ( spin );
}

static void pref_changed_resize_win_height ( GtkSpinButton *spin, Base *base )
{
	base->win_height = gtk_spin_button_get_value_as_int ( spin );
}

static void pref_create_spin ( Base *base, GtkBox *hbox )
{
	GtkImage *image = base_create_image ( "helia-window", 32 );
	gtk_widget_set_halign ( GTK_WIDGET ( image ), GTK_ALIGN_START );
	gtk_box_pack_start ( hbox, GTK_WIDGET ( image   ), TRUE, TRUE, 0 );

	GtkLabel *label = (GtkLabel *)gtk_label_new ( "" );
	gtk_widget_set_size_request ( GTK_WIDGET ( label ), 35, -1 );
	gtk_box_pack_start ( hbox, GTK_WIDGET ( label ), FALSE, FALSE, 10 );

	label = (GtkLabel *)gtk_label_new ( " ↔ " );
	gtk_box_pack_start ( hbox, GTK_WIDGET ( label ), TRUE, TRUE, 0 );

	GtkSpinButton *spin = ( GtkSpinButton * )gtk_spin_button_new_with_range ( 250, pref_screen_get_width (), 1 );
	gtk_spin_button_set_value ( spin, base->win_width );
	g_signal_connect ( spin, "value-changed", G_CALLBACK ( pref_changed_resize_win_width ), base );
	gtk_box_pack_start ( hbox, GTK_WIDGET ( spin ), TRUE, TRUE, 0 );

	label = (GtkLabel *)gtk_label_new ( " ↕ " );
	gtk_box_pack_start ( hbox, GTK_WIDGET ( label ), TRUE, TRUE, 0 );

	spin = ( GtkSpinButton * )gtk_spin_button_new_with_range ( 100, pref_screen_get_height (), 1 );
	gtk_spin_button_set_value ( spin, base->win_height );
	g_signal_connect ( spin, "value-changed", G_CALLBACK ( pref_changed_resize_win_height ), base );
	gtk_box_pack_start ( hbox, GTK_WIDGET ( spin ), TRUE, TRUE, 0 );
}


static void pref_changed_sw_vis ( GObject *gobject, G_GNUC_UNUSED GParamSpec *pspec, Base *base )
{
	base->player->vis_plugin = gtk_switch_get_state ( GTK_SWITCH ( gobject ) );
}

static GtkSwitch * pref_create_switch_vis ( Base *base )
{
	GtkSwitch *gswitch = (GtkSwitch *)gtk_switch_new ();
	gtk_switch_set_state ( gswitch, base->player->vis_plugin );
	g_signal_connect ( gswitch, "notify::active", G_CALLBACK ( pref_changed_sw_vis ), base );

	return gswitch;
}

static void pref_create_vis ( Base *base, GtkBox *h_box )
{
	GtkImage *image = base_create_image ( "helia-audio", 32 );
	gtk_widget_set_halign ( GTK_WIDGET ( image ), GTK_ALIGN_START );
	gtk_box_pack_start ( h_box, GTK_WIDGET ( image ), FALSE, FALSE, 0 );

	GtkLabel *label = (GtkLabel *)gtk_label_new ( " GstGoom " );
	gtk_box_pack_start ( h_box, GTK_WIDGET ( label ), FALSE, FALSE, 10 );

	gtk_box_pack_end ( h_box, GTK_WIDGET ( pref_create_switch_vis ( base ) ), TRUE, TRUE, 0 );
}

void pref_win ( Base *base )
{
	GtkWindow *window =      (GtkWindow *)gtk_window_new ( GTK_WINDOW_TOPLEVEL );
	gtk_window_set_transient_for ( window, base->window );
	gtk_window_set_modal     ( window, TRUE );
	gtk_window_set_position  ( window, GTK_WIN_POS_CENTER_ON_PARENT );
	gtk_window_set_title     ( window, "" );

	GdkPixbuf *pixbuf = gtk_icon_theme_load_icon ( gtk_icon_theme_get_default (), 
					 "helia-pref", 48, GTK_ICON_LOOKUP_USE_BUILTIN, NULL );

	gtk_window_set_icon ( window, pixbuf );

	if ( pixbuf ) g_object_unref ( pixbuf );

	GtkBox *m_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_VERTICAL,   0 );	
	GtkBox *h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );

	h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
		pref_create_combo ( base, "helia-locale", pref_changed_combo_lang, h_box );
	gtk_box_pack_start ( m_box, GTK_WIDGET ( h_box ), FALSE, FALSE, 5 );

	h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
		char *set_text = pref_get_prop ( "gtk-theme-name" );
			pref_create_entry ( base, "helia-style", set_text, pref_set_theme, h_box, TRUE, gtk_label_new ( "  ⏾  " ), pref_create_switch_dark );
		g_free ( set_text );
	gtk_box_pack_start ( m_box, GTK_WIDGET ( h_box ), FALSE, FALSE, 0 );

	h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
		base->pref_rec = (GtkLabel *)gtk_label_new ( "  ◉  " );
		pref_set_label_sw_rec ( base );
		pref_create_entry ( base, "helia-record", base->rec_dir, pref_set_rec_dir, h_box, TRUE, GTK_WIDGET ( base->pref_rec ), pref_create_switch_rec );
	gtk_box_pack_start ( m_box, GTK_WIDGET ( h_box ), FALSE, FALSE, 5 );

	GtkBox *vbox = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 );
		gtk_box_set_spacing ( vbox, 5 );
		pref_create_rec_enc_prop ( base, vbox );
	gtk_box_pack_start ( m_box, GTK_WIDGET ( vbox ), FALSE, FALSE, 0 );

	gtk_box_pack_start ( m_box, GTK_WIDGET ( gtk_label_new ( " " ) ), FALSE, FALSE, 0 );

	struct data_a { const char *action; const char *element; double val; double min; double max; double step; void (*f)(); GtkWindow *win; } data_a_n[] =
	{
		{ "helia-opacity", "helia-panel",  base->opacity_panel, 0.4, 1.0, 0.01, pref_changed_opacity_panel, NULL   },
		{ "helia-opacity", "helia-eqa",    base->opacity_eq,    0.4, 1.0, 0.01, pref_changed_opacity_eq,    NULL   },
		{ "helia-opacity", "helia-window", base->opacity_win,   0.4, 1.0, 0.01, pref_changed_opacity_win,   window },
		{ "helia-size",    "helia-panel",  base->size_icon,     8,   48,     1, pref_changed_resize_icon,   NULL   }
	};

	uint d = 0;
	for ( d = 0; d < G_N_ELEMENTS ( data_a_n ); d++ )
	{
		h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );

		pref_create_scale ( base, data_a_n[d].action, data_a_n[d].element, data_a_n[d].val, data_a_n[d].min, data_a_n[d].max, data_a_n[d].step, data_a_n[d].f, h_box, data_a_n[d].win );

		gtk_box_pack_start ( m_box, GTK_WIDGET ( h_box ), FALSE, FALSE, 0 );
	}

	gtk_box_pack_start ( m_box, GTK_WIDGET ( gtk_label_new ( " " ) ), FALSE, FALSE, 0 );

	h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
		pref_create_spin ( base, h_box );
	gtk_box_pack_start ( m_box, GTK_WIDGET ( h_box ), FALSE, FALSE, 0 );

	h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
			pref_create_vis ( base, h_box );
	gtk_box_pack_start ( m_box, GTK_WIDGET ( h_box ), FALSE, FALSE, 5 );

	h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );

	GtkButton *button_close = (GtkButton *)gtk_button_new_from_icon_name ( "helia-exit", GTK_ICON_SIZE_BUTTON );
	g_signal_connect_swapped ( button_close, "clicked", G_CALLBACK ( gtk_widget_destroy ), window );
	gtk_box_pack_end ( h_box, GTK_WIDGET ( button_close ), TRUE, TRUE, 0 );

	gtk_box_pack_end ( m_box, GTK_WIDGET ( h_box ), FALSE, FALSE, 5 );

	gtk_container_set_border_width ( GTK_CONTAINER ( m_box ), 10 );
	gtk_container_add   ( GTK_CONTAINER ( window ), GTK_WIDGET ( m_box ) );
	gtk_widget_show_all ( GTK_WIDGET ( window ) );

	pref_set_visible_rec_enc_prop ( base, window );

	gtk_widget_set_opacity ( GTK_WIDGET ( window ), base->opacity_win );
}



static void keyb_win_changed_sw ( GObject *gobject, G_GNUC_UNUSED GParamSpec *pspec, Base *base )
{
	base->pause_mouse = gtk_switch_get_state ( GTK_SWITCH ( gobject ) );
}

static GtkBox * keyb_win_create_switch ( Base *base, const char *name )
{
	GtkImage *image = base_create_image ( name, 32 );

	GtkSwitch *gswitch = (GtkSwitch *)gtk_switch_new ();
	gtk_switch_set_state ( gswitch, base->pause_mouse );
	g_signal_connect ( gswitch, "notify::active", G_CALLBACK ( keyb_win_changed_sw ), base );

	GtkBox *hbox = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL,  0 );
	gtk_widget_set_halign ( GTK_WIDGET ( hbox ), GTK_ALIGN_END );

	gtk_box_pack_start ( hbox, GTK_WIDGET ( gswitch ), FALSE, FALSE, 10 );
	gtk_box_pack_start ( hbox, GTK_WIDGET ( image   ), FALSE, FALSE,  0 );

	return hbox;
}

static GtkBox * keyb_win_create_play_pause ( const char *name_a, const char *name_b )
{
	GtkImage *image_a = base_create_image ( name_a, 32 );
	GtkImage *image_b = base_create_image ( name_b, 32 );

	GtkBox *hbox = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL,  0 );
	gtk_widget_set_halign ( GTK_WIDGET ( hbox ), GTK_ALIGN_START );

	gtk_box_pack_start ( hbox, GTK_WIDGET ( image_a ), FALSE, FALSE, 0 );
	gtk_box_pack_start ( hbox, GTK_WIDGET ( image_b ), FALSE, FALSE, 5 );

	return hbox;
}

void keyb_win ( Base *base )
{
	GtkWindow *window =      (GtkWindow *)gtk_window_new ( GTK_WINDOW_TOPLEVEL );
	gtk_window_set_transient_for ( window, base->window );
	gtk_window_set_modal     ( window, TRUE );
	gtk_window_set_position  ( window, GTK_WIN_POS_CENTER_ON_PARENT );
	gtk_window_set_title     ( window, "" );

	GdkPixbuf *pixbuf = gtk_icon_theme_load_icon ( gtk_icon_theme_get_default (), 
					 "helia-keyb", 48, GTK_ICON_LOOKUP_USE_BUILTIN, NULL );

	gtk_window_set_icon ( window, pixbuf );

	if ( pixbuf ) g_object_unref ( pixbuf );

	GtkBox *m_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_VERTICAL,   0 );	
	GtkBox *h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );

	GtkBox *g_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 );
	gtk_widget_set_margin_start ( GTK_WIDGET ( g_box ), 10 );
	gtk_widget_set_margin_end   ( GTK_WIDGET ( g_box ), 10 );

	GtkGrid *grid = (GtkGrid *)gtk_grid_new();
	gtk_grid_set_column_homogeneous ( GTK_GRID ( grid ), TRUE );
	gtk_grid_set_row_homogeneous    ( GTK_GRID ( grid ) ,TRUE );
	gtk_box_pack_start ( g_box, GTK_WIDGET ( grid ), TRUE, TRUE, 10 );

	const struct data_a { const char *image; const char *accel; } data_a_n[] =
	{
		{ "helia-add",          "Ctrl + O" },
		{ "helia-add-folder",   "Ctrl + D" },
		{ "helia-net", 			"Ctrl + L" },
		{ NULL, NULL },
		{ "helia-editor", 		"Ctrl + H" },
		{ "helia-muted", 		"Ctrl + M" },
		{ "helia-search", 		"Ctrl + F" },
		{ "helia-stop", 		"Ctrl + X" },
		{ NULL, NULL },
		{ "helia-frame", 		" . "      },
		{ "helia-play",   		"␣"        }
	};

	uint d = 0;
	for ( d = 0; d < G_N_ELEMENTS ( data_a_n ); d++ )
	{
		if ( !data_a_n[d].image )
		{
			gtk_grid_attach ( GTK_GRID ( grid ), gtk_label_new ( "" ), 0, d, 1, 1 );
			continue;
		}

		if ( d == 10 )
		{
			GtkBox *hbox = keyb_win_create_play_pause ( "helia-play", "helia-pause" );
			gtk_grid_attach ( GTK_GRID ( grid ), GTK_WIDGET ( hbox ), 0, d, 1, 1 );

			gtk_grid_attach ( GTK_GRID ( grid ), GTK_WIDGET ( keyb_win_create_switch ( base, "helia-mouse" ) ), 1, d, 1, 1 );

			GtkLabel *label = (GtkLabel *)gtk_label_new ( data_a_n[d].accel );
			gtk_widget_set_halign ( GTK_WIDGET ( label ), GTK_ALIGN_END );
			gtk_grid_attach ( GTK_GRID ( grid ), GTK_WIDGET ( label ), 2, d, 1, 1 );

			continue;
		}

		GtkImage *image =  base_create_image ( data_a_n[d].image, 32 );
		gtk_widget_set_halign ( GTK_WIDGET ( image ), GTK_ALIGN_START );
		gtk_grid_attach ( GTK_GRID ( grid ), GTK_WIDGET ( image ), 0, d, 1, 1 );

		GtkLabel *label = (GtkLabel *)gtk_label_new ( data_a_n[d].accel );
		gtk_widget_set_halign ( GTK_WIDGET ( label ), GTK_ALIGN_END );
		gtk_grid_attach ( GTK_GRID ( grid ), GTK_WIDGET ( label ), 2, d, 1, 1 );
	}

	gtk_box_pack_start ( m_box, GTK_WIDGET ( g_box ), TRUE, TRUE, 0 );

	h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );

	GtkButton *button_close = (GtkButton *)gtk_button_new_from_icon_name ( "helia-exit", GTK_ICON_SIZE_BUTTON );
	g_signal_connect_swapped ( button_close, "clicked", G_CALLBACK ( gtk_widget_destroy ), window );
	gtk_box_pack_end ( h_box, GTK_WIDGET ( button_close ), TRUE, TRUE, 0 );

	gtk_box_pack_end ( m_box, GTK_WIDGET ( h_box ), FALSE, FALSE, 5 );

	gtk_container_set_border_width ( GTK_CONTAINER ( m_box ), 10 );
	gtk_container_add   ( GTK_CONTAINER ( window ), GTK_WIDGET ( m_box ) );
	gtk_widget_show_all ( GTK_WIDGET ( window ) );

	gtk_widget_set_opacity ( GTK_WIDGET ( window ), base->opacity_win );
}
