/*
* Copyright 2019 Stepan Perun
* This program is free software.
* 
* License: Gnu Lesser General Public License
* http://www.gnu.org/licenses/lgpl.html
*/


#include "dvbcsa.h"

#include <stdio.h>
#include <glib.h>


typedef unsigned int  uint;
typedef unsigned char uchar;


void sys_get_key ( const char *prog_name, uint prog_num, uchar *ret_cw )
{
	uint n = 0;
	char *contents;
	GError *err = NULL;

	char *cam_key = "/etc/helia/oscam.keys";

	if ( g_file_get_contents ( cam_key, &contents, 0, &err ) )
	{
		char **lines = g_strsplit ( contents, "\n", 0 );

		for ( n = 0; lines[n] != NULL; n++ )
		{
			if ( g_str_has_prefix ( lines[n], "F" ) || g_str_has_prefix ( lines[n], "B" ) )
			{
				if ( g_strrstr ( lines[n], prog_name ) )
				{
					char **line = g_strsplit ( lines[n], " ", 0 );
					
					uint sid = 0;
					sscanf ( line[1], "%4X", &sid );
					
					if ( sid == prog_num )
					{
						uint i = 0, cw[8];

						if ( strlen ( line[3] ) == 16 )
						{
							for ( i = 0; i < 8; i++ )
							{
								sscanf ( line[3] + ( i * 2 ), "%2X", &cw[i] ); ret_cw[i] = cw[i];
							}
							
							g_debug ( "%s ", lines[n] );
							g_debug ( "Channel %s | sid: %4X | prog_num %4X | key: %s | len: %ld ", 
								prog_name, sid, prog_num, line[3], strlen ( line[3] ) );
						}
						else
						{
							g_debug ( "%s ", lines[n] );
							g_critical ( "sys_get_key: len != 8 | strlen: %ld | %s ", strlen ( line[3] ), line[3] );
						}
					
						g_strfreev ( line );
						break;
					}
					
					g_strfreev ( line );
				}
			}
		}
		
		g_strfreev ( lines );
		g_free ( contents );
	}
	else
	{
		g_critical ( "Cam key:: %s\n", err->message );
		g_error_free ( err );
	}
}

void sys_set_key ( uchar *cw, uchar *data, uint size )
{
	struct dvbcsa_key_s *key = dvbcsa_key_alloc ();

		dvbcsa_key_set ( cw, key );

		dvbcsa_decrypt ( key, data, size );

	dvbcsa_key_free ( key );
}
