/*
* Copyright 2019 Stepan Perun
* This program is free software.
*
* License: Gnu Lesser General Public License
* http://www.gnu.org/licenses/lgpl.html
*/


#include <helia-include.h>

#include <linux/dvb/frontend.h>


static void helia_info_combo_lang_changed ( GtkComboBox *widget, Helia *helia )
{
	helia_dtv_changed_audio_track ( helia, gtk_combo_box_get_active ( GTK_COMBO_BOX ( widget ) ) );
}

static GtkBox * helia_info_tv ( Helia *helia )
{
	GtkBox *v_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 );
	gtk_box_set_spacing ( v_box, 5 );
	gtk_widget_set_margin_top   ( GTK_WIDGET ( v_box ), 10 );
	gtk_widget_set_margin_start ( GTK_WIDGET ( v_box ), 10 );
	gtk_widget_set_margin_end   ( GTK_WIDGET ( v_box ), 10 );

	GtkGrid *grid = (GtkGrid *)gtk_grid_new();
	gtk_grid_set_row_homogeneous    ( GTK_GRID ( grid ) ,TRUE );
	gtk_grid_set_column_homogeneous ( GTK_GRID ( grid ) ,TRUE );


	uint j = 0, adapter = 0, frontend = 0, delsys = 0;

	GstElement *element = helia_gst_iterate_element ( helia->dtv, "dvbsrc", NULL );

	g_object_get ( element, "adapter",  &adapter,  NULL );
	g_object_get ( element, "frontend", &frontend, NULL );
	g_object_get ( element, "delsys",   &delsys,   NULL );

	char *dvb_name = helia_get_dvb_info ( adapter, frontend );

	gtk_box_pack_start ( v_box, GTK_WIDGET ( gtk_label_new ( dvb_name ) ), FALSE, FALSE, 0 );

	g_free ( dvb_name );


	char **fields = g_strsplit ( helia->file_ch, ":", 0 );
	uint numfields = g_strv_length ( fields );

	GtkEntry *entry_ch = (GtkEntry *) gtk_entry_new ();
	g_object_set ( entry_ch, "editable", FALSE, NULL );
	gtk_entry_set_text ( GTK_ENTRY ( entry_ch ), fields[0] );

	gtk_box_pack_start ( v_box, GTK_WIDGET ( entry_ch ), FALSE, FALSE, 0 );

	GtkComboBoxText *combo_lang = (GtkComboBoxText *)gtk_combo_box_text_new ();

	helia_dtv_add_audio_track ( helia, combo_lang );

	g_signal_connect ( G_OBJECT (combo_lang), "changed", G_CALLBACK ( helia_info_combo_lang_changed ), helia );

	gtk_box_pack_start ( v_box, GTK_WIDGET ( combo_lang ), FALSE, FALSE, 0 );

	for ( j = 1; j < numfields; j++ )
	{
		if ( g_strrstr ( fields[j], "delsys" ) || g_strrstr ( fields[j], "adapter" ) || g_strrstr ( fields[j], "frontend" ) ) continue;

		if ( !g_strrstr ( fields[j], "=" ) ) continue;

		char **splits = g_strsplit ( fields[j], "=", 0 );

		const char *set = helia_scan_get_info ( splits[0] );

		if ( g_strrstr ( splits[0], "code-rate-hp" ) )
		{
			if ( delsys != SYS_DVBT || delsys != SYS_DVBT2 ) set = "Inner Fec";
		}

		if ( g_str_has_prefix ( splits[0], "lnb-lof1" ) ) set = "   LO1  MHz";
		if ( g_str_has_prefix ( splits[0], "lnb-lof2" ) ) set = "   LO2  MHz";
		if ( g_str_has_prefix ( splits[0], "lnb-slof" ) ) set = "   Switch  MHz";

		GtkLabel *label = (GtkLabel *)gtk_label_new ( set );
		gtk_widget_set_halign ( GTK_WIDGET ( label ), GTK_ALIGN_START );

		gtk_grid_attach ( GTK_GRID ( grid ), GTK_WIDGET ( label ), 0, j+1, 1, 1 );

		const char *set_v = helia_scan_get_info_descr_vis ( splits[0], atoi ( splits[1] ) );

		if ( g_strrstr ( splits[0], "polarity" ) ) set_v = splits[1];

		if ( g_str_has_prefix ( splits[0], "frequency" ) || g_str_has_prefix ( splits[0], "lnb-lo" ) || g_str_has_prefix ( splits[0], "lnb-sl" ) )
		{
			long dat = atol ( splits[1] );

			if ( delsys == SYS_DVBS || delsys == SYS_TURBO || delsys == SYS_DVBS2 )
				dat = dat / 1000;
			else
				dat = dat / 1000000;

			char *str = g_strdup_printf ( "%ld", dat );

				label = (GtkLabel *)gtk_label_new ( str );

			free ( str );
		}
		else
			label = (GtkLabel *)gtk_label_new ( ( set_v ) ? set_v : splits[1] );

		gtk_widget_set_halign ( GTK_WIDGET ( label ), GTK_ALIGN_START );

		gtk_grid_attach ( GTK_GRID ( grid ), GTK_WIDGET ( label ), 1, j+1, 1, 1 );

		g_strfreev (splits);
	}

	g_strfreev (fields);

	gtk_box_pack_start ( v_box, GTK_WIDGET ( grid ), TRUE, TRUE, 10 );

	return v_box;
}



static void helia_info_changed_combo_video ( GtkComboBox *combo, HeliaPlayer *player )
{
	g_object_set ( player, "current-video", gtk_combo_box_get_active (combo), NULL );
}
static void helia_info_changed_combo_audio ( GtkComboBox *combo, HeliaPlayer *player )
{
	g_object_set ( player, "current-audio", gtk_combo_box_get_active (combo), NULL );
}
static void helia_info_changed_combo_text ( GtkComboBox *combo, HeliaPlayer *player )
{
	g_object_set ( player, "current-text", gtk_combo_box_get_active (combo), NULL );
}
static void helia_info_change_state_subtitle ( G_GNUC_UNUSED GtkButton *button, Helia *helia )
{
	helia->state_subtitle = !helia->state_subtitle;

	helia_player_set_subtitle ( helia, helia->state_subtitle );
}
static void helia_info_change_hide_show_subtitle ( GtkButton *button, GtkComboBoxText *combo )
{
	gboolean sensitive = !gtk_widget_get_sensitive ( GTK_WIDGET ( combo ) );

	gtk_widget_set_sensitive ( GTK_WIDGET ( combo ), sensitive );

	GtkImage *image = helia_create_image ( ( sensitive ) ? "helia-set" : "helia-unset", 16 );
	gtk_button_set_image ( button, GTK_WIDGET ( image ) );
}

static char * helia_info_get_str_vat ( HeliaPlayer *player, char *get_tag, int n_cur, const char *metadata, const char *metadata_2 )
{
	char *str = NULL, *str_2 = NULL, *ret_str = NULL;

	GstTagList *tags;

	g_signal_emit_by_name ( player, get_tag, n_cur, &tags );

	if ( tags )
	{
		if ( gst_tag_list_get_string ( tags, metadata, &str ) )
		{
			if ( g_strrstr ( str, " (" ) )
			{
				char **lines = g_strsplit ( str, " (", 0 );

					g_free ( str );
					str = g_strdup ( lines[0] );

				g_strfreev ( lines );
			}

			if ( metadata_2 && gst_tag_list_get_string ( tags, metadata_2, &str_2 ) )
				{ ret_str = g_strdup_printf ( "%s   %s", str_2, str ); g_free ( str_2 ); }
			else
				ret_str = ( g_str_has_prefix ( get_tag, "get-audio" ) ) ? g_strdup_printf ( "%d   %s", n_cur + 1, str ) : g_strdup_printf ( "%s", str );

			g_free ( str );
		}
	}

	if ( ret_str == NULL ) ret_str = g_strdup_printf ( "№ %d", n_cur + 1 );

	return ret_str;
}

static char * helia_info_get_int_vat ( GstElement *element, char *get_tag, int n_cur, char *metadata )
{
	char *ret_str = NULL;
	uint rate;

	GstTagList *tags;

	g_signal_emit_by_name ( element, get_tag, n_cur, &tags );

	if ( tags )
	{
		if ( gst_tag_list_get_uint ( tags, metadata, &rate ) ) 
			ret_str = g_strdup_printf ( "%d Kbits/s", rate / 1000 );
	}

	return ret_str;
}

static const char * helia_info_get_str_tag ( HeliaPlayer *player, char *get_tag, char *data )
{
	const char *name = NULL;

	GstTagList *tags;

	g_signal_emit_by_name ( player, get_tag, 0, &tags );

	if ( tags )
	{
		const GValue *value = gst_tag_list_get_value_index ( tags, data, 0 );

		if ( value && G_VALUE_HOLDS_STRING (value) )
		{
			name = g_value_get_string ( value );
		}
	}

	return name;
}

static char * helia_info_get_title_artist ( HeliaPlayer *player )
{
	char *title_new = NULL;

	const char *title  = helia_info_get_str_tag ( player, "get-audio-tags", GST_TAG_TITLE  );
	const char *artist = helia_info_get_str_tag ( player, "get-audio-tags", GST_TAG_ARTIST );

	if ( title && artist )
	{
		title_new = g_strconcat ( title, " -- ", artist, NULL );
	}
	else
	{
		if ( title )
		{
			title_new = g_strdup ( title );
		}
		else
		{
			if ( artist )
				title_new = g_strdup ( artist );
		}
	}

	return title_new;
}

static gboolean helia_info_update_bitrate_video ( Helia *helia )
{
	if ( helia->label_audio == NULL ) return FALSE;

	uint c_video;
	g_object_get ( helia->player, "current-video", &c_video, NULL );

	char *bitrate_video = helia_info_get_int_vat ( helia->player, "get-video-tags", c_video, GST_TAG_BITRATE );

	gtk_label_set_text ( helia->label_video, ( bitrate_video ) ? bitrate_video : "? Kbits/s" );

	if ( bitrate_video ) g_free ( bitrate_video );

	return TRUE;
}

static void helia_info_bitrate_video ( Helia *helia, GtkLabel *label )
{
	helia->label_video = label;

	g_timeout_add ( 250, (GSourceFunc)helia_info_update_bitrate_video, helia );
}

static gboolean helia_info_update_bitrate_audio ( Helia *helia )
{
	if ( helia->label_audio == NULL ) return FALSE;

	uint c_audio;
	g_object_get ( helia->player, "current-audio", &c_audio, NULL );

	char *bitrate_audio = helia_info_get_int_vat ( helia->player, "get-audio-tags", c_audio, GST_TAG_BITRATE );

	gtk_label_set_text ( helia->label_audio, ( bitrate_audio ) ? bitrate_audio : "? Kbits/s" );

	if ( bitrate_audio ) g_free ( bitrate_audio );

	return TRUE;
}

static void helia_info_bitrate_audio ( Helia *helia, GtkLabel *label )
{
	helia->label_audio = label;

	g_timeout_add ( 250, (GSourceFunc)helia_info_update_bitrate_audio, helia );
}

static void helia_info_title_save ( Helia *helia, const char *title )
{
	gboolean valid;
	GtkTreeIter iter;

	GtkTreeModel *model_filter = gtk_tree_view_get_model ( GTK_TREE_VIEW ( helia->treeview_mp ) );
	GtkTreeModel *model = gtk_tree_model_filter_get_model ( GTK_TREE_MODEL_FILTER ( model_filter ) );

	if ( gtk_tree_model_iter_n_children ( model, NULL ) == 0 ) return;

	for ( valid = gtk_tree_model_get_iter_first ( model, &iter ); valid;
		valid = gtk_tree_model_iter_next ( model, &iter ) )
	{
		char *data;

		gtk_tree_model_get ( model, &iter, COL_DATA,  &data, -1 );

			if ( g_str_has_suffix ( helia->file_ch, data ) )
			{
				gtk_list_store_set ( GTK_LIST_STORE ( model ), &iter, COL_FL_CH, title, -1 );

				break;
			}

		g_free ( data );
	}
}

static void helia_info_entry_title_save ( GtkEntry *entry, GtkEntryIconPosition icon_pos, G_GNUC_UNUSED GdkEvent *event, Helia *helia )
{
	const char *title = gtk_entry_get_text ( entry );

	if ( icon_pos == GTK_ENTRY_ICON_SECONDARY )
		if ( title && gtk_entry_get_text_length ( entry ) > 0 ) helia_info_title_save ( helia, title );
}

static void helia_info_changed_sw_vis ( GObject *gobject, G_GNUC_UNUSED GParamSpec *pspec, Helia *helia )
{
	gboolean vis_plugin = gtk_switch_get_state ( GTK_SWITCH ( gobject ) );

	helia_player_set_visualizer ( helia, vis_plugin );
}

static GtkSwitch * helia_info_create_switch_vis ( Helia *helia )
{
	GstElement *element = helia_gst_iterate_element ( helia->player, "goom", NULL );

	GtkSwitch *gswitch = (GtkSwitch *)gtk_switch_new ();
	gtk_switch_set_state ( gswitch, ( element ) ? TRUE : FALSE );
	g_signal_connect ( gswitch, "notify::active", G_CALLBACK ( helia_info_changed_sw_vis ), helia );

	return gswitch;
}

static void helia_info_create_vis ( Helia *helia, GtkBox *h_box )
{
	GtkLabel *label = (GtkLabel *)gtk_label_new ( " GstGoom " );
	gtk_box_pack_start ( h_box, GTK_WIDGET ( label ), FALSE, FALSE, 0 );

	gtk_box_pack_end ( h_box, GTK_WIDGET ( helia_info_create_switch_vis ( helia ) ), TRUE, TRUE, 0 );
}

static GtkBox * helia_info_mp ( Helia *helia )
{
	HeliaPlayer *player = helia->player;

	/*if ( helia->pipeline_rec ) player = helia->pipeline_rec;*/

	GtkBox *v_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 );
	gtk_box_set_spacing ( v_box, 5 );
	gtk_widget_set_margin_top   ( GTK_WIDGET ( v_box ), 10 );
	gtk_widget_set_margin_start ( GTK_WIDGET ( v_box ), 10 );
	gtk_widget_set_margin_end   ( GTK_WIDGET ( v_box ), 10 );

	GtkGrid *grid = (GtkGrid *)gtk_grid_new();
	gtk_grid_set_row_homogeneous ( GTK_GRID ( grid ) ,TRUE );
	gtk_grid_set_column_homogeneous ( GTK_GRID ( grid ) ,TRUE );

	GtkBox *h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );

	char *title_new = helia_info_get_title_artist ( player );

	GtkEntry *entry_title = (GtkEntry *) gtk_entry_new ();
	gtk_entry_set_text ( GTK_ENTRY ( entry_title ), ( title_new ) ? title_new : "None" );

	if ( title_new ) g_free ( title_new );

	gtk_entry_set_icon_from_icon_name ( entry_title, GTK_ENTRY_ICON_SECONDARY, "helia-save" );
	g_signal_connect ( entry_title, "icon-press", G_CALLBACK ( helia_info_entry_title_save ), helia );

	gtk_box_pack_start ( h_box, GTK_WIDGET ( entry_title ), TRUE, TRUE, 0 );
	gtk_box_pack_start ( v_box, GTK_WIDGET ( h_box ), FALSE, FALSE, 0 );

	h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );

	// char *name = g_path_get_basename ( helia->file_ch );

	GtkEntry *entry_fl = (GtkEntry *) gtk_entry_new ();
	g_object_set ( entry_fl, "editable", FALSE, NULL );
	gtk_entry_set_text ( GTK_ENTRY ( entry_fl ), helia->file_ch  );

	// free ( name );

	gtk_box_pack_start ( h_box, GTK_WIDGET ( entry_fl ), TRUE, TRUE, 0 );

	gtk_box_pack_start ( v_box, GTK_WIDGET ( h_box ), FALSE, FALSE, 0 );

	uint n_video, n_audio, n_text, c_video, c_audio, c_text;
	g_object_get ( player, "n-video", &n_video, NULL );
	g_object_get ( player, "n-audio", &n_audio, NULL );
	g_object_get ( player, "n-text",  &n_text,  NULL );

	g_object_get ( player, "current-video", &c_video, NULL );
	g_object_get ( player, "current-audio", &c_audio, NULL );
	g_object_get ( player, "current-text",  &c_text,  NULL );

	char *bitrate_video = helia_info_get_int_vat ( player, "get-video-tags", c_video, GST_TAG_BITRATE );
	char *bitrate_audio = helia_info_get_int_vat ( player, "get-audio-tags", c_audio, GST_TAG_BITRATE );

	struct data { const char *name; uint n_avt; uint c_avt; const char *info; void (*f)(); } data_n[] =
	{
		{ "helia-video", 	 n_video, c_video, NULL,   helia_info_changed_combo_video  },
		{ " ",   			 0,       0,       bitrate_video, helia_info_bitrate_video },
		{ "helia-audio", 	 n_audio, c_audio, NULL,   helia_info_changed_combo_audio  },
		{ " ",  			 0,       0,       bitrate_audio, helia_info_bitrate_audio },
		{ "helia-subtitles", n_text,  c_text,  NULL,   helia_info_changed_combo_text   }
	};

	uint c = 0, i = 0;
	for ( c = 0; c < G_N_ELEMENTS (data_n); c++ )
	{
		GtkImage *image = helia_create_image ( data_n[c].name, 48 );
		gtk_widget_set_halign ( GTK_WIDGET ( image ), GTK_ALIGN_START );
		gtk_grid_attach ( GTK_GRID ( grid ), GTK_WIDGET ( image ), 0, c, 1, 2 );

		if ( c == 0 || c == 2 || c == 4 )
		{
			GtkComboBoxText *combo = (GtkComboBoxText *)gtk_combo_box_text_new ();

			for ( i = 0; i < data_n[c].n_avt; i++ )
			{
				char *teg_info = NULL; // media metadata

				if ( c == 0 ) teg_info = helia_info_get_str_vat ( helia->player, "get-video-tags", i, GST_TAG_VIDEO_CODEC,   NULL );
				if ( c == 2 ) teg_info = helia_info_get_str_vat ( helia->player, "get-audio-tags", i, GST_TAG_AUDIO_CODEC,   GST_TAG_LANGUAGE_CODE );
				if ( c == 4 ) teg_info = helia_info_get_str_vat ( helia->player, "get-text-tags",  i, GST_TAG_LANGUAGE_CODE, NULL );

				gtk_combo_box_text_append_text ( combo, teg_info );

				g_free ( teg_info );
			}

			gtk_combo_box_set_active ( GTK_COMBO_BOX ( combo ), data_n[c].c_avt );

			if ( c == 1 || c == 3 )
				g_signal_connect ( combo, "changed", G_CALLBACK ( data_n[c].f ), helia );
			else
				g_signal_connect ( combo, "changed", G_CALLBACK ( data_n[c].f ), helia->player );

			if ( c == 4 && data_n[c].n_avt > 0 )
			{
				h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
				gtk_box_set_spacing ( h_box, 5 );

				GtkButton *button = helia_set_image_button ( ( helia->state_subtitle ) ? "helia-set" : "helia-unset", 16 );
				g_signal_connect ( button, "clicked", G_CALLBACK ( helia_info_change_state_subtitle     ), helia );
				g_signal_connect ( button, "clicked", G_CALLBACK ( helia_info_change_hide_show_subtitle ), combo );

				gtk_box_pack_start ( h_box, GTK_WIDGET ( combo  ), TRUE, TRUE, 0 );
				gtk_box_pack_start ( h_box, GTK_WIDGET ( button ), TRUE, TRUE, 0 );

				gtk_grid_attach ( GTK_GRID ( grid ), GTK_WIDGET ( h_box ), 1, c, 1, 1 );
				gtk_widget_set_sensitive ( GTK_WIDGET ( combo ), helia->state_subtitle );
			}
			else
				gtk_grid_attach ( GTK_GRID ( grid ), GTK_WIDGET ( combo ), 1, c, 1, 1 );
		}
		else
		{
			GtkLabel *label = (GtkLabel *)gtk_label_new ( ( data_n[c].info ) ? data_n[c].info : "? Kbits/s" );
			gtk_widget_set_halign ( GTK_WIDGET ( label ), GTK_ALIGN_START );
			gtk_grid_attach ( GTK_GRID ( grid ), GTK_WIDGET ( label ), 1, c, 1, 1 );

			data_n[c].f ( helia, label );
		}
	}

	if ( bitrate_video ) g_free ( bitrate_video );
	if ( bitrate_audio ) g_free ( bitrate_audio );

	gtk_box_pack_start ( v_box, GTK_WIDGET ( grid ), TRUE, TRUE, 10 );

	h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
	gtk_box_set_spacing ( h_box, 20 );

	helia_info_create_vis ( helia, h_box );
	gtk_box_pack_start ( v_box, GTK_WIDGET ( h_box ), FALSE, FALSE, 0 );

	if ( n_video ) gtk_widget_set_sensitive ( GTK_WIDGET ( h_box ), FALSE );

	gtk_box_pack_start ( v_box, GTK_WIDGET ( gtk_label_new ( " " ) ), FALSE, FALSE, 0 );

	return v_box;
}

static void helia_info_quit ( G_GNUC_UNUSED GtkWindow *win, Helia *helia )
{
	helia->label_audio = NULL;
	helia->label_video = NULL;
}

void helia_info_new ( Helia *helia, gboolean mp_tv )
{
	GtkWindow *window = helia_create_window_top ( helia->window, "", "helia-info", GTK_WIN_POS_CENTER_ON_PARENT, TRUE );
	gtk_window_set_default_size ( window, 400, -1 );
	g_signal_connect ( window, "destroy", G_CALLBACK ( helia_info_quit ), helia );

	GtkBox *m_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_VERTICAL,   0 );
	GtkBox *h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );

	if ( mp_tv )
		gtk_box_pack_start ( m_box, GTK_WIDGET ( helia_info_mp ( helia ) ), FALSE, FALSE, 0 );
	else
		gtk_box_pack_start ( m_box, GTK_WIDGET ( helia_info_tv ( helia ) ), FALSE, FALSE, 0 );

	GtkButton *button_close = helia_set_image_button ( "helia-exit", 16 );
	g_signal_connect_swapped ( button_close, "clicked", G_CALLBACK ( gtk_widget_destroy ), window );
	gtk_box_pack_end ( h_box, GTK_WIDGET ( button_close ), TRUE, TRUE, 5 );

	gtk_box_pack_end ( m_box, GTK_WIDGET ( h_box ), FALSE, FALSE, 5 );

	gtk_container_set_border_width ( GTK_CONTAINER ( m_box ), 10 );
	gtk_container_add   ( GTK_CONTAINER ( window ), GTK_WIDGET ( m_box ) );
	gtk_widget_show_all ( GTK_WIDGET ( window ) );

	gtk_widget_set_opacity ( GTK_WIDGET ( window ), helia->opacity_window );
}
