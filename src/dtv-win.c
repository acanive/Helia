/*
* Copyright 2019 Stepan Perun
* This program is free software.
* 
* License: Gnu Lesser General Public License
* http://www.gnu.org/licenses/lgpl.html
*/


#include <gdk/gdk.h>
#include <gdk/gdkx.h>

#include <base.h>

#include "tree-view.h"
#include "dtv-panel.h"
#include "dtv-level.h"
#include "scan.h"


static void dtv_win_draw_black ( GtkDrawingArea *widget, cairo_t *cr, GdkPixbuf *logo )
{
	GdkRGBA color; color.red = 0; color.green = 0; color.blue = 0; color.alpha = 1.0;

	int width = gtk_widget_get_allocated_width  ( GTK_WIDGET ( widget ) );
	int heigh = gtk_widget_get_allocated_height ( GTK_WIDGET ( widget ) );

	cairo_rectangle ( cr, 0, 0, width, heigh );

	gdk_cairo_set_source_rgba ( cr, &color );

	cairo_fill (cr);

	if ( logo != NULL )
	{
		int widthl  = gdk_pixbuf_get_width  ( logo );
		int heightl = gdk_pixbuf_get_height ( logo );

		cairo_rectangle ( cr, 0, 0, width, heigh );

		gdk_cairo_set_source_pixbuf ( cr, logo,
				( width / 2  ) - ( widthl  / 2 ), ( heigh / 2 ) - ( heightl / 2 ) );

		cairo_fill (cr);
	}
}

static gboolean dtv_win_draw_check ( GstElement *element, Base *base )
{
	if ( GST_ELEMENT_CAST ( element )->current_state == GST_STATE_PAUSED && base->dtv->checked_video ) return FALSE;

	if ( GST_ELEMENT_CAST ( element )->current_state < GST_STATE_PLAYING || !base->dtv->checked_video ) return TRUE;

    return FALSE;
}

static gboolean dtv_win_draw ( GtkDrawingArea *widget, cairo_t *cr, Base *base )
{
	if ( dtv_win_draw_check ( base->dtv->dvbplay, base ) ) dtv_win_draw_black ( widget, cr, base->pixbuf_tv );

	return FALSE;
}

static void dtv_win_realize ( GtkDrawingArea *drawingarea, Base *base )
{
    ulong xid = GDK_WINDOW_XID ( gtk_widget_get_window ( GTK_WIDGET ( drawingarea ) ) );

    base->dtv->window_hid = xid;

    g_debug ( "GDK_WINDOW_XID: %ld ", base->dtv->window_hid );
}

static void dtv_win_drag_in ( G_GNUC_UNUSED GtkDrawingArea *draw, GdkDragContext *ct, G_GNUC_UNUSED int x, G_GNUC_UNUSED int y, 
							  GtkSelectionData *s_data, G_GNUC_UNUSED uint info, guint32 time, Base *base )
{
	char **uris = gtk_selection_data_get_uris ( s_data );

	uint c = 0;

	for ( c = 0; uris[c] != NULL; c++ )
	{
		char *path = base_uri_get_path ( uris[c] );

		if ( g_str_has_suffix ( path, "gtv-channel.conf" ) )
		{
			treeview_add_dtv ( base, path );
		}

		if ( g_str_has_suffix ( path, "dvb_channel.conf" ) )
		{
			helia_convert_dvb5 ( base, path );
		}

		free ( path );
	}

	g_strfreev ( uris );

	gtk_drag_finish ( ct, TRUE, FALSE, time );
}

static gboolean dtv_win_fullscreen ( GtkWindow *window )
{
	GdkWindowState state = gdk_window_get_state ( gtk_widget_get_window ( GTK_WIDGET ( window ) ) );

	if ( state & GDK_WINDOW_STATE_FULLSCREEN )
		{ gtk_window_unfullscreen ( window ); return FALSE; }
	else
		{ gtk_window_fullscreen   ( window ); return TRUE;  }

	return TRUE;
}

static gboolean dtv_win_press_event ( GtkDrawingArea *drawing, GdkEventButton *event, Base *base )
{
	if ( event->button == 1 && event->type == GDK_2BUTTON_PRESS )
	{
		GtkWindow *window = GTK_WINDOW ( gtk_widget_get_toplevel ( GTK_WIDGET ( drawing ) ) );

		if ( dtv_win_fullscreen ( GTK_WINDOW ( window ) ) )
			gtk_widget_hide ( GTK_WIDGET ( base->dtv->vbox_sw_tv ) );

		return TRUE;
	}

	if ( event->button == 2 )
	{
		if ( gtk_widget_get_visible ( GTK_WIDGET ( base->dtv->vbox_sw_tv ) ) )
			gtk_widget_hide ( GTK_WIDGET ( base->dtv->vbox_sw_tv ) );
		else
			gtk_widget_show ( GTK_WIDGET ( base->dtv->vbox_sw_tv ) );

		return TRUE;
	}

	if ( event->button == 3 )
	{
		GtkWindow *window = GTK_WINDOW ( gtk_widget_get_toplevel ( GTK_WIDGET ( drawing ) ) );

		dtv_panel_win_create ( window, base );

		return TRUE;
	}

	return TRUE;
}

static gboolean dtv_win_notify_event ( GtkDrawingArea *drawing, G_GNUC_UNUSED GdkEvent *event, Base *base )
{
	time ( &base->dtv->t_cur_ne_tv );

	gdk_window_set_cursor ( gtk_widget_get_window ( GTK_WIDGET ( drawing ) ), 
							gdk_cursor_new_for_display ( gdk_display_get_default (), GDK_ARROW ) );

	return TRUE;
}

static gboolean dtv_set_cursor ( Base *base )
{
	if ( base->app_quit ) return FALSE;

	if ( !gtk_window_is_active ( base->window ) ) return TRUE;

	if ( !base->dtv->panel_quit || !base->dtv->checked_video ) return TRUE;

	if ( GST_ELEMENT_CAST ( base->dtv->dvbplay )->current_state != GST_STATE_PLAYING ) return TRUE;

	GdkWindow *window = gtk_widget_get_window ( GTK_WIDGET ( base->dtv->video ) );
	GdkCursor *cursor = gdk_window_get_cursor ( window );

	if ( cursor == NULL ) return TRUE;

	time_t t_new_ne;
	time ( &t_new_ne );

	if ( ( t_new_ne - base->dtv->t_cur_ne_tv ) < 3 ) return TRUE;

	if ( gdk_cursor_get_cursor_type ( cursor ) != GDK_BLANK_CURSOR )
		gdk_window_set_cursor ( window, gdk_cursor_new_for_display ( gdk_display_get_default (), GDK_BLANK_CURSOR ) );

	return TRUE;
}

GtkDrawingArea * dtv_win_create ( Base *base )
{
	GtkDrawingArea *video_win = (GtkDrawingArea *)gtk_drawing_area_new ();
	g_signal_connect ( video_win, "realize", G_CALLBACK ( dtv_win_realize ), base );
	g_signal_connect ( video_win, "draw",    G_CALLBACK ( dtv_win_draw    ), base );

	gtk_widget_set_events ( GTK_WIDGET ( video_win ), GDK_BUTTON_PRESS_MASK | GDK_SCROLL_MASK | GDK_POINTER_MOTION_MASK );
	g_signal_connect ( video_win, "button-press-event",  G_CALLBACK ( dtv_win_press_event  ), base );
	g_signal_connect ( video_win, "motion-notify-event", G_CALLBACK ( dtv_win_notify_event ), base );

	gtk_drag_dest_set ( GTK_WIDGET ( video_win ), GTK_DEST_DEFAULT_ALL, NULL, 0, GDK_ACTION_COPY );
	gtk_drag_dest_add_uri_targets  ( GTK_WIDGET ( video_win ) );
	g_signal_connect  ( video_win, "drag-data-received", G_CALLBACK ( dtv_win_drag_in ), base );

	g_timeout_add_seconds ( 1, (GSourceFunc)dtv_set_cursor, base );

	return video_win;
}

static void dtv_win_entry_changed ( G_GNUC_UNUSED GtkEntry *entry, Base *base )
{
	GtkTreeModel *model_filter = gtk_tree_view_get_model ( GTK_TREE_VIEW ( base->dtv->treeview ) );

	gtk_tree_model_filter_refilter ( GTK_TREE_MODEL_FILTER ( model_filter ) );
}

GtkPaned * dtv_win_paned_create ( Base *base, uint set_size )
{
	base->dtv->vbox_sw_tv = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 );

	base->dtv->search_entry = (GtkEntry *)gtk_search_entry_new ();

	gtk_widget_set_margin_start ( GTK_WIDGET ( base->dtv->search_entry ), 5 );
	gtk_widget_set_margin_end   ( GTK_WIDGET ( base->dtv->search_entry ), 5 );

	g_signal_connect ( base->dtv->search_entry, "changed", G_CALLBACK ( dtv_win_entry_changed ), base );

	gtk_box_pack_start ( base->dtv->vbox_sw_tv, GTK_WIDGET ( base->dtv->search_entry ), FALSE,  FALSE, 5 );

	GtkScrolledWindow *scroll = create_scroll_win ( base->dtv->treeview, set_size );

	GtkBox *b_box = treeview_box ( base, base->window, base->dtv->treeview, base->dtv->vbox_sw_tv, FALSE );

	gtk_box_pack_start ( base->dtv->vbox_sw_tv, GTK_WIDGET ( scroll ), TRUE,  TRUE,  0 );
	gtk_box_pack_end   ( base->dtv->vbox_sw_tv, GTK_WIDGET ( b_box  ), FALSE, FALSE, 0 );

	base->dtv->h_box_level_base = dtv_level_base_create ( base );
	gtk_box_pack_end ( base->dtv->vbox_sw_tv, GTK_WIDGET ( base->dtv->h_box_level_base ), FALSE,  FALSE,  0 );

	base->dtv->video = dtv_win_create ( base );

	GtkPaned *paned = (GtkPaned *)gtk_paned_new ( GTK_ORIENTATION_HORIZONTAL );
	gtk_paned_add1 ( paned, GTK_WIDGET ( base->dtv->vbox_sw_tv ) );
	gtk_paned_add2 ( paned, GTK_WIDGET ( base->dtv->video ) );

	return paned;
}
