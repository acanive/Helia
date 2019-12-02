/*
* Copyright 2019 Stepan Perun
* This program is free software.
*
* License: Gnu Lesser General Public License
* http://www.gnu.org/licenses/lgpl.html
*/


#include <helia-slider.h>


G_DEFINE_TYPE ( HeliaSlider, helia_slider, GTK_TYPE_BOX )


static void helia_slider_label_set_text ( GtkLabel *label, gint64 pos_dur, uint digits )
{
	char *str   = g_strdup_printf ( "%" GST_TIME_FORMAT, GST_TIME_ARGS ( pos_dur ) );
	char *str_l = g_strndup ( str, strlen ( str ) - digits );

		gtk_label_set_text ( label, str_l );

	free ( str_l );
	free ( str   );
}

void helia_slider_set_data ( HeliaSlider *hsl, gint64 pos, uint digits_pos, gint64 dur, uint digits_dur, gboolean sensitive )
{
	helia_slider_label_set_text ( hsl->lab_pos, pos, digits_pos );

	if ( dur > -1 ) helia_slider_label_set_text ( hsl->lab_dur, dur, digits_dur );

	gtk_widget_set_sensitive ( GTK_WIDGET ( hsl ), sensitive );
}

void helia_slider_update ( HeliaSlider *hsl, gdouble range, gdouble value )
{
	g_signal_handler_block   ( hsl->slider, hsl->slider_signal_id );

		gtk_range_set_range  ( GTK_RANGE ( hsl->slider ), 0, range );
		gtk_range_set_value  ( GTK_RANGE ( hsl->slider ),    value );

	g_signal_handler_unblock ( hsl->slider, hsl->slider_signal_id );
}

void helia_slider_clear_all ( HeliaSlider *hsl )
{
	helia_slider_update ( hsl, 120*60, 0 );

	gtk_label_set_text ( hsl->lab_pos, "0:00:00" );
	gtk_label_set_text ( hsl->lab_dur, "0:00:00" );

	gtk_widget_set_sensitive ( GTK_WIDGET ( hsl ), FALSE );
}

static void helia_slider_init ( HeliaSlider *hsl )
{
	gtk_orientable_set_orientation ( GTK_ORIENTABLE ( hsl ), GTK_ORIENTATION_HORIZONTAL );

	hsl->lab_pos = (GtkLabel *)gtk_label_new ( "0:00:00" );
	hsl->lab_dur = (GtkLabel *)gtk_label_new ( "0:00:00" );

	hsl->slider  = (GtkScale *)gtk_scale_new_with_range ( GTK_ORIENTATION_HORIZONTAL, 0, 120*60, 1 );

	gtk_scale_set_draw_value ( hsl->slider, 0 );
	gtk_range_set_value ( GTK_RANGE ( hsl->slider ), 0 );

	gtk_widget_set_margin_start ( GTK_WIDGET ( GTK_BOX ( hsl ) ), 10 );
	gtk_widget_set_margin_end   ( GTK_WIDGET ( GTK_BOX ( hsl ) ), 10 );
	gtk_box_set_spacing ( GTK_BOX ( hsl ), 5 );

	gtk_box_pack_start ( GTK_BOX ( hsl ), GTK_WIDGET ( hsl->lab_pos ), FALSE, FALSE, 0 );
	gtk_box_pack_start ( GTK_BOX ( hsl ), GTK_WIDGET ( hsl->slider  ), TRUE,  TRUE,  0 );
	gtk_box_pack_start ( GTK_BOX ( hsl ), GTK_WIDGET ( hsl->lab_dur ), FALSE, FALSE, 0 );

	gtk_widget_set_sensitive ( GTK_WIDGET ( hsl ), FALSE );
}

static void helia_slider_class_init ( G_GNUC_UNUSED HeliaSliderClass *klass )
{
	
}

HeliaSlider * helia_slider_new (void)
{
	return g_object_new ( helia_slider_get_type (), NULL );
}

