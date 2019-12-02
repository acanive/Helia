/*
* Copyright 2019 Stepan Perun
* This program is free software.
*
* License: Gnu Lesser General Public License
* http://www.gnu.org/licenses/lgpl.html
*/


#include <helia-include.h>

#include <gst/video/videooverlay.h>


GstElement * helia_gst_iterate_element ( GstElement *it_element, const char *name1, const char *name2 )
{
	GstIterator *it = gst_bin_iterate_recurse ( GST_BIN ( it_element ) );

	GValue item = { 0, };
	gboolean done = FALSE;

	GstElement *element_ret = NULL;

	while ( !done )
	{
		switch ( gst_iterator_next ( it, &item ) )
		{
			case GST_ITERATOR_OK:
			{
				GstElement *element = GST_ELEMENT ( g_value_get_object (&item) );

				char *object_name = gst_object_get_name ( GST_OBJECT ( element ) );

				if ( g_strrstr ( object_name, name1 ) )
				{
					if ( name2 && g_strrstr ( object_name, name2 ) )
						element_ret = element;
					else
						element_ret = element;
				}

				// g_debug ( "%s:: Object name: %s ", __func__, object_name );

				g_free ( object_name );
				g_value_reset (&item);

				break;
			}

			case GST_ITERATOR_RESYNC:
				gst_iterator_resync (it);
				break;

			case GST_ITERATOR_ERROR:
				done = TRUE;
				break;

			case GST_ITERATOR_DONE:
				done = TRUE;
				break;
		}
	}

	g_value_unset ( &item );
	gst_iterator_free ( it );

	return element_ret;
}

static void helia_gst_remove_bin ( GstElement *pipeline, const char *name )
{
	GstIterator *it = gst_bin_iterate_elements ( GST_BIN ( pipeline ) );
	GValue item = { 0, };
	gboolean done = FALSE;

	while ( !done )
	{
		switch ( gst_iterator_next ( it, &item ) )
		{
			case GST_ITERATOR_OK:
			{
				GstElement *element = GST_ELEMENT ( g_value_get_object (&item) );

				char *object_name = gst_object_get_name ( GST_OBJECT ( element ) );

				if ( name && g_strrstr ( object_name, name ) )
				{
					g_debug ( "%s:: Object Not remove: %s \n", __func__, object_name );
				}
				else
				{
					gst_element_set_state ( element, GST_STATE_NULL );
					gst_bin_remove ( GST_BIN ( pipeline ), element );

					g_debug ( "%s:: Object remove: %s \n", __func__, object_name );
				}

				g_free ( object_name );
				g_value_reset (&item);

				break;
			}

			case GST_ITERATOR_RESYNC:
				gst_iterator_resync (it);
				break;

			case GST_ITERATOR_ERROR:
				done = TRUE;
				break;

			case GST_ITERATOR_DONE:
				done = TRUE;
				break;
		}
	}

	g_value_unset ( &item );
	gst_iterator_free ( it );
}

static char * helia_time_to_str ()
{
	GDateTime *date = g_date_time_new_now_local ();

	char *str_time = g_date_time_format ( date, "%j-%Y-%T" );

	g_date_time_unref ( date );

	return str_time;
}

gboolean helia_gst_mute_get_tv ( HeliaTv *dtv )
{
	if ( GST_ELEMENT_CAST ( dtv )->current_state != GST_STATE_PLAYING ) return TRUE;

	GstElement *element = helia_gst_iterate_element ( dtv, "autoaudiosink", "actual-sink" );

	if ( element == NULL ) return TRUE;

	gboolean mute = TRUE;
	g_object_get ( element, "mute", &mute, NULL );

	return mute;
}

gboolean helia_gst_mute_get_mp ( Helia *helia )
{
	gboolean mute = TRUE;

	GstElement *element = helia->player;

	if ( helia->pipeline_rec )
		element = helia_gst_iterate_element ( helia->pipeline_rec, "autoaudiosink", "actual-sink" );

	if ( element && GST_ELEMENT_CAST ( element )->current_state == GST_STATE_PLAYING )
		g_object_get ( element, "mute", &mute, NULL );

	return mute;
}

static void helia_gst_mute_set ( GstElement *element )
{
	gboolean mute = FALSE;

	g_object_get ( element, "mute", &mute, NULL );
	g_object_set ( element, "mute", !mute, NULL );
}

void helia_gst_set_mute_mp ( Helia *helia )
{
	GstElement *element = helia->player;

	if ( helia->pipeline_rec )
		element = helia_gst_iterate_element ( helia->pipeline_rec, "autoaudiosink", "actual-sink" );

	if ( element && GST_ELEMENT_CAST ( element )->current_state == GST_STATE_PLAYING )
	{
		helia_gst_mute_set ( element );

		if ( helia->volbutton_mp )
			gtk_widget_set_sensitive ( GTK_WIDGET ( helia->volbutton_mp ), !helia_gst_mute_get_mp ( helia ) );
	}
}

void helia_gst_set_mute_tv ( Helia *helia )
{
	GstElement *element = helia_gst_iterate_element ( helia->dtv, "autoaudiosink", "actual-sink" );

	if ( element && GST_ELEMENT_CAST ( element )->current_state == GST_STATE_PLAYING )
	{
		helia_gst_mute_set ( element );

		if ( helia->volbutton_tv )
			gtk_widget_set_sensitive ( GTK_WIDGET ( helia->volbutton_tv ), !helia_gst_mute_get_tv ( helia->dtv ) );
	}
}

void helia_gst_set_volume_mp ( gdouble value, Helia *helia )
{
	GstElement *element = helia->player;

	if ( helia->pipeline_rec )
		element = helia_gst_iterate_element ( helia->pipeline_rec, "autoaudiosink", "actual-sink" );

	if ( element /*&& GST_ELEMENT_CAST ( element )->current_state == GST_STATE_PLAYING*/ )
	{
		helia->volume_mp = value;
		g_object_set ( element, "volume", value, NULL );
	}
}

void helia_gst_set_volume_tv ( gdouble value, Helia *helia )
{
	GstElement *element = helia_gst_iterate_element ( helia->dtv, "autoaudiosink", "actual-sink" );

	if ( element /*&& GST_ELEMENT_CAST ( element )->current_state == GST_STATE_PLAYING*/ )
	{
		helia->volume_tv = value;
		g_object_set ( element, "volume", value, NULL );
	}
}



// *** Player ***

void helia_player_slider_seek_changed ( GtkRange *range, Helia *helia )
{
	if ( GST_ELEMENT_CAST ( helia->player )->current_state == GST_STATE_NULL ) return;

	gdouble value = gtk_range_get_value ( GTK_RANGE (range) );

	gst_element_seek_simple ( helia->player, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT, (gint64)( value * GST_SECOND ) );

	helia_slider_set_data ( helia->slider, (gint64)( value * GST_SECOND ), 8, -1, 10, TRUE );
}

void helia_player_slider_panel_seek_changed ( GtkRange *range, Helia *helia )
{
	if ( GST_ELEMENT_CAST ( helia->player )->current_state == GST_STATE_NULL ) return;

	gdouble value = gtk_range_get_value ( GTK_RANGE (range) );

	gst_element_seek_simple ( helia->player, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT, (gint64)( value * GST_SECOND ) );

	helia_slider_set_data ( helia->slider_panel, (gint64)( value * GST_SECOND ), 8, -1, 10, TRUE );
}

void helia_player_slider_update ( Helia *helia )
{
	if ( helia->window == NULL ) return;

	GstElement *element = helia->player;

	if ( helia->pipeline_rec ) element = helia->pipeline_rec;

	if ( GST_ELEMENT_CAST ( element )->current_state != GST_STATE_PAUSED ) return;

	gboolean dur_b = FALSE, panel = FALSE;
	gint64 duration = 0, current = 0;

	if ( gst_element_query_position ( element, GST_FORMAT_TIME, &current ) )
	{
		if ( gst_element_query_duration ( element, GST_FORMAT_TIME, &duration ) ) dur_b = TRUE;

		if ( helia->slider_panel && gtk_widget_get_visible ( GTK_WIDGET ( helia->slider_panel ) ) ) panel = TRUE;

			if ( dur_b && duration / GST_SECOND > 0 )
			{
				if ( current / GST_SECOND < duration / GST_SECOND )
				{
					helia_slider_update ( ( panel ) ? helia->slider_panel : helia->slider, (gdouble)duration / GST_SECOND, (gdouble)current / GST_SECOND );

					helia_slider_set_data ( ( panel ) ? helia->slider_panel : helia->slider, current, 8, duration, 10, TRUE );
				}
			}
			else
			{
				helia_slider_set_data ( ( panel ) ? helia->slider_panel : helia->slider, current, 8, -1, 10, FALSE );
			}
	}
}

gboolean helia_player_slider_refresh ( Helia *helia )
{
	if ( helia->window == NULL ) return FALSE;

	GstElement *element = helia->player;

	if ( helia->pipeline_rec ) element = helia->pipeline_rec;

	if ( GST_ELEMENT_CAST ( element )->current_state < GST_STATE_PLAYING ) return TRUE;

	gboolean dur_b = FALSE, panel = FALSE;
	gint64 duration = 0, current = 0;

	if ( gst_element_query_position ( element, GST_FORMAT_TIME, &current ) )
	{
		if ( gst_element_query_duration ( element, GST_FORMAT_TIME, &duration ) ) dur_b = TRUE;

		if ( helia->slider_panel && gtk_widget_get_visible ( GTK_WIDGET ( helia->slider_panel ) ) ) panel = TRUE;

			if ( dur_b && duration / GST_SECOND > 0 )
			{
				if ( current / GST_SECOND < duration / GST_SECOND )
				{
					helia_slider_update ( ( panel ) ? helia->slider_panel : helia->slider, (gdouble)duration / GST_SECOND, (gdouble)current / GST_SECOND );

					helia_slider_set_data ( ( panel ) ? helia->slider_panel : helia->slider, current, 8, duration, 10, TRUE );
				}
			}
			else
			{
				helia_slider_set_data ( ( panel ) ? helia->slider_panel : helia->slider, current, 8, -1, 10, FALSE );
			}
	}

	return TRUE;
}

void helia_player_video_scroll_new_pos ( Helia *helia, gint64 set_pos, gboolean up_dwn )
{
	gboolean dur_b = FALSE;
	gint64 current = 0, duration = 0, new_pos = 0, skip = (gint64)( set_pos * GST_SECOND );

	if ( gst_element_query_position ( helia->player, GST_FORMAT_TIME, &current ) )
	{
		if ( gst_element_query_duration ( helia->player, GST_FORMAT_TIME, &duration ) ) dur_b = TRUE;

		if ( !dur_b || duration / GST_SECOND < 1 ) return;

		if ( up_dwn ) new_pos = ( duration > ( current + skip ) ) ? ( current + skip ) : duration;

		if ( !up_dwn ) new_pos = ( current > skip ) ? ( current - skip ) : 0;

		gst_element_seek_simple ( helia->player, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT, new_pos );
	}
}

static void helia_player_gst_step_pos ( Helia *helia, gint64 am )
{
	gst_element_send_event ( helia->player, gst_event_new_step ( GST_FORMAT_BUFFERS, am, 1.0, TRUE, FALSE ) );

	gint64 current = 0;

	if ( gst_element_query_position ( helia->player, GST_FORMAT_TIME, &current ) )
	{
		helia_slider_set_data ( helia->slider, current, 7, -1, 10, TRUE );
	}
}
void helia_player_step_frame ( Helia *helia )
{
	if ( GST_ELEMENT_CAST ( helia->player )->current_state == GST_STATE_NULL ) return;

	gint n_video = 0;
	g_object_get ( helia->player, "n-video", &n_video, NULL );

	if ( n_video == 0 ) return;

	if ( GST_ELEMENT_CAST ( helia->player )->current_state == GST_STATE_PLAYING )
		gst_element_set_state ( helia->player, GST_STATE_PAUSED );

	helia_player_gst_step_pos ( helia, 1 );
}

static void helia_player_stop_record ( Helia *helia, gboolean play )
{
	gst_element_set_state ( helia->pipeline_rec, GST_STATE_NULL );

	helia_gst_remove_bin ( helia->pipeline_rec, NULL );

	gst_object_unref ( helia->pipeline_rec );

	helia->pipeline_rec = NULL;

	if ( play )
		gst_element_set_state ( helia->player, GST_STATE_PLAYING );
	else
	{
		helia_slider_clear_all ( helia->slider );
		if ( helia->slider_panel ) helia_slider_clear_all ( helia->slider_panel );

		gtk_label_set_text ( helia->label_buf, " ⇄ 0% " );
		gtk_widget_queue_draw ( GTK_WIDGET ( helia->window ) );
	}
}

void helia_player_stop ( Helia *helia )
{
	if ( helia->pipeline_rec != NULL ) helia_player_stop_record ( helia, FALSE );

	if ( GST_ELEMENT_CAST ( helia->player )->current_state != GST_STATE_NULL )
	{
		gst_element_set_state ( helia->player, GST_STATE_NULL );

		helia_power_manager ( helia, FALSE );

		if ( helia->window == NULL ) return;

		helia_slider_clear_all ( helia->slider );
		if ( helia->slider_panel ) helia_slider_clear_all ( helia->slider_panel );

		gtk_label_set_text ( helia->label_buf, " ⇄ 0% " );
		gtk_widget_queue_draw ( GTK_WIDGET ( helia->window ) );
	}
}

static void helia_player_play ( Helia *helia )
{
	if ( GST_ELEMENT_CAST ( helia->player )->current_state != GST_STATE_PLAYING )
	{
		g_object_set ( helia->player, "volume", helia->volume_mp, NULL );

		gst_element_set_state ( helia->player, GST_STATE_PLAYING );
	}
}

static void helia_player_set_uri ( Helia *helia, const char *name_file )
{
	if ( g_strrstr ( name_file, "://" ) )
		g_object_set ( helia->player, "uri", name_file, NULL );
	else
	{
		char *uri = gst_filename_to_uri ( name_file, NULL );

		g_object_set ( helia->player, "uri", uri, NULL );

		g_free ( uri );
	}
}

void helia_player_stop_set_play ( Helia *helia, const char *name_file )
{
	if ( helia->file_ch ) g_free ( helia->file_ch );

	helia->file_ch = g_strdup ( name_file );

	helia_player_stop ( helia );

	helia_player_set_uri ( helia, name_file );

	helia_player_play ( helia );
}

void helia_player_set_subtitle ( Helia *helia, gboolean state_subtitle )
{
	enum gst_flags { GST_FLAG_TEXT = (1 << 2) };

	uint flags;
	g_object_get ( helia->player, "flags", &flags, NULL );

	if (  state_subtitle ) flags |=  GST_FLAG_TEXT;
	if ( !state_subtitle ) flags &= ~GST_FLAG_TEXT;

	g_object_set ( helia->player, "flags", flags, NULL );
}

void helia_player_set_visualizer ( Helia *helia, gboolean state_visualizer )
{
	enum gst_flags { GST_FLAG_VIS = (1 << 3) };

	uint flags;
	g_object_get ( helia->player, "flags", &flags, NULL );

	if (  state_visualizer ) flags |=  GST_FLAG_VIS;
	if ( !state_visualizer ) flags &= ~GST_FLAG_VIS;

	g_object_set ( helia->player, "flags", flags, NULL );

	if ( state_visualizer )
	{
		GstElement *visual_goom = gst_element_factory_make ( "goom", NULL );

		if ( visual_goom )
			g_object_set ( helia->player, "vis-plugin", visual_goom, NULL );
	}

	g_usleep ( 250000 );
	gtk_widget_queue_draw ( GTK_WIDGET ( helia->window ) );
}

static void helia_player_msg_buf ( G_GNUC_UNUSED GstBus *bus, GstMessage *msg, Helia *helia )
{
	if ( helia->window == NULL ) return;

	int percent;
	gst_message_parse_buffering ( msg, &percent );

	if ( percent == 100 )
	{
		// if ( GST_ELEMENT_CAST ( helia->player )->current_state == GST_STATE_PAUSED )
			gst_element_set_state ( helia->player, GST_STATE_PLAYING );

		gtk_label_set_text ( helia->label_buf, " ⇄ 0% " );
	}
	else
	{
		if ( GST_ELEMENT_CAST ( helia->player )->current_state == GST_STATE_PLAYING )
			gst_element_set_state ( helia->player, GST_STATE_PAUSED );

		char *str = g_strdup_printf ( " ⇄ %d%s ", percent, "%" );

			gtk_label_set_text ( helia->label_buf, str );

		free ( str );
	}
}

static GstBusSyncReply helia_player_sync_handler ( G_GNUC_UNUSED GstBus *bus, GstMessage *message, Helia *helia )
{
	if ( !gst_is_video_overlay_prepare_window_handle_message ( message ) ) return GST_BUS_PASS;

	if ( helia->xid_mp != 0 )
	{
		GstVideoOverlay *xoverlay = GST_VIDEO_OVERLAY ( GST_MESSAGE_SRC ( message ) );
		gst_video_overlay_set_window_handle ( xoverlay, helia->xid_mp );

	} else { g_warning ( "Should have obtained window_handle by now!" ); }

	gst_message_unref ( message );

	return GST_BUS_DROP;
}

static void helia_player_msg_err ( G_GNUC_UNUSED GstBus *bus, GstMessage *msg, Helia *helia )
{
	GError *err = NULL;
	char  *dbg = NULL;

	gst_message_parse_error ( msg, &err, &dbg );

	g_critical ( "%s: %s (%s)", __func__, err->message, (dbg) ? dbg : "no details" );

	helia_message_dialog ( "", err->message, GTK_MESSAGE_ERROR, helia->window );

	g_error_free ( err );
	g_free ( dbg );

	if ( GST_ELEMENT_CAST ( helia->player )->current_state != GST_STATE_PLAYING )
		helia_player_stop ( helia );
}

static void helia_player_msg_cll ( G_GNUC_UNUSED GstBus *bus, G_GNUC_UNUSED GstMessage *msg, Helia *helia )
{
	if ( GST_ELEMENT_CAST ( helia->player )->current_state == GST_STATE_PLAYING )
	{
		gst_element_set_state ( helia->player, GST_STATE_PAUSED  );
		gst_element_set_state ( helia->player, GST_STATE_PLAYING );
	}
}

static void helia_player_msg_eos ( G_GNUC_UNUSED GstBus *bus, G_GNUC_UNUSED GstMessage *msg, Helia *helia )
{
	helia_treeview_next_play ( helia );
}

static void helia_player_msg_cng ( G_GNUC_UNUSED GstBus *bus, G_GNUC_UNUSED GstMessage *msg, Helia *helia )
{
	if ( GST_MESSAGE_SRC ( msg ) != GST_OBJECT ( helia->player ) ) return;

	GstState old_state, new_state;

	gst_message_parse_state_changed ( msg, &old_state, &new_state, NULL );

	switch ( new_state )
	{
		case GST_STATE_NULL:
		case GST_STATE_READY:
			break;

		case GST_STATE_PAUSED:
		{
			helia_power_manager ( helia, FALSE );
			break;
		}

		case GST_STATE_PLAYING:
		{
			uint n_video = 0;
			g_object_get ( helia->player, "n-video", &n_video, NULL );

			if ( n_video > 0 ) helia_power_manager ( helia, TRUE );
			break;
		}

		default:
			break;
	}

	g_debug ( "%s:: Element %s changed state from %s to %s.", __func__, GST_OBJECT_NAME (msg->src), 
		gst_element_state_get_name (old_state), gst_element_state_get_name (new_state) );
}

static GstElement * helia_player_create ( Helia *helia )
{
	GstElement *playbin = gst_element_factory_make ( "playbin", NULL );

	GstElement *bin_audio, *bin_video, *asink, *vsink, *videoblnc, *equalizer;

	vsink     = gst_element_factory_make ( "autovideosink",     NULL );
	asink     = gst_element_factory_make ( "autoaudiosink",     NULL );

	videoblnc = gst_element_factory_make ( "videobalance",      NULL );
	equalizer = gst_element_factory_make ( "equalizer-nbands",  NULL );

	if ( !playbin || !vsink || !asink || !videoblnc || !equalizer )
	{
		g_critical ( "%s: playbin - not all elements could be created.", __func__ );

		return NULL;
	}

	bin_audio = gst_bin_new ( "audio-sink-bin" );
	gst_bin_add_many ( GST_BIN ( bin_audio ), equalizer, asink, NULL );
	gst_element_link_many ( equalizer, asink, NULL );

	GstPad *pad = gst_element_get_static_pad ( equalizer, "sink" );
	gst_element_add_pad ( bin_audio, gst_ghost_pad_new ( "sink", pad ) );
	gst_object_unref ( pad );

	bin_video = gst_bin_new ( "video-sink-bin" );
	gst_bin_add_many ( GST_BIN ( bin_video ), videoblnc, vsink, NULL );
	gst_element_link_many ( videoblnc, vsink, NULL );

	GstPad *padv = gst_element_get_static_pad ( videoblnc, "sink" );
	gst_element_add_pad ( bin_video, gst_ghost_pad_new ( "sink", padv ) );
	gst_object_unref ( padv );

	g_object_set ( playbin, "video-sink", bin_video, NULL );
	g_object_set ( playbin, "audio-sink", bin_audio, NULL );

	GstBus *bus = gst_element_get_bus ( playbin );

	gst_bus_add_signal_watch_full ( bus, G_PRIORITY_DEFAULT );
	gst_bus_set_sync_handler ( bus, (GstBusSyncHandler)helia_player_sync_handler, helia, NULL );

	g_signal_connect ( bus, "message::eos",   	   G_CALLBACK ( helia_player_msg_eos ), helia );
	g_signal_connect ( bus, "message::error", 	   G_CALLBACK ( helia_player_msg_err ), helia );
	g_signal_connect ( bus, "message::clock-lost", G_CALLBACK ( helia_player_msg_cll ), helia );
	g_signal_connect ( bus, "message::buffering",  G_CALLBACK ( helia_player_msg_buf ), helia );
	g_signal_connect ( bus, "message::state-changed", G_CALLBACK ( helia_player_msg_cng ), helia );

	gst_object_unref ( bus );

	return playbin;
}

HeliaPlayer * helia_player_new ( Helia *helia )
{
	return (HeliaPlayer *)helia_player_create ( helia );
}



static void helia_player_msg_err_rec ( G_GNUC_UNUSED GstBus *bus, GstMessage *msg, Helia *helia )
{
	GError *err = NULL;
	char  *dbg = NULL;

	gst_message_parse_error ( msg, &err, &dbg );

	g_critical ( "%s: %s (%s)", __func__, err->message, (dbg) ? dbg : "no details" );

	helia_message_dialog ( "", err->message, GTK_MESSAGE_ERROR, helia->window );

	g_error_free ( err );
	g_free ( dbg );

	if ( GST_ELEMENT_CAST ( helia->pipeline_rec )->current_state != GST_STATE_PLAYING )
		helia_player_stop_record ( helia, FALSE );
}

static void helia_player_msg_cng_rec ( G_GNUC_UNUSED GstBus *bus, G_GNUC_UNUSED GstMessage *msg, Helia *helia )
{
	if ( GST_MESSAGE_SRC ( msg ) != GST_OBJECT ( helia->pipeline_rec ) ) return;

	GstState old_state, new_state;

	gst_message_parse_state_changed ( msg, &old_state, &new_state, NULL );

	switch ( new_state )
	{
		case GST_STATE_NULL:
		case GST_STATE_READY:
		case GST_STATE_PAUSED:
			break;

		case GST_STATE_PLAYING:
		{
			GstElement *element = helia_gst_iterate_element ( helia->pipeline_rec, "videobalance", NULL );

			if ( element ) helia_power_manager ( helia, TRUE );

			break;
		}

		default:
			break;
	}

	g_debug ( "%s:: Element %s changed state from %s to %s.", __func__, GST_OBJECT_NAME (msg->src), 
		gst_element_state_get_name (old_state), gst_element_state_get_name (new_state) );
}

static GstElement * player_gst_create_rec_pipeline ( Helia *helia )
{
	GstElement *pipeline_rec = gst_pipeline_new ( "pipeline-record" );

	if ( !pipeline_rec )
	{
		g_critical ( "%s: pipeline-record - not created.", __func__ );

		return NULL;
	}

	GstBus *bus = gst_element_get_bus ( pipeline_rec );

	gst_bus_add_signal_watch_full ( bus, G_PRIORITY_DEFAULT );
	gst_bus_set_sync_handler ( bus, (GstBusSyncHandler)helia_player_sync_handler, helia, NULL );

	g_signal_connect ( bus, "message::error", G_CALLBACK ( helia_player_msg_err_rec ), helia );
	g_signal_connect ( bus, "message::state-changed", G_CALLBACK ( helia_player_msg_cng_rec ), helia );

	gst_object_unref ( bus );

	return pipeline_rec;
}

static void helia_player_rec_set_location ( GstElement *element, char *rec_dir )
{
	char *date_str = helia_time_to_str ();
	char *file_rec = g_strdup_printf ( "%s/Record-iptv-%s", rec_dir, date_str );

	g_object_set ( element, "location", file_rec, NULL );

	g_free ( file_rec );
	g_free ( date_str );
}

static gboolean helia_player_pad_check_type ( GstPad *pad, const char *type )
{
	gboolean ret = FALSE;

	GstCaps *caps = gst_pad_get_current_caps ( pad );

	const char *name = gst_structure_get_name ( gst_caps_get_structure ( caps, 0 ) );

	if ( g_str_has_prefix ( name, type ) ) ret = TRUE;

	gst_caps_unref (caps);

	return ret;
}

static void helia_player_pad_link ( GstPad *pad, GstElement *element, const char *name )
{
	GstPad *pad_va_sink = gst_element_get_static_pad ( element, "sink" );

	if ( gst_pad_link ( pad, pad_va_sink ) == GST_PAD_LINK_OK )
		gst_object_unref ( pad_va_sink );
	else
		g_debug ( "%s:: linking demux/decode name %s video/audio pad failed ", __func__, name );
}

static void helia_player_pad_demux_audio ( G_GNUC_UNUSED GstElement *element, GstPad *pad, GstElement *element_audio )
{
	if ( helia_player_pad_check_type ( pad, "audio" ) ) helia_player_pad_link ( pad, element_audio, "demux audio" );
}

static void helia_player_pad_demux_video ( G_GNUC_UNUSED GstElement *element, GstPad *pad, GstElement *element_video )
{
	if ( helia_player_pad_check_type ( pad, "video" ) ) helia_player_pad_link ( pad, element_video, "demux video" );
}

static void helia_player_pad_decode ( G_GNUC_UNUSED GstElement *element, GstPad *pad, GstElement *element_va )
{
	helia_player_pad_link ( pad, element_va, "decode  audio / video" );
}

static void helia_player_pad_hlsdemux ( GstElement *element, GstPad *pad_new, Helia *helia )
{
	GstElement *element_link = helia_gst_iterate_element ( helia->pipeline_rec, "tee-hls-rec", NULL ); 

	if ( element_link == NULL ) return;

	GstIterator *it = gst_element_iterate_src_pads ( element );
	GValue item = { 0, };
	gboolean done = FALSE;

	while ( !done )
	{
		switch ( gst_iterator_next ( it, &item ) )
		{
			case GST_ITERATOR_OK:
			{
				GstPad *pad_src = GST_PAD ( g_value_get_object (&item) );

				char *name = gst_object_get_name ( GST_OBJECT ( pad_src ) );

				if ( gst_pad_is_linked ( pad_src ) )
				{
					GstPad *sink_pad = gst_element_get_static_pad ( element_link, "sink" );

					if ( gst_pad_unlink ( pad_src, sink_pad ) )
						g_debug ( "%s: unlink Ok ", __func__ );
					else
						g_debug ( "%s: unlink Failed ", __func__ );

					gst_object_unref ( sink_pad );
				}
				else
				{
					helia_player_pad_link ( pad_new, element_link, "hlsdemux" );
				}

				g_free ( name );
				g_value_reset (&item);

				break;
			}

			case GST_ITERATOR_RESYNC:
				gst_iterator_resync (it);
				break;

			case GST_ITERATOR_ERROR:
				done = TRUE;
				break;

			case GST_ITERATOR_DONE:
				done = TRUE;
				break;
		}
	}

	g_value_unset ( &item );
	gst_iterator_free ( it );
}

static GstElement * helia_player_create_rec_bin ( Helia *helia, uint n_video, uint n_audio, gboolean found_hls )
{
	GstElement *pipeline_rec = player_gst_create_rec_pipeline ( helia );

	if ( !pipeline_rec ) return NULL;

	const char *name = ( found_hls ) ? "hlsdemux" : "queue2";

	struct rec_all { const char *name; } rec_all_n[] =
	{
		{ "souphttpsrc" }, { name        }, { "tee"          }, { "queue2"           }, { "decodebin"     },
		{ "queue2"      }, { "decodebin" }, { "audioconvert" }, { "equalizer-nbands" }, { "autoaudiosink" },
		{ "queue2"      }, { "decodebin" }, { "videoconvert" }, { "videobalance"     }, { "autovideosink" },
		{ "queue2"      }, { "filesink"  }  // hlssink
	};

	GstElement *elements[ G_N_ELEMENTS ( rec_all_n ) ];

	uint c = 0;
	for ( c = 0; c < G_N_ELEMENTS ( rec_all_n ); c++ )
	{
		if ( n_video == 0 && ( c > 9 && c < 15 ) ) continue;

		if (  c == 0 )
			elements[c] = gst_element_make_from_uri ( GST_URI_SRC, helia->file_ch, NULL, NULL );
		else
			elements[c] = gst_element_factory_make ( rec_all_n[c].name, ( c == 2 ) ? "tee-hls-rec" : NULL );

		if ( !elements[c] )
		{
			g_critical ( "%s:: element (factory make) - %s not created. \n", __func__, rec_all_n[c].name );

			return NULL;
		}

		gst_bin_add ( GST_BIN ( pipeline_rec ), elements[c] );

		if (  c == 0 || c == 2 || c == 5 || c == 7 || c == 10 || c == 12 || c == 15 ) continue;

		gst_element_link ( elements[c-1], elements[c] );
	}

	if ( found_hls )
		g_signal_connect ( elements[1],  "pad-added", G_CALLBACK ( helia_player_pad_hlsdemux ), helia );
	else
		gst_element_link ( elements[1], elements[2] );

	if ( n_audio > 0 ) g_signal_connect ( elements[4],  "pad-added", G_CALLBACK ( helia_player_pad_demux_audio ), elements[5]  );
	if ( n_video > 0 ) g_signal_connect ( elements[4],  "pad-added", G_CALLBACK ( helia_player_pad_demux_video ), elements[10] );

	if ( n_audio > 0 ) g_signal_connect ( elements[6],  "pad-added", G_CALLBACK ( helia_player_pad_decode ), elements[7]  );
	if ( n_video > 0 ) g_signal_connect ( elements[11], "pad-added", G_CALLBACK ( helia_player_pad_decode ), elements[12] );

	if ( g_object_class_find_property ( G_OBJECT_GET_CLASS ( elements[0] ), "location" ) )
		g_object_set ( elements[0],  "location", helia->file_ch, NULL );
	else
		g_object_set ( elements[0], "uri", helia->file_ch, NULL );

	gst_element_link ( elements[2],  elements[15] );

	helia_player_rec_set_location ( elements[16], helia->rec_dir );

	return pipeline_rec;
}

static gboolean helia_player_check_iptv ( Helia *helia )
{
	gboolean ret = TRUE;

	if ( !gst_uri_is_valid ( helia->file_ch ) || g_str_has_prefix ( helia->file_ch, "file://" ) )
	{
		helia_message_dialog ( "", _i18n_ ( "Only IPTV." ), GTK_MESSAGE_WARNING, helia->window );

		return FALSE;
	}

	char *uri_protocol = gst_uri_get_protocol ( helia->file_ch );

	gboolean is_supported = gst_uri_protocol_is_supported ( GST_URI_SRC, uri_protocol );

	if ( !is_supported )
	{
		helia_message_dialog ( "", "The protocol is not supported.", GTK_MESSAGE_WARNING, helia->window );

		ret = FALSE;
	}

	g_free ( uri_protocol );

	return ret;
}

static gboolean helia_player_update_record  ( Helia *helia )
{
	if ( helia->window == NULL ) return FALSE;

	if ( helia->pipeline_rec == NULL ) { gtk_label_set_text ( helia->label_rec, " " ); return FALSE; }

	static gboolean pulse = FALSE;

	if ( GST_ELEMENT_CAST ( helia->pipeline_rec )->current_state == GST_STATE_PLAYING )
	{
		const char *format = "<span foreground=\"#ff0000\"> %s </span>";

		char *markup = g_markup_printf_escaped ( format, ( pulse ) ? " ◉ " : " ◌ " );

			gtk_label_set_markup ( helia->label_rec, markup );

		g_free ( markup );

		pulse = !pulse;
		helia->rec_count = 0;
	}
	else
		gtk_label_set_text ( helia->label_rec, " ◌ " );

/*
	else
	{
		if ( helia->rec_count > 9 )
		{
			helia_player_stop_record ( helia, TRUE );

			gtk_label_set_text ( helia->label_rec, " " );

			helia_message_dialog ( "Error", "Record", GTK_MESSAGE_ERROR, helia->window );

			return FALSE;
		}
		else
			helia->rec_count++;
	}
*/
	return TRUE;
}

void helia_player_record ( Helia *helia )
{
	if ( helia->pipeline_rec == NULL )
	{
		if ( GST_ELEMENT_CAST ( helia->player )->current_state == GST_STATE_NULL ) return;

		if ( !helia_player_check_iptv ( helia ) ) return;

		uint n_video, n_audio;
		g_object_get ( helia->player, "n-video", &n_video, NULL );
		g_object_get ( helia->player, "n-audio", &n_audio, NULL );

		GstElement *element = helia_gst_iterate_element ( helia->player, "hlsdemux", NULL );

		helia_player_stop ( helia );

		if ( element )
			helia->pipeline_rec = helia_player_create_rec_bin ( helia, n_video, n_audio, TRUE  );
		else
			helia->pipeline_rec = helia_player_create_rec_bin ( helia, n_video, n_audio, FALSE );

		gst_element_set_state ( helia->pipeline_rec, GST_STATE_PLAYING );

		helia_gst_set_volume_mp ( helia->volume_mp, helia );

		helia->rec_count = 0;
		g_timeout_add_seconds ( 1, (GSourceFunc)helia_player_update_record, helia );
	}
	else
	{
		helia_player_stop_record ( helia, TRUE );
	}
}



// *** Tv ***

static void helia_dtv_change_audio_track ( GstElement *e_unlink, GstElement *e_link, uint num )
{
	GstIterator *it = gst_element_iterate_src_pads ( e_unlink );
	GValue item = { 0, };

	uint i = 0;
	gboolean done = FALSE;

	GstPad *pad_link = NULL;
	GstPad *pad_sink = gst_element_get_static_pad ( e_link, "sink" );

	while ( !done )
	{
		switch ( gst_iterator_next ( it, &item ) )
		{
			case GST_ITERATOR_OK:
			{
				GstPad *pad_src = GST_PAD ( g_value_get_object (&item) );

				char *name = gst_object_get_name ( GST_OBJECT ( pad_src ) );

				g_debug ( "%s: name %s | num %d | %d ", __func__, name, num , i );

				if ( g_str_has_prefix ( name, "audio" ) )
				{
					if ( gst_pad_is_linked ( pad_src ) )
					{
						if ( gst_pad_unlink ( pad_src, pad_sink ) )
							g_debug ( "%s: unlink Ok ", __func__ );
						else
							g_warning ( "%s: unlink Failed ", __func__ );
					}
					else
						if ( i == num ) pad_link = pad_src;

					i++;
				}

				g_free ( name );
				g_value_reset (&item);

				break;
			}

			case GST_ITERATOR_RESYNC:
				gst_iterator_resync (it);
				break;

			case GST_ITERATOR_ERROR:
				done = TRUE;
				break;

			case GST_ITERATOR_DONE:
				done = TRUE;
				break;
		}
	}

	if ( gst_pad_link ( pad_link, pad_sink ) == GST_PAD_LINK_OK )
		g_debug ( "%s: link Ok ", __func__ );
	else
		g_warning ( "%s: link Failed ", __func__ );

	gst_object_unref ( pad_sink );

	g_value_unset ( &item );
	gst_iterator_free ( it );
}

static gboolean helia_dtv_set_audio_track ( int set_track_audio, Helia *helia )
{
	gboolean audio_changed = TRUE;

	GstElement *element = helia_gst_iterate_element ( helia->dtv, "tsdemux", NULL );

	if ( element == NULL ) return FALSE;

	GstElement *element_l = helia_gst_iterate_element ( helia->dtv, "queue-tee-audio", NULL );

	if ( element_l == NULL ) return FALSE;

	helia_dtv_change_audio_track ( element, element_l, set_track_audio );

	return audio_changed;
}

void helia_dtv_changed_audio_track ( Helia *helia, int changed_track_audio )
{
	helia_dtv_set_audio_track ( changed_track_audio, helia );

	helia->set_audio_track = changed_track_audio;
}

void helia_dtv_add_audio_track ( Helia *helia, GtkComboBoxText *combo )
{
	uint i = 0;

	for ( i = 0; i < MAX_AUDIO; i++ )
	{
		if ( helia->audio_lang[i] )
			gtk_combo_box_text_append_text ( combo, helia->audio_lang[i] );
	}

	gtk_combo_box_set_active ( GTK_COMBO_BOX ( combo ), helia->set_audio_track );
}

static void helia_dtv_clear_audio_tracks ( Helia *helia )
{
	uint i = 0;

	for ( i = 0; i < MAX_AUDIO; i++ )
	{
		if ( helia->audio_lang[i] )
			g_free ( helia->audio_lang[i] );

		helia->audio_lang[i] = NULL;
	}

	helia->set_audio_track = 0;
}

static char * helia_dtv_str_split ( const char *data, const char *delm, uint num )
{
	char *ret_ch = NULL;

	char **lines = g_strsplit ( data, delm, 0 );

		ret_ch = g_strdup ( lines[num] );

	g_strfreev ( lines );

	return ret_ch;
}

static void helia_dtv_rec_set_location ( GstElement *element, char *rec_dir, char *ch_data, gboolean enc_ts )
{
	char *date_str = helia_time_to_str ();
	char *name     = helia_dtv_str_split ( ch_data, ":", 0 );
	char *file_rec = g_strdup_printf ( "%s/%s_%s%s", rec_dir, name, date_str, ( enc_ts ) ? "" : ".m2ts" );

	g_object_set ( element, "location", file_rec, NULL );

	g_free ( file_rec );
	g_free ( name     );
	g_free ( date_str );
}

static GstElementFactory * helia_dtv_find_factory ( GstCaps *caps, int e_num )
{
	GList *list, *list_filter;

	static GMutex mutex;

	g_mutex_lock ( &mutex );
		list = gst_element_factory_list_get_elements ( e_num, GST_RANK_MARGINAL );
		list_filter = gst_element_factory_list_filter ( list, caps, GST_PAD_SINK, gst_caps_is_fixed ( caps ) );
	g_mutex_unlock ( &mutex );

	GstElementFactory *factory = GST_ELEMENT_FACTORY_CAST ( list_filter->data );

	gst_plugin_feature_list_free ( list_filter );
	gst_plugin_feature_list_free ( list );

	return factory;
}

static gboolean helia_dtv_typefind_remove ( const char *name, Helia *helia )
{
	GstElement *element = helia_gst_iterate_element ( helia->dtv, name, NULL );

	if ( element )
	{
		gst_element_set_state ( element, GST_STATE_NULL );
		gst_bin_remove ( GST_BIN ( helia->dtv ), element );

		return TRUE;
	}

	return FALSE;
}

static void helia_dtv_gst_typefind_parser ( GstElement *typefind, uint probability, GstCaps *caps, Helia *helia )
{
	const char *name_caps = gst_structure_get_name ( gst_caps_get_structure ( caps, 0 ) );

	GstElementFactory *factory = helia_dtv_find_factory ( caps, GST_ELEMENT_FACTORY_TYPE_PARSER );

	// const char * name = gst_plugin_feature_get_name ( factory );

	GstElement *mpegtsmux = helia_gst_iterate_element ( helia->dtv, "mpegtsmux", NULL );

	GstElement *element = gst_element_factory_create ( factory, NULL );

	gboolean remove = FALSE;

	if ( g_str_has_prefix ( name_caps, "audio" ) )
	{
		remove = helia_dtv_typefind_remove ( "parser-audio", helia );
		gst_element_set_name ( element, "parser-audio" );
	}

	if ( g_str_has_prefix ( name_caps, "video" ) )
	{
		remove = helia_dtv_typefind_remove ( "parser-video", helia );
		gst_element_set_name ( element, "parser-video" );
	}

	if ( remove == FALSE ) gst_element_unlink ( typefind, mpegtsmux );

	gst_bin_add ( GST_BIN ( helia->dtv ), element );

	gst_element_link ( typefind, element );
	gst_element_link ( element, mpegtsmux );

	gst_element_set_state ( element, GST_STATE_PLAYING );

	g_debug ( "%s: probability %d%% | name_caps %s ",__func__, probability, name_caps );
}

static gboolean helia_dtv_pad_check_type ( GstPad *pad, const char *type )
{
	gboolean ret = FALSE;

	GstCaps *caps = gst_pad_get_current_caps ( pad );

	const char *name = gst_structure_get_name ( gst_caps_get_structure ( caps, 0 ) );

	if ( g_str_has_prefix ( name, type ) ) ret = TRUE;

	gst_caps_unref (caps);

	return ret;
}

static void helia_dtv_pad_link ( GstPad *pad, GstElement *element, const char *name )
{
	GstPad *pad_va_sink = gst_element_get_static_pad ( element, "sink" );

	if ( gst_pad_link ( pad, pad_va_sink ) == GST_PAD_LINK_OK )
		gst_object_unref ( pad_va_sink );
	else
		g_debug ( "%s:: linking demux/decode name %s video/audio pad failed ", __func__, name );
}

static void helia_dtv_pad_demux_audio ( G_GNUC_UNUSED GstElement *element, GstPad *pad, GstElement *element_audio )
{
	if ( helia_dtv_pad_check_type ( pad, "audio" ) ) helia_dtv_pad_link ( pad, element_audio, "demux audio" );
}

static void helia_dtv_pad_demux_video ( G_GNUC_UNUSED GstElement *element, GstPad *pad, GstElement *element_video )
{
	if ( helia_dtv_pad_check_type ( pad, "video" ) ) helia_dtv_pad_link ( pad, element_video, "demux video" );
}

static void helia_dtv_pad_decode ( G_GNUC_UNUSED GstElement *element, GstPad *pad, GstElement *element_va )
{
	helia_dtv_pad_link ( pad, element_va, "decode  audio / video" );
}

static void helia_dtv_create_bin ( GstElement *element, gboolean video_enable )
{
	struct dvb_all_list { const char *name; } dvb_all_list_n[] =
	{
		{ "dvbsrc" }, { "tsdemux"   },
		{ "queue2" }, { "decodebin" }, { "audioconvert" }, { "equalizer-nbands" }, { "autoaudiosink" },
		{ "queue2" }, { "decodebin" }, { "videoconvert" }, { "videobalance"     }, { "autovideosink" }
	};

	GstElement *elements[ G_N_ELEMENTS ( dvb_all_list_n ) ];

	uint c = 0;
	for ( c = 0; c < G_N_ELEMENTS ( dvb_all_list_n ); c++ )
	{
		if ( !video_enable && c > 6 ) continue;

		elements[c] = gst_element_factory_make ( dvb_all_list_n[c].name, NULL );

		if ( !elements[c] )
			g_critical ( "%s:: element (factory make) - %s not created. \n", __func__, dvb_all_list_n[c].name );

		if ( c == 2 ) gst_element_set_name ( elements[c], "queue-tee-audio" );

		gst_bin_add ( GST_BIN ( element ), elements[c] );

		if (  c == 0 || c == 2 || c == 4 || c == 7 || c == 9 ) continue;

		gst_element_link ( elements[c-1], elements[c] );
	}

	g_signal_connect ( elements[1], "pad-added", G_CALLBACK ( helia_dtv_pad_demux_audio ), elements[2] );
	if ( video_enable ) g_signal_connect ( elements[1], "pad-added", G_CALLBACK ( helia_dtv_pad_demux_video ), elements[7] );

	g_signal_connect ( elements[3], "pad-added", G_CALLBACK ( helia_dtv_pad_decode ), elements[4] );
	if ( video_enable ) g_signal_connect ( elements[8], "pad-added", G_CALLBACK ( helia_dtv_pad_decode ), elements[9] );
}

static void helia_dtv_create_rec_bin ( GstElement *element, gboolean video_enable, Helia *helia )
{
	struct dvb_all_list { const char *name; } dvb_all_list_n[] =
	{
		{ "tsdemux" },
		{ "tee"     }, { "queue2"     }, { "decodebin" }, { "audioconvert" }, { "equalizer-nbands" }, { "autoaudiosink" },
		{ "tee"     }, { "queue2"     }, { "decodebin" }, { "videoconvert" }, { "videobalance"     }, { "autovideosink" },
		{ "queue2"  }, { "typefind"   },
		{ "queue2"  }, { "typefind"   },
		{ "mpegtsmux" }, { "filesink" }

	};

	GstElement *elements[ G_N_ELEMENTS ( dvb_all_list_n ) ];

	uint c = 0;
	for ( c = 0; c < G_N_ELEMENTS ( dvb_all_list_n ); c++ )
	{
		if ( !video_enable && ( c > 6 && c < 13 ) ) continue;
		if ( !video_enable && ( c == 15 || c == 16 ) ) continue;

		elements[c] = gst_element_factory_make ( dvb_all_list_n[c].name, NULL );

		if ( !elements[c] )
			g_critical ( "%s:: element (factory make) - %s not created. \n", __func__, dvb_all_list_n[c].name );

		if ( c == 1 ) gst_element_set_name ( elements[c], "queue-tee-audio" );

		gst_bin_add ( GST_BIN ( element ), elements[c] );

		if ( c > 12 ) continue;

		if (  c == 0 || c == 1 || c == 4 || c == 7 || c == 10 ) continue;

		gst_element_link ( elements[c-1], elements[c] );
	}

	g_signal_connect ( elements[0], "pad-added", G_CALLBACK ( helia_dtv_pad_demux_audio ), elements[1] );
	if ( video_enable ) g_signal_connect ( elements[0], "pad-added", G_CALLBACK ( helia_dtv_pad_demux_video ), elements[7] );

	g_signal_connect ( elements[3], "pad-added", G_CALLBACK ( helia_dtv_pad_decode ), elements[4] );
	if ( video_enable ) g_signal_connect ( elements[9], "pad-added", G_CALLBACK ( helia_dtv_pad_decode ), elements[10] );


	gst_element_link_many ( elements[1], elements[13], elements[14], NULL );
	if ( video_enable ) gst_element_link_many ( elements[7], elements[15], elements[16], NULL );

	gst_element_link ( elements[14], elements[17] );
	if ( video_enable ) gst_element_link ( elements[16], elements[17] );

	gst_element_link ( elements[17], elements[18] );

	helia_dtv_rec_set_location ( elements[18], helia->rec_dir, helia->file_ch, FALSE );

	g_signal_connect ( elements[14], "have-type", G_CALLBACK ( helia_dtv_gst_typefind_parser ), helia );
	if ( video_enable ) g_signal_connect ( elements[16], "have-type", G_CALLBACK ( helia_dtv_gst_typefind_parser ), helia );
}

static void helia_dtv_gst_float_prop ( const char *prop, const char *val, GstElement *element )
{
	GString *string = g_string_new ( val );

	string = g_string_erase ( string, 0, 2 );
	string = g_string_prepend ( string, "0," );

	g_object_set ( element, prop, atof ( string->str ), NULL );

	g_string_free ( string, TRUE );
}

static void helia_dtv_gst_create_dvb_enc_set_prop ( const char *prop, GstElement *element )
{
	if ( prop && strlen ( prop ) > 0 )
	{
		if ( g_strrstr ( prop, "=" ) )
		{
			char **fields = g_strsplit ( prop, " ", 0 );
			uint j = 0, numfields = g_strv_length ( fields );

			for ( j = 0; j < numfields; j++ )
			{
				char **splits = g_strsplit ( fields[j], "=", 0 );
				uint  numsplits = g_strv_length ( splits );

				if ( numsplits == 3 )
				{
					if ( g_str_has_prefix ( splits[0], "int"   ) ) g_object_set ( element, splits[1], atoi ( splits[2] ), NULL );

					if ( g_str_has_prefix ( splits[0], "uint"  ) ) g_object_set ( element, splits[1], atol ( splits[2] ), NULL );

					if ( g_str_has_prefix ( splits[0], "float" ) )
					{
						if ( g_str_has_prefix ( splits[2], "0." ) )
							helia_dtv_gst_float_prop ( splits[1], splits[2], element );
						else
							g_object_set ( element, splits[1], atof ( splits[2] ), NULL );
					}

					if ( g_str_has_prefix ( splits[0], "bool"  ) ) g_object_set ( element, splits[1], ( g_strrstr ( splits[2], "true" ) ) ? TRUE : FALSE, NULL );

					if ( g_str_has_prefix ( splits[0], "char"  ) ) g_object_set ( element, splits[1], splits[2], NULL );

					g_debug ( "%s:: type %s | key = %s | val %s ", __func__, splits[0], splits[1], splits[2] );
				}
				else
					g_warning ( "%s:: not set prop ( length string array < 3 ) - %s \n", __func__, fields[j] );

				g_strfreev ( splits );
			}

			g_strfreev ( fields );
		}
	}
}

static void helia_dtv_create_rec_enc_bin ( GstElement *element, gboolean video_enable, Helia *helia )
{
	struct dvb_all_list { const char *name; } dvb_all_list_n[] =
	{
		{ "tsdemux" },
		{ "queue2"  }, { "decodebin" }, { "tee" }, { "queue2" }, { "audioconvert" }, { "equalizer-nbands" }, { "autoaudiosink" },
		{ "queue2"  }, { "decodebin" }, { "tee" }, { "queue2" }, { "videoconvert" }, { "videobalance"     }, { "autovideosink" },
		{ "queue2"  }, { helia->str_audio_enc },
		{ "queue2"  }, { helia->str_video_enc },
		{ helia->str_muxer_enc }, { "filesink" }
	};

	GstElement *elements[ G_N_ELEMENTS ( dvb_all_list_n ) ];

	uint c = 0;
	for ( c = 0; c < G_N_ELEMENTS ( dvb_all_list_n ); c++ )
	{
		if ( !video_enable && ( c > 7 && c < 15 ) ) continue;
		if ( !video_enable && ( c == 17 || c == 18 ) ) continue;

		elements[c] = gst_element_factory_make ( dvb_all_list_n[c].name, NULL );

		if ( !elements[c] )
			g_critical ( "%s:: element (factory make) - %s not created. \n", __func__, dvb_all_list_n[c].name );

		if ( c == 1 ) gst_element_set_name ( elements[c], "queue-tee-audio" );

		gst_bin_add ( GST_BIN ( element ), elements[c] );

		if (  c == 0 || c == 1 || c == 3 || c == 8 || c == 10 || c == 15 || c == 17 || c == 19 ) continue;

		gst_element_link ( elements[c-1], elements[c] );
	}

	g_signal_connect ( elements[0], "pad-added", G_CALLBACK ( helia_dtv_pad_demux_audio ), elements[1] );
	if ( video_enable ) g_signal_connect ( elements[0], "pad-added", G_CALLBACK ( helia_dtv_pad_demux_video ), elements[8] );

	g_signal_connect ( elements[2], "pad-added", G_CALLBACK ( helia_dtv_pad_decode ), elements[3] );
	if ( video_enable ) g_signal_connect ( elements[9], "pad-added", G_CALLBACK ( helia_dtv_pad_decode ), elements[10] );

	helia_dtv_rec_set_location ( elements[20], helia->rec_dir, helia->file_ch, TRUE );

	gst_element_link ( elements[3],  elements[15] );
	gst_element_link ( elements[16], elements[19] );

	helia_dtv_gst_create_dvb_enc_set_prop ( helia->str_audio_prop, elements[16] );

	if ( video_enable )
	{
		gst_element_link ( elements[10], elements[17] );
		gst_element_link ( elements[18], elements[19] );

		helia_dtv_gst_create_dvb_enc_set_prop ( helia->str_video_prop, elements[18] );
	}
}

static GstPadProbeReturn helia_dtv_blockpad_probe ( GstPad * pad, GstPadProbeInfo * info, gpointer data )
{
	Helia *helia = (Helia *) data;

	GstElement *dvbsrc = helia_gst_iterate_element ( helia->dtv, "dvbsrc", NULL );

	if ( dvbsrc == NULL ) return GST_PAD_PROBE_PASS;

	gst_element_set_state ( helia->dtv, GST_STATE_PAUSED );


	helia_gst_remove_bin ( helia->dtv, "dvbsrc" );

	if ( helia->rec_enc_tv )
		helia_dtv_create_rec_enc_bin ( helia->dtv, helia->checked_video, helia );
	else
		helia_dtv_create_rec_bin ( helia->dtv, helia->checked_video, helia );

	GstElement *tsdemux = helia_gst_iterate_element ( helia->dtv, "tsdemux", NULL );

	if ( tsdemux == NULL ) return GST_PAD_PROBE_PASS;

	gst_element_link ( dvbsrc, tsdemux );

	g_object_set ( tsdemux, "program-number", helia->sid, NULL );

	gst_pad_remove_probe ( pad, GST_PAD_PROBE_INFO_ID (info) );


	gst_element_set_state ( helia->dtv, GST_STATE_PLAYING );

	return GST_PAD_PROBE_OK;
}

static void helia_dtv_set_tuning_timeout ( GstElement *element )
{
	guint64 timeout = 0;
	g_object_get ( element, "tuning-timeout", &timeout, NULL );
	g_object_set ( element, "tuning-timeout", (guint64)timeout / 4, NULL );
}

static void helia_dtv_delsys ( GstElement *element )
{
	uint adapter = 0, frontend = 0, delsys = 0;

	g_object_get ( element, "adapter",  &adapter,  NULL );
	g_object_get ( element, "frontend", &frontend, NULL );
	g_object_get ( element, "delsys",   &delsys,   NULL );

	char *dvb_name = helia_get_dvb_info ( adapter, frontend );
	const char *dvb_type = helia_get_dvb_type_str ( delsys );

	g_debug ( "DVB device: %s ( %s ) | adapter %d frontend %d  ", dvb_name, dvb_type, adapter, frontend );

	g_free ( dvb_name );
}

static gboolean helia_dtv_find_property_scrambling ( GstElement *element )
{
	gboolean scrambling = FALSE;

	if ( element && g_object_class_find_property ( G_OBJECT_GET_CLASS ( element ), "scrambling" ) )
		scrambling = TRUE;
	else
		scrambling = FALSE;

	return scrambling;
}

static gboolean helia_dtv_get_property_scrambling ( Helia *helia )
{
	GstElement *element = helia_gst_iterate_element ( helia->dtv, "tsdemux", NULL );

	if ( element == NULL ) return FALSE;

	gboolean scrambling = FALSE;
	gboolean property_scrambling = helia_dtv_find_property_scrambling ( element );

	if ( property_scrambling ) g_object_get ( element, "scrambling", &scrambling, NULL );

	return scrambling;
}

static void helia_dtv_scrambling ( GstElement *element, char *name )
{
	gboolean scrambling = helia_dtv_find_property_scrambling ( element );

	g_debug ( "%s:: GstTSDemux property scrambling: %s ", __func__, scrambling ? "TRUE": "FALSE" );

	if ( scrambling ) g_object_set ( element, "prog-name", name, NULL );
}

static void helia_dtv_data_set ( GstElement *pipeline, const char *data, Helia *helia )
{
	GstElement *element = helia_gst_iterate_element ( pipeline, "dvbsrc", NULL );
	helia_dtv_set_tuning_timeout ( element );

	char **fields = g_strsplit ( data, ":", 0 );
	uint numfields = g_strv_length ( fields );

	uint j = 0;
	for ( j = 1; j < numfields; j++ )
	{
		if ( g_strrstr ( fields[j], "audio-pid" ) || g_strrstr ( fields[j], "video-pid" ) ) continue;

		if ( !g_strrstr ( fields[j], "=" ) ) continue;

		char **splits = g_strsplit ( fields[j], "=", 0 );

		g_debug ( "%s: gst-param %s | gst-value %s ", __func__, splits[0], splits[1] );

		if ( g_strrstr ( splits[0], "polarity" ) )
		{
			if ( splits[1][0] == 'v' || splits[1][0] == 'V' || splits[1][0] == '0' )
				g_object_set ( element, "polarity", "V", NULL );
			else
				g_object_set ( element, "polarity", "H", NULL );

			g_strfreev (splits);

			continue;
		}

		long dat = atol ( splits[1] );

		if ( g_strrstr ( splits[0], "program-number" ) )
		{
			helia->sid = dat;

			GstElement *demux = helia_gst_iterate_element ( pipeline, "tsdemux", NULL );

			g_object_set ( demux, "program-number", dat, NULL );

			helia_dtv_scrambling ( demux, fields[0] );
		}
		else if ( g_strrstr ( splits[0], "symbol-rate" ) )
		{
			g_object_set ( element, "symbol-rate", ( dat > 100000) ? dat/1000 : dat, NULL );
		}
		else if ( g_strrstr ( splits[0], "lnb-type" ) )
		{
			helia_set_lnb_low_high_switch ( element, dat );
		}
		else
		{
			g_object_set ( element, splits[0], dat, NULL );
		}

		g_strfreev (splits);
	}

	g_strfreev (fields);

	g_debug ( "%s:: \n\n", __func__ );

	helia_dtv_delsys ( element );
}

static gboolean helia_dtv_checked_video ( const char *data )
{
	gboolean video_enable = TRUE;

	if ( !g_strrstr ( data, "video-pid" ) || g_strrstr ( data, "video-pid=0" ) ) video_enable = FALSE;

	return video_enable;
}

void helia_dtv_stop ( Helia *helia )
{
	if ( GST_ELEMENT_CAST ( helia->dtv )->current_state != GST_STATE_NULL )
	{
		gst_element_set_state ( helia->dtv, GST_STATE_NULL );

		helia_power_manager ( helia, FALSE );

		if ( helia->window == NULL ) return;

		helia_gst_remove_bin ( helia->dtv, NULL );

		helia_dtv_clear_audio_tracks ( helia );

		helia->scrambling = FALSE;
		helia->record_tv  = FALSE;
		helia_level_set_sgn_snr ( helia->level, 0, 0, FALSE, FALSE, FALSE, FALSE );
		if ( helia->level_panel ) helia_level_set_sgn_snr ( helia->level_panel, 0, 0, FALSE, FALSE, FALSE, FALSE );

		gtk_widget_queue_draw ( GTK_WIDGET ( helia->window ) );
	}
}

static void helia_dtv_play ( Helia *helia, const char *data )
{
	if ( GST_ELEMENT_CAST ( helia->dtv )->current_state != GST_STATE_PLAYING )
	{
		helia->checked_video = helia_dtv_checked_video ( data );

		helia_dtv_create_bin ( helia->dtv, helia->checked_video );

		helia_dtv_data_set ( helia->dtv, data, helia );

		gst_element_set_state ( helia->dtv, GST_STATE_PLAYING );

		helia_gst_set_volume_tv ( helia->volume_tv, helia );

		if ( helia->checked_video ) helia_power_manager ( helia, TRUE );
	}
}

void helia_dtv_stop_set_play ( Helia *helia, const char *data )
{
	if ( helia->file_ch ) g_free ( helia->file_ch );

	helia->file_ch = g_strdup ( data );

	helia_dtv_stop ( helia );

	helia_dtv_play ( helia, data );
}

void helia_dtv_gst_record ( Helia *helia )
{
	if ( GST_ELEMENT_CAST ( helia->dtv )->current_state != GST_STATE_PLAYING ) return;

	if ( helia->record_tv )
	{
		helia_dtv_stop ( helia );

		helia_dtv_play ( helia, helia->file_ch );
	}
	else
	{
		GstElement *dvbsrc = helia_gst_iterate_element ( helia->dtv, "dvbsrc", NULL );

		if ( dvbsrc == NULL ) return;

		GstPad *blockpad = gst_element_get_static_pad ( dvbsrc, "src" );

		gst_pad_add_probe ( blockpad, GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM,
							helia_dtv_blockpad_probe, helia, NULL );

		gst_object_unref ( blockpad );

		helia->record_tv = TRUE;
	}
}

static GstBusSyncReply helia_dtv_sync_handler ( G_GNUC_UNUSED GstBus *bus, GstMessage *message, Helia *helia )
{
	if ( !gst_is_video_overlay_prepare_window_handle_message ( message ) ) return GST_BUS_PASS;

	if ( helia->xid_tv != 0 )
	{
		GstVideoOverlay *xoverlay = GST_VIDEO_OVERLAY ( GST_MESSAGE_SRC ( message ) );
		gst_video_overlay_set_window_handle ( xoverlay, helia->xid_tv );

	} else { g_warning ( "Should have obtained window_handle by now!" ); }

	gst_message_unref ( message );

	return GST_BUS_DROP;
}

static void helia_dtv_msg_err ( G_GNUC_UNUSED GstBus *bus, GstMessage *msg, Helia *helia )
{
	GError *err = NULL;
	char  *dbg = NULL;

	gst_message_parse_error ( msg, &err, &dbg );

	g_critical ( "%s: %s (%s)", __func__, err->message, (dbg) ? dbg : "no details" );

	helia_message_dialog ( "", err->message, GTK_MESSAGE_ERROR, helia->window );

	g_error_free ( err );
	g_free ( dbg );

	if ( GST_ELEMENT_CAST ( helia->dtv )->current_state != GST_STATE_PLAYING )
		helia_dtv_stop ( helia );
}

static void helia_dtv_msg_all ( G_GNUC_UNUSED GstBus *bus, GstMessage *msg, Helia *helia )
{
	if ( helia->window == NULL ) return;

	const GstStructure *structure = gst_message_get_structure ( msg );

	if ( structure )
	{
		int signal, snr;
		gboolean hlook = FALSE, play = TRUE, panel = FALSE;
		static gboolean pulse = FALSE;

		if (  gst_structure_get_int ( structure, "signal", &signal )  )
		{
			gst_structure_get_int     ( structure, "snr",  &snr   );
			gst_structure_get_boolean ( structure, "lock", &hlook );

			if ( GST_ELEMENT_CAST ( helia->dtv )->current_state < GST_STATE_PLAYING ) play = FALSE;

			if ( helia->level_panel && gtk_widget_get_visible ( GTK_WIDGET ( helia->level_panel ) ) ) panel = TRUE;

			if ( pulse )
				helia_level_set_sgn_snr ( ( panel ) ? helia->level_panel : helia->level, 
					(signal * 100) / 0xffff, (snr * 100) / 0xffff, hlook, play, helia->record_tv, helia->scrambling );

			pulse = !pulse;
		}
	}

	mpegts_pmt_lang_section ( msg, helia->sid, helia->audio_lang );
}

static void helia_dtv_msg_cng ( G_GNUC_UNUSED GstBus *bus, G_GNUC_UNUSED GstMessage *msg, Helia *helia )
{
	if ( GST_MESSAGE_SRC ( msg ) != GST_OBJECT ( helia->dtv ) ) return;

	GstState old_state, new_state;

	gst_message_parse_state_changed ( msg, &old_state, &new_state, NULL );

	switch ( new_state )
	{
		case GST_STATE_NULL:
		case GST_STATE_READY:
		case GST_STATE_PAUSED:
			break;

		case GST_STATE_PLAYING:
		{
			helia->scrambling = helia_dtv_get_property_scrambling ( helia );

			break;
		}

		default:
			break;
	}

	g_debug ( "%s:: Element %s changed state from %s to %s.", __func__, GST_OBJECT_NAME (msg->src), 
		gst_element_state_get_name (old_state), gst_element_state_get_name (new_state) );
}

static GstElement * helia_dtv_create ( Helia *helia )
{
	GstElement *dvbplay = gst_pipeline_new ( "pipeline" );

	if ( !dvbplay )
	{
		g_critical ( "%s: dvbplay - not created.", __func__ );

		return NULL;
	}

	GstBus *bus = gst_element_get_bus ( dvbplay );

	gst_bus_add_signal_watch_full ( bus, G_PRIORITY_DEFAULT );
	gst_bus_set_sync_handler ( bus, (GstBusSyncHandler)helia_dtv_sync_handler, helia, NULL );

	g_signal_connect ( bus, "message",        G_CALLBACK ( helia_dtv_msg_all ), helia );
	g_signal_connect ( bus, "message::error", G_CALLBACK ( helia_dtv_msg_err ), helia );
	g_signal_connect ( bus, "message::state-changed", G_CALLBACK ( helia_dtv_msg_cng ), helia );

	gst_object_unref (bus);

	return dvbplay;
}

HeliaTv * helia_dtv_new ( Helia *helia )
{
	return (HeliaTv *)helia_dtv_create ( helia );
}

