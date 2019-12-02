/*
* Copyright 2019 Stepan Perun
* This program is free software.
*
* License: Gnu Lesser General Public License
* http://www.gnu.org/licenses/lgpl.html
*/


#ifndef HELIA_LEVEL_H
#define HELIA_LEVEL_H

#include <gtk/gtk.h>

#define HELIA_TYPE_LEVEL ( helia_level_get_type() )

G_DECLARE_FINAL_TYPE ( HeliaLevel, helia_level, HELIA, LEVEL, GtkBox )


typedef struct _HeliaLevel HeliaLevel;

struct _HeliaLevel
{
	GtkBox parent;

	GtkLabel *sgn_snr;
	GtkProgressBar *bar_sgn;
	GtkProgressBar *bar_snr;
};

struct _HeliaLevelClass
{
	GtkBoxClass parent_class;
};


HeliaLevel * helia_level_new (void);

void helia_level_set_sgn_snr ( HeliaLevel *level, gdouble sgl, gdouble snr, gboolean hlook, gboolean play, gboolean rec, gboolean scrmb );


#endif
