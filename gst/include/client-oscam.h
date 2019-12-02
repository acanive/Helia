/*
* Copyright 2019 Stepan Perun
* This program is free software.
* 
* License: Gnu Lesser General Public License
* http://www.gnu.org/licenses/lgpl.html
*/


#ifndef CLIENT_OSCAM_H
#define CLIENT_OSCAM_H


void client_oscam_start ();
void client_oscam_stop  ();
void demux_data ( const char *name, uint sid, unsigned char *data, uint len );


#endif /* CLIENT_OSCAM_H */
