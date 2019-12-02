/*
* Copyright 2019 Stepan Perun
* This program is free software.
*
* License: Gnu Lesser General Public License
* http://www.gnu.org/licenses/lgpl.html
*/


#include <helia-level.h>


G_DEFINE_TYPE ( HeliaLevel, helia_level, GTK_TYPE_BOX )


void helia_level_set_sgn_snr ( HeliaLevel *level, gdouble sgl, gdouble snr, gboolean hlook, 
								gboolean play, gboolean rec, gboolean scrambling )
{
	gtk_progress_bar_set_fraction ( level->bar_sgn, sgl/100 );
	gtk_progress_bar_set_fraction ( level->bar_snr, snr/100 );

	char *texta = g_strdup_printf ( "Sgn %d%s", (int)sgl, "%" );
	char *textb = g_strdup_printf ( "Snr %d%s", (int)snr, "%" );

	const char *format = NULL;
	static gboolean pulse_level = FALSE;

	if ( hlook )
		format = "%s<span foreground=\"#00ff00\"> ◉ </span>%s<span foreground=\"#FF0000\"> %s</span> %s";
	else
		format = "%s<span foreground=\"#ff0000\"> ◉ </span>%s<span foreground=\"#FF0000\"> %s</span> %s";

	if ( !play )
		format = "%s<span foreground=\"#ff8000\"> ◉ </span>%s<span foreground=\"#FF0000\"> %s</span> %s";

	if ( sgl == 0 && snr == 0 )
		format = "%s<span foreground=\"#bfbfbf\"> ◉ </span>%s<span foreground=\"#FF0000\"> %s</span> %s";

	char *markup = g_markup_printf_escaped ( format, texta, textb, ( rec ) ? ( pulse_level ) ? " ◉ " : " ◌ " : "",
					scrambling ? play ? " 🔓 " : " 🔒 " : "" );

		gtk_label_set_markup ( level->sgn_snr, markup );

	g_free ( markup );

	g_free ( texta );
	g_free ( textb );

	pulse_level = !pulse_level;
}

static void helia_level_init ( HeliaLevel *level )
{
	GtkBox *blv = GTK_BOX ( level );

	gtk_orientable_set_orientation ( GTK_ORIENTABLE ( blv ), GTK_ORIENTATION_VERTICAL );

	gtk_widget_set_margin_start ( GTK_WIDGET ( blv ), 5 );
	gtk_widget_set_margin_end   ( GTK_WIDGET ( blv ), 5 );

	level->sgn_snr = (GtkLabel *)gtk_label_new ( "Signal  &  Quality" );

	level->bar_sgn = (GtkProgressBar *)gtk_progress_bar_new ();
	level->bar_snr = (GtkProgressBar *)gtk_progress_bar_new ();

	gtk_box_pack_start ( blv, GTK_WIDGET ( level->sgn_snr ), FALSE, FALSE, 5 );
	gtk_box_pack_start ( blv, GTK_WIDGET ( level->bar_sgn ), FALSE, FALSE, 0 );
	gtk_box_pack_start ( blv, GTK_WIDGET ( level->bar_snr ), FALSE, FALSE, 3 );
}

static void helia_level_class_init ( G_GNUC_UNUSED HeliaLevelClass *klass )
{

}

HeliaLevel * helia_level_new (void)
{
	return g_object_new ( helia_level_get_type (), NULL );
}

