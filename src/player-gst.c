/*
* Copyright 2019 Stepan Perun
* This program is free software.
* 
* License: Gnu Lesser General Public License
* http://www.gnu.org/licenses/lgpl.html
*/


#include <gst/video/videooverlay.h>

#include <base.h>

#include "player-slider.h"
#include "tree-view.h"
#include "info.h"
#include "lang.h"


static void player_stop_record ( Base *base, gboolean play, gboolean play_rec );


GstElement * player_gst_ret_iterate_element ( GstElement *it_element, const char *name1, const char *name2 )
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
					{
						element_ret = element;
					}
					else
						element_ret = element;
				}

				g_debug ( "%s:: Object name: %s ", __func__, object_name );

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

gboolean player_mute_get ( Base *base )
{
	GstElement *element = base->player->playbin;

	if ( base->player->record )
	{
		element = player_gst_ret_iterate_element ( base->player->pipeline_rec, "autoaudiosink", "actual-sink" );

		if ( element == NULL ) return FALSE;
	}

	if ( GST_ELEMENT_CAST ( element )->current_state != GST_STATE_PLAYING ) return TRUE;

	gboolean mute = FALSE;

	g_object_get ( element, "mute", &mute, NULL );

	return mute;
}

void player_mute_set ( Base *base )
{
	GstElement *element = base->player->playbin;

	if ( base->player->record )
	{
		element = player_gst_ret_iterate_element ( base->player->pipeline_rec, "autoaudiosink", "actual-sink" );

		if ( element == NULL ) return;
	}

	if ( GST_ELEMENT_CAST ( element )->current_state != GST_STATE_PLAYING ) return;

	gboolean mute = FALSE;

	g_object_get ( element, "mute", &mute, NULL );
	g_object_set ( element, "mute", !mute, NULL );
}

void player_volume_changed ( G_GNUC_UNUSED GtkScaleButton *button, gdouble value, Base *base )
{
	GstElement *element = base->player->playbin;

	if ( base->player->record )
	{
		element = player_gst_ret_iterate_element ( base->player->pipeline_rec, "autoaudiosink", "actual-sink" );

		if ( element == NULL ) return;
	}

	if ( GST_ELEMENT_CAST ( element )->current_state != GST_STATE_PLAYING ) return;

	base->player->volume = value;

	g_object_set ( element, "volume", value, NULL );
}

static void player_gst_step_pos ( Base *base, gint64 am )
{
    gst_element_send_event ( base->player->playbin, gst_event_new_step ( GST_FORMAT_BUFFERS, am, 1.0, TRUE, FALSE ) );

    gint64 current = 0;

	if ( gst_element_query_position ( base->player->playbin, GST_FORMAT_TIME, &current ) )
	{
		player_slider_set_data ( base, current, 7, -1, 10, TRUE );
	}
}
void player_step_frame ( Base *base )
{
	if ( base->player->record ) return;

	if ( GST_ELEMENT_CAST ( base->player->playbin )->current_state == GST_STATE_NULL ) return;

    gint n_video = 0;
    g_object_get ( base->player->playbin, "n-video", &n_video, NULL );

    if ( n_video == 0 ) return;

    if ( GST_ELEMENT_CAST ( base->player->playbin )->current_state == GST_STATE_PLAYING )
		gst_element_set_state ( base->player->playbin, GST_STATE_PAUSED );

	player_gst_step_pos ( base, 1 );
}

void player_gst_new_pos ( Base *base, gint64 set_pos, gboolean up_dwn )
{
	gboolean dur_b = FALSE;
	gint64 current = 0, duration = 0, new_pos = 0, skip = (gint64)( set_pos * GST_SECOND );

	if ( gst_element_query_position ( base->player->playbin, GST_FORMAT_TIME, &current ) )
	{
		if ( gst_element_query_duration ( base->player->playbin, GST_FORMAT_TIME, &duration ) ) dur_b = TRUE;

		if ( !dur_b || duration / GST_SECOND < 1 ) return;

		if ( up_dwn ) new_pos = ( duration > ( current + skip ) ) ? ( current + skip ) : duration;

		if ( !up_dwn ) new_pos = ( current > skip ) ? ( current - skip ) : 0;

		gst_element_seek_simple ( base->player->playbin, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT, new_pos );

		if ( GST_ELEMENT_CAST ( base->player->playbin )->current_state == GST_STATE_PAUSED )
		{
			player_slider_set_data ( base, new_pos, 8, -1, 10, TRUE );

			player_slider_update_slider ( base->player->slider_base, (gdouble)duration / GST_SECOND, (gdouble)new_pos / GST_SECOND );
		}
	}
}

void player_stop ( Base *base )
{
	base->player->duration_old = 0;

	if ( base->player->record || base->player->record_f )
	{
		player_stop_record ( base, FALSE, FALSE );
		return;
	}

	if ( GST_ELEMENT_CAST ( base->player->playbin )->current_state != GST_STATE_NULL )
	{
		gst_element_set_state ( base->player->playbin, GST_STATE_NULL );

		player_slider_clear_data ( base );

		gtk_widget_queue_draw ( GTK_WIDGET ( base->window ) );
	}
}

static gboolean player_update_win ( Base *base )
{
	if ( base->app_quit ) return FALSE;

	if ( base->player->record ) return FALSE;

	if ( GST_ELEMENT_CAST ( base->player->playbin )->current_state == GST_STATE_NULL    ) return FALSE;

	if ( GST_ELEMENT_CAST ( base->player->playbin )->current_state == GST_STATE_PAUSED  ) return FALSE;

	if ( GST_ELEMENT_CAST ( base->player->playbin )->current_state == GST_STATE_PLAYING )
	{
		char *title_new = info_get_title_artist ( base );
		
		if ( title_new )
		{
			if ( g_unichar_isprint ( title_new[0] ) )
				info_title_save ( base, title_new, TRUE );

			g_free ( title_new );
		}

		gtk_widget_queue_draw ( GTK_WIDGET ( base->window ) );

		return FALSE;
	}
	else
	{
		time ( &base->player->t_cur_mp );

		if ( ( base->player->t_cur_mp - base->player->t_start_mp ) >= 10 )
		{
			g_warning ( "%s: Time stop %ld (sec) ", __func__, base->player->t_cur_mp - base->player->t_start_mp );

			player_stop ( base );

			return FALSE;
		}
	}

	return TRUE;
}

void player_play_paused ( Base *base )
{
	if ( base->player->record ) return;

	if (    GST_ELEMENT_CAST ( base->player->playbin )->current_state == GST_STATE_PAUSED 
	     || GST_ELEMENT_CAST ( base->player->playbin )->current_state == GST_STATE_NULL   )
		gst_element_set_state ( base->player->playbin, GST_STATE_PLAYING );
	else
		if ( GST_ELEMENT_CAST ( base->player->playbin )->current_state == GST_STATE_PLAYING )
			gst_element_set_state ( base->player->playbin, GST_STATE_PAUSED );
}

void player_set_subtitle ( Base *base, gboolean state_subtitle )
{
	enum gst_flags { GST_FLAG_TEXT = (1 << 2) };

	uint flags;
	g_object_get ( base->player->playbin, "flags", &flags, NULL );

	if (  state_subtitle ) flags |=  GST_FLAG_TEXT;
	if ( !state_subtitle ) flags &= ~GST_FLAG_TEXT;

	g_object_set ( base->player->playbin, "flags", flags, NULL );
}

void player_set_vis ( Base *base )
{
	enum gst_flags { GST_FLAG_VIS = (1 << 3) };

	uint flags;
	g_object_get ( base->player->playbin, "flags", &flags, NULL );

	if (  base->player->vis_plugin ) flags |=  GST_FLAG_VIS;
	if ( !base->player->vis_plugin ) flags &= ~GST_FLAG_VIS;

	g_object_set ( base->player->playbin, "flags", flags, NULL );

	GstElement *vis = gst_element_factory_make ( "goom", NULL );
	g_object_set ( base->player->playbin, "vis-plugin", ( base->player->vis_plugin ) ? vis : NULL, NULL );
}

void player_play ( Base *base )
{
	if ( !base->player->file_play ) return;

	if ( base->player->record ) return;

	if ( GST_ELEMENT_CAST ( base->player->playbin )->current_state != GST_STATE_PLAYING )
	{
		player_set_vis ( base );

		gst_element_set_state ( base->player->playbin, GST_STATE_PLAYING );

		g_object_set ( base->player->playbin, "volume", base->player->volume, NULL );

		g_timeout_add ( 250, (GSourceFunc)player_update_win, base );

		time ( &base->player->t_start_mp );
	}
}

void player_play_set_uri ( Base *base, const char *name_file )
{
    if ( g_strrstr ( name_file, "://" ) )
        g_object_set ( base->player->playbin, "uri", name_file, NULL );
    else
    {
        char *uri = gst_filename_to_uri ( name_file, NULL );

            g_object_set ( base->player->playbin, "uri", uri, NULL );

        g_free ( uri );
    }
}

void player_stop_set_play ( Base *base, const char *name_file )
{
	player_stop ( base );

	if ( base->player->file_play ) g_free ( base->player->file_play );
	base->player->file_play = g_strdup ( name_file );

    player_play_set_uri ( base, name_file );

	player_play ( base );
}

static GstBusSyncReply player_gst_bus_sync_handler ( G_GNUC_UNUSED GstBus *bus, GstMessage *message, Base *base )
{
    if ( !gst_is_video_overlay_prepare_window_handle_message ( message ) ) return GST_BUS_PASS;

    if ( base->player->window_hid != 0 )
    {
        GstVideoOverlay *xoverlay = GST_VIDEO_OVERLAY ( GST_MESSAGE_SRC ( message ) );
        gst_video_overlay_set_window_handle ( xoverlay, base->player->window_hid );

    } else { g_warning ( "Should have obtained window_handle by now!" ); }

    gst_message_unref ( message );

    return GST_BUS_DROP;
}

static void player_gst_msg_err ( G_GNUC_UNUSED GstBus *bus, GstMessage *msg, Base *base )
{
    GError *err = NULL;
    char  *dbg = NULL;

    gst_message_parse_error ( msg, &err, &dbg );

    g_critical ( "%s: %s (%s)", __func__, err->message, (dbg) ? dbg : "no details" );

    base_message_dialog ( "", err->message, GTK_MESSAGE_ERROR, base->window );

    g_error_free ( err );
    g_free ( dbg );

    player_stop ( base );
}

static void player_gst_msg_eos ( G_GNUC_UNUSED GstBus *bus, G_GNUC_UNUSED GstMessage *msg, Base *base )
{
	if ( base->player->is_live )
	{
		player_stop ( base );

		if ( base->player->record || base->player->record_f )
			base_message_dialog ( "EOS", _i18n_ ( base, "Live recording stopped." ), GTK_MESSAGE_WARNING, base->window );
		else
			base_message_dialog ( "EOS", _i18n_ ( base, "Live playback stopped."  ), GTK_MESSAGE_WARNING, base->window );

		// gst_element_set_state ( base->player->playbin, GST_STATE_NULL    );
		// gst_element_set_state ( base->player->playbin, GST_STATE_PLAYING );
	}
	else
	{
		player_next ( base );
	}
}

static void player_gst_msg_cll ( G_GNUC_UNUSED GstBus *bus, G_GNUC_UNUSED GstMessage *msg, Base *base )
{
	if ( GST_ELEMENT_CAST ( base->player->playbin )->current_state == GST_STATE_PLAYING )
	{
		gst_element_set_state ( base->player->playbin, GST_STATE_PAUSED  );
		gst_element_set_state ( base->player->playbin, GST_STATE_PLAYING );
	}
}

static void player_gst_msg_buf ( G_GNUC_UNUSED GstBus *bus, GstMessage *msg, Base *base )
{
	if ( base->app_quit ) return;

	int percent;
	gst_message_parse_buffering ( msg, &percent );

	if ( percent == 100 )
	{
		// if ( GST_ELEMENT_CAST ( base->player->playbin )->current_state == GST_STATE_PAUSED )
			gst_element_set_state ( base->player->playbin, GST_STATE_PLAYING );

		gtk_label_set_text ( base->player->slider_base.rec_buf, "---" );

		if ( !base->player->panel_quit && gtk_widget_get_visible ( GTK_WIDGET ( base->player->h_box_slider_panel ) ) )
				gtk_label_set_text ( base->player->slider_panel.rec_buf, "---" );
	}
	else
	{
		if ( GST_ELEMENT_CAST ( base->player->playbin )->current_state == GST_STATE_PLAYING )
			gst_element_set_state ( base->player->playbin, GST_STATE_PAUSED );

		char *str = ( percent < 10 ) ? g_strdup_printf ( "0%d%s", percent, "%" ) : g_strdup_printf ( "%d%s", percent, "%" );

			gtk_label_set_text ( base->player->slider_base.rec_buf, str );

			if ( !base->player->panel_quit && gtk_widget_get_visible ( GTK_WIDGET ( base->player->h_box_slider_panel ) ) )
				gtk_label_set_text ( base->player->slider_panel.rec_buf, str );

		// g_print ( "%s: buffering: %s \n", __func__, str );

		free ( str );
	}
}

GstElement * player_gst_create ( Base *base )
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

    bin_audio = gst_bin_new ( "audio_sink_bin" );
	gst_bin_add_many ( GST_BIN ( bin_audio ), equalizer, asink, NULL );
	gst_element_link_many ( equalizer, asink, NULL );

	GstPad *pad = gst_element_get_static_pad ( equalizer, "sink" );
	gst_element_add_pad ( bin_audio, gst_ghost_pad_new ( "sink", pad ) );
	gst_object_unref ( pad );

	bin_video = gst_bin_new ( "video_sink_bin" );
	gst_bin_add_many ( GST_BIN ( bin_video ), videoblnc, vsink, NULL );
	gst_element_link_many ( videoblnc, vsink, NULL );

	GstPad *padv = gst_element_get_static_pad ( videoblnc, "sink" );
	gst_element_add_pad ( bin_video, gst_ghost_pad_new ( "sink", padv ) );
	gst_object_unref ( padv );

    g_object_set ( playbin, "video-sink", bin_video, NULL );
    g_object_set ( playbin, "audio-sink", bin_audio, NULL );

    g_object_set ( playbin, "volume", base->player->volume, NULL );

    GstBus *bus = gst_element_get_bus ( playbin);

    gst_bus_add_signal_watch_full ( bus, G_PRIORITY_DEFAULT );
    gst_bus_set_sync_handler ( bus, (GstBusSyncHandler)player_gst_bus_sync_handler, base, NULL );

    g_signal_connect ( bus, "message::eos",   		G_CALLBACK ( player_gst_msg_eos ), base );
    g_signal_connect ( bus, "message::error", 		G_CALLBACK ( player_gst_msg_err ), base );
    g_signal_connect ( bus, "message::clock-lost",  G_CALLBACK ( player_gst_msg_cll ), base );
    g_signal_connect ( bus, "message::buffering",   G_CALLBACK ( player_gst_msg_buf ), base );

    gst_object_unref ( bus );

    return playbin;
}


// Record IPTV

static char * player_time_to_str ()
{
	GDateTime *date = g_date_time_new_now_local ();

	char *str_time = g_date_time_format ( date, "%j-%Y-%T" );

	g_date_time_unref ( date );

	return str_time;
}

static void player_rec_set_location ( GstElement *element, char *rec_dir )
{
	char *date_str = player_time_to_str ();
	char *file_rec = g_strdup_printf ( "%s/Record-iptv-%s", rec_dir, date_str );

	g_object_set ( element, "location", file_rec, NULL );

	g_free ( file_rec );
	g_free ( date_str );
}

static void player_gst_pad_link ( GstPad *pad, GstElement *element, const char *name, G_GNUC_UNUSED GstElement *element_n )
{
	GstPad *pad_va_sink = gst_element_get_static_pad ( element, "sink" );

	if ( gst_pad_link ( pad, pad_va_sink ) == GST_PAD_LINK_OK )
		gst_object_unref ( pad_va_sink );
	else
		g_debug ( "%s:: linking demux/decode name %s video/audio pad failed ", __func__, name );
}

static void player_gst_pad_hlsdemux ( GstElement *element, GstPad *pad_new, Base *base )
{
	GstElement *element_link = player_gst_ret_iterate_element ( base->player->pipeline_rec, "tee-hls-rec", NULL ); 

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
					player_gst_pad_link ( pad_new, element_link, "hlsdemux", element );
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

static gboolean player_gst_pad_check_type ( GstPad *pad, const char *type )
{
	gboolean ret = FALSE;

	GstCaps *caps = gst_pad_get_current_caps ( pad );

	const char *name = gst_structure_get_name ( gst_caps_get_structure ( caps, 0 ) );

	g_debug ( "%s:: caps type: %s ( find type %s ) ",__func__, name, type );

	if ( g_str_has_prefix ( name, type ) ) ret = TRUE;

	gst_caps_unref (caps);

	return ret;
}

static void player_gst_pad_demux_audio ( GstElement *element, GstPad *pad, GstElement *element_audio )
{
	if ( player_gst_pad_check_type ( pad, "audio" ) ) player_gst_pad_link ( pad, element_audio, "demux audio", element );
}

static void player_gst_pad_demux_video ( GstElement *element, GstPad *pad, GstElement *element_video )
{
	if ( player_gst_pad_check_type ( pad, "video" ) ) player_gst_pad_link ( pad, element_video, "demux video", element );
}

static void player_gst_pad_decode ( GstElement *element, GstPad *pad, GstElement *element_va )
{
	player_gst_pad_link ( pad, element_va, "decode  audio / video", element );
}
/*
static void player_gst_msg_buf_rec ( G_GNUC_UNUSED GstBus *bus, GstMessage *msg, Base *base )
{
	if ( base->app_quit ) return;

	int percent;
	gst_message_parse_buffering ( msg, &percent );

	if ( percent == 100 )
	{
		// if ( GST_ELEMENT_CAST ( base->player->pipeline_rec )->current_state == GST_STATE_PAUSED )
			gst_element_set_state ( base->player->pipeline_rec, GST_STATE_PLAYING );

		gtk_label_set_text ( base->player->slider_base.rec_buf, "---" );

		if ( !base->player->panel_quit && gtk_widget_get_visible ( GTK_WIDGET ( base->player->h_box_slider_panel ) ) )
				gtk_label_set_text ( base->player->slider_panel.rec_buf, "---" );
	}
	else
	{
		if ( GST_ELEMENT_CAST ( base->player->pipeline_rec )->current_state == GST_STATE_PLAYING )
			gst_element_set_state ( base->player->pipeline_rec, GST_STATE_PAUSED );

		char *str = ( percent < 10 ) ? g_strdup_printf ( "0%d%s", percent, "%" ) : g_strdup_printf ( "%d%s", percent, "%" );

			gtk_label_set_text ( base->player->slider_base.rec_buf, str );

			if ( !base->player->panel_quit && gtk_widget_get_visible ( GTK_WIDGET ( base->player->h_box_slider_panel ) ) )
				gtk_label_set_text ( base->player->slider_panel.rec_buf, str );

		// g_print ( "%s: Rec buffering: %s \n", __func__, str );

		free ( str );
	}
}
*/
static void player_record_update_position ( Base *base )
{
	if ( GST_ELEMENT_CAST ( base->player->pipeline_rec )->current_state != GST_STATE_PLAYING ) return;

	gint64 duration = 0, current = 0;

	if ( gst_element_query_position ( base->player->pipeline_rec, GST_FORMAT_TIME, &current ) )
	{
		if ( gst_element_query_duration ( base->player->pipeline_rec, GST_FORMAT_TIME, &duration ) )
		{
			if ( duration / GST_SECOND > 0 )
				player_slider_set_data ( base, current, 10, duration, 10, TRUE );
			else
				player_slider_set_data ( base, current, 10, -1, 10, FALSE );
		}
		else
			player_slider_set_data ( base, current, 10, -1, 10, FALSE );
	}
}

static void player_rec_status_update ( Base *base )
{
	const char *format = "<span foreground=\"#FF0000\">%s</span>";

	char *markup = g_markup_printf_escaped ( format, ( base->player->rec_status ) ? " ◉ " : " ◌ " );

		gtk_label_set_markup ( base->player->slider_base.rec_sts, markup );

		if ( !base->player->panel_quit && gtk_widget_get_visible ( GTK_WIDGET ( base->player->h_box_slider_panel ) ) )
			gtk_label_set_markup ( base->player->slider_panel.rec_sts, markup );

		base->player->rec_status = !base->player->rec_status;

	g_free ( markup );
}

static gboolean player_record_update_hls ( Base *base )
{
	if ( base->app_quit ) return FALSE;

	if ( !base->player->record ) { player_slider_clear_data ( base ); return FALSE; }

	player_record_update_position ( base );

	player_rec_status_update ( base );

	return TRUE;
}

static gboolean player_record_update_file ( Base *base )
{
	if ( base->app_quit ) return FALSE;

	if ( !base->player->record_f ) { player_slider_clear_data ( base ); return FALSE; }

	player_rec_status_update ( base );

	return TRUE;
}

static void player_gst_rec_remove ( GstElement *pipeline )
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

				g_debug ( "%s:: Object remove: %s", __func__, object_name );

				gst_bin_remove ( GST_BIN ( pipeline ), element );

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

static GstElement * player_gst_create_rec_pipeline ( Base *base )
{
	GstElement *pipeline_rec = gst_pipeline_new ( "pipeline-record" );

    if ( !pipeline_rec )
    {
        g_critical ( "%s: pipeline-record - not created.", __func__ );

        return NULL;
    }

    GstBus *bus = gst_element_get_bus ( pipeline_rec );

    gst_bus_add_signal_watch_full ( bus, G_PRIORITY_DEFAULT );
    gst_bus_set_sync_handler ( bus, (GstBusSyncHandler)player_gst_bus_sync_handler, base, NULL );

    g_signal_connect ( bus, "message::error",     G_CALLBACK ( player_gst_msg_err     ), base );
    // g_signal_connect ( bus, "message::buffering", G_CALLBACK ( player_gst_msg_buf_rec ), base );

    gst_object_unref ( bus );

    return pipeline_rec;
}

static GstElement * player_gst_rec_create_hls ( Base *base, uint n_video, uint n_audio )
{
	GstElement *pipeline_rec = player_gst_create_rec_pipeline ( base );

	if ( !pipeline_rec ) return NULL;

	// g_print ( "%s: Rec HLS \n", __func__ );

	struct rec_all { const char *name; } rec_all_n[] =
	{
		{ "souphttpsrc" }, { "hlsdemux"  }, { "tee"          }, { "queue2"            }, { "decodebin"     },
		{ "queue2"      }, { "decodebin" }, { "audioconvert" }, { "equalizer-nbands"  }, { "autoaudiosink" },
		{ "queue2"      }, { "decodebin" }, { "videoconvert" }, { "videobalance"      }, { "autovideosink" },
		{ "queue2"      }, { "hlssink"   }
	};

	GstElement *elements[ G_N_ELEMENTS ( rec_all_n ) ];

	uint c = 0;
	for ( c = 0; c < G_N_ELEMENTS ( rec_all_n ); c++ )
	{
		if ( n_video == 0 && ( c == 10 || c == 11 || c == 12 || c == 13 || c == 14 ) ) continue;

		if (  c == 0 )
			elements[c] = gst_element_make_from_uri ( GST_URI_SRC, base->player->file_play, NULL, NULL );
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

	g_signal_connect ( elements[1], "pad-added",   G_CALLBACK ( player_gst_pad_hlsdemux ), base );

	if ( n_audio > 0 ) g_signal_connect ( elements[4], "pad-added", G_CALLBACK ( player_gst_pad_demux_audio ), elements[5] );
	if ( n_video > 0 ) g_signal_connect ( elements[4], "pad-added", G_CALLBACK ( player_gst_pad_demux_video ), elements[10] );

	if ( n_audio > 0 ) g_signal_connect ( elements[6],  "pad-added", G_CALLBACK ( player_gst_pad_decode ), elements[7]  );
	if ( n_video > 0 ) g_signal_connect ( elements[11], "pad-added", G_CALLBACK ( player_gst_pad_decode ), elements[12] );

	// g_object_set ( elements[4],  "use-buffering", TRUE, NULL );

	if ( g_object_class_find_property ( G_OBJECT_GET_CLASS ( elements[0] ), "location" ) )
		g_object_set ( elements[0],  "location", base->player->file_play, NULL );
	else
		g_object_set ( elements[0], "uri", base->player->file_play, NULL );

	gst_element_link ( elements[2],  elements[15] );

	player_rec_set_location ( elements[16], base->rec_dir );

    return pipeline_rec;
}

static GstElement * player_gst_rec_create_file ( Base *base )
{
	GstElement *pipeline_rec = player_gst_create_rec_pipeline ( base );

	if ( !pipeline_rec ) return NULL;

	// g_print ( "%s: Rec No HLS \n", __func__ );

	struct rec_all { const char *name; } rec_all_n[] =
	{
		{ "souphttpsrc" }, { "queue2" }, { "filesink" }
	};

	GstElement *elements[ G_N_ELEMENTS ( rec_all_n ) ];

	uint c = 0;
	for ( c = 0; c < G_N_ELEMENTS ( rec_all_n ); c++ )
	{
		if (  c == 0 )
			elements[c] = gst_element_make_from_uri ( GST_URI_SRC, base->player->file_play, NULL, NULL );
		else
			elements[c] = gst_element_factory_make ( rec_all_n[c].name, NULL );

		if ( !elements[c] )
		{
			g_critical ( "%s:: element (factory make) - %s not created. \n", __func__, rec_all_n[c].name );

			return NULL;
		}

		gst_bin_add ( GST_BIN ( pipeline_rec ), elements[c] );
	}

	gst_element_link_many ( elements[0], elements[1], elements[2], NULL );

	if ( g_object_class_find_property ( G_OBJECT_GET_CLASS ( elements[0] ), "location" ) )
		g_object_set ( elements[0],  "location", base->player->file_play, NULL );
	else
		g_object_set ( elements[0], "uri", base->player->file_play, NULL );

	// g_object_set ( elements[1],  "use-buffering", TRUE, NULL );

	player_rec_set_location ( elements[2], base->rec_dir );

    return pipeline_rec;
}

static gboolean player_record_new_play ( Base *base )
{
	GstElement *element = base->player->pipeline_rec;

	element = player_gst_ret_iterate_element ( base->player->pipeline_rec, "filesink", NULL );

	if ( element == NULL ) return FALSE;

	char *file = NULL;

	g_object_get ( element, "location", &file, NULL );

	g_debug ( "%s:: file rec: %s ", __func__, file );

	gst_element_set_state ( base->player->playbin, GST_STATE_NULL );

	player_play_set_uri ( base, file );

	gst_element_set_state ( base->player->playbin, GST_STATE_PLAYING );

	g_free ( file );

	return FALSE;
}

static void player_stop_record ( Base *base, gboolean play, gboolean play_rec )
{
	g_debug ( "%s:: play: %s ", __func__, play ? "TRUE": "FALSE" );

    if ( base->player->record || base->player->record_f )
    {
		gst_element_set_state ( base->player->pipeline_rec, GST_STATE_NULL );

		player_gst_rec_remove ( base->player->pipeline_rec );

		gst_object_unref ( base->player->pipeline_rec );

		base->player->pipeline_rec = NULL;

		gtk_widget_queue_draw ( GTK_WIDGET ( base->window ) );

		if ( base->player->record ) base->player->record = !base->player->record;

		if ( base->player->record_f ) base->player->record_f = !base->player->record_f;

		if ( play ) player_play ( base ); else player_stop ( base );

		if ( play_rec )
		{
			gst_element_set_state ( base->player->playbin, GST_STATE_NULL );

			player_play_set_uri ( base, base->player->file_play );

			gst_element_set_state ( base->player->playbin, GST_STATE_PLAYING );
		}
	}
}

void player_record ( Base *base )
{
    if ( base->player->record || base->player->record_f )
    {
		player_stop_record ( base, ( base->player->record ) ? TRUE : FALSE, base->player->record_f );
	}
	else
	{
		if ( GST_ELEMENT_CAST ( base->player->playbin )->current_state == GST_STATE_NULL ) return;

		uint n_video, n_audio;
		g_object_get ( base->player->playbin, "n-video", &n_video, NULL );
		g_object_get ( base->player->playbin, "n-audio", &n_audio, NULL );

		if ( !gst_uri_is_valid ( base->player->file_play ) || g_str_has_prefix ( base->player->file_play, "file://" ) )
		{
			base_message_dialog ( "", "Only IPTV.", GTK_MESSAGE_WARNING, base->window );

			return;
		}

		char *uri_protocol = gst_uri_get_protocol ( base->player->file_play );	
		gboolean is_supported = gst_uri_protocol_is_supported ( GST_URI_SRC, uri_protocol );

		if ( !is_supported )
		{
			base_message_dialog ( "", "The protocol is not supported.", GTK_MESSAGE_WARNING, base->window );

			g_free ( uri_protocol );

			return;
		}

		g_free ( uri_protocol );

		if ( n_video > 0 ) base->player->rec_video_enable = TRUE; else base->player->rec_video_enable = FALSE;

		GstElement *element = player_gst_ret_iterate_element ( base->player->playbin, "hlsdemux", NULL );

		if ( element )
			base->player->pipeline_rec = player_gst_rec_create_hls ( base, n_video, n_audio );
		else
			base->player->pipeline_rec = player_gst_rec_create_file ( base );
			// base_message_dialog ( "", "Only m3u8.", GTK_MESSAGE_WARNING, base->window );

		if ( base->player->pipeline_rec != NULL )
		{
			player_stop ( base );

			gst_element_set_state ( base->player->pipeline_rec, GST_STATE_PLAYING );

			if ( element )
			{
				base->player->record = !base->player->record;

				g_timeout_add_seconds ( 1, (GSourceFunc)player_record_update_hls, base );
			}
			else
			{
				base->player->record_f = !base->player->record_f;

				g_timeout_add_seconds ( 1, (GSourceFunc)player_record_update_file, base );

				g_timeout_add_seconds ( 3, (GSourceFunc)player_record_new_play, base );
			}
		}
	}
}
