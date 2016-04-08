# FreeRTOS development environment in Linux
[![Build Status](https://travis-ci.org/nqd/freertos_linux_devl.svg?branch=master)](https://travis-ci.org/nqd/freertos_linux_devl)

## Inspiration
For IoT, if it cannot run Linux, then should use FreeRTOS + lwIP (?).

This work is based on the the original work of William Davy on [Posix/Linux Simulator for FreeRTOS](http://www.freertos.org/FreeRTOS-simulator-for-Linux.html), and [megakilo fork](https://github.com/megakilo/FreeRTOS-Sim).

## Start
### Get the tool
Get the FreeRTOS v8.2.3 with `make -f Makefile.tools`  
`git clone https://github.com/peterzos/lwip.git`  
`git clone https://github.com/peterzos/lwip-contrib.git`  


### Run the first example
Under examples/freertos-demo, run `make`, then `./main.out` to see output.

### Run the second example
`cd examples/freertos-lwip_ppp/lwip_obj/`  
`make`  
`cd ..`  
`make`  
`./demo`  
  
For debugging use examples/freertos-lwip_ppp/.gdbinit  



See demo makefile to create your own program.
