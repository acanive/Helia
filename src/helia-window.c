/*
* Copyright 2019 Stepan Perun
* This program is free software.
*
* License: Gnu Lesser General Public License
* http://www.gnu.org/licenses/lgpl.html
*/


#include <helia-include.h>


static void helia_about_win ( GtkWindow *window )
{
	GtkAboutDialog *dialog = (GtkAboutDialog *)gtk_about_dialog_new ();
	gtk_window_set_transient_for ( GTK_WINDOW ( dialog ), window );

	GdkPixbuf *pixbuf = gtk_icon_theme_load_icon ( gtk_icon_theme_get_default (), 
				"helia-info", 48, GTK_ICON_LOOKUP_USE_BUILTIN, NULL );

	gtk_window_set_icon ( GTK_WINDOW ( dialog ), pixbuf );

	if ( pixbuf ) g_object_unref ( pixbuf );

	const char *authors[]   = { "Stepan Perun", " ", NULL };
	const char *artists[]   = { "Itzik Gur",    " ", NULL };
	const char *translators = "Anton Midyukov \nHeimen Stoffels \nPavel Fric \nMartin Gansser \n";
	const char *license     = "This program is free software. \n\nGNU Lesser General Public License \nwww.gnu.org/licenses/lgpl.html";

	gtk_about_dialog_set_program_name ( dialog, "Helia" );
	gtk_about_dialog_set_version ( dialog, "10.0" );
	gtk_about_dialog_set_license ( dialog, license );
	gtk_about_dialog_set_authors ( dialog, authors );
	gtk_about_dialog_set_artists ( dialog, artists );
	gtk_about_dialog_set_translator_credits ( dialog, translators );
	gtk_about_dialog_set_website ( dialog,   "https://github.com/vl-nix/helia" );
	gtk_about_dialog_set_copyright ( dialog, "Copyright 2019 Helia" );
	gtk_about_dialog_set_comments  ( dialog, "Media Player & IPTV & Digital TV \nDVB-T2/S2/C" );

	gtk_about_dialog_set_logo_icon_name ( dialog, "helia-logo" );

	gtk_dialog_run ( GTK_DIALOG (dialog) );

	gtk_widget_destroy ( GTK_WIDGET (dialog) );
}

GtkWindow * helia_create_window_top ( GtkWindow *base_window, const char *title, const char *icon, uint pos, gboolean modal )
{
	GtkWindow *window = (GtkWindow *)gtk_window_new ( GTK_WINDOW_TOPLEVEL );

	gtk_window_set_title ( window, title );
	gtk_window_set_modal ( window, modal );
	gtk_window_set_transient_for ( window, base_window );
	gtk_window_set_position  ( window, pos );

	GdkPixbuf *pixbuf = gtk_icon_theme_load_icon ( gtk_icon_theme_get_default (), 
				icon, 48, GTK_ICON_LOOKUP_USE_BUILTIN, NULL );

	gtk_window_set_icon ( window, pixbuf );

	if ( pixbuf ) g_object_unref ( pixbuf );

	return window;
}

void helia_message_dialog ( const char *f_error, const char *file_or_info, GtkMessageType mesg_type, GtkWindow *window )
{
	GtkMessageDialog *dialog = ( GtkMessageDialog *)gtk_message_dialog_new (
					window,    GTK_DIALOG_MODAL,
					mesg_type, GTK_BUTTONS_CLOSE,
					"%s\n%s",  f_error, file_or_info );

	gtk_dialog_run     ( GTK_DIALOG ( dialog ) );
	gtk_widget_destroy ( GTK_WIDGET ( dialog ) );
}

void helia_window_set_win_base ( G_GNUC_UNUSED GtkButton *button, Helia *helia )
{
	gtk_widget_show ( GTK_WIDGET ( helia->bs_vbox ) );
	gtk_widget_hide ( GTK_WIDGET ( helia->mp_vbox ) );
	gtk_widget_hide ( GTK_WIDGET ( helia->tv_vbox ) );

	GtkWindow *window = GTK_WINDOW ( gtk_widget_get_toplevel ( GTK_WIDGET ( helia->bs_vbox ) ) );
	gtk_window_set_title ( window, "Helia" );
}

void helia_window_set_win_mp ( G_GNUC_UNUSED GtkButton *button, Helia *helia )
{
	gtk_widget_hide ( GTK_WIDGET ( helia->bs_vbox ) );
	gtk_widget_hide ( GTK_WIDGET ( helia->tv_vbox ) );
	gtk_widget_show ( GTK_WIDGET ( helia->mp_vbox ) );

	GtkWindow *window = GTK_WINDOW ( gtk_widget_get_toplevel ( GTK_WIDGET ( helia->bs_vbox ) ) );
	gtk_window_set_title ( window, "Helia - Media Player");
}

static void helia_window_set_win_tv ( G_GNUC_UNUSED GtkButton *button, Helia *helia )
{
	gtk_widget_hide ( GTK_WIDGET ( helia->bs_vbox ) );
	gtk_widget_hide ( GTK_WIDGET ( helia->mp_vbox ) );
	gtk_widget_show ( GTK_WIDGET ( helia->tv_vbox ) );

	GtkWindow *window = GTK_WINDOW ( gtk_widget_get_toplevel ( GTK_WIDGET ( helia->bs_vbox ) ) );
	gtk_window_set_title ( window, "Helia - Digital TV" );
}

GtkImage * helia_create_image ( const char *icon, uint size )
{
	GdkPixbuf *pixbuf = gtk_icon_theme_load_icon ( gtk_icon_theme_get_default (), 
				icon, size, GTK_ICON_LOOKUP_USE_BUILTIN, NULL );

	GtkImage *image   = (GtkImage *)gtk_image_new_from_pixbuf ( pixbuf );
	gtk_image_set_pixel_size ( image, size );

	if ( pixbuf ) g_object_unref ( pixbuf );

	return image;
}

GtkButton * helia_set_image_button ( const char *icon, uint size )
{
	GtkButton *button = (GtkButton *)gtk_button_new ();

	GtkImage *image = helia_create_image ( icon, size );

	gtk_button_set_image ( button, GTK_WIDGET ( image ) );

	return button;
}

void helia_create_image_button ( GtkBox *box, const char *icon, uint size, void (*f)(), Helia *helia )
{
	GtkButton *button = helia_set_image_button ( icon, size );

	if ( f ) g_signal_connect ( button, "clicked", G_CALLBACK (f), helia );

	gtk_box_pack_start ( box, GTK_WIDGET ( button ), TRUE, TRUE, 0 );
}

static void base_create_image_flip_button ( GtkBox *box, const char *icon, uint size, void (*f)(), Helia *helia )
{
	GtkButton *button = (GtkButton *)gtk_button_new ();

	GdkPixbuf *pixbuf = gtk_icon_theme_load_icon ( gtk_icon_theme_get_default (), 
				icon, size, GTK_ICON_LOOKUP_USE_BUILTIN, NULL );

	GdkPixbuf *pixbuf_flip = gdk_pixbuf_flip ( pixbuf, TRUE );

	GtkImage *image   = (GtkImage *)gtk_image_new_from_pixbuf ( pixbuf_flip );
	gtk_image_set_pixel_size ( image, size );

	if ( pixbuf ) g_object_unref ( pixbuf );
	if ( pixbuf ) g_object_unref ( pixbuf_flip );

	gtk_button_set_image ( button, GTK_WIDGET ( image ) );

	if ( f ) g_signal_connect ( button, "clicked", G_CALLBACK (f), helia );

	gtk_box_pack_start ( box, GTK_WIDGET ( button ), TRUE, TRUE, 0 );
}

static GtkPaned * helia_window_create_paned ( Helia *helia, gboolean mp_tv )
{
	GtkPaned *paned = (GtkPaned *)gtk_paned_new ( GTK_ORIENTATION_HORIZONTAL );
	gtk_paned_add1 ( paned, GTK_WIDGET ( ( mp_tv ) ? helia->list_mp  : helia->list_tv  ) );
	gtk_paned_add2 ( paned, GTK_WIDGET ( ( mp_tv ) ? helia->video_mp : helia->video_tv ) );

	return paned;
}

static void helia_window_create_player ( GtkBox *box, Helia *helia )
{
	gtk_box_pack_start ( box, GTK_WIDGET ( helia_window_create_paned ( helia, TRUE ) ), TRUE, TRUE, 0 );

	gtk_box_pack_end   ( box, GTK_WIDGET ( helia->slider ), FALSE, FALSE, 0 );
}

static void helia_window_create_dtv ( GtkBox *box, Helia *helia )
{
	gtk_box_pack_start ( box, GTK_WIDGET ( helia_window_create_paned ( helia, FALSE ) ), TRUE, TRUE, 0 );
}

static void helia_app_quit ( Helia *helia )
{
	gst_element_set_state ( helia->dtv,    GST_STATE_NULL );
	gst_element_set_state ( helia->player, GST_STATE_NULL );

	helia_pref_save_config ( helia );
	helia_treeview_auto_save_tv ( helia->treeview_tv, helia->ch_conf );

	g_debug ( "%s:: ", __func__ );
}

static void helia_window_quit ( G_GNUC_UNUSED GtkWindow *window, Helia *helia )
{
	helia->window = NULL;

	helia_app_quit ( helia );
}

static void helia_window_close ( G_GNUC_UNUSED GtkButton *button, Helia *helia )
{
	helia_app_quit ( helia );

	g_application_quit ( G_APPLICATION ( helia ) );
}

static void helia_window_about ( G_GNUC_UNUSED GtkButton *button, Helia *helia )
{
	helia_about_win ( helia->window );
}

static void helia_window_pref ( G_GNUC_UNUSED GtkButton *button, Helia *helia )
{
	helia_pref_win ( helia );
}

static void helia_window_keyb ( G_GNUC_UNUSED GtkButton *button, Helia *helia )
{
	helia_keyb_win ( helia );
}

HeliaWindow * helia_window_new ( Helia *helia )
{
	helia_create_gaction_entry ( GTK_APPLICATION (helia), helia );

	GtkWindow *window = (GtkWindow *)gtk_application_window_new ( GTK_APPLICATION (helia) );
	gtk_window_set_title ( window, "Helia");
	gtk_window_set_default_size ( window, helia->win_width, helia->win_height );
	g_signal_connect ( window, "destroy", G_CALLBACK ( helia_window_quit ), helia );

	GdkPixbuf *pixbuf = gtk_icon_theme_load_icon ( gtk_icon_theme_get_default (), 
				"helia-tv", 48, GTK_ICON_LOOKUP_USE_BUILTIN, NULL );

	gtk_window_set_icon ( window, pixbuf );

	if ( pixbuf ) g_object_unref ( pixbuf );

	GtkBox *mn_vbox = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 );

	helia->bs_vbox = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 );
	helia->tv_vbox = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 );
	helia->mp_vbox = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 );

	gtk_container_set_border_width ( GTK_CONTAINER ( helia->bs_vbox ), 25 );

	gtk_box_set_spacing ( mn_vbox, 10 );
	gtk_box_set_spacing ( helia->bs_vbox, 10 );

	GtkBox *bt_hbox = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
	gtk_box_set_spacing ( bt_hbox,  10 );

	uint size_b = 256, size_c = 48;

	if ( helia->win_height < 400 ) { size_b = 192; size_c = 40; }
	if ( helia->win_height < 300 ) { size_b = 128; size_c = 32; }
	if ( helia->win_height < 200 ) { size_b = 64;  size_c = 24; }

	helia_create_image_button     ( bt_hbox, "helia-mp", size_b, helia_window_set_win_mp, helia );
	base_create_image_flip_button ( bt_hbox, "helia-tv", size_b, helia_window_set_win_tv, helia );

	gtk_box_pack_start ( helia->bs_vbox, GTK_WIDGET ( bt_hbox ), TRUE,  TRUE,  0 );

	GtkBox *bc_hbox = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
	gtk_box_set_spacing ( bc_hbox,  10 );

	helia_create_image_button ( bc_hbox, "helia-pref", size_c, helia_window_pref,  helia );
	helia_create_image_button ( bc_hbox, "helia-keyb", size_c, helia_window_keyb,  helia );
	helia_create_image_button ( bc_hbox, "helia-info", size_c, helia_window_about, helia );
	helia_create_image_button ( bc_hbox, "helia-quit", size_c, helia_window_close, helia );

	gtk_box_pack_start ( helia->bs_vbox, GTK_WIDGET ( bc_hbox ), FALSE, FALSE, 0 );

	gtk_box_pack_start ( mn_vbox, GTK_WIDGET ( helia->bs_vbox ), TRUE,  TRUE,  0 );

	helia_window_create_player ( helia->mp_vbox, helia );
	helia_window_create_dtv    ( helia->tv_vbox, helia );

	gtk_box_pack_start ( mn_vbox, GTK_WIDGET ( helia->mp_vbox ), TRUE, TRUE, 0 );
	gtk_box_pack_start ( mn_vbox, GTK_WIDGET ( helia->tv_vbox ), TRUE, TRUE, 0 );

	gtk_container_add   ( GTK_CONTAINER ( window ), GTK_WIDGET ( mn_vbox ) );
	gtk_widget_show_all ( GTK_WIDGET ( window ) );

	gtk_widget_hide ( GTK_WIDGET ( helia->mp_vbox ) );
	gtk_widget_hide ( GTK_WIDGET ( helia->tv_vbox ) );

	gtk_widget_hide ( GTK_WIDGET ( helia->search_entry_mp ) );
	gtk_widget_hide ( GTK_WIDGET ( helia->search_entry_tv ) );

	gtk_window_resize ( window, helia->win_width, helia->win_height );

	gtk_widget_set_opacity ( GTK_WIDGET ( window ), helia->opacity_window );

	return (HeliaWindow *)window;
}

