/*
* Copyright 2019 Stepan Perun
* This program is free software.
*
* License: Gnu Lesser General Public License
* http://www.gnu.org/licenses/lgpl.html
*/


#ifndef HELIA_H
#define HELIA_H

#include <gtk/gtk.h>
#include <gst/gst.h>

#include <helia-level.h>
#include <helia-slider.h>
#include <helia-mpegts.h>


#define MAX_AUDIO 32


#define HELIA_TYPE_APPLICATION                      ( helia_get_type () )

G_DECLARE_FINAL_TYPE ( Helia, helia, HELIA, APPLICATION, GtkApplication )


typedef GtkBox HeliaList;
typedef GtkWindow HeliaWindow;
typedef GtkDrawingArea HeliaVideo;
typedef GtkTreeView HeliaTreeview;

typedef GstElement HeliaTv;
typedef GstElement HeliaScan;
typedef GstElement HeliaPlayer;


typedef struct _Helia Helia;

struct _HeliaClass
{
	GtkApplicationClass parent_class;
};

struct _Helia
{
	GtkApplication parent_instance;

	HeliaWindow *window;

	HeliaVideo *video_mp;
	HeliaVideo *video_tv;

	HeliaTreeview *treeview_mp;
	HeliaTreeview *treeview_tv;

	HeliaList *list_mp;
	HeliaList *list_tv;

	HeliaTv *dtv;
	HeliaPlayer *player;
	HeliaPlayer *pipeline_rec;

	HeliaLevel *level;
	HeliaSlider *slider;

	HeliaLevel *level_panel;
	HeliaSlider *slider_panel;

	HeliaScan *scan;
	HeliaLevel *level_scan;
	HeliaTreeview *treeview_scan;

	MpegTs *mpegts;

	uint cookie;
	gboolean power;
	GDBusConnection *connect;

	GtkBox *bs_vbox;
	GtkBox *mp_vbox;
	GtkBox *tv_vbox;
	GtkBox *rec_vbox;

	GtkLabel *label_buf;
	GtkLabel *label_rec;
	GtkLabel *label_audio;
	GtkLabel *label_video;
	GtkLabel *label_scan;

	GtkVolumeButton *volbutton_mp;
	GtkVolumeButton *volbutton_tv;

	GtkEntry *search_entry_mp;
	GtkEntry *search_entry_tv;
	GtkEntry *net_entry;

	GtkComboBoxText *combo_delsys;
	ulong desys_signal_id;

	uint icon_size;
	ulong xid_mp, xid_tv;

	uint sid;
	uint dvb_type;
	uint lnb_type;
	uint win_width;
	uint win_height;
	uint set_audio_track;
	uint rec_count;
	uint scan_count;

	double volume_mp;
	double volume_tv;
	double opacity_eqav;
	double opacity_panel;
	double opacity_window;

	gboolean record_tv;
	gboolean rec_enc_tv;
	gboolean repeat;
	gboolean dark_theme;
	gboolean checked_video;
	gboolean state_subtitle;
	gboolean double_click;
	gboolean state_mouse_click;
	gboolean panel_ext;
	gboolean scrambling;

	char *file_ch;
	char *rec_dir;
	const char *ch_conf;
	const char *helia_conf;

	char *str_audio_enc;
	char *str_video_enc;
	char *str_muxer_enc;
	char *str_audio_prop;
	char *str_video_prop;

	char *audio_lang[MAX_AUDIO];
};

Helia *helia_new (void);

void helia_power_manager ( Helia *helia, gboolean power_off );


#endif
