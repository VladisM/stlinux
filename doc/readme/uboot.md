# U-Boot support

Support for this project is provided in the fork of U-Boot that is linked
into this project repository using git submodule. Custom target is called
`stm32f446-stlinux`.

**Warning:** support is not complete yet and may change overtime.

## Configuration overview

Configuration is pretty bare bone. Only bare minimum is implemented now.
External SDRAM is mapped at `0xC000 0000`. For console `USART2` is used.
External crystal with frequency of 25MHz is used. MCU is starting from
internal Flash and it will relocate itself into external SDRAM.

## Requirements

* Functional board that will pass board test
* USB A to mini B cable
* Toolchain (the one downloaded for testing can be used)
* STLink with some dupont cables
* make
* git
* serial port monitor (e.g. picocom)
* bash
* something to control stlink (e.g. https://github.com/stlink-org/stlink)

## How to build and test

First of all lets deal with prerequisites. We will download them using
package manager and as compiler we will use one created by board testing.
But you are free to use whatever toolchain you want, just set your PATH
variable correctly.

``` bash
#installing prerequisites
$ sudo pacman -S stlink make picocom git

#set path for toolchain
$ export PATH=./board_test/tools/arm-gnu-toolchain-12.2.rel1-x86_64-arm-none-eabi/bin/:$PATH
```

Once we have toolchain, we can start building. You can modify output forlder
using `-O` parameter to make and also speed up compilation using `-j` parameter
with count of cores you want to use.

``` bash
#go into source folder
$ cd ./u-boot/

#build U-Boot
$ make ARCH=arm CROSS_COMPILE=arm-none-eabi- O=../build/u-boot/ stm32f446-stlinux_defconfig
$ make ARCH=arm CROSS_COMPILE=arm-none-eabi- O=../build/u-boot/
```

Next step is to load U-Boot binary into device. Connect board using ST-Link
and power it up. You should also connect serial port to it (USB mini B).

``` bash
#load into MCU
$ st-flash write ../build/u-boot/u-boot.bin
```

And that is, now you can see output using serial port console.

``` bash
#connect to U-Boot console
$ picocom -b 115200 /dev/ttyUSB0
```

## Debugging U-Boot

For debugging you can use gdb. First start gdb server, you can use st-util for
example.

``` bash
$ st-util
st-util 1.7.0-237-g1bec78c
Failed to parse flash type or unrecognized flash type
2023-05-07T20:26:46 WARN common.c: NRST is not connected
2023-05-07T20:26:46 INFO common.c: STM32F446: 128 KiB SRAM, 256 KiB flash in at least 128 KiB pages.
2023-05-07T20:26:46 INFO gdb-server.c: Listening at *:4242...
```

Next step is to connect your gdb front end to remote target. That can be done
using following commands.

``` bash
$ $ arm-none-eabi-gdb build/u-boot/u-boot
GNU gdb (GDB) 13.1
Copyright (C) 2023 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.
Type "show copying" and "show warranty" for details.
This GDB was configured as "--host=x86_64-pc-linux-gnu --target=arm-none-eabi".
Type "show configuration" for configuration details.
For bug reporting instructions, please see:
<https://www.gnu.org/software/gdb/bugs/>.
Find the GDB manual and other documentation resources online at:
    <http://www.gnu.org/software/gdb/documentation/>.

For help, type "help".
Type "apropos word" to search for commands related to "word"...
Reading symbols from build/u-boot/u-boot...
(gdb) target remote :4242
```

After that you can debug as usual. Just remember, as part of init sequence
U-Boot relocate itself to run from SDRAM. Refer
[u-boot/doc/README.arm-relocation](../../u-boot/doc/README.arm-relocation) for
more info.
