#
# Copyright (c) 2001, 2002 Swedish Institute of Computer Science.
# All rights reserved. 
# 
# Redistribution and use in source and binary forms, with or without modification, 
# are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
# 3. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission. 
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
# SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
# OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
# IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
# OF SUCH DAMAGE.
#
# This file is part of the lwIP TCP/IP stack.
# 
# Author: Adam Dunkels <adam@sics.se>
#

CCDEP=gcc
CC=gcc

#To compile for openbsd: make UNIXARCH=OPENBSD
#To compile for cygwin: make UNIXARCH=CYGWIN
UNIXARCH ?= LINUX
CFLAGS=-m32 -ggdb -Wall -DLWIP_UNIX_$(UNIXARCH) -DLWIP_DEBUG -pedantic -Werror \
	-Wparentheses -Wsequence-point -Wswitch-default \
	-Wextra -Wundef -Wshadow -Wpointer-arith -Wcast-qual \
	-Wc++-compat -Wwrite-strings -Wold-style-definition -Wcast-align \
	-Wmissing-prototypes -Wredundant-decls -Wnested-externs -Wno-address \
	-Wunreachable-code -Wuninitialized -Wlogical-op
# not used for now but interesting:
# -Wpacked
# -ansi
# -std=c89
LDFLAGS=-pthread -lutil -lrt
ARFLAGS=rs

#Set this to where you have the lwip core module checked out from CVS
#default assumes it's a dir named lwip at the same level as the contrib module
LWIPDIR=../../../lwip/src

CFLAGS+=-I../ \
	-I../../../lwip_freeRTOS_port \
	-I../../../FreeRTOS/Source/include \
	-I../../../posix_port/POSIX \
	-I../../../posix_port/Source/portable/GCC/POSIX \
	-I$(LWIPDIR)/include

include $(LWIPDIR)/Filelists.mk


LWIPFILES=$(LWIPNOAPPSFILES) ../../../lwip_freeRTOS_port/sys_arch_freertos.c
LWIPOBJS=$(notdir $(LWIPFILES:.c=.o))

LWIPLIBCOMMON=liblwipcommon.a
$(LWIPLIBCOMMON): $(LWIPOBJS)
	$(AR) $(ARFLAGS) $(LWIPLIBCOMMON) $?

APPFILES=$(CONTRIBAPPFILES) $(LWIPAPPFILES)
APPLIB=liblwipapps.a
APPOBJS=$(notdir $(APPFILES:.c=.o))
$(APPLIB): $(APPOBJS)
	$(AR) $(ARFLAGS) $(APPLIB) $?

%.o:
	$(CC) $(CFLAGS) -c $(<:.o=.c)
