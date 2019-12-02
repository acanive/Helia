/*
* Copyright 2019 Stepan Perun
* This program is free software.
*
* License: Gnu Lesser General Public License
* http://www.gnu.org/licenses/lgpl.html
*/


#include <helia-include.h>


typedef struct _FuncAction FuncAction;

struct _FuncAction
{
	void (*f)();
	const char *func_name;

	uint mod_key;
	uint gdk_key;
};


static void helia_action_hide_show ( GtkBox *box )
{
	if ( gtk_widget_get_visible ( GTK_WIDGET ( box ) ) )
		gtk_widget_hide ( GTK_WIDGET ( box ) );
	else
		gtk_widget_show ( GTK_WIDGET ( box ) );
}

static void helia_action_list ( G_GNUC_UNUSED GSimpleAction *sl, G_GNUC_UNUSED GVariant *pm, Helia *helia )
{
	if ( gtk_widget_get_visible ( GTK_WIDGET ( helia->mp_vbox ) ) )
	{
		( gtk_widget_get_visible ( GTK_WIDGET ( helia->list_mp ) ) ) 
			? gtk_widget_hide ( GTK_WIDGET ( helia->slider ) )
			: gtk_widget_show ( GTK_WIDGET ( helia->slider ) );

		helia_action_hide_show ( GTK_BOX ( helia->list_mp ) );
	}

	if ( gtk_widget_get_visible ( GTK_WIDGET ( helia->tv_vbox ) ) ) helia_action_hide_show ( GTK_BOX ( helia->list_tv ) );
}

static void helia_action_mute_set_tv ( Helia *helia )
{
	helia_gst_set_mute_tv ( helia );
}

static void helia_action_mute_set_mp ( Helia *helia )
{
	helia_gst_set_mute_mp ( helia );
}

static void helia_action_mute ( G_GNUC_UNUSED GSimpleAction *sl, G_GNUC_UNUSED GVariant *pm, Helia *helia )
{
	if ( gtk_widget_get_visible ( GTK_WIDGET ( helia->mp_vbox ) ) ) helia_action_mute_set_mp ( helia );

	if ( gtk_widget_get_visible ( GTK_WIDGET ( helia->tv_vbox ) ) ) helia_action_mute_set_tv ( helia );
}

static void helia_action_play ( G_GNUC_UNUSED GSimpleAction *sl, G_GNUC_UNUSED GVariant *pm, Helia *helia )
{
	if ( helia->file_ch == NULL ) return;

	if ( helia->pipeline_rec != NULL ) return;

	if ( gtk_widget_get_visible ( GTK_WIDGET ( helia->mp_vbox ) ) )
	{
		if ( GST_ELEMENT_CAST ( helia->player )->current_state == GST_STATE_PLAYING )
			gst_element_set_state ( helia->player, GST_STATE_PAUSED );
		else
			gst_element_set_state ( helia->player, GST_STATE_PLAYING );
	}
}

static void helia_action_step ( G_GNUC_UNUSED GSimpleAction *sl, G_GNUC_UNUSED GVariant *pm, Helia *helia )
{
	if ( gtk_widget_get_visible ( GTK_WIDGET ( helia->mp_vbox ) ) ) helia_player_step_frame ( helia );
}

static void helia_action_stop ( G_GNUC_UNUSED GSimpleAction *sl, G_GNUC_UNUSED GVariant *pm, Helia *helia )
{
	if ( gtk_widget_get_visible ( GTK_WIDGET ( helia->mp_vbox ) ) ) helia_player_stop ( helia );

	if ( gtk_widget_get_visible ( GTK_WIDGET ( helia->tv_vbox ) ) ) helia_dtv_stop ( helia );
}

static void helia_action_dir ( G_GNUC_UNUSED GSimpleAction *sl, G_GNUC_UNUSED GVariant *pm, Helia *helia )
{
	if ( gtk_widget_get_visible ( GTK_WIDGET ( helia->mp_vbox ) ) ) helia_dialog_open_dir ( helia );
}

static void helia_action_files ( G_GNUC_UNUSED GSimpleAction *sl, G_GNUC_UNUSED GVariant *pm, Helia *helia )
{
	if ( gtk_widget_get_visible ( GTK_WIDGET ( helia->mp_vbox ) ) ) helia_dialog_open_files ( helia );
}

static void helia_action_net ( G_GNUC_UNUSED GSimpleAction *sl, G_GNUC_UNUSED GVariant *pm, Helia *helia )
{
	if ( gtk_widget_get_visible ( GTK_WIDGET ( helia->mp_vbox ) ) ) helia_open_net ( helia );
}

static void helia_action_slide ( G_GNUC_UNUSED GSimpleAction *sl, G_GNUC_UNUSED GVariant *pm, Helia *helia )
{
	if ( gtk_widget_get_visible ( GTK_WIDGET ( helia->mp_vbox ) ) )
	{
		if ( gtk_widget_get_visible ( GTK_WIDGET ( helia->slider ) ) )
			gtk_widget_hide ( GTK_WIDGET ( helia->slider ) );
		else
			gtk_widget_show ( GTK_WIDGET ( helia->slider ) );
	}
}

static FuncAction func_action_n[] =
{
	{ helia_action_dir,   "add_dir",     GDK_CONTROL_MASK, GDK_KEY_D },
	{ helia_action_files, "add_files",   GDK_CONTROL_MASK, GDK_KEY_O },
	{ helia_action_net,   "add_net",     GDK_CONTROL_MASK, GDK_KEY_L },
	{ helia_action_list,  "playlist",  	 GDK_CONTROL_MASK, GDK_KEY_H },
	{ helia_action_slide, "slider",  	 GDK_CONTROL_MASK, GDK_KEY_Z },
	{ helia_action_mute,  "mute", 		 GDK_CONTROL_MASK, GDK_KEY_M },
	{ helia_action_play,  "play_paused", 0, GDK_KEY_space  },
	{ helia_action_step,  "play_step",   0, GDK_KEY_period },
	{ helia_action_stop,  "stop", 		 GDK_CONTROL_MASK, GDK_KEY_X },
};

static void helia_app_add_accelerator ( GtkApplication *app, uint i )
{
	char *accel_name = gtk_accelerator_name ( func_action_n[i].gdk_key, func_action_n[i].mod_key );

	const char *accel_str[] = { accel_name, NULL };

	char *text = g_strconcat ( "app.", func_action_n[i].func_name, NULL );

		gtk_application_set_accels_for_action ( app, text, accel_str );

	g_free ( text );

	g_free ( accel_name );
}

void helia_create_gaction_entry ( GtkApplication *app, Helia *helia )
{
	GActionEntry entries[ G_N_ELEMENTS ( func_action_n ) ];

	uint i = 0;

	for ( i = 0; i < G_N_ELEMENTS ( func_action_n ); i++ )
	{
		entries[i].name           = func_action_n[i].func_name;
		entries[i].activate       = func_action_n[i].f;
		entries[i].parameter_type = NULL;
		entries[i].state          = NULL;

		helia_app_add_accelerator ( app, i );
	}

	g_action_map_add_action_entries ( G_ACTION_MAP ( app ), entries, G_N_ELEMENTS ( entries ), helia );
}

static void helia_info_change_state_mouse_click ( GtkButton *button, Helia *helia )
{
	helia->state_mouse_click = !helia->state_mouse_click;

	GtkImage *image = helia_create_image ( ( helia->state_mouse_click ) ? "helia-set" : "helia-unset", 24 );
	gtk_button_set_image ( button, GTK_WIDGET ( image ) );
}

static GtkBox * helia_keyb_win_create_mouse ( Helia *helia )
{
	GtkBox *hbox = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );

	GtkButton *button = helia_set_image_button ( ( helia->state_mouse_click ) ? "helia-set" : "helia-unset", 24 );
	g_signal_connect ( button, "clicked", G_CALLBACK ( helia_info_change_state_mouse_click ), helia );

	gtk_box_pack_end ( hbox, GTK_WIDGET ( button ), FALSE, FALSE, 0 );

	GtkImage *image_m = helia_create_image ( "helia-mouse", 32 );
	gtk_box_pack_end ( hbox, GTK_WIDGET ( image_m ), FALSE, FALSE, 0 );

	return hbox;
}

static GtkBox * helia_keyb_win_create_play_pause ( const char *name_a, const char *name_b )
{
	GtkImage *image_a = helia_create_image ( name_a, 32 );
	GtkImage *image_b = helia_create_image ( name_b, 32 );

	GtkBox *hbox = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
	gtk_widget_set_halign ( GTK_WIDGET ( hbox ), GTK_ALIGN_START );

	gtk_box_pack_start ( hbox, GTK_WIDGET ( image_a ), FALSE, FALSE, 0 );
	gtk_box_pack_start ( hbox, GTK_WIDGET ( image_b ), FALSE, FALSE, 5 );

	return hbox;
}

void helia_keyb_win ( Helia *helia )
{
	GtkWindow *window = helia_create_window_top ( helia->window, "", "helia-keyb", GTK_WIN_POS_CENTER_ON_PARENT, TRUE );
	gtk_window_set_default_size ( window, 300, -1 );

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
		{ "helia-slider", 		"Ctrl + Z" },
		{ "helia-muted", 		"Ctrl + M" },
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
			GtkBox *hbox = helia_keyb_win_create_play_pause ( "helia-play", "helia-pause" );
			gtk_grid_attach ( GTK_GRID ( grid ), GTK_WIDGET ( hbox ), 0, d, 1, 1 );

			GtkBox *mbox = helia_keyb_win_create_mouse ( helia );
			gtk_grid_attach ( GTK_GRID ( grid ), GTK_WIDGET ( mbox ), 1, d, 1, 1 );

			GtkLabel *label = (GtkLabel *)gtk_label_new ( data_a_n[d].accel );
			gtk_widget_set_halign ( GTK_WIDGET ( label ), GTK_ALIGN_END );
			gtk_grid_attach ( GTK_GRID ( grid ), GTK_WIDGET ( label ), 2, d, 1, 1 );

			continue;
		}

		GtkImage *image = helia_create_image ( data_a_n[d].image, 32 );
		gtk_widget_set_halign ( GTK_WIDGET ( image ), GTK_ALIGN_START );
		gtk_grid_attach ( GTK_GRID ( grid ), GTK_WIDGET ( image ), 0, d, 1, 1 );

		GtkLabel *label = (GtkLabel *)gtk_label_new ( data_a_n[d].accel );
		gtk_widget_set_halign ( GTK_WIDGET ( label ), GTK_ALIGN_END );
		gtk_grid_attach ( GTK_GRID ( grid ), GTK_WIDGET ( label ), 2, d, 1, 1 );
	}

	gtk_box_pack_start ( m_box, GTK_WIDGET ( g_box ), TRUE, TRUE, 0 );

	h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );

	GtkButton *button_close = helia_set_image_button ( "helia-exit", 16 );
	g_signal_connect_swapped ( button_close, "clicked", G_CALLBACK ( gtk_widget_destroy ), window );
	gtk_box_pack_end ( h_box, GTK_WIDGET ( button_close ), TRUE, TRUE, 0 );

	gtk_box_pack_end ( m_box, GTK_WIDGET ( h_box ), FALSE, FALSE, 5 );

	gtk_container_set_border_width ( GTK_CONTAINER ( m_box ), 10 );
	gtk_container_add   ( GTK_CONTAINER ( window ), GTK_WIDGET ( m_box ) );
	gtk_widget_show_all ( GTK_WIDGET ( window ) );

	gtk_widget_set_opacity ( GTK_WIDGET ( window ), helia->opacity_window );
}
