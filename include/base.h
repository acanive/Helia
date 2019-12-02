/*
* Copyright 2019 Stepan Perun
* This program is free software.
* 
* License: Gnu Lesser General Public License
* http://www.gnu.org/licenses/lgpl.html
*/


#ifndef BASE_H
#define BASE_H


#include <gtk/gtk.h>
#include <gst/gst.h>
#include <time.h>


#define MAX_WIN 4
#define MAX_AUDIO 32
#define MAX_RUN_PAT 128

enum cols_n { COL_NUM, COL_FL_CH, COL_DATA, NUM_COLS };


typedef struct _PatPmtSdtVct PatPmtSdtVct;

struct _PatPmtSdtVct
{
	uint pat_sid, pmt_sid, sdt_sid, vct_sid;
	uint pmt_vpid, pmt_apid;

	char *sdt_name, *vct_name;

	uint ca_sys, ca_pid;
};

typedef struct _MpegTs MpegTs;

struct _MpegTs
{
	gboolean pat_done,  pmt_done,  sdt_done,  vct_done;
	uint     pat_count, pmt_count, sdt_count, vct_count;

	PatPmtSdtVct ppsv[MAX_RUN_PAT];
};

typedef struct _Level Level;

struct _Level
{
	GtkLabel *signal_snr;
	GtkProgressBar *bar_sgn, *bar_snr;
};

typedef struct _Scan Scan;

struct _Scan
{
	GstElement *pipeline_scan, *scan_dvbsrc;

	GtkTreeView *treeview_scan;
	GtkLabel *dvb_device, *all_channels;

	GtkButton *button_lnb_dvb, *button_lnb_isdb;

	GtkComboBoxText *combo_delsys;
	ulong desys_signal_id;

	uint adapter_set, frontend_set, delsys_set, lnb_type;
	uint id_time_source;

	gboolean scan_quit;

	Level level_scan;

	MpegTs mpegts;

	time_t t_cur_scan, t_start_scan;
};

typedef struct _DigitalTV DigitalTV;

struct _DigitalTV
{
	GstElement *dvbplay, *mux_rec;
	guintptr window_hid;

	GtkDrawingArea *video;
	GtkTreeView *treeview;
	GtkEntry *search_entry;

	GtkBox *vbox_sw_tv, *h_box_level_base, *h_box_level_panel;
	GtkVolumeButton *volbutton;

	double volume;
	gboolean checked_video, panel_quit, scrambling;
	gboolean enable_rec, rec_ses, rec_status, rec_pulse;

	Level level_base, level_panel;

	Scan scan;

	char *ch_data;
	char *audio_lang[MAX_AUDIO];

	uint sid, set_audio_track;

	time_t t_cur_tv, t_start_tv, t_cur_ne_tv;
};


typedef struct _Slider Slider;

struct _Slider
{
	GtkScale *slider;
	GtkLabel *lab_pos, *lab_dur;
	GtkLabel *rec_buf, *rec_sts;

	ulong slider_signal_id;
};

typedef struct _Player Player;

struct _Player
{
	GstElement *playbin, *pipeline_rec;
	guintptr window_hid;

	GtkDrawingArea *video;
	GtkTreeView *treeview;
	GtkEntry *search_entry, *net_entry;

	GtkBox *vbox_sw_mp, *h_box_slider_base, *h_box_slider_panel;
	GtkVolumeButton *volbutton;

	double volume;
	gboolean next_repeat, state_subtitle, panel_quit;
	gboolean record, record_f, rec_status, rec_video_enable;
	gboolean double_clicked, vis_plugin, is_live, set_state;

	char *file_play;
	gint64 duration_old;

	Slider slider_base, slider_panel;

	GtkLabel *label_audio, *label_video;

	time_t t_cur_mp, t_start_mp, t_cur_ne;
};

typedef struct _RecEnc RecEnc;

struct _RecEnc
{
	GtkSwitch *gswitch;

	GtkEntry *audio_enc, *audio_prop;
	GtkEntry *video_enc, *video_prop;
	GtkEntry *muxer_enc;

	char *str_audio_enc, *str_video_enc, *str_muxer_enc;
	char *str_audio_prop, *str_video_prop;

	gboolean rec_enc_prop;
};

typedef struct _Base Base;

struct _Base
{
	GtkWindow *window;
	GdkPixbuf *pixbuf_mp, *pixbuf_tv;

	GtkLabel *pref_rec;

	Player *player;

	DigitalTV *dtv;

	RecEnc rec_enc;

	double opacity_panel, opacity_eq, opacity_win;

	uint num_lang, size_icon, win_width, win_height;

	gboolean pause_mouse, dark_theme;
	gboolean info_quit, app_quit;

	char *helia_conf, *rec_dir, *ch_conf;
};


/* Returns a newly-allocated string holding the result. Free with free() */
char * base_uri_get_path ( const char *uri );

void base_set_win_base ( GtkButton *button, Base *base );

GtkImage  * base_create_image     ( const char *icon, uint size );
GtkButton * base_set_image_button ( const char *icon, uint size );
void base_create_image_button ( GtkBox *box, const char *icon, uint size, void (*f)(), Base *base );

void base_message_dialog ( const char *f_error, const char *file_or_info, GtkMessageType mesg_type, GtkWindow *window );


#endif /* BASE_H */
