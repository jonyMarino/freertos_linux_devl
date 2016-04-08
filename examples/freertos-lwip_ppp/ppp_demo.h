/*
 * ppp_demo.h
 *
 *  Created on: Mar 23, 2016
 *      Author: peterz
 */

#ifndef FREERTOS_LINUX_DEVL_EXAMPLES_FREERTOS_LWIP_PPP_PPP_DEMO_H_
#define FREERTOS_LINUX_DEVL_EXAMPLES_FREERTOS_LWIP_PPP_PPP_DEMO_H_


void task_ppp_demo (void * param __attribute__((__unused__)));
void task_serial_rx(void *arg);


int demo_socket_tcp_create(char* ip_address, uint16_t port);

#endif /* FREERTOS_LINUX_DEVL_EXAMPLES_FREERTOS_LWIP_PPP_PPP_DEMO_H_ */
