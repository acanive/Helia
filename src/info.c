/*
* Copyright 2019 Stepan Perun
* This program is free software.
* 
* License: Gnu Lesser General Public License
* http://www.gnu.org/licenses/lgpl.html
*/


#include <base.h>

#include "dtv-gst.h"
#include "player-gst.h"
#include "scan.h"

#include <linux/dvb/frontend.h>


static const char *all_code[] = 
{
	"CP437", "CP737", "CP850", "CP852", "CP855", "CP857", "CP858", 
	"CP860", "CP861", "CP862", "CP863", "CP865", "CP866", "CP869",

	"ISO-8859-1", "ISO-8859-2",  "ISO-8859-3",  "ISO-8859-4", 
	"ISO-8859-5", "ISO-8859-6",  "ISO-8859-7",  "ISO-8859-8", 
	"ISO-8859-9", "ISO-8859-10", "ISO-8859-11", "ISO-8859-13", 
	"ISO-8859-14", "ISO-8859-15", "ISO-8859-16", 

	"ISO-2022-CN", "ISO-2022-JP", "ISO-2022-KR",

	"GB2312", "GB18030", "GBK", "BIG-5",
	"EUC-CN", "EUC-JP", "EUC-KR", "EUC-TW", "SHIFT-JIS", 

	"KOI-7", "KOI-8", "KOI8-R", "KOI8-T", "KOI8-U"

	"MAC-CENTRALEUROPE", "MAC-CYRILLIC", "MAC-IS", "MAC-SAMI", "MAC-UK", "MAC", "MIK",

	"WINDOWS-1250", "WINDOWS-1251", "WINDOWS-1252", "WINDOWS-1253", "WINDOWS-1254", 
	"WINDOWS-1255", "WINDOWS-1256", "WINDOWS-1257", "WINDOWS-1258", 

	"ISO6937", 

	"ASCII", "UTF-8"
};


static void info_combo_lang_changed ( GtkComboBox *widget, Base *base )
{
    dtv_gst_changed_audio_track ( base, gtk_combo_box_get_active ( GTK_COMBO_BOX ( widget ) ) );
}

static GtkBox * info_tv ( Base *base )
{
	GtkBox *v_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 );
	gtk_box_set_spacing ( v_box, 5 );
	gtk_widget_set_margin_top   ( GTK_WIDGET ( v_box ), 10 );
	gtk_widget_set_margin_start ( GTK_WIDGET ( v_box ), 10 );
	gtk_widget_set_margin_end   ( GTK_WIDGET ( v_box ), 10 );

	GtkGrid *grid = (GtkGrid *)gtk_grid_new();
	gtk_grid_set_row_homogeneous    ( GTK_GRID ( grid ) ,TRUE );
    gtk_grid_set_column_homogeneous ( GTK_GRID ( grid ) ,TRUE );

	char **fields = g_strsplit ( base->dtv->ch_data, ":", 0 );
	uint numfields = g_strv_length ( fields );

	GtkEntry *entry_ch = (GtkEntry *) gtk_entry_new ();
	g_object_set ( entry_ch, "editable", FALSE, NULL );
	gtk_entry_set_text ( GTK_ENTRY ( entry_ch ), fields[0] );

	gtk_box_pack_start ( v_box, GTK_WIDGET ( entry_ch ), FALSE, FALSE, 0 );

	GtkComboBoxText *combo_lang = (GtkComboBoxText *)gtk_combo_box_text_new ();

	dtv_gst_add_audio_track ( base, combo_lang );

	g_signal_connect (G_OBJECT (combo_lang), "changed", G_CALLBACK ( info_combo_lang_changed ), base );

	gtk_box_pack_start ( v_box, GTK_WIDGET ( combo_lang ), FALSE, FALSE, 0 );

	uint j = 0, delsys = 0;

	for ( j = 1; j < numfields; j++ )
	{
		if ( g_strrstr ( fields[j], "adapter" )|| g_strrstr ( fields[j], "frontend" ) ) continue;

		if ( !g_strrstr ( fields[j], "=" ) ) continue;

		char **splits = g_strsplit ( fields[j], "=", 0 );

		if ( g_str_has_prefix ( splits[0], "delsys" ) )
		{
			delsys = atoi ( splits[1] );

			g_strfreev (splits);

			continue;
		}

		const char *set = scan_get_info ( splits[0] );

		if ( g_str_has_prefix ( splits[0], "lnb-lof1" ) ) set = "   LO1  MHz";
		if ( g_str_has_prefix ( splits[0], "lnb-lof2" ) ) set = "   LO2  MHz";
		if ( g_str_has_prefix ( splits[0], "lnb-slof" ) ) set = "   Switch  MHz";

		GtkLabel *label = (GtkLabel *)gtk_label_new ( set );
		gtk_widget_set_halign ( GTK_WIDGET ( label ), GTK_ALIGN_START );

		gtk_grid_attach ( GTK_GRID ( grid ), GTK_WIDGET ( label ), 0, j+1, 1, 1 );

		const char *set_v = scan_get_info_descr_vis ( splits[0], atoi ( splits[1] ) );

		if ( g_strrstr ( splits[0], "polarity" ) ) set_v = splits[1];

		if ( g_str_has_prefix ( splits[0], "frequency" ) || g_str_has_prefix ( splits[0], "lnb-lo" ) || g_str_has_prefix ( splits[0], "lnb-sl" ) )
		{
			long dat = atol ( splits[1] );

			if ( delsys == SYS_DVBS || delsys == SYS_DVBS2 || delsys == SYS_ISDBS )
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

static char * info_get_str_vat ( Base *base, char *get_tag, int n_cur, const char *metadata, const char *metadata_2 )
{
	char *str = NULL, *str_2 = NULL, *ret_str = NULL;

    GstTagList *tags;

	g_signal_emit_by_name ( base->player->playbin, get_tag, n_cur, &tags );

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
static char * info_get_int_vat ( GstElement *element, char *get_tag, int n_cur, char *metadata )
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

static void info_changed_combo_video ( GtkComboBox *combo, Base *base )
{
    g_object_set ( base->player->playbin, "current-video", gtk_combo_box_get_active (combo), NULL );
}
static void info_changed_combo_audio ( GtkComboBox *combo, Base *base )
{
    g_object_set ( base->player->playbin, "current-audio", gtk_combo_box_get_active (combo), NULL );
}
static void info_changed_combo_text ( GtkComboBox *combo, Base *base )
{
	g_object_set ( base->player->playbin, "current-text", gtk_combo_box_get_active (combo), NULL );	
}
static void info_change_state_subtitle ( G_GNUC_UNUSED GtkButton *button, Base *base )
{
	base->player->state_subtitle = !base->player->state_subtitle;

	player_set_subtitle ( base, base->player->state_subtitle );
	// g_object_set ( base->player->playbin, "flags", ( base->player->state_subtitle ) ? 1559 : 1555, NULL );
}
static void info_change_hide_show_subtitle ( GtkButton *button, GtkComboBoxText *combo )
{
	gboolean sensitive = !gtk_widget_get_sensitive ( GTK_WIDGET ( combo ) );

	gtk_widget_set_sensitive ( GTK_WIDGET ( combo ), sensitive );

	GtkImage *image = base_create_image ( ( sensitive ) ? "helia-set" : "helia-unset", 16 );
	gtk_button_set_image ( button, GTK_WIDGET ( image ) );
}

static gboolean info_update_bitrate_video ( Base *base )
{
	if ( base->info_quit ) return FALSE;

	uint c_video;
	g_object_get ( base->player->playbin, "current-video", &c_video, NULL );

	char *bitrate_video = info_get_int_vat ( base->player->playbin, "get-video-tags", c_video, GST_TAG_BITRATE );

	gtk_label_set_text ( base->player->label_video, ( bitrate_video ) ? bitrate_video : "? Kbits/s" );

	if ( bitrate_video ) g_free ( bitrate_video );

	return TRUE;
}

static void info_bitrate_video ( Base *base, GtkLabel *label )
{
	base->player->label_video = label;

	g_timeout_add ( 250, (GSourceFunc)info_update_bitrate_video, base );
}

static gboolean info_update_bitrate_audio ( Base *base )
{
	if ( base->info_quit ) return FALSE;

	uint c_audio;
	g_object_get ( base->player->playbin, "current-audio", &c_audio, NULL );

	char *bitrate_audio = info_get_int_vat ( base->player->playbin, "get-audio-tags", c_audio, GST_TAG_BITRATE );

	gtk_label_set_text ( base->player->label_audio, ( bitrate_audio ) ? bitrate_audio : "? Kbits/s" );

	if ( bitrate_audio ) g_free ( bitrate_audio );

	return TRUE;
}

static void info_bitrate_audio ( Base *base, GtkLabel *label )
{
	base->player->label_audio = label;

	g_timeout_add ( 250, (GSourceFunc)info_update_bitrate_audio, base );
}

static const char * info_get_str_tag ( Base *base, char *get_tag, char *data )
{
	const char *name = NULL;

    GstTagList *tags;

	g_signal_emit_by_name ( base->player->playbin, get_tag, 0, &tags );

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

/* Returns a newly-allocated string holding the result. Free with free() */
char * info_get_title_artist ( Base *base )
{
	char *title_new = NULL;

	const char *title  = info_get_str_tag ( base, "get-audio-tags", GST_TAG_TITLE  );
	const char *artist = info_get_str_tag ( base, "get-audio-tags", GST_TAG_ARTIST );

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

void info_title_save ( Base *base, const char *title, gboolean file_name )
{
	gboolean valid;
	GtkTreeIter iter;

	GtkTreeModel *model_filter = gtk_tree_view_get_model ( base->player->treeview );
    GtkTreeModel *model = gtk_tree_model_filter_get_model ( GTK_TREE_MODEL_FILTER ( model_filter ) );

	for ( valid = gtk_tree_model_get_iter_first ( model, &iter ); valid;
		  valid = gtk_tree_model_iter_next ( model, &iter ) )
	{
		char *data;

		gtk_tree_model_get ( model, &iter, ( file_name ) ? COL_FL_CH : COL_DATA,  &data, -1 );

			if ( g_str_has_suffix ( base->player->file_play, data ) )
			{
				gtk_list_store_set ( GTK_LIST_STORE ( model ), &iter, COL_FL_CH, title, -1 );

				break;
			}

		g_free ( data );
	}
}

void info_win_code ( Base *base, const char *title )
{
	GtkWindow *window =      (GtkWindow *)gtk_window_new ( GTK_WINDOW_TOPLEVEL );
	gtk_window_set_transient_for ( window, base->window );
	gtk_window_set_modal     ( window, TRUE );
	gtk_window_set_position  ( window, GTK_WIN_POS_CENTER_ON_PARENT );
	gtk_window_set_title     ( window, "" );
	gtk_window_set_default_size ( window, 700, 400 );

	GdkPixbuf *pixbuf = gtk_icon_theme_load_icon ( gtk_icon_theme_get_default (), 
					 "helia-locale", 48, GTK_ICON_LOOKUP_USE_BUILTIN, NULL );

	gtk_window_set_icon ( window, pixbuf );

	if ( pixbuf ) g_object_unref ( pixbuf );

	GtkBox *m_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_VERTICAL,   0 );
	GtkBox *h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );

	GtkTextBuffer *buffer = gtk_text_buffer_new ( NULL );

	GString *string = g_string_new ( NULL );

	uint z = 0;
    for ( z = 0; z < G_N_ELEMENTS ( all_code ); z++ )
	{
		char *convt = g_convert ( title,  -1, "utf-8", all_code[z], NULL, NULL, NULL );

		if ( convt == NULL ) continue;

		g_string_append ( string, all_code[z] );
		g_string_append ( string, ":    " );
		g_string_append ( string, convt );
		g_string_append ( string, "\n" );

		g_free ( convt );
	}

	if ( string->str )
	{
		gtk_text_buffer_set_text (  buffer, string->str, -1 );

		g_string_free ( string, TRUE );
	}

	GtkTextView *textview = (GtkTextView *)gtk_text_view_new_with_buffer ( buffer );

	GtkScrolledWindow *scroll = (GtkScrolledWindow *)gtk_scrolled_window_new ( NULL, NULL );
    gtk_scrolled_window_set_policy ( scroll, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );

    gtk_container_add ( GTK_CONTAINER ( scroll ), GTK_WIDGET ( textview ) );
	gtk_box_pack_start ( m_box, GTK_WIDGET ( scroll ), TRUE, TRUE, 5 );


	GtkButton *button_close = (GtkButton *)gtk_button_new_from_icon_name ( "helia-exit", GTK_ICON_SIZE_BUTTON );
	g_signal_connect_swapped ( button_close, "clicked", G_CALLBACK ( gtk_widget_destroy ), window );
	gtk_box_pack_end ( h_box, GTK_WIDGET ( button_close ), TRUE, TRUE, 5 );

	gtk_box_pack_end ( m_box, GTK_WIDGET ( h_box ), FALSE, FALSE, 5 );

	gtk_container_set_border_width ( GTK_CONTAINER ( m_box ), 10 );
	gtk_container_add   ( GTK_CONTAINER ( window ), GTK_WIDGET ( m_box ) );
	gtk_widget_show_all ( GTK_WIDGET ( window ) );

	gtk_widget_set_opacity ( GTK_WIDGET ( window ), base->opacity_win );
}

static void info_entry_title_save ( GtkEntry *entry, GtkEntryIconPosition icon_pos, G_GNUC_UNUSED GdkEvent *event, Base *base )
{
	const char *title = gtk_entry_get_text ( entry );

	if ( icon_pos == GTK_ENTRY_ICON_PRIMARY )
		if ( title && gtk_entry_get_text_length ( entry ) > 0 ) info_win_code ( base, title );

	if ( icon_pos == GTK_ENTRY_ICON_SECONDARY )
		if ( title && gtk_entry_get_text_length ( entry ) > 0 ) info_title_save ( base, title, FALSE );
}

static GtkBox * info_mp ( Base *base )
{
	GtkBox *v_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 );
	gtk_box_set_spacing ( v_box, 5 );
	gtk_widget_set_margin_top   ( GTK_WIDGET ( v_box ), 10 );
	gtk_widget_set_margin_start ( GTK_WIDGET ( v_box ), 10 );
	gtk_widget_set_margin_end   ( GTK_WIDGET ( v_box ), 10 );

	GtkGrid *grid = (GtkGrid *)gtk_grid_new();
	gtk_grid_set_row_homogeneous ( GTK_GRID ( grid ) ,TRUE );
    gtk_grid_set_column_homogeneous ( GTK_GRID ( grid ) ,TRUE );

	GtkBox *h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );

	char *title_new = info_get_title_artist ( base );

	GtkEntry *entry_title = (GtkEntry *) gtk_entry_new ();
	gtk_entry_set_text ( GTK_ENTRY ( entry_title ), ( title_new ) ? title_new : "None" );

	if ( title_new ) g_free ( title_new );

	gtk_entry_set_icon_from_icon_name ( entry_title, GTK_ENTRY_ICON_PRIMARY, "helia-locale" );
	gtk_entry_set_icon_from_icon_name ( entry_title, GTK_ENTRY_ICON_SECONDARY, "helia-save" );
	g_signal_connect ( entry_title, "icon-press", G_CALLBACK ( info_entry_title_save ), base );

	gtk_box_pack_start ( h_box, GTK_WIDGET ( entry_title ), TRUE, TRUE, 0 );
	gtk_box_pack_start ( v_box, GTK_WIDGET ( h_box ), FALSE, FALSE, 0 );

	h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );

	char *name = g_path_get_basename ( base->player->file_play );

	GtkEntry *entry_fl = (GtkEntry *) gtk_entry_new ();
	g_object_set ( entry_fl, "editable", FALSE, NULL );
	gtk_entry_set_text ( GTK_ENTRY ( entry_fl ), name );

	free ( name );

	gtk_box_pack_start ( h_box, GTK_WIDGET ( entry_fl ), TRUE, TRUE, 0 );

	gtk_box_pack_start ( v_box, GTK_WIDGET ( h_box ), FALSE, FALSE, 0 );

	uint n_video, n_audio, n_text, c_video, c_audio, c_text;
    g_object_get ( base->player->playbin, "n-video", &n_video, NULL );
    g_object_get ( base->player->playbin, "n-audio", &n_audio, NULL );
    g_object_get ( base->player->playbin, "n-text",  &n_text,  NULL );

    g_object_get ( base->player->playbin, "current-video", &c_video, NULL );
    g_object_get ( base->player->playbin, "current-audio", &c_audio, NULL );
    g_object_get ( base->player->playbin, "current-text",  &c_text,  NULL );

	char *bitrate_video = info_get_int_vat ( base->player->playbin, "get-video-tags", c_video, GST_TAG_BITRATE );
	char *bitrate_audio = info_get_int_vat ( base->player->playbin, "get-audio-tags", c_audio, GST_TAG_BITRATE );

	struct data { const char *name; uint n_avt; uint c_avt; const char *info; void (*f)(); } data_n[] =
	{
		{ "helia-video", 	 n_video, c_video, NULL,   info_changed_combo_video  },
		{ " ",   			 0,       0,       bitrate_video, info_bitrate_video },
		{ "helia-audio", 	 n_audio, c_audio, NULL,   info_changed_combo_audio  },
		{ " ",  			 0,       0,       bitrate_audio, info_bitrate_audio },
		{ "helia-subtitles", n_text,  c_text,  NULL,   info_changed_combo_text   }
	};

    uint c = 0, i = 0;
    for ( c = 0; c < G_N_ELEMENTS (data_n); c++ )
    {
		GtkImage *image = base_create_image ( data_n[c].name, 48 );
        gtk_widget_set_halign ( GTK_WIDGET ( image ), GTK_ALIGN_START );
        gtk_grid_attach ( GTK_GRID ( grid ), GTK_WIDGET ( image ), 0, c, 1, 2 );

        if ( c == 0 || c == 2 || c == 4 )
        {
            GtkComboBoxText *combo = (GtkComboBoxText *)gtk_combo_box_text_new ();

            for ( i = 0; i < data_n[c].n_avt; i++ )
            {
				char *teg_info = NULL; // media metadata

				if ( c == 0 ) teg_info = info_get_str_vat ( base, "get-video-tags", i, GST_TAG_VIDEO_CODEC,   NULL );
				if ( c == 2 ) teg_info = info_get_str_vat ( base, "get-audio-tags", i, GST_TAG_AUDIO_CODEC,   GST_TAG_LANGUAGE_CODE );
				if ( c == 4 ) teg_info = info_get_str_vat ( base, "get-text-tags",  i, GST_TAG_LANGUAGE_CODE, NULL );

				gtk_combo_box_text_append_text ( combo, teg_info );

				g_free ( teg_info );
			}

			gtk_combo_box_set_active ( GTK_COMBO_BOX ( combo ), data_n[c].c_avt );

			g_signal_connect ( combo, "changed", G_CALLBACK ( data_n[c].f ), base );

			if ( c == 4 && data_n[c].n_avt > 0 )
			{
				h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
				gtk_box_set_spacing ( h_box, 5 );

				GtkButton *button = base_set_image_button ( ( base->player->state_subtitle ) ? "helia-set" : "helia-unset", 16 );
				g_signal_connect ( button, "clicked", G_CALLBACK ( info_change_state_subtitle     ), base  );
				g_signal_connect ( button, "clicked", G_CALLBACK ( info_change_hide_show_subtitle ), combo );

				gtk_box_pack_start ( h_box, GTK_WIDGET ( combo  ), TRUE, TRUE, 0 );
				gtk_box_pack_start ( h_box, GTK_WIDGET ( button ), TRUE, TRUE, 0 );

				gtk_grid_attach ( GTK_GRID ( grid ), GTK_WIDGET ( h_box ), 1, c, 1, 1 );
				gtk_widget_set_sensitive ( GTK_WIDGET ( combo ), base->player->state_subtitle );
			}
			else
				gtk_grid_attach ( GTK_GRID ( grid ), GTK_WIDGET ( combo ), 1, c, 1, 1 );
        }
        else
        {
            GtkLabel *label = (GtkLabel *)gtk_label_new ( ( data_n[c].info ) ? data_n[c].info : "? Kbits/s" );
            gtk_widget_set_halign ( GTK_WIDGET ( label ), GTK_ALIGN_START );
            gtk_grid_attach ( GTK_GRID ( grid ), GTK_WIDGET ( label ), 1, c, 1, 1 );

            data_n[c].f ( base, label );
        }
    }

    if ( bitrate_video ) g_free ( bitrate_video );
    if ( bitrate_audio ) g_free ( bitrate_audio );

	gtk_box_pack_start ( v_box, GTK_WIDGET ( grid ), TRUE, TRUE, 10 );

	return v_box;
}

static void info_quit ( G_GNUC_UNUSED GtkWindow *win, Base *base )
{
	base->info_quit = TRUE;
}

void info_win_create ( Base *base, gboolean tv_mp )
{
	GtkWindow *window =      (GtkWindow *)gtk_window_new ( GTK_WINDOW_TOPLEVEL );
	gtk_window_set_transient_for ( window, base->window );
	gtk_window_set_modal     ( window, TRUE );
	gtk_window_set_position  ( window, GTK_WIN_POS_CENTER_ON_PARENT );
	gtk_window_set_title     ( window, "" );
	gtk_window_set_default_size ( window, 400, -1 );
	g_signal_connect ( window, "destroy", G_CALLBACK ( info_quit ), base );

	GdkPixbuf *pixbuf = gtk_icon_theme_load_icon ( gtk_icon_theme_get_default (), 
					 "helia-info", 48, GTK_ICON_LOOKUP_USE_BUILTIN, NULL );

	gtk_window_set_icon ( window, pixbuf );

	if ( pixbuf ) g_object_unref ( pixbuf );

	base->info_quit = FALSE;

	GtkBox *m_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_VERTICAL,   0 );
	GtkBox *h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );

	if ( tv_mp )
		gtk_box_pack_start ( m_box, GTK_WIDGET ( info_tv ( base ) ), FALSE, FALSE, 0 );
	else
		gtk_box_pack_start ( m_box, GTK_WIDGET ( info_mp ( base ) ), FALSE, FALSE, 0 );

	GtkButton *button_close = (GtkButton *)gtk_button_new_from_icon_name ( "helia-exit", GTK_ICON_SIZE_BUTTON );
	g_signal_connect_swapped ( button_close, "clicked", G_CALLBACK ( gtk_widget_destroy ), window );
	gtk_box_pack_end ( h_box, GTK_WIDGET ( button_close ), TRUE, TRUE, 5 );

	gtk_box_pack_end ( m_box, GTK_WIDGET ( h_box ), FALSE, FALSE, 5 );

	gtk_container_set_border_width ( GTK_CONTAINER ( m_box ), 10 );
	gtk_container_add   ( GTK_CONTAINER ( window ), GTK_WIDGET ( m_box ) );
	gtk_widget_show_all ( GTK_WIDGET ( window ) );

	gtk_widget_set_opacity ( GTK_WIDGET ( window ), base->opacity_win );
}
