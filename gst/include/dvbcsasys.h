/*
* Copyright 2019 Stepan Perun
* This program is free software.
* 
* License: Gnu Lesser General Public License
* http://www.gnu.org/licenses/lgpl.html
*/


#ifndef DVBCSASYS_H
#define DVBCSASYS_H


void sys_get_key ( const char *prog_name, unsigned int prog_num, unsigned char *ret_cw );
void sys_set_key ( unsigned char *cw, unsigned char *data, unsigned int size );


#endif /* DVBCSASYS_H */
