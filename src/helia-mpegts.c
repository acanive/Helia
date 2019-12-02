/*
* Copyright 2019 Stepan Perun
* This program is free software.
*
* License: Gnu Lesser General Public License
* http://www.gnu.org/licenses/lgpl.html
*/


#include <helia-include.h>

#define GST_USE_UNSTABLE_API
#include <gst/mpegts/mpegts.h>


void mpegts_initialize ()
{
	gst_mpegts_initialize ();

	g_type_class_ref ( GST_TYPE_MPEGTS_SECTION_TYPE );
	g_type_class_ref ( GST_TYPE_MPEGTS_SECTION_TABLE_ID );
	g_type_class_ref ( GST_TYPE_MPEGTS_RUNNING_STATUS );
	g_type_class_ref ( GST_TYPE_MPEGTS_DESCRIPTOR_TYPE );
	g_type_class_ref ( GST_TYPE_MPEGTS_DVB_DESCRIPTOR_TYPE );
	g_type_class_ref ( GST_TYPE_MPEGTS_MISC_DESCRIPTOR_TYPE );
	g_type_class_ref ( GST_TYPE_MPEGTS_ISO639_AUDIO_TYPE );
	g_type_class_ref ( GST_TYPE_MPEGTS_DVB_SERVICE_TYPE );
	g_type_class_ref ( GST_TYPE_MPEGTS_DVB_TELETEXT_TYPE );
	g_type_class_ref ( GST_TYPE_MPEGTS_STREAM_TYPE );
	g_type_class_ref ( GST_TYPE_MPEGTS_SECTION_DVB_TABLE_ID );
	g_type_class_ref ( GST_TYPE_MPEGTS_COMPONENT_STREAM_CONTENT );
}

void mpegts_clear ( MpegTs *mpegts )
{
	mpegts->pat_done = FALSE;
	mpegts->pmt_done = FALSE;
	mpegts->sdt_done = FALSE;

	mpegts->pat_count = 0;
	mpegts->pmt_count = 0;
	mpegts->sdt_count = 0;

	uint j = 0;
	for ( j = 0; j < MAX_RUN_PAT; j++ )
	{
		mpegts->pids[j].pat_sid = 0;
		mpegts->pids[j].pmt_sid = 0;
		mpegts->pids[j].sdt_sid = 0;

		mpegts->pids[j].pmt_apid = 0;
		mpegts->pids[j].pmt_vpid = 0;

		mpegts->pids[j].ch_name = NULL;
	}
}

static const char * mpegts_enum_name ( GType instance_type, int val )
{
	GEnumValue *en = g_enum_get_value ( G_ENUM_CLASS ( g_type_class_peek ( instance_type ) ), val );

	if ( en == NULL ) return "Unknown";

	return en->value_nick;
}

static void mpegts_pat ( GstMpegtsSection *section, MpegTs *mpegts )
{
	GPtrArray *pat = gst_mpegts_section_get_pat ( section );

	g_debug ( "PAT: %d Programs ", pat->len );

	uint i = 0;
	for ( i = 0; i < pat->len; i++ )
	{
		if ( i >= MAX_RUN_PAT )
		{
			g_print ( "MAX %d: PAT scan stop  \n", MAX_RUN_PAT );
			break;
		}

		GstMpegtsPatProgram *patp = g_ptr_array_index ( pat, i );

		if ( patp->program_number == 0 ) continue;

		mpegts->pids[mpegts->pat_count].pat_sid = patp->program_number;
		mpegts->pat_count++;

		g_debug ( "     Program number: %d (0x%04x) | network or pg-map pid: 0x%04x ", 
				patp->program_number, patp->program_number, patp->network_or_program_map_PID );
	}

	g_ptr_array_unref ( pat );

	mpegts->pat_done = TRUE;
	g_debug ( "PAT Done: pat_count %d \n", mpegts->pat_count );
}

static void mpegts_pmt ( GstMpegtsSection *section, MpegTs *mpegts )
{
	if ( mpegts->pmt_count >= MAX_RUN_PAT )
	{
		g_print ( "MAX %d: PMT scan stop  \n", MAX_RUN_PAT );
		return;
	}

	uint i = 0, len = 0;
	gboolean first_audio = TRUE;

	const GstMpegtsPMT *pmt = gst_mpegts_section_get_pmt ( section );
	len = pmt->streams->len;

	mpegts->pids[mpegts->pmt_count].pmt_sid = pmt->program_number;

	g_debug ( "PMT: %d  ( %d )", mpegts->pmt_count + 1, len );

	g_debug ( "     Program number     : %d (0x%04x) ", pmt->program_number, pmt->program_number );
	g_debug ( "     Pcr pid            : %d (0x%04x) ", pmt->pcr_pid, pmt->pcr_pid );
	g_debug ( "     %d Streams: ", len );

	for ( i = 0; i < len; i++ )
	{
		GstMpegtsPMTStream *stream = g_ptr_array_index ( pmt->streams, i );

		g_debug ( "       pid: %d (0x%04x), stream_type:0x%02x (%s) ", stream->pid, stream->pid, stream->stream_type,
			mpegts_enum_name (GST_TYPE_MPEGTS_STREAM_TYPE, stream->stream_type) );

		const char *name_t = mpegts_enum_name ( GST_TYPE_MPEGTS_STREAM_TYPE, stream->stream_type );

		if ( g_strrstr ( name_t, "video" ) )
			mpegts->pids[mpegts->pmt_count].pmt_vpid = stream->pid;

		if ( g_strrstr ( name_t, "audio" ) )
		{
			if ( first_audio )
				mpegts->pids[mpegts->pmt_count].pmt_apid = stream->pid;

			first_audio = FALSE;
		}
	}

	mpegts->pmt_count++;

	if ( mpegts->pat_count > 0 && mpegts->pat_count == mpegts->pmt_count )
	{
		mpegts->pmt_done = TRUE;
		g_debug ( "PMT Done: pmt_count %d \n", mpegts->pmt_count );
	}
}

static void mpegts_sdt ( GstMpegtsSection *section, MpegTs *mpegts )
{
	if ( mpegts->sdt_done ) return;

	if ( mpegts->sdt_count >= MAX_RUN_PAT )
	{
		g_print ( "MAX %d: SDT scan stop  \n", MAX_RUN_PAT );
		return;
	}

	uint i = 0, z = 0, c = 0, len = 0;

	const GstMpegtsSDT *sdt = gst_mpegts_section_get_sdt ( section );

	len = sdt->services->len;
	g_debug ( "Services: %d  ( %d ) ", mpegts->sdt_count + 1, len );

	for ( i = 0; i < len; i++ )
	{
		if ( i >= MAX_RUN_PAT ) break;

		GstMpegtsSDTService *service = g_ptr_array_index ( sdt->services, i );

		mpegts->pids[mpegts->sdt_count].ch_name = NULL;
		mpegts->pids[mpegts->sdt_count].sdt_sid = service->service_id;

		gboolean get_descr = FALSE;

		if ( mpegts->pat_done )
		{
			for ( z = 0; z < mpegts->pat_count; z++ )
				if ( mpegts->pids[z].pat_sid == service->service_id )
					{  get_descr = TRUE; break; }
		}

		if ( !get_descr ) continue;

		g_debug ( "  Service id:  %d | %d  ( 0x%04x ) ", mpegts->sdt_count + 1, service->service_id, service->service_id );

		GPtrArray *descriptors = service->descriptors;
		for ( c = 0; c < descriptors->len; c++ )
		{
			GstMpegtsDescriptor *desc = g_ptr_array_index ( descriptors, c );

			char *service_name, *provider_name;
			GstMpegtsDVBServiceType service_type;

			if ( desc->tag == GST_MTS_DESC_DVB_SERVICE )
			{
				if ( gst_mpegts_descriptor_parse_dvb_service ( desc, &service_type, &service_name, &provider_name ) )
				{
					mpegts->pids[mpegts->sdt_count].ch_name = g_strdup ( service_name );

					g_debug ( "    Service Descriptor, type:0x%02x (%s) ",
						service_type, mpegts_enum_name (GST_TYPE_MPEGTS_DVB_SERVICE_TYPE, service_type) );
					g_debug ( "    Service  (name) : %s ", service_name  );
					g_debug ( "    Provider (name) : %s \n", provider_name );

					g_free ( service_name  );
					g_free ( provider_name );
				}
			}

		}

		mpegts->sdt_count++;
	}

	if ( mpegts->pat_count > 0 && mpegts->pat_count == mpegts->sdt_count )
	{
		mpegts->sdt_done = TRUE;
		g_debug ( "SDT Done: sdt_count %d \n", mpegts->sdt_count );
	}
}

void mpegts_parse_section ( GstMessage *message, MpegTs *mpegts )
{
	GstMpegtsSection *section = gst_message_parse_mpegts_section ( message );

	if ( section )
	{
		switch ( GST_MPEGTS_SECTION_TYPE ( section ) )
		{
			case GST_MPEGTS_SECTION_PAT:
				mpegts_pat ( section, mpegts );
				break;

			case GST_MPEGTS_SECTION_PMT:
				mpegts_pmt ( section, mpegts );
				break;

			case GST_MPEGTS_SECTION_SDT:
				mpegts_sdt ( section, mpegts );
				break;

			default:
			break;
		}

		gst_mpegts_section_unref ( section );
	}
}

static void mpegts_pmt_info ( GstMpegtsSection *section, uint sid, char *audio_lang[] )
{
    uint num = 0, j = 0, c = 0;

    const GstMpegtsPMT *pmt = gst_mpegts_section_get_pmt ( section );
    GPtrArray *streams = pmt->streams;

    if ( sid == pmt->program_number )
    {
        for ( j = 0; j < streams->len; j++ )
        {
            GstMpegtsPMTStream *pmtstream = g_ptr_array_index ( streams, j );
            const char *name_t = mpegts_enum_name ( GST_TYPE_MPEGTS_STREAM_TYPE, pmtstream->stream_type );

            if ( g_strrstr ( name_t, "audio" ) )
            {
                char *lang = NULL;

                GPtrArray *descriptors = pmtstream->descriptors;
                for ( c = 0; c < descriptors->len; c++ )
                {
                    GstMpegtsDescriptor *desc = g_ptr_array_index ( descriptors, c );

                    GstMpegtsISO639LanguageDescriptor *res;
                    if (  gst_mpegts_descriptor_parse_iso_639_language ( desc, &res )  )
                    {
                        lang = g_strdup ( res->language[0] );
                        gst_mpegts_iso_639_language_descriptor_free (res);
                    }
                }

                audio_lang[num] = ( lang ) ? g_strdup ( lang ) : g_strdup_printf ( "%d", pmtstream->pid );

                g_debug ( "mpegts_pmt_info: lang %s | pmtstream->pid %d | num %d ", lang, pmtstream->pid, num );

                if ( lang ) g_free ( lang );

                num++;
            }
        }
    }
}

void mpegts_pmt_lang_section ( GstMessage *message, uint sid, char *audio_lang[] )
{
    GstMpegtsSection *section = gst_message_parse_mpegts_section ( message );

    if ( section )
    {
        switch ( GST_MPEGTS_SECTION_TYPE ( section ) )
        {
            case GST_MPEGTS_SECTION_PMT:
                mpegts_pmt_info ( section, sid, audio_lang );
                break;

            default:
                break;
        }

        gst_mpegts_section_unref ( section );
    }
}
