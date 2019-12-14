/*
* Copyright 2019 Stepan Perun
* This program is free software.
*
* License: Gnu Lesser General Public License
* http://www.gnu.org/licenses/lgpl.html
*/


#include <helia-include.h>

#include <glib/gi18n.h>
#include <glib/gstdio.h>


G_DEFINE_TYPE ( Helia, helia, GTK_TYPE_APPLICATION )


static uint helia_power_manager_inhibit ( GDBusConnection *connect )
{
	uint cookie;
	GError *err = NULL;

	GVariant *reply = g_dbus_connection_call_sync ( connect,
						"org.freedesktop.PowerManagement",
						"/org/freedesktop/PowerManagement/Inhibit",
						"org.freedesktop.PowerManagement.Inhibit",
						"Inhibit",
						g_variant_new ("(ss)", "Helia", "Video" ),
						G_VARIANT_TYPE ("(u)"),
						G_DBUS_CALL_FLAGS_NONE,
						-1,
						NULL,
						&err );

	if ( reply != NULL )
	{
		g_variant_get ( reply, "(u)", &cookie, NULL );
		g_variant_unref ( reply );

		return cookie;
	}

	if ( err )
	{
		g_warning ( "Inhibiting failed %s", err->message );
		g_error_free ( err );
	}

	return 0;
}

static void helia_power_manager_uninhibit ( GDBusConnection *connect, uint cookie )
{
	GError *err = NULL;

	GVariant *reply = g_dbus_connection_call_sync ( connect,
						"org.freedesktop.PowerManagement",
						"/org/freedesktop/PowerManagement/Inhibit",
						"org.freedesktop.PowerManagement.Inhibit",
						"UnInhibit",
						g_variant_new ("(u)", cookie),
						NULL,
						G_DBUS_CALL_FLAGS_NONE,
						-1,
						NULL,
						&err );

	if ( err )
	{
		g_warning ( "Uninhibiting failed %s", err->message );
		g_error_free ( err );
	}

	g_variant_unref ( reply );
}

static void helia_power_manager_on ( Helia *helia )
{
	if ( helia->cookie > 0 ) helia_power_manager_uninhibit ( helia->connect, helia->cookie );

	helia->cookie = 0;
}

static void helia_power_manager_off ( Helia *helia )
{
	helia->cookie = helia_power_manager_inhibit ( helia->connect );

	g_debug ( "%s:: cookie %d ", __func__, helia->cookie );
}

void helia_power_manager ( Helia *helia, gboolean power_off )
{
	if ( helia->power )
	{
		if ( power_off )
			helia_power_manager_off ( helia );
		else
			helia_power_manager_on  ( helia );
	}
}

static GDBusConnection * helia_dbus_init ()
{
	GError *err = NULL;

	GDBusConnection *connect = g_bus_get_sync ( G_BUS_TYPE_SESSION, NULL, &err );

	if ( err )
	{
		g_warning ( "%s:: Failed to get session bus: %s", __func__, err->message );
		g_error_free ( err );
	}

	return connect;
}

static void helia_new_window ( GApplication *app, GFile **files, int n_files )
{
	Helia *helia = HELIA_APPLICATION ( app );

	if ( g_file_test ( helia->helia_conf, G_FILE_TEST_EXISTS ) ) helia_pref_read_config ( helia );

	helia->video_mp = helia_video_new ( helia, TRUE );
	helia->video_tv = helia_video_new ( helia, FALSE );

	helia->dtv    = helia_dtv_new ( helia );
	helia->player = helia_player_new ( helia );

	helia->scan   = helia_scan_new ( helia );
	helia->mpegts = g_new ( MpegTs, 1 );

	helia->treeview_mp = helia_treeview_new ( helia, TRUE );
	helia->treeview_tv = helia_treeview_new ( helia, FALSE );

	helia->level   = helia_level_new ();
	helia->slider  = helia_slider_new ();

	helia->list_mp = helia_list_new ( helia, TRUE  );
	helia->list_tv = helia_list_new ( helia, FALSE );

	helia->window = helia_window_new ( helia );

	helia->connect = helia_dbus_init ();

	helia_treeview_add_start ( files, n_files, helia );

	g_timeout_add ( 100, (GSourceFunc)helia_player_slider_refresh, helia );
}

static void helia_activate ( GApplication *app )
{
	helia_new_window ( app, NULL, 0 );
}

static void helia_open ( GApplication  *app, GFile **files, int n_files, G_GNUC_UNUSED const char *hint )
{
	helia_new_window ( app, files, n_files );
}

static void helia_init ( Helia *helia )
{
	helia->pipeline_rec = NULL;

	helia->power = TRUE;
	helia->dark_theme = TRUE;
	helia->cookie = 0;
	helia->volume_mp  = 0.5;
	helia->volume_tv  = 0.5;
	helia->win_width  = 900;
	helia->win_height = 400;
	helia->icon_size  = 24;
	helia->opacity_eqav   = 0.85;
	helia->opacity_panel  = 0.85;
	helia->opacity_window = 1.0;

	helia->xid_mp     = 0;
	helia->xid_tv     = 0;
	helia->sid        = 0;
	helia->lnb_type   = 0;

	helia->record_tv  = FALSE;
	helia->rec_enc_tv = FALSE;
	helia->repeat     = FALSE;
	helia->set_audio_track = 0;
	helia->state_subtitle  = TRUE;
	helia->double_click    = FALSE;
	helia->state_mouse_click = FALSE;
	helia->panel_ext = FALSE;
	helia->scrambling = FALSE;

	helia->label_audio  = NULL;
	helia->label_video  = NULL;
	helia->volbutton_mp = NULL;
	helia->volbutton_tv = NULL;
	helia->level_panel  = NULL;
	helia->slider_panel = NULL;

	helia->str_audio_enc  = g_strdup ( "vorbisenc" );
	helia->str_video_enc  = g_strdup ( "theoraenc" );
	helia->str_muxer_enc  = g_strdup ( "oggmux"    );
	helia->str_audio_prop = g_strdup ( "float=quality=0,5" );  // "uint=bitrate=128000"
	helia->str_video_prop = g_strdup ( "int=quality=48" );     // "uint=bitrate=2000"

	helia->file_ch = NULL;
	helia->rec_dir = g_strdup    ( g_get_home_dir () );
	helia->ch_conf = g_strconcat ( g_get_user_config_dir (), "/helia/gtv-channel.conf", NULL );
	helia->helia_conf = g_strconcat ( g_get_user_config_dir (), "/helia/gtv.conf", NULL );

	uint i; for ( i = 0; i < MAX_AUDIO; i++ ) helia->audio_lang[i] = NULL;

	char *dir_conf = g_strdup_printf ( "%s/helia", g_get_user_config_dir () );

	if ( !g_file_test ( dir_conf, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR ) )
	{
		g_mkdir ( dir_conf, 0775 );
		g_print ( "Creating %s directory. \n", dir_conf );
	}

	g_free ( dir_conf );
}

static void helia_finalize ( GObject *object )
{
	G_OBJECT_CLASS (helia_parent_class)->finalize (object);
}

static void helia_class_init ( HeliaClass *klass )
{
	G_APPLICATION_CLASS (klass)->activate = helia_activate;
	G_APPLICATION_CLASS (klass)->open     = helia_open;

	G_OBJECT_CLASS (klass)->finalize      = helia_finalize;
}

Helia * helia_new (void)
{
	return g_object_new ( helia_get_type (), "flags", G_APPLICATION_HANDLES_OPEN, NULL );
}

static void helia_help ( const char *arg_1 )
{
	const char *help = 
"Examples: \n\
  File ( path ):  helia \"/path/to/playlist.m3u\" \n\
  File ( uri  ):  helia \"file:///path/to/playlist.m3u\" \n\
  Net  ( uri  ):  helia \"http://radio.hbr1.com:19800/ambient.ogg\" \n\
  Channel:        helia channel \"BBC World\"\n\n\
Showing debug: \n\
  G_MESSAGES_DEBUG=all helia";

	if ( g_str_has_prefix ( arg_1, "--help" ) || g_str_has_prefix ( arg_1, "-h" ) )
		g_print ( "\n%s \n\n", help );
}

int main ( int argc, char *argv[] )
{
	if ( argc > 1 ) helia_help ( argv[1] );

	gst_init ( NULL, NULL );

	bindtextdomain ( "helia", "/usr/share/locale" );
	// bindtextdomain ( "helia", "build/locale" ); // test gettext
	bind_textdomain_codeset ( "helia", "UTF-8" );
	textdomain ( "helia" );

	Helia *app = helia_new ();

	int status = g_application_run ( G_APPLICATION (app), argc, argv );

	g_object_unref (app);

	return status;
}
