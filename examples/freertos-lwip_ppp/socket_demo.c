#include "sio_gsm.h"
#include "lwip/debug.h"
#include "lwip/def.h"
#include "lwip/sys.h"
#include "lwip/arch.h"
#include "lwip/sio.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"

#include <stdlib.h>
#include <stdio.h>

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "ppp_demo.h"


int demo_socket_tcp_create(char* ip_address, uint16_t port)
{
	u32_t  opt;
	int socket;
	struct sockaddr_in addr;

	socket = lwip_socket(AF_INET, SOCK_STREAM, 0);
	if ( socket < 0 )
	{
		return -1;
	}

	opt = 1;
	lwip_ioctl(socket, FIONBIO, &opt);	/* set non-blocking */

	memset(&addr, 0, sizeof(addr));
	addr.sin_len = sizeof(addr);
	addr.sin_family = AF_INET;
	addr.sin_port = PP_HTONS(port);

	if (((ip_address[0]>'A') && (ip_address[0]<'Z')) ||
		((ip_address[0]>'a') && (ip_address[0]<'z')))
	{
		struct hostent      *phe;
		phe = lwip_gethostbyname(ip_address);
		if (NULL == phe)
		{
			/*err = sock_get_last_err();*/
			return -1;
		}
		memcpy(&addr.sin_addr, phe->h_addr, phe->h_length);
	}
	else
	{
		addr.sin_addr.s_addr = inet_addr((char*)ip_address);
	}


	if (lwip_connect(socket, (struct sockaddr*)&addr, sizeof(addr)) != 0)
	{
		if (errno != EINPROGRESS)
		{
			printf("scoket connect error:%d\n ",errno);
			lwip_close(socket);
			return -1;
		}
	}

	return socket;
}

