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

Bootloader shall be loaded to the board using STLink, and shall be written
to the internal Flash memory of MCU.

```bash
st-flash write build/images/u-boot.bin 0x08000000
```

Other components are by default loaded from QSPI Flash, there are two ways how
to get them there.

 * Using SD card
 * Using serial port (legacy)

### Loading QSPI Flash using serial port

This method was used for first revision of board with nonworking SD card slot.
If you have second revision of board, use SD card, it is much more faster.

Once you are in u-boot console, you can use xmodem protocol to send all files
and then write them to the QSPI Flash. For writing there is mtd command. Follow
next code snippet.

```bash
> loadx
# send kernel file (zImage) using your console emulator
> mtd erase kernel
> mtd write kernel ${loadaddr}
> loadx
# send rootfs file (clfs.uImage) using your console emulator
> mtd erase rootfs
> mtd write rootfs ${loadaddr}
> loadx
# send dtb file (stm32f446-stlinux.dtb) using your console emulator
> mtd erase rootfs
> mtd write rootfs ${loadaddr}
```

Then you can simply reset the board or type `run bootcmd` and you should be
welcomed by kernel boot log.

### Loading QSPI Flash using SD card

Compared to using serial port, this one is much much faster because data are
transferred over SD card, not xmodem at 115200 bauds.

Create one partition on your SD card, format it as FAT32, and copy there
image files from build directory. Then insert your card to the board and run
following commands.

```bash
> fatload mmc 0:1 ${loadaddr} zImage
> mtd erase kernel
> mtd write kernel ${loadaddr}
> fatload mmc 0:1 ${loadaddr} clfs.uImage
> mtd erase rootfs
> mtd write rootfs ${loadaddr}
> fatload mmc 0:1 ${loadaddr} stm32f446-stlinux.dtb
> mtd erase rootfs
> mtd write rootfs ${loadaddr}
```

After reset, your board will boot from QSPI Flash or you can simply type
`run bootcmd`.

### Loading directly from SD card

If you are feeling lazy, just copypaste this to bootloader console. And it will boot.

```bash
fatload mmc 0:1 ${kernel_addr_l} zImage ; fatload mmc 0:1 ${rootfs_addr_l} clfs.uImage ; fatload mmc 0:1 ${dtb_addr_l} stm32f446-stlinux.dtb ; run set_args ; bootz ${kernel_addr_l} ${rootfs_addr_l} ${dtb_addr_l}
```

It will load everything from SD card into RAM and boot kernel that way.

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
