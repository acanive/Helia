/*
* Copyright 2019 Stepan Perun
* This program is free software.
* 
* License: Gnu Lesser General Public License
* http://www.gnu.org/licenses/lgpl.html
*/


#include <gst/video/videooverlay.h>

#include <base.h>

#include "dtv-level.h"
#include "scan.h"
#include "mpegts.h"


GstElement * dtv_gst_ret_iterate_element ( GstElement *it_element, const char *name1, const char *name2 )
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

				g_debug ( "%s: Object name: %s ", __func__, object_name );

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

gboolean dtv_mute_get ( GstElement *dvbplay )
{
	if ( GST_ELEMENT_CAST ( dvbplay )->current_state != GST_STATE_PLAYING ) return TRUE;

	GstElement *element = dtv_gst_ret_iterate_element ( dvbplay, "autoaudiosink", "actual-sink" );

	if ( element == NULL ) return FALSE;

	gboolean mute = FALSE;

	g_object_get ( element, "mute", &mute, NULL );

	return mute;
}

void dtv_mute_set ( GstElement *dvbplay )
{
	if ( GST_ELEMENT_CAST ( dvbplay )->current_state != GST_STATE_PLAYING ) return;

	GstElement *element = dtv_gst_ret_iterate_element ( dvbplay, "autoaudiosink", "actual-sink" );

	if ( element == NULL ) return;

	gboolean mute = FALSE;

	g_object_get ( element, "mute", &mute, NULL );
	g_object_set ( element, "mute", !mute, NULL );
}

static void dtv_gst_volume_set ( GstElement *dvbplay, gdouble value )
{
	if ( GST_ELEMENT_CAST ( dvbplay )->current_state != GST_STATE_PLAYING ) return;

	GstElement *element = dtv_gst_ret_iterate_element ( dvbplay, "autoaudiosink", "actual-sink" );

	if ( element == NULL ) return;

	g_object_set ( element, "volume", value, NULL );
}

void dtv_volume_changed ( G_GNUC_UNUSED GtkScaleButton *button, gdouble value, Base *base )
{
	if ( GST_ELEMENT_CAST ( base->dtv->dvbplay )->current_state != GST_STATE_PLAYING ) return;

	base->dtv->volume = value;

	dtv_gst_volume_set ( base->dtv->dvbplay, base->dtv->volume );
}

static void dtv_gst_change_audio_track ( GstElement *e_unlink, GstElement *e_link, uint num )
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
							g_debug ( "%s: unlink Failed ", __func__ );
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
		g_debug ( "%s: link Failed ", __func__ );

	g_value_unset ( &item );
	gst_iterator_free ( it );
}

static gboolean dtv_gst_set_audio_track ( int set_track_audio, Base *base )
{
	gboolean audio_changed = TRUE;

	GstElement *element = dtv_gst_ret_iterate_element ( base->dtv->dvbplay, "tsdemux", NULL );

	if ( element == NULL ) return FALSE;

	GstElement *element_l = dtv_gst_ret_iterate_element ( base->dtv->dvbplay, "tee-audio", NULL );

	if ( element_l == NULL ) return FALSE;

	dtv_gst_change_audio_track ( element, element_l, set_track_audio );

	return audio_changed;
}

void dtv_gst_changed_audio_track ( Base *base, int changed_track_audio )
{
	dtv_gst_set_audio_track ( changed_track_audio, base );

	base->dtv->set_audio_track = changed_track_audio;
}

void dtv_gst_add_audio_track ( Base *base, GtkComboBoxText *combo )
{
	uint i = 0;

	for ( i = 0; i < MAX_AUDIO; i++ )
	{
		if ( base->dtv->audio_lang[i] )
			gtk_combo_box_text_append_text ( combo, base->dtv->audio_lang[i] );
	}

	gtk_combo_box_set_active ( GTK_COMBO_BOX ( combo ), base->dtv->set_audio_track );
}

static void dtv_gst_pad_link ( GstPad *pad, GstElement *element, const char *name, G_GNUC_UNUSED GstElement *element_n )
{
	GstPad *pad_va_sink = gst_element_get_static_pad ( element, "sink" );

	if ( gst_pad_link ( pad, pad_va_sink ) == GST_PAD_LINK_OK )
		gst_object_unref ( pad_va_sink );
	else
		g_debug ( "%s:: linking demux/decode name %s video/audio pad failed ", __func__, name );
}

static GstElementFactory * dtv_gst_find_factory ( GstCaps *caps, int e_num )
{
	GList *list, *list_filter;

	static GMutex mutex;

	g_mutex_lock ( &mutex );
		list = gst_element_factory_list_get_elements ( e_num, GST_RANK_MARGINAL );
		list_filter = gst_element_factory_list_filter ( list, caps, GST_PAD_SINK, gst_caps_is_fixed ( caps ) );
	g_mutex_unlock ( &mutex );

	GstElementFactory *factory = GST_ELEMENT_FACTORY_CAST ( list_filter->data );

	const char *metadata = gst_element_factory_get_metadata ( factory, GST_ELEMENT_METADATA_KLASS ); // "long-name"
	g_debug ( "%s:: metadata: %s | g_list_length: %d ",__func__, metadata, g_list_length ( list_filter ) );

	gst_plugin_feature_list_free ( list_filter );
	gst_plugin_feature_list_free ( list );

	return factory;
}

static gboolean dtv_gst_pad_check_type ( GstPad *pad, const char *type )
{
	gboolean ret = FALSE;

	GstCaps *caps = gst_pad_get_current_caps ( pad );

	char *str = gst_caps_to_string ( caps );

	if ( g_str_has_prefix ( str, type ) ) ret = TRUE;

	if ( ret )
	{
		GstElementFactory *factory = dtv_gst_find_factory ( caps, GST_ELEMENT_FACTORY_TYPE_PARSER );

		const char *plugin_name = gst_plugin_feature_get_name ( GST_PLUGIN_FEATURE (factory) );

		g_debug ( "%s: caps %s | plugin_name %s ",__func__, str, plugin_name );
	}

	g_free ( str );

	return ret;
}

static gboolean dtv_gst_typefind_remove ( const char *name, Base *base )
{
	GstElement *element = dtv_gst_ret_iterate_element ( base->dtv->dvbplay, name, NULL );

	if ( element )
	{
		gst_element_set_state ( element, GST_STATE_NULL );
		gst_bin_remove ( GST_BIN ( base->dtv->dvbplay ), element );

		return TRUE;
	}

	return FALSE;
}

static void dtv_gst_typefind_parser ( GstElement *typefind, uint probability, GstCaps *caps, Base *base )
{
	const char *name_caps = gst_structure_get_name ( gst_caps_get_structure ( caps, 0 ) );

	GstElementFactory *factory = dtv_gst_find_factory ( caps, GST_ELEMENT_FACTORY_TYPE_PARSER );

	GstElement *element = gst_element_factory_create ( factory, NULL );

	gboolean remove = FALSE;

	if ( g_str_has_prefix ( name_caps, "audio" ) )
	{
		remove = dtv_gst_typefind_remove ( "parser-audio-rec", base );
		gst_element_set_name ( element, "parser-audio-rec" );
	}

	if ( g_str_has_prefix ( name_caps, "video" ) )
	{
		remove = dtv_gst_typefind_remove ( "parser-video-rec", base );
		gst_element_set_name ( element, "parser-video-rec" );
	}

	if ( remove == FALSE ) gst_element_unlink ( typefind, base->dtv->mux_rec );

	gst_bin_add ( GST_BIN ( base->dtv->dvbplay ), element );

	gst_element_link ( typefind, element );
	gst_element_link ( element, base->dtv->mux_rec );

	gst_element_set_state ( element, GST_STATE_PLAYING );

	g_debug ( "%s: probability %d%% | name_caps %s ",__func__, probability, name_caps );
}

static void dtv_gst_pad_demux_audio ( GstElement *element, GstPad *pad, GstElement *element_audio )
{
	if ( dtv_gst_pad_check_type ( pad, "audio" ) ) dtv_gst_pad_link ( pad, element_audio, "audio", element );
}

static void dtv_gst_pad_demux_video ( GstElement *element, GstPad *pad, GstElement *element_video )
{
	if ( dtv_gst_pad_check_type ( pad, "video" ) ) dtv_gst_pad_link ( pad, element_video, "video", element );
}

static void dtv_gst_pad_decode ( GstElement *element, GstPad *pad, GstElement *element_va )
{
	const char *name = gst_structure_get_name ( gst_caps_get_structure ( gst_pad_query_caps ( pad, NULL ), 0 ) );

	dtv_gst_pad_link ( pad, element_va, name, element );
}

static void dtv_gst_create_dvb_bin ( GstElement *element, gboolean video_enable, Base *base )
{
	struct dvb_all_list { const char *name; const char *u_name; } dvb_all_list_n[] =
	{
		{ "dvbsrc", NULL }, { "tsdemux", NULL },
			{ "tee", "tee-audio" }, { "queue2", NULL }, { "decodebin", "dec-audio" }, 
				{ "audioconvert", NULL }, { "equalizer-nbands", NULL }, { "autoaudiosink", NULL },
			{ "tee", "tee-video" }, { "queue2", NULL }, { "decodebin", "dec-video" }, 
				{ "videoconvert", NULL }, { "videobalance",     NULL }, { "autovideosink", NULL }
	};

	GstElement *elements[ G_N_ELEMENTS ( dvb_all_list_n ) ];

	uint c = 0;
	for ( c = 0; c < G_N_ELEMENTS ( dvb_all_list_n ); c++ )
	{
		if ( !video_enable && c > 7 ) continue;

		elements[c] = gst_element_factory_make ( dvb_all_list_n[c].name, dvb_all_list_n[c].u_name );

		if ( !elements[c] )
			g_critical ( "%s:: element (factory make) - %s not created. \n", __func__, dvb_all_list_n[c].name );

		gst_bin_add ( GST_BIN ( element ), elements[c] );

		if (  c == 0 || c == 2 || c == 5 || c == 8 || c == 11 ) continue;

		gst_element_link ( elements[c-1], elements[c] );
	}

	g_signal_connect ( elements[1], "pad-added", G_CALLBACK ( dtv_gst_pad_demux_audio ), elements[2] );
	if ( video_enable ) g_signal_connect ( elements[1], "pad-added", G_CALLBACK ( dtv_gst_pad_demux_video ), elements[8] );

	g_signal_connect ( elements[4], "pad-added", G_CALLBACK ( dtv_gst_pad_decode ), elements[5] );
	if ( video_enable ) g_signal_connect ( elements[10], "pad-added", G_CALLBACK ( dtv_gst_pad_decode ), elements[11] );

	// record

	struct dvb_all_rec { const char *name; const char *u_name; } dvb_all_rec_n[] =
	{
		{ "queue2",    NULL }, { "typefind", NULL },
		{ "queue2",    NULL }, { "typefind", NULL },
		{ "mpegtsmux", NULL }, { "filesink", NULL }
	};

	GstElement *elements_rec[ G_N_ELEMENTS ( dvb_all_rec_n ) ];

	for ( c = 0; c < G_N_ELEMENTS ( dvb_all_rec_n ); c++ )
	{
		if ( !video_enable && ( c == 2 || c == 3 ) ) continue;

		elements_rec[c] = gst_element_factory_make ( dvb_all_rec_n[c].name, dvb_all_rec_n[c].u_name );

		if ( !elements_rec[c] )
			g_critical ( "%s:: element (factory make) - %s not created. \n", __func__, dvb_all_rec_n[c].name );

		gst_bin_add ( GST_BIN ( element ), elements_rec[c] );
	}

	base->dtv->mux_rec = elements_rec[4];
	g_object_set ( elements_rec[5], "location", "/dev/null", NULL );

	gst_element_link ( elements[2], elements_rec[0] );
	gst_element_link ( elements_rec[0], elements_rec[1] );
	gst_element_link ( elements_rec[1], elements_rec[4] );

	if ( video_enable )
	{
		gst_element_link ( elements[8], elements_rec[2] );
		gst_element_link ( elements_rec[2], elements_rec[3] );
		gst_element_link ( elements_rec[3], elements_rec[4] );
	}

	gst_element_link ( elements_rec[4], elements_rec[5] );

	g_signal_connect ( elements_rec[1], "have-type", G_CALLBACK ( dtv_gst_typefind_parser ), base );
	if ( video_enable ) g_signal_connect ( elements_rec[3], "have-type", G_CALLBACK ( dtv_gst_typefind_parser ), base );
}

static void dtv_gst_remove_dvb_bin ( GstElement *pipeline )
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

				g_debug ( "%s: Object remove: %s", __func__, object_name );

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
static char * dtv_time_to_str ()
{
	GDateTime *date = g_date_time_new_now_local ();

	char *str_time = g_date_time_format ( date, "%j-%Y-%T" );

	g_date_time_unref ( date );

	return str_time;
}

static char * dtv_str_split ( const char *data, const char *delm, uint num )
{
	char *ret_ch = NULL;

	char **lines = g_strsplit ( data, delm, 0 );

		ret_ch = g_strdup ( lines[num] );

	g_strfreev ( lines );

	return ret_ch;
}

static void dtv_rec_set_location ( GstElement *element, char *rec_dir, char *ch_data )
{
	char *date_str = dtv_time_to_str ();
	char *name     = dtv_str_split ( ch_data, ":", 0 );
	char *file_rec = g_strdup_printf ( "%s/%s_%s.%s", rec_dir, name, date_str, "m2ts" );

	g_object_set ( element, "location", file_rec, NULL );

	g_free ( file_rec );
	g_free ( name     );
	g_free ( date_str );
}

static GstPadProbeReturn dtv_gst_blockpad_probe ( GstPad * pad, GstPadProbeInfo * info, gpointer data )
{
	Base *base = (Base *) data;

	GstElement *element = dtv_gst_ret_iterate_element ( base->dtv->dvbplay, "filesink", NULL );

	if ( element == NULL ) return GST_PAD_PROBE_PASS;
	
	char *file_name = NULL;
	g_object_get ( element, "location", &file_name, NULL );

	gst_element_set_state ( element, GST_STATE_NULL );

	if ( g_str_has_prefix ( file_name, "/dev/null" ) )
	{
		dtv_rec_set_location ( element, base->rec_dir, base->dtv->ch_data );

		base->dtv->rec_ses = TRUE;
	}
	else
	{
		g_object_set ( element, "location", "/dev/null", NULL );

		base->dtv->rec_ses = FALSE;
	}

	gst_pad_remove_probe ( pad, GST_PAD_PROBE_INFO_ID (info) );

	gst_element_set_state ( element, GST_STATE_PLAYING );

	g_free ( file_name );

	return GST_PAD_PROBE_OK;
}

static gboolean dtv_gst_checked_video ( const char *data )
{
	gboolean video_enable = TRUE;

	if ( !g_strrstr ( data, "video-pid" ) || g_strrstr ( data, "video-pid=0" ) ) video_enable = FALSE;

	return video_enable;
}

static gboolean dtv_gst_find_property_scrambling ( GstElement *element )
{
	gboolean scrambling = FALSE;

	if ( element && g_object_class_find_property ( G_OBJECT_GET_CLASS ( element ), "scrambling" ) )
		scrambling = TRUE;
	else
		scrambling = FALSE;

	return scrambling;
}
static gboolean dtv_gst_get_property_scrambling ( Base *base )
{
	GstElement *element = dtv_gst_ret_iterate_element ( base->dtv->dvbplay, "tsdemux", NULL );

	if ( element == NULL ) return FALSE;

	gboolean scrambling = FALSE;
	gboolean property_scrambling = dtv_gst_find_property_scrambling ( element );

	if ( property_scrambling ) g_object_get ( element, "scrambling", &scrambling, NULL );

	return scrambling;
}
static void dtv_gst_scrambling ( GstElement *element, char *name )
{
	gboolean scrambling = dtv_gst_find_property_scrambling ( element );

	g_debug ( "%s:: GstTSDemux property scrambling: %s ", __func__, scrambling ? "TRUE": "FALSE" );

	if ( scrambling ) g_object_set ( element, "prog-name", name, NULL );
}

static void dtv_gst_set_tuning_timeout ( GstElement *element )
{
	guint64 timeout = 0;
	g_object_get ( element, "tuning-timeout", &timeout, NULL );
	g_object_set ( element, "tuning-timeout", (guint64)timeout / 5, NULL );
}

static void dtv_gst_tv_delsys ( GstElement *element, Base *base )
{
	uint adapter = 0, frontend = 0, delsys = 0;
	g_object_get ( element, "adapter",  &adapter,  NULL );
	g_object_get ( element, "frontend", &frontend, NULL );
	g_object_get ( element, "delsys",   &delsys,   NULL );

	char *dvb_name = helia_get_dvb_info ( base, adapter, frontend );
	const char *dvb_type = helia_get_dvb_type_str ( delsys );

	g_debug ( "DVB device: %s ( %s ) | adapter %d frontend %d \n", dvb_name, dvb_type, adapter, frontend );

	g_free ( dvb_name );
}

static void dtv_gst_data_set ( GstElement *pipeline, const char *data, Base *base )
{
	GstElement *element = dtv_gst_ret_iterate_element ( pipeline, "dvbsrc", NULL );
	dtv_gst_set_tuning_timeout ( element );

	char **fields = g_strsplit ( data, ":", 0 );
	uint numfields = g_strv_length ( fields );

	uint j = 0;
	for ( j = 1; j < numfields; j++ )
	{
		if ( g_strrstr ( fields[j], "audio-pid" ) || g_strrstr ( fields[j], "video-pid" ) ) continue;

		if ( !g_strrstr ( fields[j], "=" ) ) continue;

		char **splits = g_strsplit ( fields[j], "=", 0 );

		g_debug ( "%s: gst-param %s | gst-value %s", __func__, splits[0], splits[1] );

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
			base->dtv->sid = dat;

			GstElement *demux = dtv_gst_ret_iterate_element ( pipeline, "tsdemux", NULL );

			g_object_set ( demux, "program-number", base->dtv->sid, NULL );

			dtv_gst_scrambling ( demux, fields[0] );
		}
		else if ( g_strrstr ( splits[0], "symbol-rate" ) )
		{
			g_object_set ( element, "symbol-rate", ( dat > 100000) ? dat/1000 : dat, NULL );
		}
		else if ( g_strrstr ( splits[0], "lnb-type" ) )
		{
			set_lnb_low_high_switch ( element, dat );
		}
		else
		{
			g_object_set ( element, splits[0], dat, NULL );
		}

		g_strfreev (splits);
	}

	g_strfreev (fields);

	dtv_gst_tv_delsys ( element, base );
}

static uint dtv_gst_data_set_sid ( const char *data, GstElement *demux )
{
	uint sid = 0;

	char **fields = g_strsplit ( data, ":", 0 );
	uint numfields = g_strv_length ( fields );

	uint j = 0;
	for ( j = 1; j < numfields; j++ )
	{
		if ( !g_strrstr ( fields[j], "=" ) ) continue;

		char **splits = g_strsplit ( fields[j], "=", 0 );

		if ( g_strrstr ( splits[0], "program-number" ) )
		{
			sid = atol ( splits[1] );

			g_object_set ( demux, "program-number", sid, NULL );

			dtv_gst_scrambling ( demux, fields[0] );

			g_strfreev (splits);
			g_strfreev (fields);

			return sid;
		}

		g_strfreev (splits);
	}

	g_strfreev (fields);

	return sid;
}

void dtv_stop ( Base *base )
{
	if ( GST_ELEMENT_CAST ( base->dtv->dvbplay )->current_state != GST_STATE_NULL )
	{
		gst_element_set_state ( base->dtv->dvbplay, GST_STATE_NULL );

		dtv_gst_remove_dvb_bin ( base->dtv->dvbplay );

		base->dtv->rec_ses    = FALSE;
		base->dtv->scrambling = FALSE;

		base->dtv->set_audio_track = 0;

		uint i; for ( i = 0; i < MAX_AUDIO; i++ )
			{ if ( base->dtv->audio_lang[i] ) g_free ( base->dtv->audio_lang[i] ); base->dtv->audio_lang[i] = NULL; }

		dtv_level_set_sgn_snr ( base, base->dtv->level_base, 0, 0, FALSE );

		if ( !base->dtv->panel_quit && gtk_widget_get_visible ( GTK_WIDGET ( base->dtv->h_box_level_panel ) ) )
		{
			dtv_level_set_sgn_snr ( base, base->dtv->level_panel, 0, 0, FALSE );
		}

		gtk_widget_queue_draw ( GTK_WIDGET ( base->window ) );
	}
}

static gboolean dtv_update_win ( Base *base )
{
	if ( base->app_quit ) return FALSE;

	if ( GST_ELEMENT_CAST ( base->dtv->dvbplay )->current_state == GST_STATE_NULL ) return FALSE;

	if ( GST_ELEMENT_CAST ( base->dtv->dvbplay )->current_state == GST_STATE_PLAYING )
	{
		base->dtv->scrambling = dtv_gst_get_property_scrambling ( base );

		dtv_gst_volume_set ( base->dtv->dvbplay, base->dtv->volume );

		gtk_widget_queue_draw ( GTK_WIDGET ( base->window ) );

		return FALSE;
	}
	else
	{
		base->dtv->scrambling = dtv_gst_get_property_scrambling ( base );

		time ( &base->dtv->t_cur_tv );

		if ( ( base->dtv->t_cur_tv - base->dtv->t_start_tv ) >= 10 )
		{
			g_warning ( "%s: Time stop %ld (sec) ", __func__, base->dtv->t_cur_tv - base->dtv->t_start_tv );

			dtv_stop ( base );

			return FALSE;
		}
	}

	return TRUE;
}

static void dtv_play ( Base *base, const char *data )
{
	if ( GST_ELEMENT_CAST ( base->dtv->dvbplay )->current_state != GST_STATE_PLAYING )
	{
		base->dtv->checked_video = dtv_gst_checked_video ( data );

		dtv_gst_create_dvb_bin ( base->dtv->dvbplay, base->dtv->checked_video, base );

		dtv_gst_data_set ( base->dtv->dvbplay, data, base );

		gst_element_set_state ( base->dtv->dvbplay, GST_STATE_PLAYING );

		g_timeout_add ( 250, (GSourceFunc)dtv_update_win, base );

		time ( &base->dtv->t_start_tv );
	}
}

static gboolean dtv_check_tp ( Base *base, const char *data )
{
	gboolean tp_id = FALSE;

	char *tp_a = dtv_str_split ( data, "delsys", 1 );
	char *tp_b = dtv_str_split ( base->dtv->ch_data, "delsys", 1 );

		tp_id = g_str_has_suffix ( tp_a, tp_b );

	free ( tp_a );
	free ( tp_b );

	return tp_id;
}

void dtv_stop_set_play ( Base *base, const char *data )
{
	gboolean tp_id = FALSE;

	GstElement *tsdemux = dtv_gst_ret_iterate_element ( base->dtv->dvbplay, "tsdemux", NULL );

	if ( tsdemux ) tp_id = dtv_check_tp ( base, data );

	gboolean checked_video = dtv_gst_checked_video ( data );

	if ( base->dtv->checked_video != checked_video ) tp_id = FALSE;

	if ( base->dtv->ch_data ) g_free ( base->dtv->ch_data );
	base->dtv->ch_data = g_strdup ( data );

	if ( tp_id && !base->dtv->rec_ses )
	{
		base->dtv->set_audio_track = 0;

		uint i; for ( i = 0; i < MAX_AUDIO; i++ )
			{ if ( base->dtv->audio_lang[i] ) g_free ( base->dtv->audio_lang[i] ); base->dtv->audio_lang[i] = NULL; }

		gst_element_set_state ( base->dtv->dvbplay, GST_STATE_READY );

		base->dtv->sid = dtv_gst_data_set_sid ( data, tsdemux );

		gst_element_set_state ( base->dtv->dvbplay, GST_STATE_PLAYING );

		g_timeout_add ( 250, (GSourceFunc)dtv_update_win, base );

		time ( &base->dtv->t_start_tv );
	}
	else
	{
		g_debug ( "%s: new transponder ", __func__ );

		dtv_stop ( base );
		dtv_play ( base, data );
	}
}

void dtv_gst_record ( Base *base )
{
	if ( GST_ELEMENT_CAST ( base->dtv->dvbplay )->current_state != GST_STATE_PLAYING ) return;

	GstElement *mpegtsmux = dtv_gst_ret_iterate_element ( base->dtv->dvbplay, "mpegtsmux", NULL );

	if ( mpegtsmux == NULL ) return;

	GstPad *blockpad = gst_element_get_static_pad ( mpegtsmux, "src" );

	gst_pad_add_probe ( blockpad, GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM,
						dtv_gst_blockpad_probe, base, NULL );

	gst_object_unref ( blockpad );
}

static GstBusSyncReply dtv_gst_bus_sync_handler ( G_GNUC_UNUSED GstBus *bus, GstMessage *message, Base *base )
{
    if ( !gst_is_video_overlay_prepare_window_handle_message ( message ) ) return GST_BUS_PASS;

    if ( base->dtv->window_hid != 0 )
    {
        GstVideoOverlay *xoverlay = GST_VIDEO_OVERLAY ( GST_MESSAGE_SRC ( message ) );
        gst_video_overlay_set_window_handle ( xoverlay, base->dtv->window_hid );

    } else { g_warning ( "Should have obtained window_handle by now!" ); }

    gst_message_unref ( message );

    return GST_BUS_DROP;
}

static void dtv_gst_msg_err ( G_GNUC_UNUSED GstBus *bus, GstMessage *msg, Base *base )
{
    GError *err = NULL;
    char  *dbg = NULL;

    gst_message_parse_error ( msg, &err, &dbg );

    g_printerr ( "%s: %s (%s)\n", __func__, err->message, (dbg) ? dbg : "no details" );

    base_message_dialog ( "", err->message, GTK_MESSAGE_ERROR, base->window );

    g_error_free ( err );
    g_free ( dbg );

	dtv_stop ( base );
}

static void dtv_gst_msg_all ( G_GNUC_UNUSED GstBus *bus, GstMessage *msg, Base *base )
{
	if ( base->app_quit ) return;

	const GstStructure *structure = gst_message_get_structure ( msg );

	if ( structure )
	{
		int signal, snr;
		gboolean hlook = FALSE;

		if (  gst_structure_get_int ( structure, "signal", &signal )  )
		{
			gst_structure_get_int     ( structure, "snr",  &snr   );
			gst_structure_get_boolean ( structure, "lock", &hlook );

			dtv_level_set_sgn_snr ( base, base->dtv->level_base, (signal * 100) / 0xffff, (snr * 100) / 0xffff, hlook );

			if ( !base->dtv->panel_quit && gtk_widget_get_visible ( GTK_WIDGET ( base->dtv->h_box_level_panel ) ) )
			{
				dtv_level_set_sgn_snr ( base, base->dtv->level_panel, (signal * 100) / 0xffff, (snr * 100) / 0xffff, hlook );
			}

			base->dtv->rec_pulse = !base->dtv->rec_pulse;

			if ( base->dtv->rec_pulse ) base->dtv->rec_status = !base->dtv->rec_status;
		}
	}

	mpegts_pmt_lang_section ( msg, base );
}

GstElement * dtv_gst_create ( Base *base )
{
    GstElement *dvbplay = gst_pipeline_new ( "pipeline" );

    if ( !dvbplay )
    {
        g_printerr ( "%s: dvbplay - not created.\n", __func__ );

        return NULL;
    }

	base->dtv->set_audio_track = 0;

	uint i; for ( i = 0; i < MAX_AUDIO; i++ ) base->dtv->audio_lang[i] = NULL;

    GstBus *bus = gst_element_get_bus ( dvbplay );

    gst_bus_add_signal_watch_full ( bus, G_PRIORITY_DEFAULT );
    gst_bus_set_sync_handler ( bus, (GstBusSyncHandler)dtv_gst_bus_sync_handler, base, NULL );

	g_signal_connect ( bus, "message",        G_CALLBACK ( dtv_gst_msg_all ), base );
    g_signal_connect ( bus, "message::error", G_CALLBACK ( dtv_gst_msg_err ), base );

	gst_object_unref (bus);

    return dvbplay;
}
