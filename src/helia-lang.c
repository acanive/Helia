/*
* Copyright 2019 Stepan Perun
* This program is free software.
*
* License: Gnu Lesser General Public License
* http://www.gnu.org/licenses/lgpl.html
*/


#include <helia-include.h>

#include <glib/gi18n.h>


struct MsgGettext { const char *text; } MsgGettext_n[] =
{
	{ N_( "Audio equalizer" ) },
	{ N_( "Bandwidth Hz" 	) },
	{ N_( "Brightness" 	) },
	{ N_( "Channels" 	) },
	{ N_( "Contrast" 	) },
	{ N_( "Files" 		) },
	{ N_( "Frequency Hz" 	) },
	{ N_( "Hue" 		) },
	{ N_( "Level dB" 	) },
	{ N_( "Saturation" 	) },
	{ N_( "Scanner" 	) },
	{ N_( "Undefined" 	) },
	{ N_( "Video equalizer" ) },
	{ N_( "Only IPTV." ) }
};

const char * _i18n_ ( const char *text )
{
	return _( text );
}
