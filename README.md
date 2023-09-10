# STlinux

Simple board with STM32F446 and SDRAM that is capable of running MMU less linux
build. Purpose of this project is to learn new stuffs. It may seems like good
thing to have Linux in such small form factor but it isn't. MMU is somehow
important in security and using this approach to build device that will be
connected to the network is a bad idea.

This project using submodules. After clone use following to init them.

```bash
$ git submodule update --init --recursive
```

## System specification

* Hardware:
    * 32MB SDRAM
    * 8MB QSPI Flash
    * UART console, sd card slot, EEPROM
    * two LEDs and two buttons
* Software
    * Linux kernel 6.3
    * Busybox 1.36.1

## Manufacturing board

Gerber files required for manufacturing are stored in `board/ger` subdirectory.
To manufacture board you shall just find yourself a suitable manufacturer
and use latest archive.

To generate BOM you shall use KiCad and revisit folder `board/kicad`. All
project files are stored there.

After soldering board you should apply ftdi configuration into
integrated UART bridge. Correct configuration is provided
by the file `board/ftdi_configuration.xml`. For that you are going
to need Windows PC and FT_PROG tool (available on their website).

To test if board was correctly manufactured and most importantly
assembled. You can use firmware available in `board_test` folder.

Compile and load firmware, connect using serial port at speed of 115200
and follow instructions.

**Warning:** at this moment SDIO and SD Card slot test are
not implemented!

## Compile system and bootloader

Everything is compiled using clfs.sh script in root folder of this project. So,
if you want to compile whole system simply call following command. (note, adjust
number of cores for compilation using -j)

```bash
$ ./clfs.sh --build all -j 8
```

After some time, you can find all output images in `build/images/`. There are:

 * bootloader - u-boot.bin
 * linux kernel - zImage
 * device tree - stm32f446-stlinux.dtb
 * system image - clfs.uImage

Loading into board can be little bit tricky as at this moment, only working
interface is UART. We can use for example xmodem.

Write u-boot into internal flash of MCU using stlink for example. Once booted
in u-boot console use xmodem protocol to send all other files, and write them to
their partitions in external flash using mtd. For example, this is how to load
kernel.

```bash
> loadx
# send kernel file using your console emulator
> mtd erase kernel
> mtd write kernel ${loadaddr}
```

Repeat this approach for all required binaries, after all binaries are loaded
you can execute boot by running bootcmd variable or reseting board, you should
end up in working system. Login as root.

## License

This project is using a lot of software components that are licensed under
various licenses. Always make sure to check what component are you working
with to prevent any misunderstanding.

Board manufacturing files, firmware for testing, and build script is licensed
using the Do What The Fuck You Want To Public License, Version 2,
as published by Sam Hocevar. See COPYING file for more details.

Note: CLFS is abbreviation from Cross Linux From Scratch, in general describing process
of cross compiling [Linux From Scratch](https://www.linuxfromscratch.org/). Please
also see [CLFS project](https://trac.clfs.org/). My work is influenced by these
projects and I learned from their materials a lot.
