/*
* Copyright 2019 Stepan Perun
* This program is free software.
*
* License: Gnu Lesser General Public License
* http://www.gnu.org/licenses/lgpl.html
*/


#ifndef HELIA_SLIDER_H
#define HELIA_SLIDER_H

#include <gtk/gtk.h>
#include <gst/gst.h>

#define HELIA_TYPE_SLIDER ( helia_slider_get_type() )

G_DECLARE_FINAL_TYPE ( HeliaSlider, helia_slider, HELIA, SLIDER, GtkBox )


typedef struct _HeliaSlider HeliaSlider;

struct _HeliaSlider
{
	GtkBox parent;

	GtkScale *slider;
	GtkLabel *lab_pos;
	GtkLabel *lab_dur;

	ulong slider_signal_id;
};

struct _HeliaSliderClass
{
	GtkBoxClass parent_class;
};


HeliaSlider * helia_slider_new (void);

void helia_slider_clear_all ( HeliaSlider *hsl );

void helia_slider_update ( HeliaSlider *hsl, gdouble range, gdouble value );

void helia_slider_set_data ( HeliaSlider *hsl, gint64 pos, uint digits_pos, gint64 dur, uint digits_dur, gboolean sensitive );


#endif
