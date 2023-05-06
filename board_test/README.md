# Board testing utility

Firmware for testing board. It should ensure you that
board was properly assembled and soldered. It is trying
to run peripherals at its max speed to also ensure that
PCB layout is capable to achieve desired frequencies.

**Warning:** at this moment SDIO and SD Card slot test are
not implemented!

## Requirements

* Assembled board with FTDI configuration loaded
* USB A to mini B cable
* STLink with some dupont cables
* CMake
* git
* serial port monitor (e.g. picocom)
* bash
* tar
* wget
* something to control stlink (e.g. https://github.com/stlink-org/stlink)

## How to build and test

This repository is using submodules to deal with dependencies,
so if you don't yet, init them. Then you have to obtain
toolchain, this can be achieved by script `toolchain.sh`.
To build test utility you have to call cmake.

Compiled binary can be loaded into MCU using stlink.

``` bash
#installing prerequisites
$ sudo pacman -S stlink cmake picocom git tar wget

#git submodules
$ git submodule update --init --recursive

#toolchain
$ ./toolchain.sh

#compile
$ cmake -S . -B build/ -DCMAKE_TOOLCHAIN_FILE=./toolchain.cmake -DCMAKE_BUILD_TYPE=Release
$ cmake --build build/
```

At this moment you have to connect board to your computer.
Ensure your board is powered according your setting of jumpers
(consult schematic for this topic). And connect your STLink
programmed using SWI to MCU. SWI interface is available thru
expansion headers. Also please check schematic for information.

``` bash
#load into MCU
$ st-flash write build/tester.bin 0x08000000
```

Test itself is performed by code and communication with test
operator is achieved through serial port available on
the port. Ensure your board is connected and access correct
port (ttyUSB0 in example).

``` bash
#connect to fw console
$ picocom -b 115200 /dev/ttyUSB0
```

After connecting, press reset button and follow instructions
in console. Test can take a few minutes. At the end small
summary will appear.

## Troubleshooting

If testing software will come with some errors double check
your soldering.
