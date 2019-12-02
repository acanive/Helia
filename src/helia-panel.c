/*
* Copyright 2019 Stepan Perun
* This program is free software.
*
* License: Gnu Lesser General Public License
* http://www.gnu.org/licenses/lgpl.html
*/


#include <helia-include.h>


static void helia_panel_mute_mp ( G_GNUC_UNUSED GtkButton *button, Helia *helia )
{
	helia_gst_set_mute_mp ( helia );
}

static void helia_panel_mute_tv ( G_GNUC_UNUSED GtkButton *button, Helia *helia )
{
	helia_gst_set_mute_tv ( helia );
}

static void helia_panel_volume_changed_mp ( G_GNUC_UNUSED GtkScaleButton *button, gdouble value, Helia *helia )
{
	helia_gst_set_volume_mp ( value, helia );
}

static void helia_panel_volume_changed_tv ( G_GNUC_UNUSED GtkScaleButton *button, gdouble value, Helia *helia )
{
	helia_gst_set_volume_tv ( value, helia );
}

static void helia_panel_eqa_mp ( G_GNUC_UNUSED GtkButton *button, Helia *helia )
{
	GstElement *element = helia_gst_iterate_element ( helia->player, "equalizer", NULL );

	if ( helia->pipeline_rec )
		element = helia_gst_iterate_element ( helia->pipeline_rec, "equalizer", NULL );

	if ( element && GST_ELEMENT_CAST ( element )->current_state == GST_STATE_PLAYING )
		helia_eqa_win ( element, helia );
}

static void helia_panel_eqa_tv ( G_GNUC_UNUSED GtkButton *button, Helia *helia )
{
	if ( GST_ELEMENT_CAST ( helia->dtv )->current_state != GST_STATE_PLAYING ) return;

	GstElement *element = helia_gst_iterate_element ( helia->dtv, "equalizer", NULL );

	if ( element ) helia_eqa_win ( element, helia );
}

static void helia_panel_eqv_mp ( G_GNUC_UNUSED GtkButton *button, Helia *helia )
{
	GstElement *element = helia_gst_iterate_element ( helia->player, "videobalance", NULL );

	if ( helia->pipeline_rec )
		element = helia_gst_iterate_element ( helia->pipeline_rec, "videobalance", NULL );

	if ( element && GST_ELEMENT_CAST ( element )->current_state == GST_STATE_PLAYING )
		helia_eqv_win ( element, helia );
}

static void helia_panel_eqv_tv ( G_GNUC_UNUSED GtkButton *button, Helia *helia )
{
	if ( GST_ELEMENT_CAST ( helia->dtv )->current_state != GST_STATE_PLAYING ) return;

	GstElement *element = helia_gst_iterate_element ( helia->dtv, "videobalance", NULL );

	if ( element ) helia_eqv_win ( element, helia );
}

static void helia_panel_list_clear_file_ch ( Helia *helia )
{
	if ( helia->file_ch ) g_free ( helia->file_ch );

	helia->file_ch = NULL;
}

static void helia_panel_base ( G_GNUC_UNUSED GtkButton *button, Helia *helia )
{
	helia_player_stop ( helia );
	helia_dtv_stop ( helia );

	helia_panel_list_clear_file_ch ( helia );

	helia_window_set_win_base ( NULL, helia );
}

static void helia_panel_list_hide_show ( GtkBox *box )
{
	if ( gtk_widget_get_visible ( GTK_WIDGET ( box ) ) )
		gtk_widget_hide ( GTK_WIDGET ( box ) );
	else
		gtk_widget_show ( GTK_WIDGET ( box ) );
}

static void helia_panel_hide_mp ( Helia *helia )
{
	if ( gtk_widget_get_visible ( GTK_WIDGET ( helia->slider ) ) )
	{
		GtkWindow *window = GTK_WINDOW ( gtk_widget_get_toplevel ( GTK_WIDGET ( helia->slider_panel ) ) );

		gtk_widget_hide ( GTK_WIDGET ( helia->slider_panel ) );
		gtk_window_resize ( window, helia->icon_size * 15, helia->icon_size * 2 );
	}
	else
	{
		gtk_widget_show ( GTK_WIDGET ( helia->slider_panel ) );
	}

	helia_player_slider_update ( helia );
}

static void helia_panel_hide_tv ( Helia *helia )
{
	if ( gtk_widget_get_visible ( GTK_WIDGET ( helia->list_tv ) ) )
	{
		GtkWindow *window = GTK_WINDOW ( gtk_widget_get_toplevel ( GTK_WIDGET ( helia->level_panel ) ) );

		gtk_widget_hide ( GTK_WIDGET ( helia->level_panel ) );
		gtk_window_resize ( window, helia->icon_size * 15, helia->icon_size * 2 );
	}
	else
		gtk_widget_show ( GTK_WIDGET ( helia->level_panel ) );
}

static void helia_panel_list_mp ( G_GNUC_UNUSED GtkButton *button, Helia *helia )
{
	( gtk_widget_get_visible ( GTK_WIDGET ( helia->list_mp ) ) ) 
		? gtk_widget_hide ( GTK_WIDGET ( helia->slider ) )
		: gtk_widget_show ( GTK_WIDGET ( helia->slider ) );

	helia_panel_list_hide_show ( GTK_BOX ( helia->list_mp ) );

	if ( helia->panel_ext ) helia_panel_hide_mp ( helia );
}

static void helia_panel_list_tv ( G_GNUC_UNUSED GtkButton *button, Helia *helia )
{
	helia_panel_list_hide_show ( GTK_BOX ( helia->list_tv ) );

	if ( helia->panel_ext )  helia_panel_hide_tv ( helia );
}

static void helia_panel_button_set_icon ( GtkButton *button, const char *name, int icon_size )
{
	GdkPixbuf *pixbuf = gtk_icon_theme_load_icon ( gtk_icon_theme_get_default (), 
				name, icon_size, GTK_ICON_LOOKUP_USE_BUILTIN, NULL );

	GtkImage *image   = (GtkImage  *)gtk_image_new_from_pixbuf ( pixbuf );
	gtk_button_set_image ( button, GTK_WIDGET ( image ) );

	if ( pixbuf ) g_object_unref ( pixbuf );
}

static void helia_panel_play_mp ( GtkButton *button, Helia *helia )
{
	if ( helia->file_ch == NULL ) return;

	if ( helia->pipeline_rec != NULL ) return;

	const char *name = NULL;

	if ( GST_ELEMENT_CAST ( helia->player )->current_state == GST_STATE_PLAYING )
	{
		gst_element_set_state ( helia->player, GST_STATE_PAUSED  );
		name = "helia-play";

		gtk_widget_set_sensitive ( GTK_WIDGET ( helia->volbutton_mp ), FALSE );
	}
	else
	{
		gst_element_set_state ( helia->player, GST_STATE_PLAYING );
		name = "helia-pause";

		g_usleep ( 1000000 );
		gtk_widget_set_sensitive ( GTK_WIDGET ( helia->volbutton_mp ), !helia_gst_mute_get_mp ( helia ) );
	}

	helia_panel_button_set_icon ( button, name, helia->icon_size );
}

static void helia_panel_stop_set_icon ( GtkButton *button, GtkButton *button_play )
{
	int size_image = gtk_image_get_pixel_size ( GTK_IMAGE ( gtk_button_get_image ( button ) ) );

	helia_panel_button_set_icon ( button_play, "helia-play", size_image );
}

static void helia_panel_stop_mp ( G_GNUC_UNUSED GtkButton *button, Helia *helia )
{
	helia_player_stop ( helia );

	gtk_widget_set_sensitive ( GTK_WIDGET ( helia->volbutton_mp ), FALSE );
}

static void helia_panel_stop_tv ( G_GNUC_UNUSED GtkButton *button, Helia *helia )
{
	helia_dtv_stop ( helia );

	gtk_widget_set_sensitive ( GTK_WIDGET ( helia->volbutton_tv ), FALSE );
}

static void helia_panel_info_mp ( G_GNUC_UNUSED GtkButton *button, Helia *helia )
{
	GstElement *element = helia->player;

	/*if ( helia->pipeline_rec ) element = helia->pipeline_rec;*/

	if ( GST_ELEMENT_CAST ( element )->current_state == GST_STATE_PLAYING )
		helia_info_new ( helia, TRUE );
}

static void helia_panel_info_tv ( G_GNUC_UNUSED GtkButton *button, Helia *helia )
{
	if ( GST_ELEMENT_CAST ( helia->dtv )->current_state == GST_STATE_PLAYING )
		helia_info_new ( helia, FALSE );
}

static void helia_panel_rec_mp ( G_GNUC_UNUSED GtkButton *button, Helia *helia )
{
	helia_player_record ( helia );
}

static void helia_panel_rec_tv ( G_GNUC_UNUSED GtkButton *button, Helia *helia )
{
	helia_dtv_gst_record ( helia );
}

static void helia_panel_scan ( G_GNUC_UNUSED GtkButton *button, Helia *helia )
{
	helia_scan_win_create ( helia );
}

static void helia_panel_exit ( G_GNUC_UNUSED GtkButton *button, G_GNUC_UNUSED Helia *helia )
{
	// swapped
}

static GtkBox * helia_panel_create ( GtkWindow *window, Helia *helia, gboolean mp_tv, IconFunc *ic_fn, uint num, GtkVolumeButton *volbutton, double vol, gboolean mute, void (*f)() )
{
	GtkBox *m_box  = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_VERTICAL,   0 );

	GtkBox *b_box  = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
	GtkBox *l_box  = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_VERTICAL,   0 );
	GtkBox *r_box  = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_VERTICAL,   0 );
	GtkBox *a_box  = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_VERTICAL,   0 );

	GtkBox *h_box  = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
	GtkBox *hm_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );

	gtk_box_set_spacing ( b_box,  5 );
	gtk_box_set_spacing ( l_box,  5 );
	gtk_box_set_spacing ( h_box,  5 );
	gtk_box_set_spacing ( hm_box, 5 );
	gtk_box_set_spacing ( a_box,  5 );

	GtkButton *button_play = NULL;

	gtk_scale_button_set_value ( GTK_SCALE_BUTTON ( volbutton ), vol );
	gtk_widget_set_sensitive ( GTK_WIDGET ( volbutton ), !mute );
	g_signal_connect ( volbutton, "value-changed", G_CALLBACK ( f ), helia );

	uint i = 0;
	for ( i = 0; i < num; i++ )
	{
		GtkButton *button = helia_set_image_button ( ic_fn[i].name, helia->icon_size );
		g_signal_connect ( button, "clicked", G_CALLBACK ( ic_fn[i].f ), helia );

		if ( ic_fn[i].swap_close ) g_signal_connect_swapped ( button, "clicked", G_CALLBACK ( gtk_widget_destroy ), window );

		if ( i == 4 ) gtk_box_pack_start ( l_box, GTK_WIDGET ( h_box ), TRUE, TRUE, 0 );

		gtk_box_pack_start ( ( i < 5 ) ? h_box : hm_box, GTK_WIDGET ( button ), TRUE, TRUE, 0 );

		if ( i == 5 ) button_play = button;
		if ( mp_tv && i == 6 ) g_signal_connect ( button, "clicked", G_CALLBACK ( helia_panel_stop_set_icon ), button_play );
	}

	gtk_box_pack_start ( l_box, GTK_WIDGET ( hm_box ), TRUE, TRUE, 0 );

	gtk_box_pack_start ( r_box, GTK_WIDGET ( volbutton ), TRUE, TRUE, 0 );

	gtk_box_pack_start ( b_box, GTK_WIDGET ( l_box ), TRUE,  TRUE,  0 );
	gtk_box_pack_start ( b_box, GTK_WIDGET ( a_box ), FALSE, FALSE, 0 );
	gtk_box_pack_start ( b_box, GTK_WIDGET ( r_box ), FALSE, FALSE, 0 );

	gtk_box_pack_start ( m_box, GTK_WIDGET ( b_box ), TRUE,  TRUE,  0 );

	gtk_container_set_border_width ( GTK_CONTAINER ( m_box ), 10 );
	gtk_container_add ( GTK_CONTAINER ( window ), GTK_WIDGET ( m_box ) );

	return m_box; 
}

static void helia_panel_set_mp ( GtkWindow *window, Helia *helia )
{
	const char *name = ( GST_ELEMENT_CAST ( helia->player )->current_state == GST_STATE_PLAYING ) ? "helia-pause" : "helia-play";

	IconFunc IconFunc_n[] =
	{
		{ "helia-mp", 		TRUE,  helia_panel_base 	},
		{ "helia-editor", 	FALSE, helia_panel_list_mp 	},
		{ "helia-eqa", 		TRUE,  helia_panel_eqa_mp 	},
		{ "helia-eqv", 		TRUE,  helia_panel_eqv_mp	},
		{ "helia-muted", 	FALSE, helia_panel_mute_mp 	},

		{ name,		   		FALSE, helia_panel_play_mp 	},
		{ "helia-stop", 	FALSE, helia_panel_stop_mp  },
		{ "helia-record", 	FALSE, helia_panel_rec_mp	},
		{ "helia-info", 	TRUE,  helia_panel_info_mp 	},
		{ "helia-exit", 	TRUE,  helia_panel_exit 	}
	};

	helia->volbutton_mp = (GtkVolumeButton *)gtk_volume_button_new ();

	GtkBox *m_box = helia_panel_create ( window, helia, TRUE, IconFunc_n, G_N_ELEMENTS ( IconFunc_n ), 
		helia->volbutton_mp, helia->volume_mp, helia_gst_mute_get_mp ( helia ), helia_panel_volume_changed_mp );

	if ( !helia->panel_ext ) return;

	helia->slider_panel = helia_slider_new ();
	helia->slider_panel->slider_signal_id = g_signal_connect ( helia->slider_panel->slider, "value-changed", G_CALLBACK ( helia_player_slider_panel_seek_changed ), helia );
	gtk_box_pack_start ( m_box, GTK_WIDGET ( helia->slider_panel ), TRUE, TRUE, 0 );
}

static void helia_panel_set_tv ( GtkWindow *window, Helia *helia )
{
	IconFunc IconFunc_n[] =
	{
		{ "helia-tv", 		TRUE,  helia_panel_base 	},
		{ "helia-editor", 	FALSE, helia_panel_list_tv 	},
		{ "helia-eqa", 		TRUE,  helia_panel_eqa_tv 	},
		{ "helia-eqv", 		TRUE,  helia_panel_eqv_tv 	},
		{ "helia-muted", 	FALSE, helia_panel_mute_tv 	},

		{ "helia-stop",   	FALSE, helia_panel_stop_tv 	},
		{ "helia-record", 	FALSE, helia_panel_rec_tv  	},
		{ "helia-display", 	TRUE,  helia_panel_scan  	},
		{ "helia-info", 	TRUE,  helia_panel_info_tv 	},
		{ "helia-exit", 	TRUE,  helia_panel_exit 	}
	};

	helia->volbutton_tv = (GtkVolumeButton *)gtk_volume_button_new ();

	GtkBox *m_box = helia_panel_create ( window, helia, FALSE, IconFunc_n, G_N_ELEMENTS ( IconFunc_n ), 
		helia->volbutton_tv, helia->volume_tv, helia_gst_mute_get_tv ( helia->dtv ), helia_panel_volume_changed_tv );

	if ( !helia->panel_ext ) return;

	helia->level_panel = helia_level_new ();
	gtk_box_pack_start ( m_box, GTK_WIDGET ( helia->level_panel ), TRUE, TRUE, 0 );
}

static void helia_panel_quit ( G_GNUC_UNUSED GtkWindow *win, Helia *helia )
{
	helia->volbutton_mp = NULL;
	helia->volbutton_tv = NULL;

	helia->level_panel  = NULL;;
	helia->slider_panel = NULL;
}

void helia_panel_new ( Helia *helia, gboolean mp_tv )
{
	GtkWindow *window = helia_create_window_top ( helia->window, "", "helia-panel", GTK_WIN_POS_CENTER_ON_PARENT, TRUE );
	gtk_window_set_decorated ( window, FALSE  );
	gtk_window_set_default_size ( window, helia->icon_size * 15, helia->icon_size * 2 );
	g_signal_connect ( window, "destroy", G_CALLBACK ( helia_panel_quit ), helia );

	( mp_tv ) ? helia_panel_set_mp ( window, helia ) : helia_panel_set_tv ( window, helia );

	gtk_widget_show_all ( GTK_WIDGET ( window ) );
	gtk_widget_set_opacity ( GTK_WIDGET ( window ), helia->opacity_panel );

	if ( !helia->panel_ext ) return;

	( mp_tv ) ? helia_panel_hide_mp ( helia ) : helia_panel_hide_tv ( helia );
}
