
#include "sio_gsm.h"
#include "lwip/debug.h"
#include "lwip/def.h"
#include "lwip/sys.h"
#include "lwip/arch.h"
#include "lwip/sio.h"

#include <stdlib.h>
#include <stdio.h>

#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <errno.h>

#define MAX_SIO_GSM_DEVICE	4

static sio_status_t statusar[MAX_SIO_GSM_DEVICE];

static int16_t gsm_exchange(int f, uint8_t *tx_buff, uint16_t tx_len, uint8_t *rx_buff, uint16_t rx_len, int timeout_ms);
static uint16_t gsm_atcmd_exchange(int f, uint8_t *atcmd, uint8_t *rx_buff, uint16_t rx_len, char* expected_str, int timeout_ms);

static int16_t gsm_exchange(int f, uint8_t *tx_buff, uint16_t tx_len, uint8_t *rx_buff, uint16_t rx_len, int timeout_ms)
{
	fd_set fdsR;
	struct timeval timeout;
	int rlen=0;
	int ret;
	int counter = timeout_ms/10;

	if(f == -1)
		return 0;

	memset(rx_buff,0,rx_len);


	lseek(f,0,SEEK_END);

	if ((tx_buff != NULL) && (tx_len > 0))
	{
		//printf(">>%s\n",tx_buff);
		write(f,tx_buff, tx_len);
	}
	if (rx_len == 0)
		return 0;
	do
	{
		FD_ZERO (&fdsR);
		FD_SET (f, &fdsR);
		timeout.tv_sec = 0;
		timeout.tv_usec = 10000;		/* 10ms */

		ret = select (f + 1, &fdsR, NULL, NULL, &timeout);
		switch(ret)
		{
		case -1:

			if (EINTR == errno)	/* Interrupted system call */
				continue;
			printf("error\n");
			goto error;

		case 0:
			printf("timeout\n");
			break;

		default:
			if(FD_ISSET(f, &fdsR))
			{
				ret = read(f,&rx_buff[rlen],rx_len-rlen);
				if(ret <= 0)
				{
					//printf("error reading:%d,%s!\n",errno,strerror(errno));
					rlen = -1;
					goto error;
				}
				rlen +=ret;
				//rx_buff[rlen]=0;
				if (rlen >= rx_len)
				{
					goto end;
				}

			}
		}
		counter--;
	}
	while(counter > 0);

end:
	return (rlen);


error:
	printf("modem read error\n");
	return (rlen);

}

static uint16_t gsm_atcmd_exchange(int f, uint8_t *atcmd, uint8_t *rx_buff, uint16_t rx_len, char* expected_str, int timeout_ms)
{
	int counter = timeout_ms/10;
	int rlen=0;
	int ret;

	lseek(f,0,SEEK_END);
	if (atcmd != NULL)
	{
		uint8_t dummy[1000];
		/* dummy read URC - befor write atcmd */
		memset(dummy,0,1000);
		rlen = read(f,dummy,1000);
		rlen=0;

		write(f,atcmd,(int)strlen((char*)atcmd));
	}
	if (rx_len == 0)
	{
		return 0;
	}

	do
	{
		counter--;
		ret = gsm_exchange(f, NULL, 0, &rx_buff[rlen],rx_len-rlen-1, 10);
		if (ret== -1)
			break;


		rlen += ret;

		rx_buff[rlen] = 0;

		if (expected_str == NULL)
		{
			if( (NULL != strstr((char*)rx_buff,"OK")) || (NULL != strstr((char*)rx_buff,"ERROR")) )
			{
				//printf("<<%s\n",recbuff);
				break;
			}
		}
		else
		{
			if(NULL != strstr((char*)rx_buff,expected_str))
			{
				//printf("match\n");
				//printf("<<%s\n",buff);
				break;
			}
		}
	}
	while((counter > 0) && (rlen<(rx_len-1)));

	return (rlen);

}

sio_fd_t sio_open(u8_t devnum)
{
	char dev[20];
	sio_status_t * siostate;
	struct termios ops;
	uint16_t received;
	uint8_t recBuffer[100];

	if (devnum >= MAX_SIO_GSM_DEVICE)
	{
		return NULL;
	}
	siostate = &statusar[ devnum ];

	snprintf( dev, sizeof(dev), "/dev/ttyACM%d", devnum );

	printf("sio_open for dev:%s\n",dev);

	siostate->fd = open(
						dev,/*"/dev/ttyACM0"*/	/*"/dev/ttyUSB0"*/
						O_RDWR | O_NONBLOCK);

	if(siostate->fd == -1)
	{
		printf("sio_open error: Could not open %s device : %d=%s\n", dev,errno,strerror(errno));
		return NULL;
	}
	tcgetattr(siostate->fd, &ops);
	cfmakeraw(&ops);

	/* accept as raw */
	ops.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	ops.c_iflag &= ~(INLCR | ICRNL | IGNCR);

	/* make raw output */
	ops.c_oflag &= ~OPOST;

	/* set port to 8N1 */
	ops.c_cflag &= ~PARENB;
	ops.c_cflag &= ~CSTOPB;
	ops.c_cflag &= ~CSIZE;
	ops.c_cflag |= CS8;

	/* disable hardware flow control */
    /*ops.c_cflag &= ~CNEW_RTSCTS;*/

	/* disable software flow ctrl */
	ops.c_iflag &= ~(IXON | IXOFF | IXANY);

	/* set speed */
	cfsetospeed ((struct termios *) &ops, B115200);

	tcflush (siostate->fd, TCIFLUSH);

	tcsetattr(siostate->fd, TCSANOW, &ops);

	received = gsm_atcmd_exchange(siostate->fd,(uint8_t*)"ATE0\r",recBuffer,100,NULL,1000);
	if (received == 0)
	{
		printf("ATE0 error\n");
		return 0;
	}
	printf("ATE0 OK\n");

	received = gsm_atcmd_exchange(siostate->fd,(uint8_t*)"AT\r",recBuffer,100,NULL,1000);
	if (received == 0)
	{
		printf("ATE error\n");
		return 0;
	}
	printf("AT OK\n");
	received = gsm_atcmd_exchange(siostate->fd,(uint8_t*)"AT+CGDCONT=1,\"IP\",\"internet\"\r",recBuffer,100,NULL,5000);
	if (received == 0)
	{
		printf("ATE error\n");
		return 0;
	}
	printf("ATCGDCONT OK\n");
	received = gsm_atcmd_exchange(siostate->fd,(uint8_t*)"ATD*99***1#\r",recBuffer,100,"\r\nCONNECT\r\n",1000);
	if (received == 0)
	{
		printf("ATE error\n");
		return 0;
	}
	printf("ATD OK\n");

	printf("init OK\n");

	return siostate;
}

//u32_t sio_read(sio_fd_t fd, u8_t *data, u32_t len)
u32_t sio_read(sio_status_t * siostat, u8_t *buf, u32_t size)
{
	if (siostat == NULL)
		return 0;
	if (siostat->fd == 0)
		return 0;

	ssize_t rsz = read( siostat->fd, buf, size );
	return rsz < 0 ? 0 : rsz;
}

//u32_t sio_write(sio_fd_t fd, u8_t *data, u32_t len)
u32_t sio_write(sio_status_t * siostat, u8_t *buf, u32_t size)
{
	  ssize_t wsz = write( siostat->fd, buf, size );
	  return wsz < 0 ? 0 : wsz;
}

