/*
* Copyright 2019 Stepan Perun
* This program is free software.
* 
* License: Gnu Lesser General Public License
* http://www.gnu.org/licenses/lgpl.html
*/


#include <base.h>

#if USE_GETTEXT

#include <glib/gi18n.h>

struct MsgGettext { const char *text; } MsgGettext_n[] =
{
    { N_( "Audio equalizer" ) },
    { N_( "Bandwidth Hz" 	) },
    { N_( "Brightness" 		) },
    { N_( "Channels" 		) },
    { N_( "Contrast" 		) },
    { N_( "Files" 			) },
    { N_( "Frequency Hz" 	) },
    { N_( "Hue" 			) },
    { N_( "Level dB" 		) },
	{ N_( "Saturation" 		) },
	{ N_( "Scanner" 		) },
	{ N_( "Undefined" 		) },
	{ N_( "Video equalizer" ) }
};

#else

	#include "i18n.h"

#endif

const char * _i18n_ ( G_GNUC_UNUSED Base *base, const char *text )
{

#if USE_GETTEXT

	return _( text );

#else

	if ( base->num_lang == 0 ) return text;

	uint i = 0;

    for ( i = 0; i < langs_n[base->num_lang].num; i++ )
    {
		if ( g_str_has_prefix ( langs_n[base->num_lang].msgidstr[i].msgid, text ) ) return langs_n[base->num_lang].msgidstr[i].msgstr;
	}

	return text;

#endif

}

void lang_add_combo ( GtkComboBoxText *combo )
{

#if USE_GETTEXT

	gtk_combo_box_text_append_text ( combo, "Gettext is used" );

#else

	uint i = 0;

    for ( i = 0; i < G_N_ELEMENTS ( langs_n ); i++ )
    {
		gtk_combo_box_text_append_text ( combo, langs_n[i].lang_name );
	}

#endif

}

uint lang_get_def ()
{

#if USE_GETTEXT

	return 0;

#else

	const gchar *lang = g_getenv ( "LANG" );
	
	g_debug ( "lang_get_def: %s ", lang );

	uint res = 0, i = 0;

    for ( i = 0; i < G_N_ELEMENTS ( langs_n ); i++ )
    {
		if ( g_str_has_prefix ( lang, langs_n[i].lang_sys ) ) return i;
	}

	return res;

#endif

}
