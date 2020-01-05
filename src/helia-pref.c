/*
* Copyright 2019 Stepan Perun
* This program is free software.
*
* License: Gnu Lesser General Public License
* http://www.gnu.org/licenses/lgpl.html
*/


#include <helia-include.h>



enum helia_pref_page_n
{
	PAGE_STL,
	PAGE_DRC,
	PAGE_NUM
};

const struct HeliaPrefLabel { uint page; const char *name; } helia_pref_label_n[] =
{
	{ PAGE_STL, "Style"  },
	{ PAGE_DRC, "Record" }
};


static char * helia_pref_get_prop ( const char *prop )
{
	char *name = NULL;

	g_object_get ( gtk_settings_get_default (), prop, &name, NULL );

	return name;
}

static void helia_pref_set_prop ( const char *prop, char *path )
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

static void helia_pref_set_theme ( GtkEntry *entry, G_GNUC_UNUSED GtkEntryIconPosition icon_pos, G_GNUC_UNUSED GdkEvent *event, Helia *helia )
{
	char *path = helia_open_dir ( helia, "/usr/share/themes" );

	if ( path == NULL ) return;

	helia_pref_set_prop ( "gtk-theme-name", path );

	char *name = g_path_get_basename ( path );

		gtk_entry_set_text ( entry, name );

	g_free ( name );

	g_free ( path );
}

void helia_pref_read_config ( Helia *helia )
{
	uint n = 0;
	char *contents;

	GError *err = NULL;

	if ( g_file_get_contents ( helia->helia_conf, &contents, 0, &err ) )
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
				if ( helia->rec_dir ) g_free ( helia->rec_dir );
				helia->rec_dir = g_strdup ( key_val[1] );
			}

			if ( g_strrstr ( lines[n], "opacity-control" ) )
				helia->opacity_panel = atof ( key_val[1] );

			if ( g_strrstr ( lines[n], "opacity-equalizer" ) )
				helia->opacity_eqav = atof ( key_val[1] );

			if ( g_strrstr ( lines[n], "opacity-window" ) )
				helia->opacity_window = atof ( key_val[1] );

			if ( g_strrstr ( lines[n], "resize-icon" ) )
				helia->icon_size = atoi ( key_val[1] );

			if ( g_strrstr ( lines[n], "window-width" ) )
				helia->win_width = ( atoi ( key_val[1] ) );

			if ( g_strrstr ( lines[n], "window-height" ) )
				helia->win_height = ( atoi ( key_val[1] ) );

			if ( g_strrstr ( lines[n], "mp-volume" ) )
				helia->volume_mp = atof ( key_val[1] );

			if ( g_strrstr ( lines[n], "tv-volume" ) )
				helia->volume_tv = atof ( key_val[1] );

			if ( g_strrstr ( lines[n], "dark-theme" ) )
			{
				helia->dark_theme = ( atoi ( key_val[1] ) ) ? TRUE : FALSE;
				g_object_set ( gtk_settings_get_default(), "gtk-application-prefer-dark-theme", helia->dark_theme, NULL );
			}

			if ( g_strrstr ( lines[n], "mouse-click" ) )
			{
				helia->state_mouse_click = ( atoi ( key_val[1] ) ) ? TRUE : FALSE;
			}

			if ( g_strrstr ( lines[n], "power-off" ) )
			{
				helia->power = ( atoi ( key_val[1] ) ) ? TRUE : FALSE;
			}

			if ( g_strrstr ( lines[n], "panel-ext" ) )
			{
				helia->panel_ext = ( atoi ( key_val[1] ) ) ? TRUE : FALSE;
			}

			// g_print ( "%s:: Set %s -> %s \n", __func__, key_val[0], key_val[1] );

			g_strfreev ( key_val );
		}

		g_strfreev ( lines );
		g_free ( contents );

		if ( helia->opacity_panel  == 0 || helia->opacity_panel  > 1 ) helia->opacity_panel  = 0.75;
		if ( helia->opacity_eqav   == 0 || helia->opacity_eqav   > 1 ) helia->opacity_eqav   = 0.75;
		if ( helia->opacity_window == 0 || helia->opacity_window > 1 ) helia->opacity_window = 1.00;
		if ( helia->volume_mp == 0 || helia->volume_mp > 1 ) helia->volume_mp = 0.5;
		if ( helia->volume_tv == 0 || helia->volume_tv > 1 ) helia->volume_tv = 0.5;

	}
	else
	{
		g_critical ( "%s:: %s\n", __func__, err->message );
		g_error_free ( err );
	}
}

void helia_pref_save_config ( Helia *helia )
{
	char *conf_t = helia_pref_get_prop ( "gtk-theme-name" );

	GString *gstring = g_string_new ( "# Helia conf \n" );

	g_string_append_printf ( gstring, "record-dir=%s\n",          helia->rec_dir  );
	g_string_append_printf ( gstring, "gtk-theme-name=%s\n",      conf_t  );
	g_string_append_printf ( gstring, "opacity-control=%f\n",     helia->opacity_panel  );
	g_string_append_printf ( gstring, "opacity-equalizer=%f\n",   helia->opacity_eqav   );
	g_string_append_printf ( gstring, "opacity-window=%f\n",      helia->opacity_window );
	g_string_append_printf ( gstring, "mp-volume=%f\n",      	  helia->volume_mp );
	g_string_append_printf ( gstring, "tv-volume=%f\n",      	  helia->volume_tv );
	g_string_append_printf ( gstring, "resize-icon=%d\n",         helia->icon_size );
	g_string_append_printf ( gstring, "window-width=%d\n",        helia->win_width  );
	g_string_append_printf ( gstring, "window-height=%d\n",       helia->win_height );
	g_string_append_printf ( gstring, "dark-theme=%d\n",          helia->dark_theme ? 1 : 0 );
	g_string_append_printf ( gstring, "mouse-click=%d\n",         helia->state_mouse_click ? 1 : 0 );
	g_string_append_printf ( gstring, "power-off=%d\n",           helia->power ? 1 : 0 );
	g_string_append_printf ( gstring, "panel-ext=%d\n",           helia->panel_ext ? 1 : 0 );

	GError *err = NULL;

	if ( !g_file_set_contents ( helia->helia_conf, gstring->str, -1, &err ) )
	{
		g_critical ( "%s:: %s\n", __func__, err->message );
		g_error_free ( err );
	}

	g_string_free ( gstring, TRUE );

	g_free ( conf_t );
}

static GdkRectangle helia_pref_monitor_get_geometry ()
{
	GdkDisplay *display = gdk_display_get_default ();
	GdkMonitor *monitor = gdk_display_get_primary_monitor ( display );

	GdkRectangle geom;
	gdk_monitor_get_geometry ( monitor, &geom );

	g_debug ( "%s: width %d | height %d ", __func__, geom.width, geom.height );

	return geom;
}
static uint helia_pref_screen_get_width ()
{
	GdkRectangle geom = helia_pref_monitor_get_geometry ();

	return geom.width;
}
static uint helia_pref_screen_get_height ()
{
	GdkRectangle geom = helia_pref_monitor_get_geometry ();

	return geom.height;
}

static void helia_pref_changed_sw_dark ( GObject *gobject, G_GNUC_UNUSED GParamSpec *pspec, Helia *helia )
{
	helia->dark_theme = gtk_switch_get_state ( GTK_SWITCH ( gobject ) );
	g_object_set ( gtk_settings_get_default(), "gtk-application-prefer-dark-theme", helia->dark_theme, NULL );
}

static GtkSwitch * helia_pref_create_switch_dark ( Helia *helia )
{
	GtkSwitch *gswitch = (GtkSwitch *)gtk_switch_new ();
	gtk_switch_set_state ( gswitch, helia->dark_theme );
	g_signal_connect ( gswitch, "notify::active", G_CALLBACK ( helia_pref_changed_sw_dark ), helia );

	return gswitch;
}

static void helia_pref_create_entry ( Helia *helia, const char *text, const char *set_text, void (*f)(), GtkBox *h_box, 
					gboolean swe, GtkWidget *label_swe, GtkSwitch * (*fs)(Helia *) )
{
	GtkImage *image = helia_create_image ( text, 24 );
	gtk_widget_set_halign ( GTK_WIDGET ( image ), GTK_ALIGN_START );
	gtk_box_pack_start ( h_box, GTK_WIDGET ( image ), FALSE, FALSE, 0 );

	GtkLabel *label = (GtkLabel *)gtk_label_new ( "" );
	gtk_widget_set_size_request ( GTK_WIDGET ( label ), 50, -1 );
	gtk_box_pack_start ( h_box, GTK_WIDGET ( label ), FALSE, FALSE, 0 );

	GtkEntry *entry = (GtkEntry *)gtk_entry_new ();
	gtk_entry_set_text ( entry, set_text );

	g_object_set ( entry, "editable", FALSE, NULL );
	gtk_entry_set_icon_from_icon_name ( entry, GTK_ENTRY_ICON_SECONDARY, "folder" );
	g_signal_connect ( entry, "icon-press", G_CALLBACK ( f ), helia );

	gtk_box_pack_start ( h_box, GTK_WIDGET ( entry ), TRUE, TRUE, 0 );

	if ( swe )
	{
		gtk_box_pack_start ( h_box, GTK_WIDGET ( label_swe    ), FALSE, FALSE, 0 );
		gtk_box_pack_start ( h_box, GTK_WIDGET ( fs ( helia ) ), TRUE,  TRUE,  0 );
	}
}

static void helia_pref_changed_opacity_panel ( GtkRange *range, Helia *helia )
{
	helia->opacity_panel = gtk_range_get_value ( range );
}
static void helia_pref_changed_opacity_eq ( GtkRange *range, Helia *helia )
{
	helia->opacity_eqav = gtk_range_get_value ( range );
}
static void helia_pref_changed_opacity_win ( GtkRange *range, Helia *helia )
{
	helia->opacity_window = gtk_range_get_value ( range );
	gtk_widget_set_opacity ( GTK_WIDGET ( helia->window ), helia->opacity_window );
}
static void helia_pref_changed_opacity_base_win ( GtkRange *range, GtkWindow *window )
{
	gdouble opacity = gtk_range_get_value ( range );
	gtk_widget_set_opacity ( GTK_WIDGET ( window ), opacity );
}
static void helia_pref_changed_resize_icon ( GtkRange *range, Helia *helia )
{
	helia->icon_size = (uint)gtk_range_get_value ( range );
}

static void helia_pref_create_scale ( Helia *helia, const char *action, const char *element, double val, double min, double max, double step, 
								void (*f)(), GtkBox *h_box, GtkWindow *window )
{
	GtkImage *image = helia_create_image ( action, 24 );
	gtk_widget_set_halign ( GTK_WIDGET ( image ), GTK_ALIGN_START );
	gtk_box_pack_start ( h_box, GTK_WIDGET ( image ), FALSE, FALSE, 0 );

	image = helia_create_image ( element, 24 );
	gtk_widget_set_halign ( GTK_WIDGET ( image ), GTK_ALIGN_START );
	gtk_box_pack_start ( h_box, GTK_WIDGET ( image ), FALSE, FALSE, 5 );

	GtkScale *scale = (GtkScale *)gtk_scale_new_with_range ( GTK_ORIENTATION_HORIZONTAL, min, max, step );
	gtk_range_set_value ( GTK_RANGE ( scale ), val );
	g_signal_connect ( scale, "value-changed", G_CALLBACK ( f ), helia  );
	if ( window ) g_signal_connect ( scale, "value-changed", G_CALLBACK ( helia_pref_changed_opacity_base_win ), window );

	gtk_widget_set_size_request ( GTK_WIDGET (scale), 400, -1 );

	gtk_box_pack_start ( h_box, GTK_WIDGET ( scale ), TRUE, TRUE, 0 );
}

static void helia_pref_changed_resize_win_width ( GtkSpinButton *spin, Helia *helia )
{
	helia->win_width = gtk_spin_button_get_value_as_int ( spin );
}

static void helia_pref_changed_resize_win_height ( GtkSpinButton *spin, Helia *helia )
{
	helia->win_height = gtk_spin_button_get_value_as_int ( spin );
}

static void helia_pref_create_spin ( Helia *helia, GtkBox *hbox )
{
	GtkImage *image = helia_create_image ( "helia-window", 24 );
	gtk_widget_set_halign ( GTK_WIDGET ( image ), GTK_ALIGN_START );
	gtk_box_pack_start ( hbox, GTK_WIDGET ( image   ), TRUE, TRUE, 0 );

	GtkLabel *label = (GtkLabel *)gtk_label_new ( "" );
	gtk_widget_set_size_request ( GTK_WIDGET ( label ), 35, -1 );
	gtk_box_pack_start ( hbox, GTK_WIDGET ( label ), FALSE, FALSE, 10 );

	label = (GtkLabel *)gtk_label_new ( " ↔ " );
	gtk_box_pack_start ( hbox, GTK_WIDGET ( label ), TRUE, TRUE, 0 );

	GtkSpinButton *spin = ( GtkSpinButton * )gtk_spin_button_new_with_range ( 250, helia_pref_screen_get_width (), 1 );
	gtk_spin_button_set_value ( spin, helia->win_width );
	g_signal_connect ( spin, "value-changed", G_CALLBACK ( helia_pref_changed_resize_win_width ), helia );
	gtk_box_pack_start ( hbox, GTK_WIDGET ( spin ), TRUE, TRUE, 0 );

	label = (GtkLabel *)gtk_label_new ( " ↕ " );
	gtk_box_pack_start ( hbox, GTK_WIDGET ( label ), TRUE, TRUE, 0 );

	spin = ( GtkSpinButton * )gtk_spin_button_new_with_range ( 100, helia_pref_screen_get_height (), 1 );
	gtk_spin_button_set_value ( spin, helia->win_height );
	g_signal_connect ( spin, "value-changed", G_CALLBACK ( helia_pref_changed_resize_win_height ), helia );
	gtk_box_pack_start ( hbox, GTK_WIDGET ( spin ), TRUE, TRUE, 0 );
}

static void helia_info_changed_sw_panel_ext ( GObject *gobject, G_GNUC_UNUSED GParamSpec *pspec, Helia *helia )
{
	helia->panel_ext = gtk_switch_get_state ( GTK_SWITCH ( gobject ) );
}

static GtkSwitch * helia_info_create_switch_panel_ext ( Helia *helia )
{
	GtkSwitch *gswitch = (GtkSwitch *)gtk_switch_new ();
	gtk_switch_set_state ( gswitch, helia->panel_ext );
	g_signal_connect ( gswitch, "notify::active", G_CALLBACK ( helia_info_changed_sw_panel_ext ), helia );

	return gswitch;
}

static void helia_pref_create_panel_ext ( Helia *helia, GtkBox *h_box )
{
	GtkImage *image = helia_create_image ( "helia-panel", 24 );
	gtk_widget_set_halign ( GTK_WIDGET ( image ), GTK_ALIGN_START );
	gtk_box_pack_start ( h_box, GTK_WIDGET ( image ), FALSE, FALSE, 0 );

	image = helia_create_image ( "helia-add", 24 );
	gtk_widget_set_halign ( GTK_WIDGET ( image ), GTK_ALIGN_START );
	gtk_box_pack_start ( h_box, GTK_WIDGET ( image ), FALSE, FALSE, 5 );

	GtkLabel *label = (GtkLabel *)gtk_label_new ( "" );
	gtk_widget_set_size_request ( GTK_WIDGET ( label ), 15, -1 );
	gtk_box_pack_start ( h_box, GTK_WIDGET ( label ), FALSE, FALSE, 0 );

	gtk_box_pack_end ( h_box, GTK_WIDGET ( helia_info_create_switch_panel_ext ( helia ) ), TRUE, TRUE, 0 );
}

static void helia_info_changed_sw_power ( GObject *gobject, G_GNUC_UNUSED GParamSpec *pspec, Helia *helia )
{
	helia->power = gtk_switch_get_state ( GTK_SWITCH ( gobject ) );

	/* helia_power_manager ( helia ); */
}

static GtkSwitch * helia_info_create_switch_power ( Helia *helia )
{
	GtkSwitch *gswitch = (GtkSwitch *)gtk_switch_new ();
	gtk_switch_set_state ( gswitch, helia->power );
	g_signal_connect ( gswitch, "notify::active", G_CALLBACK ( helia_info_changed_sw_power ), helia );

	return gswitch;
}

static void helia_pref_create_power ( Helia *helia, GtkBox *h_box )
{
	GtkImage *image = helia_create_image ( "helia-power", 24 );
	gtk_widget_set_halign ( GTK_WIDGET ( image ), GTK_ALIGN_START );
	gtk_box_pack_start ( h_box, GTK_WIDGET ( image ), FALSE, FALSE, 0 );

	gtk_box_pack_end ( h_box, GTK_WIDGET ( helia_info_create_switch_power ( helia ) ), TRUE, TRUE, 0 );
}

static GtkBox * helia_pref_style ( Helia *helia, GtkWindow *window )
{
	GtkBox *g_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 );
	gtk_widget_set_margin_start ( GTK_WIDGET ( g_box ), 10 );
	gtk_widget_set_margin_end   ( GTK_WIDGET ( g_box ), 10 );

	GtkBox *m_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_VERTICAL,   0 );
	GtkBox *h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );

	gtk_box_pack_start ( m_box, GTK_WIDGET ( gtk_label_new ( " " ) ), FALSE, FALSE, 0 );

	h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
		char *set_text = helia_pref_get_prop ( "gtk-theme-name" );
			helia_pref_create_entry ( helia, "helia-theme", set_text, helia_pref_set_theme, h_box, TRUE, gtk_label_new ( "  ⏾  " ), helia_pref_create_switch_dark );
		g_free ( set_text );
	gtk_box_pack_start ( m_box, GTK_WIDGET ( h_box ), FALSE, FALSE, 0 );

	gtk_box_pack_start ( m_box, GTK_WIDGET ( gtk_label_new ( " " ) ), FALSE, FALSE, 0 );

	struct data_a { const char *action; const char *element; double val; double min; double max; double step; void (*f)(); GtkWindow *win; } data_a_n[] =
	{
		{ "helia-opacity", "helia-panel",  helia->opacity_panel,  0.4, 1.0, 0.01, helia_pref_changed_opacity_panel, NULL   },
		{ "helia-opacity", "helia-eqa",    helia->opacity_eqav,   0.4, 1.0, 0.01, helia_pref_changed_opacity_eq,    NULL   },
		{ "helia-opacity", "helia-window", helia->opacity_window, 0.4, 1.0, 0.01, helia_pref_changed_opacity_win,   window },
		{ "helia-size",    "helia-panel",  helia->icon_size,      8,   48,     1, helia_pref_changed_resize_icon,   NULL   }
	};

	uint d = 0;
	for ( d = 0; d < G_N_ELEMENTS ( data_a_n ); d++ )
	{
		h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );

		helia_pref_create_scale ( helia, data_a_n[d].action, data_a_n[d].element, data_a_n[d].val, data_a_n[d].min, data_a_n[d].max, data_a_n[d].step, data_a_n[d].f, h_box, data_a_n[d].win );

		gtk_box_pack_start ( m_box, GTK_WIDGET ( h_box ), FALSE, FALSE, 0 );
	}

	gtk_box_pack_start ( m_box, GTK_WIDGET ( gtk_label_new ( " " ) ), FALSE, FALSE, 0 );

	h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
	//gtk_box_set_spacing ( h_box, 50 );

	helia_pref_create_panel_ext ( helia, h_box );
	gtk_box_pack_start ( m_box, GTK_WIDGET ( h_box ), FALSE, FALSE, 0 );

	gtk_box_pack_start ( m_box, GTK_WIDGET ( gtk_label_new ( " " ) ), FALSE, FALSE, 0 );

	h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
	gtk_box_set_spacing ( h_box, 50 );

	helia_pref_create_power ( helia, h_box );
	gtk_box_pack_start ( m_box, GTK_WIDGET ( h_box ), FALSE, FALSE, 0 );

	gtk_box_pack_start ( m_box, GTK_WIDGET ( gtk_label_new ( " " ) ), FALSE, FALSE, 0 );

	h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
		helia_pref_create_spin ( helia, h_box );
	gtk_box_pack_start ( m_box, GTK_WIDGET ( h_box ), FALSE, FALSE, 0 );

	gtk_box_pack_start ( m_box, GTK_WIDGET ( gtk_label_new ( " " ) ), FALSE, FALSE, 0 );

	gtk_box_pack_start ( g_box, GTK_WIDGET ( m_box ), FALSE, FALSE, 0 );

	return g_box;
}


static gboolean helia_pref_element_factory_check_enc ( GtkEntry *entry, const char *type )
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
static void helia_pref_changed_entry_rec_enc_audio ( GtkEntry *entry, Helia *helia )
{
	if ( helia_pref_element_factory_check_enc ( entry, "Encoder/Audio" ) ) // GST_ELEMENT_FACTORY_TYPE_AUDIO_ENCODER
	{
		const char *text = gtk_entry_get_text ( entry );

		g_free ( helia->str_audio_enc );

		helia->str_audio_enc = g_strdup ( text );
	}
}
static void helia_pref_changed_entry_rec_enc_video ( GtkEntry *entry, Helia *helia )
{
	if ( helia_pref_element_factory_check_enc ( entry, "Encoder/Video" ) ) // GST_ELEMENT_FACTORY_TYPE_VIDEO_ENCODER
	{
		const char *text = gtk_entry_get_text ( entry );

		g_free ( helia->str_video_enc );

		helia->str_video_enc = g_strdup ( text );
	}
}
static void helia_pref_changed_entry_rec_enc_muxer ( GtkEntry *entry, Helia *helia )
{
	if ( helia_pref_element_factory_check_enc ( entry, "Muxer" ) ) // GST_ELEMENT_FACTORY_TYPE_MUXER
	{
		const char *text = gtk_entry_get_text ( entry );

		g_free ( helia->str_muxer_enc );

		helia->str_muxer_enc = g_strdup ( text );
	}
}

static gboolean helia_pref_gst_element_find_property ( GstElement *element, const char *prop )
{
	gboolean find_prop = FALSE;

	if ( element && g_object_class_find_property ( G_OBJECT_GET_CLASS ( element ), prop ) )
		find_prop = TRUE;
	else
		find_prop = FALSE;

	return find_prop;
}
static gboolean helia_pref_check_prop_gst_element ( const char *prop, char *str_va_enc )
{
	gboolean ret = FALSE;

	GstElementFactory *factory = gst_element_factory_find ( str_va_enc );

	if ( factory == NULL ) return ret;

	GstElement *element = gst_element_factory_create ( factory, NULL );

	if ( element )
	{
		if ( helia_pref_gst_element_find_property ( element, prop ) ) ret = TRUE;

		gst_object_unref ( element );
	}

	return ret;
}
static void helia_pref_entry_rec_check_prop_set_icon ( GtkEntry *entry, char *str_va_enc )
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

				if ( numsplits > 1 && helia_pref_check_prop_gst_element ( splits[1], str_va_enc ) )
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
static void helia_pref_changed_entry_rec_prop_audio ( GtkEntry *entry, Helia *helia )
{
	const char *text = gtk_entry_get_text ( entry );

	if ( gtk_entry_get_text_length ( entry ) > 0 )
	{
		helia_pref_entry_rec_check_prop_set_icon ( entry, helia->str_audio_enc );

		g_free ( helia->str_audio_prop );

		helia->str_audio_prop = g_strdup ( text );
	}
}
static void helia_pref_changed_entry_rec_prop_video ( GtkEntry *entry, Helia *helia )
{
	const char *text = gtk_entry_get_text ( entry );

	if ( gtk_entry_get_text_length ( entry ) > 0 )
	{
		helia_pref_entry_rec_check_prop_set_icon ( entry, helia->str_video_enc );

		g_free ( helia->str_video_prop );

		helia->str_video_prop = g_strdup ( text );
	}
}
static GtkEntry * helia_pref_create_entry_rec_enc_prop ( Helia *helia, const char *set_text, void (*f)(), gboolean enc_prop, const char *type, char *str_va_enc )
{
	GtkEntry *entry = (GtkEntry *)gtk_entry_new ();
	gtk_entry_set_text ( entry, set_text );
	g_signal_connect ( entry, "changed", G_CALLBACK ( f ), helia );

	if ( enc_prop ) helia_pref_element_factory_check_enc ( entry, type ); else helia_pref_entry_rec_check_prop_set_icon ( entry, str_va_enc );

	return entry;
}

static void helia_pref_changed_sw_rec_enc_prop ( GObject *gobject, G_GNUC_UNUSED GParamSpec *pspec, Helia *helia )
{
	helia->rec_enc_tv = gtk_switch_get_state ( GTK_SWITCH ( gobject ) );

	gtk_widget_set_sensitive ( GTK_WIDGET ( helia->rec_vbox ), helia->rec_enc_tv );
}

static GtkSwitch * helia_pref_create_switch_rec_enc_prop ( Helia *helia )
{
	GtkSwitch *gswitch = (GtkSwitch *)gtk_switch_new ();
	gtk_switch_set_state ( gswitch, helia->rec_enc_tv );
	g_signal_connect ( gswitch, "notify::active", G_CALLBACK ( helia_pref_changed_sw_rec_enc_prop ), helia );

	return gswitch;
}

static void helia_pref_create_rec_enc_prop ( Helia *helia, GtkBox *v_box )
{
	helia->rec_vbox = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 );
	gtk_box_set_spacing ( helia->rec_vbox, 5 );

	GtkBox *h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
	gtk_box_set_spacing ( h_box, 5 );

	GtkEntry *audio_enc = helia_pref_create_entry_rec_enc_prop ( helia, helia->str_audio_enc, helia_pref_changed_entry_rec_enc_audio, TRUE, "Encoder/Audio", NULL );
	gtk_box_pack_start ( h_box, GTK_WIDGET ( audio_enc ), TRUE, TRUE, 0 );

	GtkEntry *audio_prop = helia_pref_create_entry_rec_enc_prop ( helia, helia->str_audio_prop, helia_pref_changed_entry_rec_prop_audio, FALSE, "Encoder/Audio", helia->str_audio_enc );
	gtk_box_pack_start ( h_box, GTK_WIDGET ( audio_prop ), TRUE, TRUE, 0 );

	gtk_box_pack_start ( helia->rec_vbox, GTK_WIDGET ( h_box ), TRUE, TRUE, 0 );
	h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
	gtk_box_set_spacing ( h_box, 5 );

	GtkEntry *video_enc = helia_pref_create_entry_rec_enc_prop ( helia, helia->str_video_enc, helia_pref_changed_entry_rec_enc_video, TRUE, "Encoder/Video", NULL );
	gtk_box_pack_start ( h_box, GTK_WIDGET ( video_enc ), TRUE, TRUE, 0 );

	GtkEntry *video_prop = helia_pref_create_entry_rec_enc_prop ( helia, helia->str_video_prop, helia_pref_changed_entry_rec_prop_video, FALSE, "Encoder/Video", helia->str_audio_enc );
	gtk_box_pack_start ( h_box, GTK_WIDGET ( video_prop ), TRUE, TRUE, 0 );

	gtk_box_pack_start ( helia->rec_vbox, GTK_WIDGET ( h_box ), TRUE, TRUE, 0 );
	h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );

	GtkEntry *muxer_enc = helia_pref_create_entry_rec_enc_prop ( helia, helia->str_muxer_enc, helia_pref_changed_entry_rec_enc_muxer, TRUE, "Muxer", NULL );
	gtk_box_pack_start ( h_box, GTK_WIDGET ( muxer_enc ), TRUE, TRUE, 0 );

	gtk_box_pack_start ( helia->rec_vbox, GTK_WIDGET ( h_box ), TRUE, TRUE, 0 );

	gtk_box_pack_start ( v_box, GTK_WIDGET ( helia->rec_vbox ), FALSE, FALSE, 0 );
}

static void helia_pref_set_rec_dir ( GtkEntry *entry, G_GNUC_UNUSED GtkEntryIconPosition icon_pos, G_GNUC_UNUSED GdkEvent *event, Helia *helia )
{
	char *path = helia_open_dir ( helia, g_get_home_dir () );

	if ( path == NULL ) return;

	if ( helia->rec_dir ) g_free ( helia->rec_dir );

	helia->rec_dir = path;

	gtk_entry_set_text ( entry, helia->rec_dir );
}

static void helia_pref_create_entry_rec_folder ( Helia *helia, const char *icon, const char *set_text, void (*f)(), GtkBox *h_box )
{
	GtkEntry *entry = (GtkEntry *)gtk_entry_new ();
	gtk_entry_set_text ( entry, set_text );

	g_object_set ( entry, "editable", FALSE, NULL );
	gtk_entry_set_icon_from_icon_name ( entry, GTK_ENTRY_ICON_PRIMARY,   icon );
	gtk_entry_set_icon_from_icon_name ( entry, GTK_ENTRY_ICON_SECONDARY, "folder" );
	g_signal_connect ( entry, "icon-press", G_CALLBACK ( f ), helia );

	gtk_box_pack_start ( h_box, GTK_WIDGET ( entry ), TRUE, TRUE, 0 );
}

static GtkBox * helia_pref_record ( Helia *helia )
{
	GtkBox *g_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 );
	gtk_widget_set_margin_start ( GTK_WIDGET ( g_box ), 10 );
	gtk_widget_set_margin_end   ( GTK_WIDGET ( g_box ), 10 );
	gtk_box_set_spacing ( g_box, 5 );

	gtk_box_pack_start ( g_box, GTK_WIDGET ( gtk_label_new ( " " ) ), FALSE, FALSE, 0 );

	GtkBox *h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );

	helia_pref_create_entry_rec_folder ( helia, "helia-record", helia->rec_dir, helia_pref_set_rec_dir, h_box );
	gtk_box_pack_start ( g_box, GTK_WIDGET ( h_box ), FALSE, FALSE, 5 );

	h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );

	gtk_box_pack_start ( g_box, GTK_WIDGET ( gtk_label_new ( " " ) ), FALSE, FALSE, 0 );
	gtk_box_pack_start ( g_box, GTK_WIDGET ( gtk_label_new ( "Encoding  -  Digital TV" ) ), FALSE, FALSE, 5 );

	GtkSwitch *gswitch = helia_pref_create_switch_rec_enc_prop ( helia );
	gtk_box_pack_start ( h_box, GTK_WIDGET ( gswitch ), TRUE, TRUE, 0 );
	gtk_box_pack_start ( g_box, GTK_WIDGET ( h_box ), FALSE, FALSE, 0 );

	helia_pref_create_rec_enc_prop ( helia, g_box );

	gtk_box_pack_start ( g_box, GTK_WIDGET ( gtk_label_new ( " " ) ), FALSE, FALSE, 0 );

	gtk_widget_set_sensitive ( GTK_WIDGET ( helia->rec_vbox ), helia->rec_enc_tv );

	return g_box;
}

static GtkBox * helia_pref_all_box ( Helia *helia, uint i, GtkWindow *window )
{
	if ( i == PAGE_STL )  { return helia_pref_style   ( helia, window ); }
	if ( i == PAGE_DRC )  { return helia_pref_record  ( helia ); }

	return (GtkBox *)gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 );
}

void helia_pref_win ( Helia *helia )
{
	GtkWindow *window = window = helia_create_window_top ( helia->window, "", "helia-pref", GTK_WIN_POS_CENTER_ON_PARENT, TRUE );

	GtkBox *m_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_VERTICAL,   0 );
	GtkBox *h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );

	GtkNotebook *notebook = (GtkNotebook *)gtk_notebook_new ();
	gtk_notebook_set_scrollable ( notebook, TRUE );

	gtk_widget_set_margin_top    ( GTK_WIDGET ( notebook ), 5 );
	gtk_widget_set_margin_bottom ( GTK_WIDGET ( notebook ), 5 );
	gtk_widget_set_margin_start  ( GTK_WIDGET ( notebook ), 5 );
	gtk_widget_set_margin_end    ( GTK_WIDGET ( notebook ), 5 );

	GtkBox *m_box_n[PAGE_NUM];

	uint j = 0; for ( j = 0; j < PAGE_NUM; j++ )
	{
		m_box_n[j] = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 );
		gtk_box_pack_start ( m_box_n[j], GTK_WIDGET ( helia_pref_all_box ( helia, j, window ) ), TRUE, TRUE, 0 );
		gtk_notebook_append_page ( notebook, GTK_WIDGET ( m_box_n[j] ), gtk_label_new ( helia_pref_label_n[j].name ) );
	}

	gtk_notebook_set_tab_pos ( notebook, GTK_POS_TOP );
	gtk_box_pack_start ( m_box, GTK_WIDGET (notebook), TRUE, TRUE, 0 );

	GtkButton *button = helia_set_image_button ( "helia-exit", 16 );
	g_signal_connect_swapped ( button, "clicked", G_CALLBACK ( gtk_widget_destroy ), window );
	gtk_box_pack_start ( h_box,  GTK_WIDGET ( button ), TRUE, TRUE, 5 );

	gtk_box_pack_start ( m_box, GTK_WIDGET ( h_box ), FALSE, FALSE, 5 );
	gtk_container_set_border_width ( GTK_CONTAINER ( m_box ), 5 );
	gtk_container_add ( GTK_CONTAINER ( window ), GTK_WIDGET ( m_box ) );

	gtk_widget_show_all ( GTK_WIDGET ( window ) );

	gtk_widget_set_opacity ( GTK_WIDGET ( window ), helia->opacity_window );
}
