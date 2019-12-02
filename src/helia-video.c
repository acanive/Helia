/*
* Copyright 2019 Stepan Perun
* This program is free software.
*
* License: Gnu Lesser General Public License
* http://www.gnu.org/licenses/lgpl.html
*/


#include <helia-include.h>

#include <gdk/gdk.h>
#include <gdk/gdkx.h>


static void helia_video_draw_black ( GtkDrawingArea *widget, cairo_t *cr, const char *name, int size )
{
	GdkRGBA color; color.red = 0; color.green = 0; color.blue = 0; color.alpha = 1.0;

	int width = gtk_widget_get_allocated_width  ( GTK_WIDGET ( widget ) );
	int heigh = gtk_widget_get_allocated_height ( GTK_WIDGET ( widget ) );

	cairo_rectangle ( cr, 0, 0, width, heigh );

	gdk_cairo_set_source_rgba ( cr, &color );

	cairo_fill (cr);

	GdkPixbuf *pixbuf = gtk_icon_theme_load_icon ( gtk_icon_theme_get_default (), 
				name, size, GTK_ICON_LOOKUP_USE_BUILTIN, NULL );

	if ( pixbuf != NULL )
	{
		cairo_rectangle ( cr, 0, 0, width, heigh );

		gdk_cairo_set_source_pixbuf ( cr, pixbuf,
			( width / 2  ) - ( 128  / 2 ), ( heigh / 2 ) - ( 128 / 2 ) );

		cairo_fill (cr);
	}

	if ( pixbuf ) g_object_unref ( pixbuf );
}

static gboolean helia_video_draw_check_mp ( Helia *helia )
{
	if ( helia->pipeline_rec )
	{
		if ( GST_ELEMENT_CAST ( helia->pipeline_rec )->current_state < GST_STATE_PAUSED ) return TRUE;

		GstElement *element = helia_gst_iterate_element ( helia->pipeline_rec, "videobalance", NULL );

		if ( element == NULL ) return TRUE;
	}
	else
	{
		if ( GST_ELEMENT_CAST ( helia->player )->current_state < GST_STATE_PAUSED ) return TRUE;

		uint n_video = 0;
		g_object_get ( helia->player, "n-video", &n_video, NULL );

		GstElement *element = helia_gst_iterate_element ( helia->player, "goom", NULL );

		if ( !n_video && !element ) return TRUE;
	}

	return FALSE;
}
static gboolean helia_video_draw_mp ( GtkDrawingArea *widget, cairo_t *cr, Helia *helia )
{
	if ( helia_video_draw_check_mp ( helia ) ) helia_video_draw_black ( widget, cr, "helia-mp", 128 );

	return FALSE;
}

static gboolean helia_video_draw_check_tv ( Helia *helia )
{
	if ( GST_ELEMENT_CAST ( helia->dtv )->current_state < GST_STATE_PLAYING || !helia->checked_video ) return TRUE;

	return FALSE;
}
static gboolean helia_video_draw_tv ( GtkDrawingArea *widget, cairo_t *cr, Helia *helia )
{
	if ( helia_video_draw_check_tv ( helia ) ) helia_video_draw_black ( widget, cr, "helia-tv", 128 );

	return FALSE;
}

static void helia_video_realize_mp ( GtkDrawingArea *draw, Helia *helia )
{
	helia->xid_mp = GDK_WINDOW_XID ( gtk_widget_get_window ( GTK_WIDGET ( draw ) ) );

	g_debug ( "GDK_WINDOW_XID: %ld ", helia->xid_mp );
}

static void helia_video_realize_tv ( GtkDrawingArea *draw, Helia *helia )
{
	helia->xid_tv = GDK_WINDOW_XID ( gtk_widget_get_window ( GTK_WIDGET ( draw ) ) );

	g_debug ( "GDK_WINDOW_XID: %ld ", helia->xid_tv );
}

static gboolean helia_video_fullscreen ( GtkWindow *window )
{
	GdkWindowState state = gdk_window_get_state ( gtk_widget_get_window ( GTK_WIDGET ( window ) ) );

	if ( state & GDK_WINDOW_STATE_FULLSCREEN )
		{ gtk_window_unfullscreen ( window ); return FALSE; }
	else
		{ gtk_window_fullscreen   ( window ); return TRUE;  }

	return TRUE;
}

static gboolean helia_video_check_double_clicked ( Helia *helia )
{
	if ( helia->file_ch == NULL ) return FALSE;

	if ( helia->pipeline_rec != NULL ) return FALSE;

	if ( !helia->double_click )
	{
		if ( GST_ELEMENT_CAST ( helia->player )->current_state == GST_STATE_PLAYING )
			gst_element_set_state ( helia->player, GST_STATE_PAUSED );
		else
			gst_element_set_state ( helia->player, GST_STATE_PLAYING );
	}

	g_debug ( "%s: %s ", __func__, helia->double_click ? "true" : "false" );

	return FALSE;
}

static void helia_video_press_event ( GtkWindow *window, GdkEventButton *event, gboolean mp_tv, HeliaList *list, Helia *helia )
{
	if ( event->button == 1 )
	{
		if ( event->type == GDK_2BUTTON_PRESS )
		{
			gboolean play = TRUE;

			helia->double_click = TRUE;

			if ( mp_tv )
			{
				if ( GST_ELEMENT_CAST ( helia->player )->current_state < GST_STATE_PAUSED ) play = FALSE;
				if ( helia->pipeline_rec && GST_ELEMENT_CAST ( helia->pipeline_rec )->current_state < GST_STATE_PLAYING ) play = FALSE;
			}
			else
				if ( GST_ELEMENT_CAST ( helia->dtv )->current_state < GST_STATE_PLAYING ) play = FALSE;

			if ( play )
			if ( helia_video_fullscreen ( GTK_WINDOW ( window ) ) )
			{
				gtk_widget_hide ( GTK_WIDGET ( list ) );
				if ( mp_tv ) gtk_widget_hide ( GTK_WIDGET ( helia->slider ) );
			}
		}
		else
		{
			helia->double_click = FALSE;

			if ( helia->state_mouse_click && mp_tv ) g_timeout_add ( 250, (GSourceFunc)helia_video_check_double_clicked, helia );
		}

		return;
	}

	if ( event->button == 2 )
	{
		if ( gtk_widget_get_visible ( GTK_WIDGET ( list ) ) )
		{
			gtk_widget_hide ( GTK_WIDGET ( list ) );
			if ( mp_tv ) gtk_widget_hide ( GTK_WIDGET ( helia->slider ) );
		}
		else
		{
			gtk_widget_show ( GTK_WIDGET ( list ) );
			if ( mp_tv ) gtk_widget_show ( GTK_WIDGET ( helia->slider ) );
		}

		return;
	}

	if ( event->button == 3 )
	{
		helia_panel_new ( helia, mp_tv );

		return;
	}
}

static gboolean helia_video_press_event_mp ( GtkDrawingArea *draw, GdkEventButton *event, Helia *helia )
{
	GtkWindow *window = GTK_WINDOW ( gtk_widget_get_toplevel ( GTK_WIDGET ( draw ) ) );

	helia_video_press_event ( window, event, TRUE, helia->list_mp, helia );

	return TRUE;
}

static gboolean helia_video_press_event_tv ( GtkDrawingArea *draw, GdkEventButton *event, Helia *helia )
{
	GtkWindow *window = GTK_WINDOW ( gtk_widget_get_toplevel ( GTK_WIDGET ( draw ) ) );

	helia_video_press_event ( window, event, FALSE, helia->list_tv, helia );

	return TRUE;
}

static gboolean helia_video_scroll_even_mp ( G_GNUC_UNUSED GtkDrawingArea *widget, GdkEventScroll *evscroll, Helia *helia )
{
	if ( GST_ELEMENT_CAST ( helia->player )->current_state == GST_STATE_NULL ) return TRUE;

	gdouble skip = 20;

	gboolean up_dwn = TRUE;

	if ( evscroll->direction == GDK_SCROLL_DOWN ) up_dwn = FALSE;

	if ( evscroll->direction == GDK_SCROLL_UP   ) up_dwn = TRUE;

	if ( evscroll->direction == GDK_SCROLL_DOWN || evscroll->direction == GDK_SCROLL_UP )
		helia_player_video_scroll_new_pos ( helia, skip, up_dwn );

	return TRUE;
}

static void helia_video_drag_in_mp ( G_GNUC_UNUSED GtkDrawingArea *draw, GdkDragContext *ct, G_GNUC_UNUSED int x, G_GNUC_UNUSED int y, 
									 GtkSelectionData *s_data, G_GNUC_UNUSED uint info, guint32 time, Helia *helia )
{
	char **uris = gtk_selection_data_get_uris ( s_data );

	uint c = 0;

	for ( c = 0; uris[c] != NULL; c++ )
	{
		char *path = helia_uri_get_path ( uris[c] );

		if ( path == NULL ) { g_debug ( "%s: file NULL", __func__ ); continue; }

		if ( g_file_test ( path, G_FILE_TEST_IS_DIR ) )
		{
			helia_treeview_add_dir ( helia, path );
		}

		if ( g_file_test ( path, G_FILE_TEST_IS_REGULAR ) )
		{
			helia_treeview_add_file ( helia, path, ( c == 0 ) ? TRUE : FALSE );
		}

		free ( path );
	}

	g_strfreev ( uris );

	gtk_drag_finish ( ct, TRUE, FALSE, time );
}

static void helia_video_drag_in_tv ( G_GNUC_UNUSED GtkDrawingArea *draw, GdkDragContext *ct, G_GNUC_UNUSED int x, G_GNUC_UNUSED int y, 
									 GtkSelectionData *s_data, G_GNUC_UNUSED uint info, guint32 time, Helia *helia )
{
	char **uris = gtk_selection_data_get_uris ( s_data );

	uint c = 0;

	for ( c = 0; uris[c] != NULL; c++ )
	{
		char *path = helia_uri_get_path ( uris[c] );

		if ( path == NULL ) { g_debug ( "%s: file NULL", __func__ ); continue; }

		if ( g_str_has_suffix ( path, "gtv-channel.conf" ) )
		{
			helia_treeview_add_channels ( helia, path );
		}

		free ( path );
	}

	g_strfreev ( uris );

	gtk_drag_finish ( ct, TRUE, FALSE, time );
}

static void helia_video_notify_event ( GtkDrawingArea *drawing )
{
	gdk_window_set_cursor ( gtk_widget_get_window ( GTK_WIDGET ( drawing ) ), 
							gdk_cursor_new_for_display ( gdk_display_get_default (), GDK_ARROW ) );
}

static gboolean helia_video_notify_event_mp ( GtkDrawingArea *drawing, G_GNUC_UNUSED GdkEventMotion *event, G_GNUC_UNUSED Helia *helia )
{
	helia_video_notify_event ( drawing );

	return TRUE;
}

static gboolean helia_video_notify_event_tv ( GtkDrawingArea *drawing, G_GNUC_UNUSED GdkEventMotion *event, G_GNUC_UNUSED Helia *helia )
{
	helia_video_notify_event ( drawing );

	return TRUE;
}

static void helia_video_set_cursor_blank ( HeliaVideo *video, uint enum_cursor )
{
	gdk_window_set_cursor ( gtk_widget_get_window ( GTK_WIDGET ( video ) ), 
							gdk_cursor_new_for_display ( gdk_display_get_default (), enum_cursor ) );
}

static void helia_video_set_cursor_mp ( Helia *helia, uint enum_cursor )
{
	if ( helia->pipeline_rec )
	{
		if ( GST_ELEMENT_CAST ( helia->pipeline_rec )->current_state < GST_STATE_PAUSED ) return;

		GstElement *element = helia_gst_iterate_element ( helia->pipeline_rec, "videobalance", NULL );

		if ( element == NULL ) return;
	}
	else
	{
		if ( GST_ELEMENT_CAST ( helia->player )->current_state < GST_STATE_PAUSED ) return;

		uint n_video = 0;
		g_object_get ( helia->player, "n-video", &n_video, NULL );

		GstElement *element = helia_gst_iterate_element ( helia->player, "goom", NULL );

		if ( !n_video && !element ) return;
	}

	helia_video_set_cursor_blank ( helia->video_mp, enum_cursor );
}

static void helia_video_set_cursor_tv ( Helia *helia, uint enum_cursor )
{
	if ( GST_ELEMENT_CAST ( helia->dtv )->current_state != GST_STATE_PLAYING || !helia->checked_video ) return;

	helia_video_set_cursor_blank ( helia->video_tv, enum_cursor );
}

static gboolean helia_video_set_cursor ( Helia *helia )
{
	if ( helia->window == NULL ) return FALSE;

	uint enum_cursor = GDK_BLANK_CURSOR;

	if ( !gtk_window_is_active ( helia->window ) ) enum_cursor = GDK_ARROW;

	if ( gtk_widget_get_visible ( GTK_WIDGET ( helia->mp_vbox ) ) ) helia_video_set_cursor_mp ( helia, enum_cursor );

	if ( gtk_widget_get_visible ( GTK_WIDGET ( helia->tv_vbox ) ) ) helia_video_set_cursor_tv ( helia, enum_cursor );

	return TRUE;
}

HeliaVideo * helia_video_new ( Helia *helia, gboolean mp_tv )
{
	GtkDrawingArea *video = (GtkDrawingArea *)gtk_drawing_area_new ();

	g_signal_connect ( video, "draw",    G_CALLBACK ( ( mp_tv ) ? helia_video_draw_mp    : helia_video_draw_tv    ), helia );
	g_signal_connect ( video, "realize", G_CALLBACK ( ( mp_tv ) ? helia_video_realize_mp : helia_video_realize_tv ), helia );

	gtk_widget_set_events ( GTK_WIDGET ( video ), GDK_BUTTON_PRESS_MASK | GDK_SCROLL_MASK | GDK_POINTER_MOTION_MASK );

	if ( mp_tv ) g_signal_connect ( video, "scroll-event", G_CALLBACK ( helia_video_scroll_even_mp ), helia );

	g_signal_connect ( video, "button-press-event",  G_CALLBACK ( ( mp_tv ) ? helia_video_press_event_mp  : helia_video_press_event_tv  ), helia );
	g_signal_connect ( video, "motion-notify-event", G_CALLBACK ( ( mp_tv ) ? helia_video_notify_event_mp : helia_video_notify_event_tv ), helia );

	gtk_drag_dest_set ( GTK_WIDGET ( video ), GTK_DEST_DEFAULT_ALL, NULL, 0, GDK_ACTION_COPY );
	gtk_drag_dest_add_uri_targets  ( GTK_WIDGET ( video ) );
	g_signal_connect  ( video, "drag-data-received", G_CALLBACK ( ( mp_tv ) ? helia_video_drag_in_mp : helia_video_drag_in_tv ), helia );

	g_timeout_add_seconds ( 1, (GSourceFunc)helia_video_set_cursor, helia );

	return (HeliaVideo *)video;
}
