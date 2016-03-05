TREEROOT = ../..

PROJECT_SRC = main.c

PROJECT = demo

LIBS = lwip_obj/liblwipcommon.a

CFLAGS =-I. \
		-I../../lwip/src/include \
		-I../../lwip_freeRTOS_port \
		-I../../FreeRTOS/Source/include \
		-I../../posix_port/POSIX \
		-I../../posix_port/Source/portable/GCC/POSIX \
		-I$(LWIPDIR)/include

# need to include this makefile for common setting for all program
include $(TREEROOT)/Makefile.common
