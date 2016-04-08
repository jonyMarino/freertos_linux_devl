TREEROOT = ../..

PROJECT_SRC = main.c sio_gsm.c task_ppp_demo.c socket_demo.c

PROJECT = demo

LIBS = lwip_obj/liblwipcommon.a
#LIBS = /home/peterz/work/lpc_workspace/zzz_FreeRTOS_x86_lwip/freertos_linux_devl/examples/freertos-lwip_ppp/lwip_obj/liblwipcommon.a

CFLAGS =-I. \
		-I../../lwip/src/include \
		-I../../lwip_freeRTOS_port \
		-I../../FreeRTOS/Source/include \
		-I../../posix_port/POSIX \
		-I../../posix_port/Source/portable/GCC/POSIX \
		-I$(LWIPDIR)/include

# need to include this makefile for common setting for all program
include $(TREEROOT)/Makefile.common
