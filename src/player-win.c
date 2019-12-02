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
#include "player-panel.h"
#include "player-gst.h"


static void player_win_draw_black ( GtkDrawingArea *widget, cairo_t *cr, GdkPixbuf *logo )
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

static gboolean player_win_draw_check ( Base *base )
{
    if ( GST_ELEMENT_CAST ( base->player->playbin )->current_state == GST_STATE_NULL ) return TRUE;

	if ( base->player->vis_plugin ) return FALSE;

    uint n_video = 0;
    g_object_get ( base->player->playbin, "n-video", &n_video, NULL );

    if ( n_video == 0 )
    {
        if ( GST_ELEMENT_CAST ( base->player->playbin )->current_state == GST_STATE_PLAYING ||
             GST_ELEMENT_CAST ( base->player->playbin )->current_state == GST_STATE_PAUSED     )
            return TRUE;
    }

    return FALSE;
}

static gboolean player_win_draw ( GtkDrawingArea *widget, cairo_t *cr, Base *base )
{
	if ( player_win_draw_check ( base ) ) player_win_draw_black ( widget, cr, base->pixbuf_mp );

	return TRUE;
}

static void player_win_realize ( GtkDrawingArea *drawingarea, Base *base )
{
    ulong xid = GDK_WINDOW_XID ( gtk_widget_get_window ( GTK_WIDGET ( drawingarea ) ) );

    base->player->window_hid = xid;

    g_print ( "GDK_WINDOW_XID: %ld \n", base->player->window_hid );
}

static void player_win_drag_in ( G_GNUC_UNUSED GtkDrawingArea *draw, GdkDragContext *ct, G_GNUC_UNUSED int x, G_GNUC_UNUSED int y, 
							  GtkSelectionData *s_data, G_GNUC_UNUSED uint info, guint32 time, Base *base )
{
	char **uris = gtk_selection_data_get_uris ( s_data );

	uint d = 0, c = 0;

	for ( d = 0; uris[d] != NULL; d++ );;

	for ( c = 0; uris[c] != NULL; c++ )
	{
		char *path = base_uri_get_path ( uris[c] );

		if ( !path ) { g_debug ( "player_win_drag_in: file NULL \n" ); free ( path ); continue; }

		if ( g_file_test ( path, G_FILE_TEST_IS_DIR ) )
		{
			treeview_add_dir ( base, path );
		}

		if ( g_file_test ( path, G_FILE_TEST_IS_REGULAR ) )
		{
			treeview_add_file ( base, path, ( d == 1 ) ? FALSE : TRUE, ( c == 0 ) ? TRUE : FALSE );
		}

		free ( path );
	}

	g_strfreev ( uris );

	gtk_drag_finish ( ct, TRUE, FALSE, time );
}

static gboolean player_win_fullscreen ( GtkWindow *window )
{
	GdkWindowState state = gdk_window_get_state ( gtk_widget_get_window ( GTK_WIDGET ( window ) ) );

	if ( state & GDK_WINDOW_STATE_FULLSCREEN )
		{ gtk_window_unfullscreen ( window ); return FALSE; }
	else
		{ gtk_window_fullscreen   ( window ); return TRUE;  }

	return TRUE;
}

static gboolean player_check_double_clicked ( Base *base )
{
	if ( !base->player->double_clicked )
		player_play_paused ( base );

	g_debug ( "%s: %s ", __func__, base->player->double_clicked ? "true" : "false" );

	return FALSE;
}

static gboolean player_win_press_event ( GtkDrawingArea *drawing, GdkEventButton *event, Base *base )
{
	if ( event->button == 1 && event->type == GDK_2BUTTON_PRESS )
	{
		GtkWindow *window = GTK_WINDOW ( gtk_widget_get_toplevel ( GTK_WIDGET ( drawing ) ) );

		if ( player_win_fullscreen ( GTK_WINDOW ( window ) ) )
		{
			gtk_widget_hide ( GTK_WIDGET ( base->player->vbox_sw_mp ) );

			gtk_widget_hide ( GTK_WIDGET ( base->player->h_box_slider_base ) );
		}

		base->player->double_clicked = TRUE;

		return TRUE;
	}

	if ( event->button == 2 )
	{
		if ( gtk_widget_get_visible ( GTK_WIDGET ( base->player->vbox_sw_mp ) ) )
		{
			gtk_widget_hide ( GTK_WIDGET ( base->player->vbox_sw_mp ) );

			gtk_widget_hide ( GTK_WIDGET ( base->player->h_box_slider_base ) );
		}
		else
		{
			gtk_widget_show ( GTK_WIDGET ( base->player->vbox_sw_mp ) );

			gtk_widget_show ( GTK_WIDGET ( base->player->h_box_slider_base ) );
		}

		return TRUE;
	}

	if ( event->button == 3 )
	{
		GtkWindow *window = GTK_WINDOW ( gtk_widget_get_toplevel ( GTK_WIDGET ( drawing ) ) );

		player_panel_win_create ( window, base );

		return TRUE;
	}

	if ( base->pause_mouse )
	{
		base->player->double_clicked = FALSE;

		if ( event->button == 1 )
			g_timeout_add ( 250, (GSourceFunc)player_check_double_clicked, base );
	}

	return TRUE;
}

static gboolean player_win_scroll_event ( G_GNUC_UNUSED GtkDrawingArea *widget, GdkEventScroll *evscroll, Base *base )
{
	if ( GST_ELEMENT_CAST ( base->player->playbin )->current_state == GST_STATE_NULL ) return TRUE;

	gdouble skip = 20;

	gboolean up_dwn = TRUE;

	if ( evscroll->direction == GDK_SCROLL_DOWN ) up_dwn = FALSE;

	if ( evscroll->direction == GDK_SCROLL_UP   ) up_dwn = TRUE;

	if ( evscroll->direction == GDK_SCROLL_DOWN || evscroll->direction == GDK_SCROLL_UP )
		player_gst_new_pos ( base, skip, up_dwn );

	return TRUE;
}

GtkDrawingArea * player_win_create ( Base *base )
{
	GtkDrawingArea *video_win = (GtkDrawingArea *)gtk_drawing_area_new ();
	g_signal_connect ( video_win, "realize", G_CALLBACK ( player_win_realize ), base );
	g_signal_connect ( video_win, "draw",    G_CALLBACK ( player_win_draw    ), base );

	gtk_widget_set_events ( GTK_WIDGET ( video_win ), GDK_BUTTON_PRESS_MASK | GDK_SCROLL_MASK  );
	g_signal_connect ( video_win, "button-press-event", G_CALLBACK ( player_win_press_event  ), base );
	g_signal_connect ( video_win, "scroll-event",       G_CALLBACK ( player_win_scroll_event ), base );

	gtk_drag_dest_set ( GTK_WIDGET ( video_win ), GTK_DEST_DEFAULT_ALL, NULL, 0, GDK_ACTION_COPY );
	gtk_drag_dest_add_uri_targets  ( GTK_WIDGET ( video_win ) );
	g_signal_connect  ( video_win, "drag-data-received", G_CALLBACK ( player_win_drag_in ), base );

	return video_win;
}

static void player_win_entry_changed ( G_GNUC_UNUSED GtkEntry *entry, Base *base )
{
	GtkTreeModel *model_filter = gtk_tree_view_get_model ( GTK_TREE_VIEW ( base->player->treeview ) );

	gtk_tree_model_filter_refilter ( GTK_TREE_MODEL_FILTER ( model_filter ) );
}

GtkPaned * player_win_paned_create ( Base *base, uint set_size )
{
	base->player->vbox_sw_mp = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 );

	base->player->search_entry = (GtkEntry *)gtk_search_entry_new ();

	gtk_widget_set_margin_start ( GTK_WIDGET ( base->player->search_entry ), 5 );
	gtk_widget_set_margin_end   ( GTK_WIDGET ( base->player->search_entry ), 5 );

	g_signal_connect ( base->player->search_entry, "changed", G_CALLBACK ( player_win_entry_changed ), base );

	gtk_box_pack_start ( base->player->vbox_sw_mp, GTK_WIDGET ( base->player->search_entry ), FALSE,  FALSE, 5 );

	GtkScrolledWindow *scroll = create_scroll_win ( base->player->treeview, set_size );

	GtkBox *b_box = treeview_box ( base, base->window, base->player->treeview, base->player->vbox_sw_mp, TRUE );

	gtk_box_pack_start ( base->player->vbox_sw_mp, GTK_WIDGET ( scroll ), TRUE,  TRUE,  0 );
	gtk_box_pack_end   ( base->player->vbox_sw_mp, GTK_WIDGET ( b_box  ), FALSE, FALSE, 0 );

	base->player->video = player_win_create ( base );

	GtkPaned *paned = (GtkPaned *)gtk_paned_new ( GTK_ORIENTATION_HORIZONTAL );
	gtk_paned_add1 ( paned, GTK_WIDGET ( base->player->vbox_sw_mp ) );
	gtk_paned_add2 ( paned, GTK_WIDGET ( base->player->video ) );

	return paned;
}
