#! /bin/bash

set -e

gcc_archive_name=arm-gnu-toolchain-12.2.rel1-x86_64-arm-none-eabi.tar.xz
gcc_archive_link=https://developer.arm.com/-/media/Files/downloads/gnu/12.2.rel1/binrel/$gcc_archive_name
gcc_archive_sum=84be93d0f9e96a15addd490b6e237f588c641c8afdf90e7610a628007fc96867
gcc_folder_name=$(basename -s '.tar.xz' $gcc_archive_name)

mkdir -p dwn
mkdir -p tools

[ ! -f ./dwn/checksum.txt ] && echo "$gcc_archive_sum  $gcc_archive_name" >> dwn/checksum.txt

[ ! -f ./dwn/$gcc_archive_name ] && cd dwn/ && wget  $gcc_archive_link && sha256sum -c checksum.txt && cd ..

[ ! -d ./tools/$gcc_folder_name ] && tar -C tools -xf dwn/$gcc_archive_name

[ ! -f ./toolchain.cmake ] && cat << EOF > toolchain.cmake
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_CROSSCOMPILING 1)
set(CMAKE_C_COMPILER    $(pwd)/tools/$gcc_folder_name/bin/arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER  $(pwd)/tools/$gcc_folder_name/bin/arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER  $(pwd)/tools/$gcc_folder_name/bin/arm-none-eabi-as)
set(CMAKE_OBJCOPY       $(pwd)/tools/$gcc_folder_name/bin/arm-none-eabi-objcopy)
set(CMAKE_SIZE          $(pwd)/tools/$gcc_folder_name/bin/arm-none-eabi-size)
set(CMAKE_GDB           $(pwd)/tools/$gcc_folder_name/bin/arm-none-eabi-gdb)
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)
set(CMAKE_NOT_USING_CONFIG_FLAGS true)
EOF

echo "All set!"
