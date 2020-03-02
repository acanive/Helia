/*
* Copyright 2019 Stepan Perun
* This program is free software.
* 
* License: Gnu General Public License
* http://www.gnu.org/licenses/lgpl.html
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h> 
#include <errno.h>
#include <sys/types.h>

#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h> 
#include <netinet/in.h> 

#include <linux/ioctl.h>
#include <linux/dvb/dmx.h>
#include <linux/dvb/ca.h>

#include <glib.h>


#define SIZE 				1024 
#define PORT 				8989
#define HOST 				"127.0.0.1"
#define INFO_CLIENT 			"helia-dvbapi"

#define DVBAPI_PROTOCOL 	 	2
#define DVBAPI_FILTER_DATA 		0xFFFF0000
#define DVBAPI_CLIENT_INFO 		0xFFFF0001
#define DVBAPI_SERVER_INFO 		0xFFFF0002
#define DVBAPI_ECM_INFO 		0xFFFF0003

#define CA_SET_DESCR_AES 		0x40106f87
#define CA_SET_DESCR_MODE 		0x400c6f88
#define CA_SET_PID 			0x40086f87


/* kernel ver. ? */
typedef struct ca_pid
{
	unsigned int pid;
	int index; 		/* -1 == disable */
} ca_pid_t;

typedef struct ca_descr_aes
{
	unsigned int index;
	unsigned int parity;    /* 0 == even, 1 == odd */
	unsigned char cw[16];
} ca_descr_aes_t;

enum ca_descr_algo
{
	CA_ALGO_DVBCSA,
	CA_ALGO_DES,
	CA_ALGO_AES128,
};

enum ca_descr_cipher_mode
{
	CA_MODE_ECB,
	CA_MODE_CBC,
};

typedef struct ca_descr_mode
{
	uint32_t index;
	enum ca_descr_algo algo;
	enum ca_descr_cipher_mode cipher_mode;
} ca_descr_mode_t;


typedef struct _Client Client;

struct _Client
{
	int fd_sock;
	gboolean server_run;
	gboolean client_start;
};

static Client client;


static void client_close_connection ( int sockfd );

static void socket_write ( int sockfd, unsigned char *data, int len );

static void _read_cmd_1 ( const char *title, unsigned char *buf )
{
	printf ( "%s: read %s \n", __func__, title );
}

static void _read_cmd_2 ( const char *title, unsigned char *buf )
{
	printf ( "%s: read %s \n", __func__, title );
}

static void _read_cmd_3 ( const char *title, unsigned char *buf )
{
	printf ( "%s: read %s \n", __func__, title );
}

static void _read_cmd_4 ( const char *title, unsigned char *buf )
{
	printf ( "%s: read %s \n", __func__, title );
}

static void _read_cmd_5 ( const char *title, unsigned char *buf )
{
	printf ( "%s: read %s \n", __func__, title );
}

static void _read_cmd_6 ( const char *title, unsigned char *buf )
{
	printf ( "%s: read %s \n", __func__, title );
}

static void _read_cmd_7 ( const char *title, unsigned char *buf )
{
	uint16_t *proto_ptr = (uint16_t *) &buf[4];
	uint16_t protocol   = ntohs ( *proto_ptr );

	printf ( "%s: read %s | %s, protocol_version = %d \n", __func__, title, &buf[6], protocol );
}

static void _read_cmd_8 ( const char *title, unsigned char *buf )
{
	printf ( "%s: read %s \n", __func__, title );
}

static struct cmd_size { uint _cmd_size; } cmd_size_n[] =
{
	{ sizeof ( ca_pid_t ) },
	{ sizeof ( ca_descr_t ) },
	{ sizeof ( ca_descr_aes_t ) },
	{ sizeof ( ca_descr_mode_t ) },
	{ sizeof ( struct dmx_sct_filter_params ) },
	{  4  },
	{  2  },
	{  14 }
};

typedef struct _DvbApiCmd DvbApiCmd;

struct _DvbApiCmd
{
	const char *title;
	uint32_t cmd;
	void ( *f ) ( const char *, unsigned char * );
};

static DvbApiCmd dvb_api_cmd_n[] = 
{
	{ "CA_SET_PID", 	CA_SET_PID, 		_read_cmd_1 },
	{ "CA_SET_DESCR", 	CA_SET_DESCR, 		_read_cmd_2 },
	{ "CA_SET_DESCR_AES", 	CA_SET_DESCR_AES, 	_read_cmd_3 },
	{ "CA_SET_DESCR_MODE",	CA_SET_DESCR_MODE, 	_read_cmd_4 },
	{ "DMX_SET_FILTER", 	DMX_SET_FILTER, 	_read_cmd_5 },
	{ "DMX_STOP", 		DMX_STOP, 		_read_cmd_6 },
	{ "DVBAPI_SERVER_INFO", DVBAPI_SERVER_INFO, 	_read_cmd_7 },
	{ "DVBAPI_ECM_INFO", 	DVBAPI_ECM_INFO, 	_read_cmd_8 }
};

static void socket_read ( int sockfd )
{
	uint i = 0;
	uint32_t *request;
	uint8_t adapter_index = 0;
	unsigned char buf[SIZE];

	int res = recv ( sockfd, &buf[0], sizeof(int), MSG_DONTWAIT );

	if ( res == 0 ) { client_close_connection ( sockfd ); return; }
	if ( res <  0 ) { g_debug ( "%s: read empty \n", __func__ ); return; }

	request  = (uint32_t *) &buf;

	if ( ntohl ( *request ) != DVBAPI_SERVER_INFO )
	{
		res = recv ( sockfd, &adapter_index, 1, MSG_DONTWAIT );

		if ( res == 0 ) { client_close_connection ( sockfd ); return; }
		if ( res <  0 ) return;

		g_debug ( "%s: read adapter_index %d | command: %08x ", __func__, adapter_index, *request );
	}

	*request = ntohl ( *request );

	for ( i = 0; i < 8; i++ ) if ( *request == dvb_api_cmd_n[i].cmd ) break;

	if ( i == 8 ) { g_debug ( "%s: read unknown command: %08x ", __func__, *request ); return; }

	res = recv ( sockfd, buf + 4, cmd_size_n[i]._cmd_size, MSG_DONTWAIT );

	if ( dvb_api_cmd_n[i].cmd == DVBAPI_SERVER_INFO )
	{
		unsigned char len;

		recv ( sockfd, &len, 1, MSG_DONTWAIT );
		res = recv ( sockfd, buf + 6, len, MSG_DONTWAIT );

		buf[ 6 + len ] = 0;
	}

	if ( res == 0 ) { g_debug ( "%s: socket close ", __func__ ); client_close_connection ( sockfd ); return; }

	if ( res <  0 )
		g_debug ( "%s: %s error read command: %08x ", __func__, dvb_api_cmd_n[i].title, *request );
	else
		dvb_api_cmd_n[i].f ( dvb_api_cmd_n[i].title, buf );
}

static void client_send_info ( int sockfd )
{
	int len = sizeof ( INFO_CLIENT ) - 1;
	unsigned char buff[ 7 + len ];

	uint32_t req = htonl ( DVBAPI_CLIENT_INFO );
	memcpy ( &buff[0], &req, 4 );

	int16_t proto_version = htons ( DVBAPI_PROTOCOL );
	memcpy ( &buff[4], &proto_version, 2 );

	buff[6] = len;
	memcpy ( &buff[7], &INFO_CLIENT, len );

	socket_write ( sockfd, buff, sizeof ( buff ) );
}

static void socket_write ( int sockfd, unsigned char *data, int len )
{
	if ( sockfd > 0 )
	{
		int res = send ( sockfd, data, len, MSG_DONTWAIT );

		if ( res != len )
		{
			client_close_connection ( sockfd );

			printf ( "%s: res != len \n", __func__ );
		}
	}
}

static void client_send_stop_descrambling ( int sockfd )
{
	unsigned char buf[8] =
	{
		0x9F, 0x80, 0x3F, 0x04, 0x83, 0x02, 0x00, 0xFF
	};

	socket_write ( sockfd, buf, 8 );
}

static void client_close_connection ( int sockfd )
{
	if ( sockfd > 0 )
	{
		client_send_stop_descrambling ( sockfd );
		close ( sockfd );
		client.fd_sock = -1;

		printf ( "Close connected \n" );
	}
}

static int socket_connect () 
{
	int sockfd = socket ( AF_INET, SOCK_STREAM, 0 );

	if ( sockfd == -1 )
	{
		printf ( "Socket creation failed. \nError: %s \n", strerror ( errno ) );

		return sockfd;
	}

	struct sockaddr_in servaddr;
	bzero ( &servaddr, sizeof ( servaddr ) );

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr ( HOST );
	servaddr.sin_port = htons ( PORT );

	int connect_res = connect ( sockfd, ( struct sockaddr* )&servaddr, sizeof ( servaddr ) );

	if ( connect_res != 0 )
	{
		close ( sockfd );
		sockfd = -1;

		printf ( "Connection with the server failed. \nError: %s \n", strerror ( errno ) );

		return sockfd;
	}

	printf ( "Socket conect: fd %d \n", sockfd );

	client_send_info ( sockfd );

	return sockfd;
}

static gboolean socket_read_timeout ()
{
	if ( !client.client_start ) return client.client_start;

	if ( client.fd_sock == -1 )
		client.fd_sock = socket_connect ();
	else
		socket_read ( client.fd_sock );

	return client.client_start;
}

static gboolean server_is_run ( const char *line )
{
	/* g_autofree char *find_srv = g_strdup_printf ( "%s:%d", HOST, PORT ); */
	const char *find_srv = "0.0.0.0:8989";

	g_debug ( "%s: find_srv %s \n %s \n", __func__, find_srv, line );

	if ( g_strrstr ( line, find_srv ) ) return TRUE;

	return FALSE;
}

/* Returns a newly-allocated string holding the result. Free with free() */
static char * find_program ( const char *prog )
{
	g_autofree char *out_str = NULL;
	g_autofree char *err_str = NULL;

	if ( g_spawn_command_line_sync ( prog, &out_str, &err_str, NULL, NULL ) )
		return g_strdup ( out_str );

	return out_str;
}

void client_oscam_start ()
{
	client.server_run   = FALSE;
	client.client_start = FALSE;

	g_autofree char *buf_netstat = find_program ( "netstat -tuanp" ); // tcp & udp

	if ( buf_netstat )
	{
		client.server_run = server_is_run ( buf_netstat );
		
		g_print ( "Server running - %s \n", ( client.server_run ) ? "OK" : "NO & EXIT" );
	}
	else
		g_print ( "Error: Netstat not installed. \n" );

	if ( !client.server_run || !buf_netstat ) return;

	client.fd_sock = socket_connect ();

	if ( client.fd_sock == -1 ) return;

	client.client_start = TRUE;

	g_timeout_add ( 25, (GSourceFunc)socket_read_timeout, NULL ); // timeout 25 milliseconds

	printf ( "%s: sock fd %d \n", __func__, client.fd_sock );
}

void client_oscam_stop ()
{
	client.client_start = FALSE;

	client_close_connection ( client.fd_sock );

	printf ( "%s: stop all \n", __func__ );
}

void demux_data ( const char *name, uint sid, unsigned char *data, uint len )
{
	
}
