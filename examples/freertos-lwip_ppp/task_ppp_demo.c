#include <stdio.h>
#include <stdlib.h>

#include "lwip/opt.h"

#include "FreeRTOS.h"
#include "task.h"

#include "lwip/tcpip.h"
#include "lwip/dns.h"

#include "netif/ppp/pppos.h"
#include "lwip/sockets.h"

#include "ppp_demo.h"

sio_fd_t ppp_sio = NULL;
ppp_pcb *ppp = NULL;
struct netif pppos_netif;

uint8_t connected = 0;


void task_serial_rx(void *arg)
{
	u32_t len;
	u8_t buffer[128];
	LWIP_UNUSED_ARG(arg);
	uint8_t i;

	/* Please read the "PPPoS input path" chapter in the PPP documentation. */
	while (1)
	{
		len = sio_read(ppp_sio, buffer, sizeof(buffer));
		if (len > 0)
		{
			/* Pass received raw characters from PPPoS to be decoded through lwIP
			 * TCPIP thread using the TCPIP API. This is thread safe in all cases
			 * but you should avoid passing data byte after byte. */
			printf("PPP Read:(%d)",len);
			for (i=0;i<len;i++)
				printf("0x%x ",buffer[i]);
			printf("\n");

			if (ppp != NULL)
				pppos_input_tcpip(ppp, buffer, len);
		}
	}
}



static void
ppp_link_status_cb(ppp_pcb *pcb, int err_code, void *ctx)
{
    struct netif *pppif = ppp_netif(pcb);
    LWIP_UNUSED_ARG(ctx);

    switch(err_code) {
    case PPPERR_NONE:               /* No error. */
        {
#if LWIP_DNS
        ip_addr_t ns;
#endif /* LWIP_DNS */
        fprintf(stderr, "ppp_link_status_cb: PPPERR_NONE\n\r");
#if LWIP_IPV4
        fprintf(stderr, "   our_ip4addr = %s\n\r", ip4addr_ntoa(netif_ip4_addr(pppif)));
        fprintf(stderr, "   his_ipaddr  = %s\n\r", ip4addr_ntoa(netif_ip4_gw(pppif)));
        fprintf(stderr, "   netmask     = %s\n\r", ip4addr_ntoa(netif_ip4_netmask(pppif)));
#endif /* LWIP_IPV4 */
#if LWIP_IPV6
        fprintf(stderr, "   our_ip6addr = %s\n\r", ip6addr_ntoa(netif_ip6_addr(pppif, 0)));
#endif /* LWIP_IPV6 */

#if LWIP_DNS

        ns = dns_getserver(0);
        fprintf(stderr, "   dns1        = %s\n\r", ipaddr_ntoa(&ns));
        ns = dns_getserver(1);
        fprintf(stderr, "   dns2        = %s\n\r", ipaddr_ntoa(&ns));

#endif /* LWIP_DNS */
#if PPP_IPV6_SUPPORT
        fprintf(stderr, "   our6_ipaddr = %s\n\r", ip6addr_ntoa(netif_ip6_addr(pppif, 0)));
#endif /* PPP_IPV6_SUPPORT */
        connected = 1;
        }
        break;

    case PPPERR_PARAM:             /* Invalid parameter. */
        printf("ppp_link_status_cb: PPPERR_PARAM\n");
        break;

    case PPPERR_OPEN:              /* Unable to open PPP session. */
        printf("ppp_link_status_cb: PPPERR_OPEN\n");
        break;

    case PPPERR_DEVICE:            /* Invalid I/O device for PPP. */
        printf("ppp_link_status_cb: PPPERR_DEVICE\n");
        break;

    case PPPERR_ALLOC:             /* Unable to allocate resources. */
        printf("ppp_link_status_cb: PPPERR_ALLOC\n");
        break;

    case PPPERR_USER:              /* User interrupt. */
        printf("ppp_link_status_cb: PPPERR_USER\n");
        break;

    case PPPERR_CONNECT:           /* Connection lost. */
        printf("ppp_link_status_cb: PPPERR_CONNECT\n");
        break;

    case PPPERR_AUTHFAIL:          /* Failed authentication challenge. */
        printf("ppp_link_status_cb: PPPERR_AUTHFAIL\n");
        break;

    case PPPERR_PROTOCOL:          /* Failed to meet protocol. */
        printf("ppp_link_status_cb: PPPERR_PROTOCOL\n");
        break;

    case PPPERR_PEERDEAD:          /* Connection timeout. */
        printf("ppp_link_status_cb: PPPERR_PEERDEAD\n");
        break;

    case PPPERR_IDLETIMEOUT:       /* Idle Timeout. */
        printf("ppp_link_status_cb: PPPERR_IDLETIMEOUT\n");
        break;

    case PPPERR_CONNECTTIME:       /* PPPERR_CONNECTTIME. */
        printf("ppp_link_status_cb: PPPERR_CONNECTTIME\n");
        break;

    case PPPERR_LOOPBACK:          /* Connection timeout. */
        printf("ppp_link_status_cb: PPPERR_LOOPBACK\n");
        break;

    default:
        printf("ppp_link_status_cb: unknown errCode %d\n", err_code);
        break;
    }
}

static u32_t
ppp_output_cb(ppp_pcb *pcb, u8_t *data, u32_t len, void *ctx)
{
  LWIP_UNUSED_ARG(pcb);
  LWIP_UNUSED_ARG(ctx);
  return sio_write(ppp_sio, data, len);
}


static const char demo_http_req[] = "GET /ip HTTP/1.1\r\n"
   		                   	   	   	   	"User-Agent: cpot_bl\r\n"
				                        "Accept: */*\r\n"
				           	   	   	    "Host: httpbin.org\r\n"
				           	   	   	    "Connection: close\r\n\r\n";	/*Keep-Alive*/



void task_ppp_demo (void * param __attribute__((__unused__)))
{
	uint8_t state=0;
	int socket;
	int ret;

	uint16_t len;
	uint16_t read;
	uint16_t written;
	uint8_t rx_buffer[512];
	fd_set readset;
	fd_set writeset;
	fd_set errset;
	struct timeval tv;
	int error = 1;

	ppp_sio = sio_open(0);
	ppp = pppos_create(&pppos_netif, ppp_output_cb, ppp_link_status_cb, NULL);
	if (!ppp)
	{
		printf("Could not create PPP control interface");
		exit(1);
	}

	ppp_set_auth(ppp, PPPAUTHTYPE_CHAP, "mobitel", "internet");
	ppp_set_default(ppp);

	ppp_connect(ppp, 0);

	while(1)
	{
		if (connected )
		{
			switch(state)
			{
			case 0:
				/*socket = demo_socket_tcp_create("54.175.219.8",80);*/
				socket = demo_socket_tcp_create("httpbin.org",80);
				if ( socket < 0 )
				{
					state = 0xFF;
					lwip_close(socket);
				}
				else
				{
					state = 1;
					written = 0;
				}
				break;

			case 1:
				FD_ZERO(&writeset);
				FD_SET(socket, &writeset);
				tv.tv_sec = 0;
				tv.tv_usec = 200*1000;
				ret = lwip_select(socket + 1, NULL, &writeset, NULL, &tv);
				switch(ret)
				{
				case -1:	/* error */
					state = 0xFF;
					lwip_close(socket);
					break;

				case 0: /* timeout */
					break;

				default:	/* CONNECTED */
					len = sizeof(error);
					if (lwip_getsockopt(socket, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len) == 0)
					{
						if (error == 0)
						{
							ret = lwip_write(socket, &demo_http_req[written], strlen(demo_http_req)-written);
							if (ret > 0)
							{
								written += ret;
								if(written == strlen(demo_http_req))
								{
									printf("write all OK\n");
									memset(rx_buffer, 0, 512);
									read = 0;
									state = 2;
									break;
								}
								else
								{
									break;
								}
							}
						}
					}
					printf("Write error\n");
					state = 0xFF;
					lwip_close(socket);
					break;
				}
				break;

			case 2:
				FD_ZERO(&readset);
				FD_ZERO(&errset);
				FD_SET(socket, &readset);
				FD_SET(socket, &errset);
				tv.tv_sec = 0;
				tv.tv_usec = 200*1000;
				ret = lwip_select(socket + 1, &readset, NULL, &errset, &tv);
				switch (ret)
				{
				case -1:	/* error */
					state = 0xFF;
					lwip_close(socket);
					break;

				case 0: /* timeout */
					break;

				default:
					if (FD_ISSET (socket, &errset))
					{
						printf("ERRSET:\n");
						state = 0xFF;
						lwip_close(socket);
						break;
					}
					len = sizeof(error);
					if (lwip_getsockopt(socket, SOL_SOCKET, SO_ERROR, &error, (socklen_t*)&len) == 0)
					{
						if (error == 0)
						{
							if (FD_ISSET (socket, &readset))
							{
								printf("READ *********************\n");
								ret = lwip_read(socket, &rx_buffer[read], 512-read);
								if ( ret > 0 )
									read += ret;
								else
								{
									int i;
									printf("Read Data(%d):%s\n\n",read,rx_buffer);
									for (i=0;i<read;i++)
									{
										printf("%02x ",rx_buffer[i]);
									}
									printf("\n");
									state = 3;
									lwip_close(socket);
								}
								printf("Read:%d/%d\n",ret,read);
								break;
							}
						}
					}
					printf("ERRSET11111:\n");
					state = 0xFF;
					lwip_close(socket);
					break;
				}
				break;

			case 3:
				printf("Done\n");
				vTaskDelay(1500 / portTICK_RATE_MS);
				break;

			case 0xFF:
				printf("error state\n");
				vTaskDelay(1500 / portTICK_RATE_MS);
				break;
			}
		}
		else
		{
			vTaskDelay(500 / portTICK_RATE_MS);
		}

	}

}


