/*
* Copyright 2019 Stepan Perun
* This program is free software.
*
* License: Gnu Lesser General Public License
* http://www.gnu.org/licenses/lgpl.html
*/


#ifndef HELIA_MPEGTS_H
#define HELIA_MPEGTS_H

#define MAX_RUN_PAT 128


typedef struct _PatPmtSdt PatPmtSdt;

struct _PatPmtSdt
{
	uint pat_sid, pmt_sid, sdt_sid;
	uint pmt_vpid, pmt_apid;

	char *ch_name;
};

typedef struct _MpegTs MpegTs;

struct _MpegTs
{
	gboolean pat_done,  pmt_done,  sdt_done;
	uint     pat_count, pmt_count, sdt_count;

	PatPmtSdt pids[MAX_RUN_PAT];
};


void mpegts_initialize ();

void mpegts_clear ( MpegTs *mpegts );


#endif
