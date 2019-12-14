/*
* Copyright 2019 Stepan Perun
* This program is free software.
*
* License: Gnu Lesser General Public License
* http://www.gnu.org/licenses/lgpl.html
*/


#ifndef HELIA_INCLUDE_H
#define HELIA_INCLUDE_H

#include <helia.h>


enum cols_n
{
	COL_NUM,
	COL_FL_CH,
	COL_DATA,
	NUM_COLS
};

typedef struct _IconFunc IconFunc;

struct _IconFunc
{
	const char *name;
	gboolean swap_close;
	void (* f)();
};


// Window
HeliaWindow * helia_window_new ( Helia *helia );
GtkWindow * helia_create_window_top ( GtkWindow *base_window, const char *title, const char *icon, uint pos, gboolean modal );

void helia_window_set_win_mp   ( GtkButton *button, Helia *helia );
void helia_window_set_win_tv   ( GtkButton *button, Helia *helia );
void helia_window_set_win_base ( GtkButton *button, Helia *helia );

GtkImage * helia_create_image ( const char *icon, uint size );
GtkButton * helia_set_image_button ( const char *icon, uint size );
void helia_create_image_button ( GtkBox *box, const char *icon, uint size, void (*f)(), Helia *helia );
void helia_message_dialog ( const char *f_error, const char *file_or_info, GtkMessageType mesg_type, GtkWindow *window );


// Gettext
const char * _i18n_ ( const char *text );


// Keyboard
void helia_create_gaction_entry ( GtkApplication *app, Helia *helia );
void helia_keyb_win ( Helia *helia );


// Location
void helia_open_net ( Helia *helia );
void helia_dialog_open_dir ( Helia *helia );
void helia_dialog_open_files ( Helia *helia );
char * helia_open_file ( Helia *helia, const char *path );
char * helia_open_dir  ( Helia *helia, const char *path );


// Preference
void helia_pref_win ( Helia *helia );
void helia_pref_read_config ( Helia *helia );
void helia_pref_save_config ( Helia *helia );


// Treeview
HeliaTreeview * helia_treeview_new ( Helia *helia, gboolean mp_tv );

char * helia_uri_get_path ( const char *uri ); // Returns a newly-allocated string holding the result. Free with free()
void helia_treeview_add_channels ( Helia *helia, const char *file );
void helia_treeview_add_start ( GFile **files, int n_files, Helia *helia );
void helia_treeview_add_dir ( Helia *helia, const char *dir_path );
void helia_treeview_add_file ( Helia *helia, const char *path, gboolean play );

void helia_treeview_next_play ( Helia *helia );


// void helia_dialog_open_dir ( Helia *helia );
void helia_treeview_win_save_mp ( GtkTreeView *tree_view, GtkWindow *window );
void helia_treeview_win_save_tv ( GtkTreeView *tree_view, GtkWindow *window );
void helia_treeview_auto_save_tv ( GtkTreeView *tree_view, const char *filename );


// Panel Treeview
GtkBox * helia_panel_treeview_new ( GtkTreeView *tree_view );


// Video
HeliaVideo * helia_video_new ( Helia *helia, gboolean mp_tv );


// PlayList
HeliaList * helia_list_new ( Helia *helia, gboolean mp_tv );

GtkScrolledWindow * helia_create_scroll ( HeliaTreeview *tree_view, uint set_size );


// Panel
void helia_panel_new ( Helia *helia, gboolean mp_tv );


// Info
void helia_info_new ( Helia *helia, gboolean mp_tv );


// Gst
HeliaTv * helia_dtv_new ( Helia *helia );
HeliaPlayer * helia_player_new ( Helia *helia );

void helia_player_stop ( Helia *helia );
void helia_player_step_frame ( Helia *helia );
void helia_player_stop_set_play ( Helia *helia, const char *name_file );
void helia_player_set_subtitle ( Helia *helia, gboolean state_subtitle );
void helia_player_set_visualizer ( Helia *helia, gboolean state_visualizer );

void helia_dtv_stop ( Helia *helia );
void helia_player_record ( Helia *helia );
void helia_dtv_stop_set_play ( Helia *helia, const char *data );

void helia_player_slider_update ( Helia *helia );
gboolean helia_player_slider_refresh ( Helia *helia );
void helia_player_slider_seek_changed ( GtkRange *range, Helia *helia );
void helia_player_slider_panel_seek_changed ( GtkRange *range, Helia *helia );

void helia_player_video_scroll_new_pos ( Helia *helia, gint64 set_pos, gboolean up_dwn );
GstElement * helia_gst_iterate_element ( GstElement *it_element, const char *name1, const char *name2 );

void helia_dtv_gst_record ( Helia *helia );
void helia_dtv_add_audio_track ( Helia *helia, GtkComboBoxText *combo );
void helia_dtv_changed_audio_track ( Helia *helia, int changed_track_audio );

void mpegts_pmt_lang_section ( GstMessage *message, uint sid, char *audio_lang[] );

gboolean helia_gst_mute_get_tv ( HeliaTv *dtv );
gboolean helia_gst_mute_get_mp ( Helia *helia );
void helia_gst_set_mute_mp ( Helia *helia );
void helia_gst_set_mute_tv ( Helia *helia );
void helia_gst_set_volume_mp ( gdouble value, Helia *helia );
void helia_gst_set_volume_tv ( gdouble value, Helia *helia );


// Scan
HeliaScan * helia_scan_new ( Helia *helia );
void helia_scan_win_create ( Helia *helia );

const char * helia_get_dvb_type_str ( int delsys );
char * helia_get_dvb_info ( uint adapter, uint frontend );
uint helia_get_dvb_delsys ( uint adapter, uint frontend );
void helia_set_dvb_delsys ( uint adapter, uint frontend, uint delsys );
void helia_set_lnb_low_high_switch ( GstElement *element, int type_lnb );

const char * helia_scan_get_info ( const char *data );
const char * helia_scan_get_info_descr_vis ( const char *data, int num );

void mpegts_parse_section ( GstMessage *message, MpegTs *mpegts );


// EQ
void helia_eqa_win ( GstElement *element, Helia *helia );
void helia_eqv_win ( GstElement *element, Helia *helia );


#endif
