/*
* Copyright 2019 Stepan Perun
* This program is free software.
*
* License: Gnu Lesser General Public License
* http://www.gnu.org/licenses/lgpl.html
*/


#include <helia-include.h>

#include "descr.h"

#include <stdlib.h>
#include <glib/gstdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>


static void helia_convert_dvb5 ( Helia *helia, const char *file );


enum page_n
{
	PAGE_SC,
	PAGE_DT,
	PAGE_DS,
	PAGE_DC,
	PAGE_CH,
	PAGE_NUM
};

const struct HeliaScanLabel { uint page; const char *name; } scan_label_n[] =
{
	{ PAGE_SC, "Scanner"  },
	{ PAGE_DT, "DVB-T/T2" },
	{ PAGE_DS, "DVB-S/S2" },
	{ PAGE_DC, "DVB-C"    },
	{ PAGE_CH, "Channels" }
};

struct DvbDescrGstParam { const char *name; const char *dvb_v5_name; const char *gst_param; 
						  const DvbDescrAll *dvb_descr; uint cdsc; } gst_param_dvb_descr_n[] =
{
// descr
{ "Inversion",      "INVERSION",         "inversion",        dvb_descr_inversion_type_n,  G_N_ELEMENTS ( dvb_descr_inversion_type_n  ) },
{ "Code Rate HP",   "CODE_RATE_HP",      "code-rate-hp",     dvb_descr_coderate_type_n,   G_N_ELEMENTS ( dvb_descr_coderate_type_n   ) },
{ "Code Rate LP",   "CODE_RATE_LP",      "code-rate-lp",     dvb_descr_coderate_type_n,   G_N_ELEMENTS ( dvb_descr_coderate_type_n   ) },
{ "Inner Fec",      "INNER_FEC",         "code-rate-hp",     dvb_descr_coderate_type_n,   G_N_ELEMENTS ( dvb_descr_coderate_type_n   ) },
{ "Modulation",     "MODULATION",        "modulation",       dvb_descr_modulation_type_n, G_N_ELEMENTS ( dvb_descr_modulation_type_n ) },
{ "Transmission",   "TRANSMISSION_MODE", "trans-mode",       dvb_descr_transmode_type_n,  G_N_ELEMENTS ( dvb_descr_transmode_type_n  ) },
{ "Guard interval", "GUARD_INTERVAL",    "guard",            dvb_descr_guard_type_n,      G_N_ELEMENTS ( dvb_descr_guard_type_n 	 ) },
{ "Hierarchy",      "HIERARCHY",         "hierarchy",        dvb_descr_hierarchy_type_n,  G_N_ELEMENTS ( dvb_descr_hierarchy_type_n  ) },
{ "Pilot",          "PILOT",             "pilot",            dvb_descr_pilot_type_n,      G_N_ELEMENTS ( dvb_descr_pilot_type_n 	 ) },
{ "Rolloff",        "ROLLOFF",           "rolloff",          dvb_descr_roll_type_n,       G_N_ELEMENTS ( dvb_descr_roll_type_n 		 ) },
{ "Polarity",       "POLARIZATION",      "polarity",         dvb_descr_polarity_type_n,   G_N_ELEMENTS ( dvb_descr_polarity_type_n   ) },
{ "LNB",            "LNB",               "lnb-type",         dvb_descr_lnb_type_n,        G_N_ELEMENTS ( dvb_descr_lnb_type_n 		 ) },
{ "DiSEqC",         "SAT_NUMBER",        "diseqc-source",    dvb_descr_diseqc_num_n,      G_N_ELEMENTS ( dvb_descr_diseqc_num_n 	 ) },
{ "Interleaving",   "INTERLEAVING",      "interleaving",     dvb_descr_ileaving_type_n,   G_N_ELEMENTS ( dvb_descr_ileaving_type_n 	 ) },

// digits
{ "Frequency  MHz", 	"FREQUENCY",         "frequency",        NULL, 0 },
{ "Bandwidth  Hz",  	"BANDWIDTH_HZ",      "bandwidth-hz",     NULL, 0 },
{ "Symbol rate  kBd", 	"SYMBOL_RATE",       "symbol-rate",      NULL, 0 },
{ "Stream ID",      	"STREAM_ID",         "stream-id",        NULL, 0 },
{ "Service Id",     	"SERVICE_ID",        "program-number",   NULL, 0 },
{ "Audio Pid",      	"AUDIO_PID",         "audio-pid",        NULL, 0 },
{ "Video Pid",      	"VIDEO_PID",         "video-pid",        NULL, 0 },

// ISDB
{ "Layer enabled",      "ISDBT_LAYER_ENABLED",            "isdbt-layer-enabled",            NULL, 0 },
{ "Partial",            "ISDBT_PARTIAL_RECEPTION",        "isdbt-partial-reception",        NULL, 0 },
{ "Sound",              "ISDBT_SOUND_BROADCASTING",       "isdbt-sound-broadcasting",       NULL, 0 },
{ "Subchannel  SB",     "ISDBT_SB_SUBCHANNEL_ID",         "isdbt-sb-subchannel-id",         NULL, 0 },
{ "Segment idx  SB",    "ISDBT_SB_SEGMENT_IDX",           "isdbt-sb-segment-idx",           NULL, 0 },
{ "Segment count  SB",  "ISDBT_SB_SEGMENT_COUNT",         "isdbt-sb-segment-count",         NULL, 0 },
{ "Inner Fec  LA",      "ISDBT_LAYERA_FEC",               "isdbt-layera-fec",               dvb_descr_coderate_type_n,   G_N_ELEMENTS ( dvb_descr_coderate_type_n   ) },
{ "Modulation  LA",     "ISDBT_LAYERA_MODULATION",        "isdbt-layera-modulation",        dvb_descr_modulation_type_n, G_N_ELEMENTS ( dvb_descr_modulation_type_n ) },
{ "Segment count  LA",  "ISDBT_LAYERA_SEGMENT_COUNT",     "isdbt-layera-segment-count",     NULL, 0  },
{ "Interleaving  LA",   "ISDBT_LAYERA_TIME_INTERLEAVING", "isdbt-layera-time-interleaving", NULL, 0  },
{ "Inner Fec  LB",      "ISDBT_LAYERB_FEC",               "isdbt-layerb-fec",               dvb_descr_coderate_type_n,   G_N_ELEMENTS ( dvb_descr_coderate_type_n   ) },
{ "Modulation  LB",     "ISDBT_LAYERB_MODULATION",        "isdbt-layerb-modulation",        dvb_descr_modulation_type_n, G_N_ELEMENTS ( dvb_descr_modulation_type_n ) },
{ "Segment count  LB",  "ISDBT_LAYERB_SEGMENT_COUNT",     "isdbt-layerb-segment-count",     NULL, 0  },
{ "Interleaving  LB",   "ISDBT_LAYERB_TIME_INTERLEAVING", "isdbt-layerb-time-interleaving", NULL, 0  },
{ "Inner Fec  LC",      "ISDBT_LAYERC_FEC",               "isdbt-layerc-fec",               dvb_descr_coderate_type_n,   G_N_ELEMENTS ( dvb_descr_coderate_type_n   ) },
{ "Modulation  LC",     "ISDBT_LAYERC_MODULATION",        "isdbt-layerc-modulation",        dvb_descr_modulation_type_n, G_N_ELEMENTS ( dvb_descr_modulation_type_n ) },
{ "Segment count  LC",  "ISDBT_LAYERC_SEGMENT_COUNT",     "isdbt-layerc-segment-count",     NULL, 0  },
{ "Interleaving  LC",   "ISDBT_LAYERC_TIME_INTERLEAVING", "isdbt-layerc-time-interleaving", NULL, 0  }
};


static uint get_lnb_low_high_switch ( GstElement *element, const char *param )
{
	uint freq = 0;

	g_object_get ( element, param, &freq, NULL );

	return freq / 1000;
}

static void lnb_win_changed_spin_all ( GtkSpinButton *button, GstElement *element )
{
	gtk_spin_button_update ( button );

	uint freq = gtk_spin_button_get_value ( button );

	g_object_set ( element, gtk_widget_get_name ( GTK_WIDGET ( button ) ), freq *= 1000, NULL );
}

static void lnb_win_set_low_high_switch ( G_GNUC_UNUSED GtkButton *button, Helia *helia )
{
	if ( helia->lnb_type != LNB_MNL ) return;

	GtkWindow *window = helia_create_window_top ( helia->window, "", "helia-display", GTK_WIN_POS_CENTER_ON_PARENT, TRUE );
	gtk_window_set_default_size ( window, 400, -1 );

	GstElement *element = helia_gst_iterate_element ( helia->scan, "dvbsrc", NULL );

	GtkBox *m_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 );

	GtkGrid *grid = (GtkGrid *)gtk_grid_new();
	gtk_grid_set_column_homogeneous ( GTK_GRID ( grid ), TRUE );
	gtk_grid_set_row_spacing ( grid, 5 );
	gtk_box_pack_start ( m_box, GTK_WIDGET ( grid ), TRUE, TRUE, 10 );

	struct data_a { const char *text; const char *name; uint value; } data_a_n[] =
	{
		{ "LNB LOf1   MHz", "lnb-lof1", get_lnb_low_high_switch ( element, "lnb-lof1" ) },
		{ "LNB LOf2   MHz", "lnb-lof2", get_lnb_low_high_switch ( element, "lnb-lof2" ) },
		{ "LNB Switch MHz", "lnb-slof", get_lnb_low_high_switch ( element, "lnb-slof" ) }
	};

	uint d = 0;
	for ( d = 0; d < G_N_ELEMENTS ( data_a_n ); d++ )
	{
		GtkLabel *label = (GtkLabel *)gtk_label_new ( data_a_n[d].text );
		gtk_widget_set_halign ( GTK_WIDGET ( label ), GTK_ALIGN_START );
		gtk_grid_attach ( GTK_GRID ( grid ), GTK_WIDGET ( label ), 0, d, 1, 1 );

		GtkSpinButton *spinbutton = (GtkSpinButton *)gtk_spin_button_new_with_range ( lnb_type_lhs_n[LNB_MNL].min_val, lnb_type_lhs_n[LNB_MNL].max_val, 1 );
		gtk_widget_set_name ( GTK_WIDGET ( spinbutton ), data_a_n[d].name );
		gtk_spin_button_set_value ( spinbutton, data_a_n[d].value );
		g_signal_connect ( spinbutton, "changed", G_CALLBACK ( lnb_win_changed_spin_all ), element );

		gtk_grid_attach ( GTK_GRID ( grid ), GTK_WIDGET ( spinbutton ), 1, d, 1, 1 );
	}

	GtkButton *button_close = helia_set_image_button ( "helia-exit", 16 );
	g_signal_connect_swapped ( button_close, "clicked", G_CALLBACK ( gtk_widget_destroy ), window );

	gtk_box_pack_end ( m_box, GTK_WIDGET ( button_close ), FALSE, FALSE, 0 );

	gtk_container_set_border_width ( GTK_CONTAINER ( m_box ), 10 );
	gtk_container_add   ( GTK_CONTAINER ( window ), GTK_WIDGET ( m_box ) );
	gtk_widget_show_all ( GTK_WIDGET ( window ) );

	gtk_widget_set_opacity ( GTK_WIDGET ( window ), helia->opacity_window );
}

void helia_set_lnb_low_high_switch ( GstElement *element, int type_lnb )
{
	if ( type_lnb == LNB_MNL )
	{
		g_debug ( "%s: LNB_MNL ( num %d ) ", __func__, LNB_MNL );

		return;
	}

	g_object_set ( element, "lnb-lof1", lnb_type_lhs_n[type_lnb].lo1_val,    NULL );
	g_object_set ( element, "lnb-lof2", lnb_type_lhs_n[type_lnb].lo2_val,    NULL );
	g_object_set ( element, "lnb-slof", lnb_type_lhs_n[type_lnb].switch_val, NULL );
}

const char * helia_get_dvb_type_str ( int delsys )
{
	const char *dvb_type = "Undefined";

	uint i = 0;
	for ( i = 0; i < G_N_ELEMENTS ( dvb_descr_delsys_type_n ); i++ )
	{
		if ( dvb_descr_delsys_type_n[i].descr_num == delsys )
			dvb_type = dvb_descr_delsys_type_n[i].text_vis;
	}

	return dvb_type;
}

const char * helia_scan_get_info ( const char *data )
{
	const char *res = NULL;

	uint c = 0;
	for ( c = 0; c < G_N_ELEMENTS ( gst_param_dvb_descr_n ); c++ )
	{
		if ( g_str_has_suffix ( data, gst_param_dvb_descr_n[c].gst_param ) )
		{
			res = gst_param_dvb_descr_n[c].name;

			break;
		}
	}

	return res;
}

const char * helia_scan_get_info_descr_vis ( const char *data, int num )
{
	const char *res = NULL;

	uint c = 0;
	for ( c = 0; c < G_N_ELEMENTS ( gst_param_dvb_descr_n ); c++ )
	{
		if ( gst_param_dvb_descr_n[c].cdsc == 0 ) continue;

		if ( g_str_has_suffix ( data, gst_param_dvb_descr_n[c].gst_param ) )
		{
			if ( g_str_has_prefix ( data, "diseqc-source" ) ) num += 1;

			res = gst_param_dvb_descr_n[c].dvb_descr[num].text_vis;

			break;
		}
	}

	return res;
}

static void helia_scan_changed_spin_all ( GtkSpinButton *button, Helia *helia )
{
	gtk_spin_button_update ( button );

	GstElement *element = helia_gst_iterate_element ( helia->scan, "dvbsrc", NULL );

	long num = gtk_spin_button_get_value ( button );
	const char *name = gtk_widget_get_name ( GTK_WIDGET ( button ) );

	if ( g_str_has_prefix ( name, "Frequency" ) ) num *= ( num < 1000 ) ? 1000000 : 1000;

	uint c = 0;
	for ( c = 0; c < G_N_ELEMENTS ( gst_param_dvb_descr_n ); c++ )
	{
		if ( g_str_has_suffix ( name, gst_param_dvb_descr_n[c].name ) )
		{
			g_object_set ( element, gst_param_dvb_descr_n[c].gst_param, num, NULL );

			g_debug ( "name = %s | num = %ld | gst_param = %s ", name, 
				num, gst_param_dvb_descr_n[c].gst_param );
		}
	}
}
static void helia_scan_changed_combo_all ( GtkComboBox *combo_box, Helia *helia )
{
	uint num = gtk_combo_box_get_active ( combo_box );
	const char *name = gtk_widget_get_name ( GTK_WIDGET ( combo_box ) );

	GstElement *element = helia_gst_iterate_element ( helia->scan, "dvbsrc", NULL );

	if ( g_str_has_prefix ( name, "LNB" ) )
	{
		helia->lnb_type = num;
		helia_set_lnb_low_high_switch ( element, num );

		g_debug ( "name %s | set %s: %d ( low %ud | high %ud | switch %ud )", name, lnb_type_lhs_n[num].name, num, 
			lnb_type_lhs_n[num].lo1_val, lnb_type_lhs_n[num].lo2_val, lnb_type_lhs_n[num].switch_val );

		return;
	}

	uint c = 0;
	for ( c = 0; c < G_N_ELEMENTS ( gst_param_dvb_descr_n ); c++ )
	{
		if ( g_str_has_suffix ( name, gst_param_dvb_descr_n[c].name ) )
		{
			if ( g_str_has_prefix ( name, "Polarity" ) )
			{
				g_object_set ( element, "polarity", ( num == 1 || num == 3 ) ? "V" : "H", NULL );
			}
			else
				g_object_set ( element, gst_param_dvb_descr_n[c].gst_param, 
						gst_param_dvb_descr_n[c].dvb_descr[num].descr_num, NULL );

			g_debug ( "name = %s | num = %d | gst_param = %s | descr_text_vis = %s | descr_num = %d ", 
				name, num, gst_param_dvb_descr_n[c].gst_param, 
				gst_param_dvb_descr_n[c].dvb_descr[num].text_vis, 
				gst_param_dvb_descr_n[c].dvb_descr[num].descr_num );
		}
	}
}

static GtkBox * helia_scan_dvb_all ( Helia *helia, uint num, const DvbTypes *dvball, const char *type )
{
	g_debug ( "%s:: %s", __func__, type );

	GstElement *element = helia_gst_iterate_element ( helia->scan, "dvbsrc", NULL );

	GtkBox *g_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 );
	gtk_widget_set_margin_start ( GTK_WIDGET ( g_box ), 10 );
	gtk_widget_set_margin_end   ( GTK_WIDGET ( g_box ), 10 );

	GtkGrid *grid = (GtkGrid *)gtk_grid_new();
	gtk_grid_set_column_homogeneous ( GTK_GRID ( grid ), TRUE );
	gtk_box_pack_start ( g_box, GTK_WIDGET ( grid ), TRUE, TRUE, 10 );

	GtkLabel *label;
	GtkSpinButton *spinbutton;
	GtkComboBoxText *scan_combo_box;

	gboolean freq = FALSE;
	int d_data = 0, set_freq = 1000000;
	uint d = 0, c = 0, z = 0;

	if ( g_str_has_prefix ( type, "DVB-S" ) ) set_freq = 1000;

	for ( d = 0; d < num; d++ )
	{
		label = (GtkLabel *)gtk_label_new ( dvball[d].param );
		gtk_widget_set_halign ( GTK_WIDGET ( label ), GTK_ALIGN_START );
		gtk_grid_attach ( GTK_GRID ( grid ), GTK_WIDGET ( label ), 0, d, 1, 1 );

		if ( !dvball[d].descr )
		{
			if ( g_str_has_prefix ( dvball[d].param, "Frequency" ) ) freq = TRUE; else freq = FALSE;

			spinbutton = (GtkSpinButton *) gtk_spin_button_new_with_range ( dvball[d].min, dvball[d].max, 1 );
			gtk_widget_set_name ( GTK_WIDGET ( spinbutton ), dvball[d].param );
			gtk_grid_attach ( GTK_GRID ( grid ), GTK_WIDGET ( spinbutton ), 1, d, 1, 1 );

			for ( c = 0; c < G_N_ELEMENTS ( gst_param_dvb_descr_n ); c++ )
			{
				if ( g_str_has_suffix ( dvball[d].param, gst_param_dvb_descr_n[c].name ) )
				{
					g_object_get ( element, gst_param_dvb_descr_n[c].gst_param, &d_data, NULL );
					gtk_spin_button_set_value ( spinbutton, freq ? d_data / set_freq : d_data );
				}
			}

			g_signal_connect ( spinbutton, "changed", G_CALLBACK ( helia_scan_changed_spin_all ), helia );
		}
		else
		{
			scan_combo_box = (GtkComboBoxText *) gtk_combo_box_text_new ();
			gtk_widget_set_name ( GTK_WIDGET ( scan_combo_box ), dvball[d].param );

			if ( g_str_has_prefix ( dvball[d].param, "LNB" ) )
			{
				GtkBox *h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );

				GtkButton *button_lnb = (GtkButton *)gtk_button_new_with_label ( " 🤚 " );
				g_signal_connect ( button_lnb, "clicked", G_CALLBACK ( lnb_win_set_low_high_switch ), helia );

				gtk_box_pack_start ( h_box, GTK_WIDGET ( scan_combo_box ), TRUE, TRUE, 0 );
				gtk_box_pack_start ( h_box, GTK_WIDGET ( button_lnb     ), TRUE, TRUE, 0 );

				gtk_grid_attach ( GTK_GRID ( grid ), GTK_WIDGET ( h_box ), 1, d, 1, 1 );
			}
			else
				gtk_grid_attach ( GTK_GRID ( grid ), GTK_WIDGET ( scan_combo_box ), 1, d, 1, 1 );

			for ( c = 0; c < G_N_ELEMENTS ( gst_param_dvb_descr_n ); c++ )
			{
				if ( g_str_has_suffix ( dvball[d].param, gst_param_dvb_descr_n[c].name ) )
				{
					for ( z = 0; z < gst_param_dvb_descr_n[c].cdsc; z++ )
						gtk_combo_box_text_append_text ( scan_combo_box, gst_param_dvb_descr_n[c].dvb_descr[z].text_vis );

					if ( g_str_has_prefix ( dvball[d].param, "Polarity" ) )
					{
						char *pol = NULL;

						g_object_get ( element, gst_param_dvb_descr_n[c].gst_param, &pol, NULL );

						if ( g_str_has_prefix ( pol, "V" ) || g_str_has_prefix ( pol, "v" ) )
							gtk_combo_box_set_active ( GTK_COMBO_BOX ( scan_combo_box ), 1 );
						else
							gtk_combo_box_set_active ( GTK_COMBO_BOX ( scan_combo_box ), 0 );

						g_free ( pol );

						continue;
					}

					if ( g_str_has_prefix ( dvball[d].param, "LNB" ) )
						d_data = helia->lnb_type;
					else
						g_object_get ( element, gst_param_dvb_descr_n[c].gst_param, &d_data, NULL );

					if ( g_str_has_prefix ( dvball[d].param, "DiSEqC" ) ) d_data += 1;

					gtk_combo_box_set_active ( GTK_COMBO_BOX ( scan_combo_box ), d_data );
				}
			}

			if ( gtk_combo_box_get_active ( GTK_COMBO_BOX ( scan_combo_box ) ) == -1 )
				 gtk_combo_box_set_active ( GTK_COMBO_BOX ( scan_combo_box ), 0 );

			g_signal_connect ( scan_combo_box, "changed", G_CALLBACK ( helia_scan_changed_combo_all ), helia );
		}
	}

	return g_box;
}

static void helia_scan_get_tp_data ( Helia *helia, GString *gstring )
{
	GstElement *element = helia_gst_iterate_element ( helia->scan, "dvbsrc", NULL );

	uint c = 0, d = 0, l = 0;
	int  d_data = 0, DVBTYPE = 0;

	g_object_get ( element, "delsys", &DVBTYPE, NULL );

	DVBTYPE = helia->dvb_type;

	const char *dvb_f[] = { "delsys", "adapter", "frontend" };

	for ( d = 0; d < G_N_ELEMENTS ( dvb_f ); d++ )
	{
		g_object_get ( element, dvb_f[d], &d_data, NULL );
		g_string_append_printf ( gstring, ":%s=%d", dvb_f[d], d_data );
	}

	if ( DVBTYPE == SYS_DVBT || DVBTYPE == SYS_DVBT2 )
	{
		for ( c = 0; c < G_N_ELEMENTS ( dvbt_props_n ); c++ )
		{
			if ( DVBTYPE == SYS_DVBT )
				if ( g_str_has_prefix ( dvbt_props_n[c].param, "Stream ID" ) ) continue;

			for ( d = 0; d < G_N_ELEMENTS ( gst_param_dvb_descr_n ); d++ )
			{
				if ( g_str_has_suffix ( dvbt_props_n[c].param, gst_param_dvb_descr_n[d].name ) )
				{
					g_object_get ( element, gst_param_dvb_descr_n[d].gst_param, &d_data, NULL );
					g_string_append_printf ( gstring, ":%s=%d", gst_param_dvb_descr_n[d].gst_param, d_data );

					break;
				}
			}
		}
	}

	if ( DVBTYPE == SYS_DVBC_ANNEX_A || DVBTYPE == SYS_DVBC_ANNEX_C )
	{
		for ( c = 0; c < G_N_ELEMENTS ( dvbc_props_n ); c++ )
		{
			for ( d = 0; d < G_N_ELEMENTS ( gst_param_dvb_descr_n ); d++ )
			{
				if ( g_str_has_suffix ( dvbc_props_n[c].param, gst_param_dvb_descr_n[d].name ) )
				{
					g_object_get ( element, gst_param_dvb_descr_n[d].gst_param, &d_data, NULL );
					g_string_append_printf ( gstring, ":%s=%d", gst_param_dvb_descr_n[d].gst_param, d_data );

					break;
				}
			}
		}
	}

	if ( DVBTYPE == SYS_DVBC_ANNEX_B )
	{
		for ( c = 0; c < G_N_ELEMENTS ( atsc_props_n ); c++ )
		{
			for ( d = 0; d < G_N_ELEMENTS ( gst_param_dvb_descr_n ); d++ )
			{
				if ( g_str_has_suffix ( atsc_props_n[c].param, gst_param_dvb_descr_n[d].name ) )
				{
					g_object_get ( element, gst_param_dvb_descr_n[d].gst_param, &d_data, NULL );
					g_string_append_printf ( gstring, ":%s=%d", gst_param_dvb_descr_n[d].gst_param, d_data );

					break;
				}
			}
		}
	}

	if ( DVBTYPE == SYS_DVBS || DVBTYPE == SYS_TURBO || DVBTYPE == SYS_DVBS2 )
	{
		for ( c = 0; c < G_N_ELEMENTS ( dvbs_props_n ); c++ )
		{
			if ( DVBTYPE == SYS_TURBO )
				if ( g_str_has_prefix ( dvbs_props_n[c].param, "Pilot" ) || g_str_has_prefix ( dvbs_props_n[c].param, "Rolloff" ) || g_str_has_prefix ( dvbs_props_n[c].param, "Stream ID" ) ) continue;

			if ( DVBTYPE == SYS_DVBS )
				if ( g_str_has_prefix ( dvbs_props_n[c].param, "Modulation" ) || g_str_has_prefix ( dvbs_props_n[c].param, "Pilot" ) || g_str_has_prefix ( dvbs_props_n[c].param, "Rolloff" ) || g_str_has_prefix ( dvbs_props_n[c].param, "Stream ID" ) ) continue;

			for ( d = 0; d < G_N_ELEMENTS ( gst_param_dvb_descr_n ); d++ )
			{
				if ( g_str_has_suffix ( dvbs_props_n[c].param, gst_param_dvb_descr_n[d].name ) )
				{
					if ( g_str_has_prefix ( "polarity", gst_param_dvb_descr_n[d].gst_param ) )
					{
						char *pol = NULL;

						g_object_get ( element, gst_param_dvb_descr_n[d].gst_param, &pol, NULL );

						g_string_append_printf ( gstring, ":polarity=%s", pol );

						g_free ( pol );

						continue;
					}

					if ( g_str_has_prefix ( "lnb-type", gst_param_dvb_descr_n[d].gst_param ) )
					{
						g_string_append_printf ( gstring, ":%s=%d", "lnb-type", helia->lnb_type );

						if ( helia->lnb_type == LNB_MNL )
						{
							const char *lnbf_gst[] = { "lnb-lof1", "lnb-lof2", "lnb-slof" };

							for ( l = 0; l < G_N_ELEMENTS ( lnbf_gst ); l++ )
							{
								g_object_get ( element, lnbf_gst[l], &d_data, NULL );
								g_string_append_printf ( gstring, ":%s=%d", lnbf_gst[l], d_data );
							}
						}

						continue;
					}

					g_object_get ( element, gst_param_dvb_descr_n[d].gst_param, &d_data, NULL );
					g_string_append_printf ( gstring, ":%s=%d", gst_param_dvb_descr_n[d].gst_param, d_data );

					break;
				}
			}
		}
	}
}


static char * _strip_ch_name ( char *name )
{
	uint i = 0;
	for ( i = 0; name[i] != '\0'; i++ )
	{
		if ( name[i] == ':' ) name[i] = ' ';
	}
	return g_strstrip ( name );
}

static void helia_scan_set_data_to_treeview_scan ( const char *ch_name, const char *ch_data, HeliaTreeview *tree_view )
{
	GtkTreeIter iter;

	GtkTreeModel *model_filter = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );
	GtkTreeModel *model = gtk_tree_model_filter_get_model ( GTK_TREE_MODEL_FILTER ( model_filter ) );

	uint ind = gtk_tree_model_iter_n_children ( model, NULL );

	gtk_list_store_append ( GTK_LIST_STORE ( model ), &iter );
	gtk_list_store_set    ( GTK_LIST_STORE ( model ), &iter,
				COL_NUM, ind + 1,
				COL_FL_CH, ch_name,
				COL_DATA,  ch_data,
				-1 );
}

static void helia_scan_read_ch_to_treeview ( Helia *helia )
{
	if ( helia->mpegts->pmt_count == 0 ) return;

	GString *gstr_data = g_string_new ( NULL );

	helia_scan_get_tp_data ( helia, gstr_data );

	uint i = 0, c = 0;

	for ( i = 0; i < helia->mpegts->pmt_count; i++ )
	{
		char *ch_name = NULL;

		for ( c = 0; c < helia->mpegts->sdt_count; c++ )
			if ( helia->mpegts->pids[i].pmt_sid == helia->mpegts->pids[c].sdt_sid )
				{ ch_name = helia->mpegts->pids[c].ch_name; break; }

		GString *gstring = g_string_new ( NULL );

		if ( ch_name )
			ch_name = _strip_ch_name ( ch_name );
		else
			ch_name = g_strdup_printf ( "Program-%d", helia->mpegts->pids[i].pmt_sid );

		g_string_append_printf ( gstring, "%s:program-number=%d:video-pid=%d:audio-pid=%d%s", 
					ch_name,
					helia->mpegts->pids[i].pmt_sid, 
					helia->mpegts->pids[i].pmt_vpid,
					helia->mpegts->pids[i].pmt_apid, 
					gstr_data->str );

		if ( helia->treeview_scan != NULL && helia->mpegts->pids[i].pmt_apid != 0 ) // ignore other
			helia_scan_set_data_to_treeview_scan ( ch_name, gstring->str, helia->treeview_scan );

		g_string_free ( gstring, TRUE );
		g_free ( ch_name );
	}

	g_string_free ( gstr_data, TRUE );
}

void helia_scan_treeview_save_channels ( HeliaTreeview *tree_view, const char *name, const char *data )
{
	GtkTreeIter iter;

	GtkTreeModel *model_filter = gtk_tree_view_get_model ( tree_view );
	GtkTreeModel *model = gtk_tree_model_filter_get_model ( GTK_TREE_MODEL_FILTER ( model_filter ) );

	uint ind = gtk_tree_model_iter_n_children ( model, NULL );

	gtk_list_store_append ( GTK_LIST_STORE ( model ), &iter);
	gtk_list_store_set    ( GTK_LIST_STORE ( model ), &iter,
				COL_NUM, ind+1,
				COL_FL_CH, name,
				COL_DATA,  data,
				-1 );
}

static void helia_scan_channels_save ( G_GNUC_UNUSED GtkButton *button, Helia *helia )
{
	GtkTreeIter iter;

	GtkTreeModel *model_filter = gtk_tree_view_get_model ( helia->treeview_scan );
	GtkTreeModel *model = gtk_tree_model_filter_get_model ( GTK_TREE_MODEL_FILTER ( model_filter ) );

	gboolean valid;
	for ( valid = gtk_tree_model_get_iter_first ( model, &iter ); valid;
		valid = gtk_tree_model_iter_next ( model, &iter ) )
	{
		char *name, *data;

		gtk_tree_model_get ( model, &iter, COL_DATA,  &data, -1 );
		gtk_tree_model_get ( model, &iter, COL_FL_CH, &name, -1 );

			helia_scan_treeview_save_channels ( helia->treeview_tv, name, data );

		g_free ( name );
		g_free ( data );
	}
}

static void helia_scan_channels_clear ( G_GNUC_UNUSED GtkButton *button, Helia *helia )
{
	GtkTreeModel *model_filter = gtk_tree_view_get_model ( helia->treeview_scan );
	GtkTreeModel *model = gtk_tree_model_filter_get_model ( GTK_TREE_MODEL_FILTER ( model_filter ) );

	gtk_list_store_clear ( GTK_LIST_STORE ( model ) );
}

static GtkBox * helia_scan_channels ( Helia *helia )
{
	GtkBox *g_box  = (GtkBox *)gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_widget_set_margin_start ( GTK_WIDGET ( g_box ), 0 );
	gtk_widget_set_margin_end   ( GTK_WIDGET ( g_box ), 0 );

	helia->treeview_scan = helia_treeview_new ( helia, FALSE );
	gtk_box_pack_start ( g_box, GTK_WIDGET ( helia_create_scroll ( helia->treeview_scan, 250 ) ), TRUE, TRUE, 0 );

	GtkBox *hb_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
	gtk_widget_set_margin_start ( GTK_WIDGET ( hb_box ), 5 );
	gtk_widget_set_margin_end   ( GTK_WIDGET ( hb_box ), 5 );
	gtk_box_set_spacing ( hb_box, 5 );

	helia_create_image_button ( hb_box, "helia-clear",  16, helia_scan_channels_clear, helia );
	helia_create_image_button ( hb_box, "helia-save",   16, helia_scan_channels_save,  helia );

	gtk_box_pack_start ( g_box, GTK_WIDGET ( hb_box ), FALSE, FALSE, 5 );

	return g_box;
}

static void helia_scan_set_label_device ( GtkLabel *label, uint adapter, uint frontend, uint delsys )
{
	g_autofree char *dvb_name = helia_get_dvb_info ( adapter, frontend );

	gtk_label_set_text ( label, dvb_name );

	g_debug ( "DVB device: %s ( %s ) ", dvb_name, helia_get_dvb_type_str ( delsys ) );
}

static void helia_scan_set_new_adapter ( Helia *helia )
{
	uint c = 0, num = 0, frontend = 0, adapter = 0;

	GstElement *element = helia_gst_iterate_element ( helia->scan, "dvbsrc", NULL );

	g_object_get ( element, "adapter",  &adapter,  NULL );
	g_object_get ( element, "frontend", &frontend, NULL );

	helia->dvb_type = helia_get_dvb_delsys ( adapter, frontend );
	helia_scan_set_label_device ( helia->label_scan, adapter, frontend, helia->dvb_type );

	for ( c = 0; c < G_N_ELEMENTS ( dvb_descr_delsys_type_n ); c++ )
	{
		if ( g_str_has_suffix ( helia_get_dvb_type_str ( helia->dvb_type ), dvb_descr_delsys_type_n[c].text_vis ) ) num = c;
	}

	g_signal_handler_block   ( helia->combo_delsys, helia->desys_signal_id );
		gtk_combo_box_set_active ( GTK_COMBO_BOX ( helia->combo_delsys ), num );
	g_signal_handler_unblock ( helia->combo_delsys, helia->desys_signal_id );
}

static void helia_scan_set_adapter ( GtkSpinButton *button, Helia *helia )
{
	gtk_spin_button_update ( button );

	uint adapter_set = gtk_spin_button_get_value_as_int ( button );

	GstElement *element = helia_gst_iterate_element ( helia->scan, "dvbsrc", NULL );

	g_object_set ( element, "adapter",  adapter_set, NULL );

	helia_scan_set_new_adapter ( helia );
}

static void helia_scan_set_frontend ( GtkSpinButton *button, Helia *helia )
{
	gtk_spin_button_update ( button );

	uint frontend_set = gtk_spin_button_get_value_as_int ( button );

	GstElement *element = helia_gst_iterate_element ( helia->scan, "dvbsrc", NULL );

	g_object_set ( element, "frontend",  frontend_set, NULL );

	helia_scan_set_new_adapter ( helia );
}

static void helia_scan_set_delsys ( GtkComboBox *combo_box, Helia *helia )
{
	uint frontend = 0, adapter = 0, num = gtk_combo_box_get_active ( combo_box );

	helia->dvb_type = dvb_descr_delsys_type_n[num].descr_num;

	GstElement *element = helia_gst_iterate_element ( helia->scan, "dvbsrc", NULL );

	g_object_get ( element, "adapter",  &adapter,  NULL );
	g_object_get ( element, "frontend", &frontend, NULL );

	helia_set_dvb_delsys ( adapter, frontend, helia->dvb_type );

	g_object_set ( element, "delsys", helia->dvb_type, NULL );

	g_print ( "%s:: delsys %d ( %s ) \n", __func__, helia->dvb_type, helia_get_dvb_type_str ( helia->dvb_type ) );
}

static void helia_convert_file ( const char *file, Helia *helia )
{
	if ( file && g_str_has_suffix ( file, "dvb_channel.conf" ) )
	{
		if ( g_file_test ( file, G_FILE_TEST_EXISTS ) )
			helia_convert_dvb5 ( helia, file );
		else
			helia_message_dialog ( file, g_strerror ( errno ), GTK_MESSAGE_ERROR, helia->window );
	}
	else
	{
		g_warning ( "%s:: no convert %s ", __func__, file );
	}
}

static void helia_convert_set_file ( GtkEntry *entry, G_GNUC_UNUSED GtkEntryIconPosition icon_pos, G_GNUC_UNUSED GdkEventButton *event, Helia *helia )
{
	if ( icon_pos == GTK_ENTRY_ICON_PRIMARY )
	{
		char *file = helia_open_file ( helia, g_get_home_dir () );

		if ( file == NULL ) return;

		gtk_entry_set_text ( entry, file );

		g_free ( file );
	}

	if ( icon_pos == GTK_ENTRY_ICON_SECONDARY )
	{
		const char *file = gtk_entry_get_text ( entry );

		helia_convert_file ( file, helia );
	}
}

static GtkBox * helia_scan_convert ( Helia *helia )
{
	GtkBox *g_box  = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 );

	GtkLabel *label = (GtkLabel *)gtk_label_new ( "DVBv5   ⇨  GstDvbSrc" );
	gtk_box_pack_start ( g_box, GTK_WIDGET ( label ), FALSE, FALSE, 5 );

	GtkEntry *entry = (GtkEntry *)gtk_entry_new ();
	gtk_entry_set_text ( entry, "dvb_channel.conf" );

	g_object_set ( entry, "editable", FALSE, NULL );
	gtk_entry_set_icon_from_icon_name ( entry, GTK_ENTRY_ICON_PRIMARY,   "folder" );
	gtk_entry_set_icon_from_icon_name ( entry, GTK_ENTRY_ICON_SECONDARY, "helia-convert" );
	g_signal_connect ( entry, "icon-press", G_CALLBACK ( helia_convert_set_file ), helia );

	gtk_box_pack_start ( g_box, GTK_WIDGET ( entry ), FALSE, FALSE, 5 );

	return g_box;
}

static GtkBox * helia_scan_device ( Helia *helia )
{
	uint adapter = 0, frontend = 0;

	GstElement *element = helia_gst_iterate_element ( helia->scan, "dvbsrc", NULL );

	g_object_get ( element, "adapter",  &adapter, NULL );
	g_object_get ( element, "frontend", &frontend, NULL );

	helia->dvb_type = helia_get_dvb_delsys ( adapter, frontend );

	g_autofree char *dvb_name = helia_get_dvb_info ( adapter, frontend );

	GtkBox *g_box  = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 );
	gtk_widget_set_margin_start ( GTK_WIDGET ( g_box ), 10 );
	gtk_widget_set_margin_end   ( GTK_WIDGET ( g_box ), 10 );

	GtkGrid *grid = (GtkGrid *)gtk_grid_new();
	gtk_grid_set_column_homogeneous ( GTK_GRID ( grid ), TRUE );
	gtk_grid_set_row_spacing ( grid, 5 );
	gtk_box_pack_start ( g_box, GTK_WIDGET ( grid ), TRUE, TRUE, 10 );

	struct DataDevice { const char *text; uint value; void (*f)(); } data_n[] =
	{
		{ dvb_name,     0, NULL },
		{ "Adapter",    adapter,  	helia_scan_set_adapter  },
		{ "Frontend",   frontend, 	helia_scan_set_frontend },
		{ "DelSys",     0, 			helia_scan_set_delsys   }
	};

	uint d = 0, c = 0, num = 0;
	for ( d = 0; d < G_N_ELEMENTS ( data_n ); d++ )
	{
		GtkLabel *label = (GtkLabel *)gtk_label_new ( data_n[d].text );
		gtk_widget_set_halign ( GTK_WIDGET ( label ), ( d == 0 ) ? GTK_ALIGN_CENTER : GTK_ALIGN_START );
		gtk_grid_attach ( GTK_GRID ( grid ), GTK_WIDGET ( label ), 0, d, ( d == 0 ) ? 2 : 1, 1 );

		if ( d == 0 ) { helia->label_scan = label; continue; }
		if ( d == 3 ) continue;

		GtkSpinButton *spinbutton = (GtkSpinButton *)gtk_spin_button_new_with_range ( 0, 16, 1 );
		gtk_spin_button_set_value ( spinbutton, data_n[d].value );
		g_signal_connect ( spinbutton, "changed", G_CALLBACK ( data_n[d].f ), helia );

		gtk_grid_attach ( GTK_GRID ( grid ), GTK_WIDGET ( spinbutton ), 1, d, 1, 1 );
	}

	helia->combo_delsys = (GtkComboBoxText *) gtk_combo_box_text_new ();

	for ( c = 0; c < G_N_ELEMENTS ( dvb_descr_delsys_type_n ); c++ )
	{
		if ( g_str_has_suffix ( helia_get_dvb_type_str ( helia->dvb_type ), dvb_descr_delsys_type_n[c].text_vis ) ) num = c;

		gtk_combo_box_text_append_text ( helia->combo_delsys, dvb_descr_delsys_type_n[c].text_vis );
	}

	gtk_combo_box_set_active ( GTK_COMBO_BOX ( helia->combo_delsys ), num );
	helia->desys_signal_id = g_signal_connect ( helia->combo_delsys, "changed", G_CALLBACK ( data_n[3].f ), helia );

	gtk_grid_attach ( GTK_GRID ( grid ), GTK_WIDGET ( helia->combo_delsys ), 1, 3, 1, 1 );

	gtk_box_pack_start ( g_box, GTK_WIDGET ( helia_scan_convert ( helia ) ), TRUE, TRUE, 10 );

	return g_box;
}

void helia_scan_stop ( G_GNUC_UNUSED GtkButton *button, Helia *helia )
{
	if ( GST_ELEMENT_CAST ( helia->scan )->current_state == GST_STATE_NULL ) return;

	gst_element_set_state ( helia->scan, GST_STATE_NULL );

	if ( helia->treeview_scan == NULL ) return;

	helia_level_set_sgn_snr ( helia->level_scan, 0, 0, FALSE, FALSE, FALSE, FALSE );

	helia_scan_read_ch_to_treeview ( helia );
}

static gboolean helia_scan_start_time ( Helia *helia )
{
	if ( GST_ELEMENT_CAST ( helia->scan )->current_state == GST_STATE_NULL ) return FALSE;

	if ( helia->scan_count > 9 ) { helia->scan_count = 0; helia_scan_stop ( NULL, helia ); return FALSE; } else helia->scan_count++;

	if ( helia->mpegts->pat_done && helia->mpegts->pmt_done && helia->mpegts->sdt_done )
	{
		helia_scan_stop ( NULL, helia );

		return FALSE;
	}

	return TRUE;
}

static void helia_scan_start ( G_GNUC_UNUSED GtkButton *button, Helia *helia )
{
	if ( GST_ELEMENT_CAST ( helia->scan )->current_state == GST_STATE_PLAYING ) return;

	mpegts_clear ( helia->mpegts );

	gst_element_set_state ( helia->scan, GST_STATE_PLAYING );

	helia->scan_count = 0;
	g_timeout_add_seconds ( 1, (GSourceFunc)helia_scan_start_time, helia );
}

static void helia_scan_create_control_battons ( Helia *helia, GtkBox *b_box )
{
	helia->level_scan = helia_level_new ();
	gtk_box_pack_start ( b_box, GTK_WIDGET ( helia->level_scan ), FALSE, FALSE, 0 );

	GtkBox *hb_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
	gtk_widget_set_margin_start ( GTK_WIDGET ( hb_box ), 5 );
	gtk_widget_set_margin_end   ( GTK_WIDGET ( hb_box ), 5 );
	gtk_box_set_spacing ( hb_box, 5 );

	helia_create_image_button ( hb_box, "helia-play", 16, helia_scan_start, helia );
	helia_create_image_button ( hb_box, "helia-stop", 16, helia_scan_stop,  helia );

	gtk_box_pack_start ( b_box, GTK_WIDGET ( hb_box ), FALSE, FALSE, 5 );
}

static GtkBox * helia_scan_all_box ( Helia *helia, uint i )
{
	GtkBox *only_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 );

	if ( i == PAGE_SC )  { return helia_scan_device   ( helia ); }
	if ( i == PAGE_CH )  { return helia_scan_channels ( helia ); }
	if ( i == PAGE_DT )  { return helia_scan_dvb_all  ( helia, G_N_ELEMENTS ( dvbt_props_n  ), dvbt_props_n,  "DVB-T"  ); }
	if ( i == PAGE_DS )  { return helia_scan_dvb_all  ( helia, G_N_ELEMENTS ( dvbs_props_n  ), dvbs_props_n,  "DVB-S"  ); }
	if ( i == PAGE_DC )  { return helia_scan_dvb_all  ( helia, G_N_ELEMENTS ( dvbc_props_n  ), dvbc_props_n,  "DVB-C"  ); }

	return only_box;
}

static void helia_scan_quit ( G_GNUC_UNUSED GtkWindow *win, Helia *helia )
{
	helia->treeview_scan = NULL;

	helia_scan_stop ( NULL, helia );
}

void helia_scan_win_create ( Helia *helia )
{
	GtkWindow *window = helia_create_window_top ( helia->window, "", "helia-display", GTK_WIN_POS_CENTER_ON_PARENT, TRUE );
	g_signal_connect ( window, "destroy", G_CALLBACK ( helia_scan_quit ), helia );

	GtkBox *m_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_VERTICAL,   0 );
	GtkBox *h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );

	GtkNotebook *notebook = (GtkNotebook *)gtk_notebook_new ();
	gtk_notebook_set_scrollable ( notebook, TRUE );

	gtk_widget_set_margin_top    ( GTK_WIDGET ( notebook ), 5 );
	gtk_widget_set_margin_bottom ( GTK_WIDGET ( notebook ), 5 );
	gtk_widget_set_margin_start  ( GTK_WIDGET ( notebook ), 5 );
	gtk_widget_set_margin_end    ( GTK_WIDGET ( notebook ), 5 );

	GtkBox *m_box_n[PAGE_NUM];

	uint j = 0;
	for ( j = 0; j < PAGE_NUM; j++ )
	{
		m_box_n[j] = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 );
		gtk_box_pack_start ( m_box_n[j], GTK_WIDGET ( helia_scan_all_box ( helia, j ) ), TRUE, TRUE, 0 );
		gtk_notebook_append_page ( notebook, GTK_WIDGET ( m_box_n[j] ),  gtk_label_new ( _i18n_ ( scan_label_n[j].name ) ) );

		if ( j == PAGE_SC ) helia_scan_create_control_battons ( helia, m_box_n[PAGE_SC] );
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

static void helia_scan_msg_all ( G_GNUC_UNUSED GstBus *bus, GstMessage *message, Helia *helia )
{
	if ( helia->treeview_scan == NULL ) return;

	const GstStructure *structure = gst_message_get_structure ( message );

	if ( structure )
	{
		int signal, snr;
		gboolean hlook = FALSE, play = TRUE;

		if (  gst_structure_get_int ( structure, "signal", &signal )  )
		{
			gst_structure_get_boolean ( structure, "lock", &hlook );
			gst_structure_get_int ( structure, "snr", &snr);

			if ( GST_ELEMENT_CAST ( helia->scan )->current_state < GST_STATE_PLAYING ) play = FALSE;

			helia_level_set_sgn_snr ( helia->level_scan, (signal * 100) / 0xffff, (snr * 100) / 0xffff, hlook, play, FALSE, FALSE );
		}
	}

	mpegts_parse_section ( message, helia->mpegts );
}

static void helia_scan_msg_err ( G_GNUC_UNUSED GstBus *bus, GstMessage *msg, Helia *helia )
{
	helia_scan_stop ( NULL, helia );

	GError *err = NULL;
	char  *dbg = NULL;

	gst_message_parse_error ( msg, &err, &dbg );

	g_critical ( "%s:: %s (%s)\n", __func__, err->message, (dbg) ? dbg : "no details" );

	helia_message_dialog ( "", err->message, GTK_MESSAGE_ERROR, helia->window );

	g_error_free ( err );
	g_free ( dbg );
}

static void helia_scan_set_tune_timeout ( GstElement *element, guint64 time_set )
{
	guint64 timeout = 0, timeout_set = 0, timeout_get = 0, timeout_def = 10000000000;

	g_object_get ( element, "tuning-timeout", &timeout, NULL );

	timeout_set = timeout_def / 10 * time_set;

	g_object_set ( element, "tuning-timeout", (guint64)timeout_set, NULL );

	g_object_get ( element, "tuning-timeout", &timeout_get, NULL );

	g_debug ( "scan_set_tune_timeout: timeout %ld | timeout set %ld", timeout, timeout_get );
}
static void helia_scan_set_tune_def ( GstElement *element )
{
	g_object_set ( element, "bandwidth-hz", 8000000,  NULL );
	g_object_set ( element, "modulation",   QAM_AUTO, NULL );
}

static void helia_dvb_init ( Helia *helia )
{
	g_autofree char *dvb_name = helia_get_dvb_info ( 0, 0 );

	helia->dvb_type = helia_get_dvb_delsys ( 0, 0 );

	g_debug ( "DVB device: %s ( %s ) ", dvb_name, helia_get_dvb_type_str ( helia->dvb_type ) );
}

static GstElement * helia_scan_create ( Helia *helia )
{
	mpegts_initialize ();

	if ( g_file_test ( "/dev/dvb/adapter0/frontend0", G_FILE_TEST_EXISTS ) )
		 helia_dvb_init ( helia );

	GstElement *pipeline_scan, *scan_dvbsrc, *scan_tsparse, *scan_filesink;

	pipeline_scan = gst_pipeline_new ( "pipeline-scan" );
	scan_dvbsrc   = gst_element_factory_make ( "dvbsrc",   NULL );
	scan_tsparse  = gst_element_factory_make ( "tsparse",  NULL );
	scan_filesink = gst_element_factory_make ( "fakesink", NULL );

	if ( !pipeline_scan || !scan_dvbsrc || !scan_tsparse || !scan_filesink )
		g_critical ( "%s:: pipeline scan - not be created.\n", __func__ );

	gst_bin_add_many ( GST_BIN ( pipeline_scan ), scan_dvbsrc, scan_tsparse, scan_filesink, NULL );
	gst_element_link_many ( scan_dvbsrc, scan_tsparse, scan_filesink, NULL );

	GstBus *bus_scan = gst_element_get_bus ( pipeline_scan );
	gst_bus_add_signal_watch ( bus_scan );

	g_signal_connect ( bus_scan, "message",          G_CALLBACK ( helia_scan_msg_all ), helia );
	g_signal_connect ( bus_scan, "message::error",   G_CALLBACK ( helia_scan_msg_err ), helia );

	gst_object_unref ( bus_scan );

	helia_scan_set_tune_timeout ( scan_dvbsrc, 5 );
	helia_scan_set_tune_def ( scan_dvbsrc );

	return pipeline_scan;
}

HeliaScan * helia_scan_new ( Helia *helia )
{
	return helia_scan_create ( helia );
}


// ***** Linux Dvb *****

static void helia_set_dvb_delsys_fd ( int fd, uint del_sys )
{
	struct dtv_property dvb_prop[1];
	struct dtv_properties cmdseq;

	dvb_prop[0].cmd = DTV_DELIVERY_SYSTEM;
	dvb_prop[0].u.data = del_sys;

	cmdseq.num = 1;
	cmdseq.props = dvb_prop;

	const char *type = helia_get_dvb_type_str ( del_sys );

	if ( ioctl ( fd, FE_SET_PROPERTY, &cmdseq ) == -1 )
		helia_message_dialog ( g_strerror ( errno ), type, GTK_MESSAGE_ERROR, NULL );
	else
		g_print ( "Set DTV_DELIVERY_SYSTEM - %s Ok \n", type );
}

void helia_set_dvb_delsys ( uint adapter, uint frontend, uint delsys )
{
	int fd = 0, flags = O_RDWR;

	char *fd_name = g_strdup_printf ( "/dev/dvb/adapter%d/frontend%d", adapter, frontend );

	if ( ( fd = g_open ( fd_name, flags ) ) == -1 )
	{
		g_critical ( "%s: %s %s \n", __func__, fd_name, g_strerror ( errno ) );

		g_free  ( fd_name );

		return;
	}

	struct dvb_frontend_info info;

	if ( ( ioctl ( fd, FE_GET_INFO, &info ) ) == -1 )
		perror ( "helia_set_dvb_delsys: ioctl FE_GET_INFO " );
	else
		helia_set_dvb_delsys_fd ( fd, delsys );

	g_close ( fd, NULL );
	g_free  ( fd_name  );
}

static uint helia_get_dvb_delsys_fd ( int fd, struct dvb_frontend_info info )
{
	uint dtv_del_sys = SYS_UNDEFINED, dtv_api_ver = 0, SYS_DVBC = SYS_DVBC_ANNEX_A;

	struct dtv_property dvb_prop[2];
	struct dtv_properties cmdseq;

	dvb_prop[0].cmd = DTV_DELIVERY_SYSTEM;
	dvb_prop[1].cmd = DTV_API_VERSION;

	cmdseq.num = 2;
	cmdseq.props = dvb_prop;

	if ( ( ioctl ( fd, FE_GET_PROPERTY, &cmdseq ) ) == -1 )
	{
		perror ( "helia_get_dvb_delsys_fd: ioctl FE_GET_PROPERTY " );

		dtv_api_ver = 0x300;
		gboolean legacy = FALSE;

		switch ( info.type )
		{
			case FE_QPSK:
				legacy = TRUE;
				dtv_del_sys = SYS_DVBS;
				break;

			case FE_OFDM:
				legacy = TRUE;
				dtv_del_sys = SYS_DVBT;
				break;

			case FE_QAM:
				legacy = TRUE;
				dtv_del_sys = SYS_DVBC;
				break;

			case FE_ATSC:
				legacy = TRUE;
				dtv_del_sys = SYS_ATSC;
				break;

			default:
				break;
		}

		if ( legacy )
			g_debug ( "DVBv3  Ok " );
		else
			g_critical ( "DVBv3  Failed \n" );
	}
	else
	{
		g_debug ( "DVBv5  Ok " );

		dtv_del_sys = dvb_prop[0].u.data;
		dtv_api_ver = dvb_prop[1].u.data;
	}

	g_debug ( "DVB DTV_DELIVERY_SYSTEM: %d | DVB API Version: %d.%d ", dtv_del_sys, dtv_api_ver / 256, dtv_api_ver % 256 );

	return dtv_del_sys;
}

uint helia_get_dvb_delsys ( uint adapter, uint frontend )
{
	uint dtv_delsys = SYS_UNDEFINED;

	int fd = 0, flags = O_RDWR;

	char *fd_name = g_strdup_printf ( "/dev/dvb/adapter%d/frontend%d", adapter, frontend );

	if ( ( fd = g_open ( fd_name, flags ) ) == -1 )
	{
		flags = O_RDONLY;

		if ( ( fd = g_open ( fd_name, flags ) ) == -1 )
		{
			g_critical ( "%s: %s %s \n", __func__, fd_name, g_strerror ( errno ) );

			g_free  ( fd_name );

			return dtv_delsys;
		}
	}

	struct dvb_frontend_info info;

	if ( ( ioctl ( fd, FE_GET_INFO, &info ) ) == -1 )
	{
		perror ( "helia_get_dvb_delsys: ioctl FE_GET_INFO " );
	}
	else
	{
		dtv_delsys = helia_get_dvb_delsys_fd ( fd, info );
	}

	g_close ( fd, NULL );
	g_free  ( fd_name  );

	return dtv_delsys;
}

char * helia_get_dvb_info ( uint adapter, uint frontend )
{
	int fd = 0, flags = O_RDWR;

	g_autofree char *fd_name = g_strdup_printf ( "/dev/dvb/adapter%d/frontend%d", adapter, frontend );

	if ( ( fd = g_open ( fd_name, flags ) ) == -1 )
	{
		flags = O_RDONLY;

		if ( ( fd = g_open ( fd_name, flags ) ) == -1 )
		{
			g_critical ( "%s: %s %s \n", __func__, fd_name, g_strerror ( errno ) );

			return g_strdup ( _i18n_ ( "Undefined" ) );
		}
	}

	struct dvb_frontend_info info;

	if ( ( ioctl ( fd, FE_GET_INFO, &info ) ) == -1 )
	{
		perror ( "helia_get_dvb_info: ioctl FE_GET_INFO " );

		g_close ( fd, NULL );

		return g_strdup ( _i18n_ ( "Undefined" ) );
	}

	g_debug ( "DVB device: %s ( %s ) \n", info.name, fd_name );

	g_close ( fd, NULL );

	return g_strdup ( info.name );
}



// Convert  DVBv5 ⇨ GstDvbSrc

static char * _strip_ch_name_convert ( char *name )
{
	uint i = 0;
	for ( i = 0; name[i] != '\0'; i++ )
	{
		if ( name[i] == ':' || name[i] == '[' || name[i] == ']' ) name[i] = ' ';
	}
	return g_strstrip ( name );
}

static void helia_read_dvb5_data ( Helia *helia, const char *ch_data, uint num, uint adapter, uint frontend, uint delsys )
{
	uint n = 0, z = 0, x = 0;

	char **data = g_strsplit ( ch_data, "\n", 0 );

	if ( data[0] != NULL && *data[0] )
	{
		GString *gstring = g_string_new ( _strip_ch_name_convert ( data[0] ) );
		g_string_append_printf ( gstring, ":delsys=%d:adapter=%d:frontend=%d", delsys, adapter, frontend );

		g_debug ( "Channel ( %d ): %s ", num, data[0] );

		for ( n = 1; data[n] != NULL && *data[n]; n++ )
		{
			char **value_key = g_strsplit ( data[n], " = ", 0 );

			for ( z = 0; z < G_N_ELEMENTS ( gst_param_dvb_descr_n ); z++ )
			{
				if ( g_strrstr ( gst_param_dvb_descr_n[z].dvb_v5_name, g_strstrip ( value_key[0] ) ) )
				{
					g_string_append_printf ( gstring, ":%s=", gst_param_dvb_descr_n[z].gst_param );

					if ( gst_param_dvb_descr_n[z].cdsc == 0 || g_strrstr ( value_key[0], "SAT_NUMBER" ) )
					{
						g_string_append ( gstring, value_key[1] );
					}
					else
					{
						for ( x = 0; x < gst_param_dvb_descr_n[z].cdsc; x++ )
							if ( g_strrstr ( value_key[1], gst_param_dvb_descr_n[z].dvb_descr[x].dvb_v5_name ) )
								g_string_append_printf ( gstring, "%d", gst_param_dvb_descr_n[z].dvb_descr[x].descr_num );
					}

					// g_debug ( "  %s = %s ", gst_param_dvb_descr_n[z].gst_param, value_key[1] );
				}
			}

			g_strfreev ( value_key );
		}

		if ( g_strrstr ( gstring->str, "audio-pid" ) ) // ignore other
			helia_scan_treeview_save_channels ( helia->treeview_tv, data[0], gstring->str );

		g_string_free ( gstring, TRUE );
	}

	g_strfreev ( data );
}

static void helia_convert_dvb5 ( Helia *helia, const char *file )
{
	char *contents;
	GError *err = NULL;

	uint n = 0, adapter = 0, frontend = 0, delsys = 0;
	GstElement *element = helia_gst_iterate_element ( helia->scan, "dvbsrc", NULL );

	g_object_get ( element, "adapter",  &adapter,  NULL );
	g_object_get ( element, "frontend", &frontend, NULL );

	delsys = helia_get_dvb_delsys ( adapter, frontend );

	if ( g_file_get_contents ( file, &contents, 0, &err ) )
	{
		char **lines = g_strsplit ( contents, "[", 0 );
		uint length = g_strv_length ( lines );

		for ( n = 1; n < length; n++ )
			helia_read_dvb5_data ( helia, lines[n], n, adapter, frontend, delsys );

		g_strfreev ( lines );
		g_free ( contents );
	}
	else
	{
		helia_message_dialog ( "", err->message, GTK_MESSAGE_ERROR, helia->window );
		g_error_free ( err );

		return;
	}
}
