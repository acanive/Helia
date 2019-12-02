/*
* Copyright 2019 Stepan Perun
* This program is free software.
* 
* License: Gnu Lesser General Public License
* http://www.gnu.org/licenses/lgpl.html
*/


#ifndef INFO_H
#define INFO_H


void info_win_create ( Base *base, gboolean tv_mp );

/* Returns a newly-allocated string holding the result. Free with free() */
char * info_get_title_artist ( Base *base );

void info_title_save ( Base *base, const char *title, gboolean file_name );


#endif // INFO_H
