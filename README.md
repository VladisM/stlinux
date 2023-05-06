# STlinux

Simple board with STM32F446 and SDRAM that should be able
to run MMU less Linux build. Purposes of this projects are

 * Learn to use KiCad
 * Learn something more about uBoot
 * Learn something more about Linux (MMU less variant)

This project using submodules. After clone use following to init them.

```$ git submodule update --init --recursive```

## Manufacturing board

Gerber files required for manufacturing are stored in `ger` subdirectory.
To manufacture board you shall just find yourself a suitable manufacturer
and use latest archive.

To generate BOM you shall use KiCad and revisit folder `sch`. All project
files are stored there.

After soldering board you should apply ftdi configuration into
integrated UART bridge. Correct configuration is provided
by the file `ftdi_configuration.xml`. For that you are going
to need Windows PC and FT_PROG tool (available on their website).

To test if board was correctly manufactured and most importantly
assembled. You can use software available in `board_test` folder.

## License

Copyright © 2022 Vladislav Mlejnecký <v.mlejnecky@seznam.cz>

This work is free. You can redistribute it and/or modify it under the
terms of the Do What The Fuck You Want To Public License, Version 2,
as published by Sam Hocevar. See COPYING file for more details.
