#! /bin/bash
#
# Bc. Vladislav Mlejnecký <v.mlejnecky@seznam.cz> (2023)
#
# Script for building linux based system for STM32F446 based device.

set -e
set +h

top=$(pwd)

# ------------------------------------------------------------------------------
# Configuration

build_dir=${top}/build

images=${build_dir}/images
tools=${build_dir}/tools
sysroot=${build_dir}/sysroot
staging=${build_dir}/staging

uboot_src=${top}/system/u-boot
linux_src=${top}/system/linux
busybox_src=${top}/system/busybox
gcc_src=${top}/system/gcc
uclibc_src=${top}/system/uclibc-ng
binutils_src=${top}/system/binutils
elf2flt_src=${top}/system/elf2flt

configs=${top}/system/configs
skeleton=${top}/system/skeleton

# ------------------------------------------------------------------------------
# Creating folder structure

do_dir_structure () {
	mkdir -p ${build_dir}
	mkdir -p ${images}
	mkdir -p ${tools}
	mkdir -p ${sysroot}
	mkdir -p ${staging}

	# creating sysroot
	if [ ! -L ${sysroot}/usr ]; then
		ln -sf . ${sysroot}/usr
	fi
}

# ------------------------------------------------------------------------------
# PATH variable

do_update_path () {
	export PATH=$(do_toolchain_get_path bin):$PATH
}

# ------------------------------------------------------------------------------
# Parsing arguments

action="none"
target="none"
core_count=1
beVerbose=0

usage () {
	cat > /dev/stdout <<- EOF
		Usage: $0 [OPTIONS]

		Script for developing CLFS like system for STM32F446 stlinux project.
		This script will let you to easily configure all parts of systems and
		provide a way for partial rebuilds (e.g. clean build of kernel only).

		Generic options:
		  -h   --help            Display this help text.
		  -j N                   Use N cores while compiling.
		  -v                     Be verbose during execution of script.
		       --build <target>  Building specified target.
		       --config <target> Edit configuration for target.
		       --clean <target>  Remove build files for given target.

		Targets to build:
		  headers,binutils,gcc_1,uclibc,gcc_2,elf2flt,toolchain,u-boot,linux,
		  dtb,busybox,skeleton,cpio,rootfs,all

		Targets to config:
		  uclibc,u-boot,linux,busybox

		Targets to clean:
		  binutils,uclibc,gcc,elf2flt,toolchain,u-boot,linux,dtb,busybox,
		  rootfs,cpio,all

		Example:
		  * Build whole system, all output files will be in '$(realpath --relative-to=$(pwd) ${images})'.
		      \$ $0 --build all -j 8
		  * Rebuild toolchain
		      \$ $0 --clean toolchain
		      \$ $0 --build toolchain -j 8
	EOF
}

parse_args () {
	VALID_ARGS=$(getopt -o "j:hv" -l help,build:,config:,clean: -- "$@")
	if [[ $? -ne 0 ]]; then
		exit 1;
	fi

	eval set -- "$VALID_ARGS"
	while [ : ]; do
		case "$1" in

			--build | --config | --clean)
				action=$(echo $1 | sed "s/--//")
				target=$2
				shift 2
				;;

			-h | --help)
				usage
				exit
				;;

			-j)
				core_count=$2
				shift 2
				;;

			-v)
				beVerbose=1
				shift
				;;

			--)
				shift;
				break
				;;
		esac
	done
}

verbose_echo () {
	if [ $beVerbose = 1 ]; then
		echo $1
	fi
}

# ------------------------------------------------------------------------------
# Toolchain integration

do_toolchain_get_path () {
	case $1 in
		"bin")
			echo ${tools}/toolchain/bin/
			;;
		"install")
			echo ${tools}/toolchain/
			;;
		"sysroot")
			echo ${sysroot}
			;;
		*)
			echo /dev/null
			;;
	esac
}

# ------------------------------------------------------------------------------
# Binutils

do_binutils_get_path () {
	case $1 in
		"build")
			echo ${build_dir}/build_binutils
			;;
		"src")
			echo ${binutils_src}
			;;
		"install")
			do_toolchain_get_path install
			;;
		*)
			echo /dev/null
			;;
	esac
}

do_binutils () {
	local build_path=$(do_binutils_get_path build)
	local src_path=$(do_binutils_get_path src)
	local install_path=$(do_binutils_get_path install)
	local sysroot_path=$(do_toolchain_get_path sysroot)

	mkdir -p ${build_path}

	pushd ${build_path} >> /dev/null
	${src_path}/configure \
		--prefix=${install_path} \
		--target=arm-none-uclinux-uclibcgnueabi \
		--with-sysroot=${sysroot_path} \
		--disable-gtk-doc \
		--disable-gtk-doc-html \
		--disable-doc \
		--disable-docs \
		--disable-documentation \
		--disable-debug \
		--with-xmlto=no \
		--with-fop=no \
		--disable-nls \
		--disable-dependency-tracking \
		--disable-multilib \
		--disable-werror \
		--disable-shared \
		--enable-static \
		--without-debuginfod \
		--enable-plugins \
		--enable-lto \
		--disable-sim \
		--disable-gdb \
		--disable-gprofng

	make -j ${core_count}
	make install
	popd >> /dev/null
}

# ------------------------------------------------------------------------------
# GCC

do_gcc_get_path () {
	case $1 in
		"build")
			echo ${build_dir}/build_gcc
			;;
		"src")
			echo ${gcc_src}
			;;
		"install")
			do_toolchain_get_path install
			;;
		*)
			echo /dev/null
			;;
	esac
}

do_gcc_pass_1 () {
	local build_path=$(do_gcc_get_path build)/pass_1
	local src_path=$(do_gcc_get_path src)
	local install_path=$(do_gcc_get_path install)
	local sysroot_path=$(do_toolchain_get_path sysroot)

	mkdir -p ${build_path}

	pushd ${build_path} >> /dev/null

	${src_path}/configure \
		--prefix=${install_path} \
		--target=arm-none-uclinux-uclibcgnueabi \
		--with-sysroot=${sysroot_path} \
		--enable-static \
		--disable-gtk-doc \
		--disable-gtk-doc-html \
		--disable-doc \
		--disable-docs \
		--disable-documentation \
		--disable-debug \
		--with-xmlto=no \
		--with-fop=no \
		--disable-nls \
		--disable-dependency-tracking \
		--enable-__cxa_atexit \
		--with-gnu-ld \
		--disable-libssp \
		--disable-multilib \
		--disable-decimal-float \
		--enable-plugins \
		--enable-lto \
		--without-zstd \
		--disable-libquadmath \
		--disable-libquadmath-support \
		--disable-libsanitizer \
		--disable-tls \
		--without-isl \
		--without-cloog \
		--disable-shared \
		--without-headers \
		--disable-threads \
		--with-newlib \
		--disable-largefile \
		--enable-languages=c \
		--with-float=soft \
		--with-mode=thumb \
		--with-cpu=cortex-m4

	make all-gcc all-target-libgcc -j${core_count}
	make install-gcc install-target-libgcc
	popd >> /dev/null
}

do_gcc_pass_2 () {
	local build_path=$(do_gcc_get_path build)/pass_2
	local src_path=$(do_gcc_get_path src)
	local install_path=$(do_gcc_get_path install)
	local sysroot_path=$(do_toolchain_get_path sysroot)

	mkdir -p ${build_path}

	pushd ${build_path} >> /dev/null

	${src_path}/configure \
		--prefix=${install_path} \
		--target=arm-none-uclinux-uclibcgnueabi \
		--with-sysroot=${sysroot_path} \
		--enable-__cxa_atexit \
		--with-gnu-ld \
		--disable-libssp \
		--disable-decimal-float \
		--enable-plugins \
		--enable-lto \
		--without-zstd \
		--disable-libquadmath \
		--disable-libquadmath-support \
		--disable-libsanitizer \
		--disable-tls \
		--enable-threads \
		--without-isl \
		--without-cloog \
		--disable-nls \
		--enable-static \
		--disable-shared \
		--disable-libgomp \
		--disable-multilib \
		--enable-languages=c \
		--with-float=soft \
		--with-mode=thumb \
		--with-cpu=cortex-m4 \
		--with-abi=aapcs-linux

	make -j${core_count}
	make install
	popd >> /dev/null
}

# ------------------------------------------------------------------------------
# uclibc

do_uclibc_get_path () {
	case $1 in
		"src")
			echo ${uclibc_src}
			;;
		"build")
			echo ${build_dir}/build_uclibc
			;;
		"defconfig")
			echo ${configs}/uclibc.config
			;;
		*)
			echo /dev/null
			;;
	esac
}

do_uclibc_config () {
	local src_path=$(do_uclibc_get_path src)
	local defconfig=$(do_uclibc_get_path defconfig)
	local build_path=$(do_uclibc_get_path build)

	pushd ${src_path} >> /dev/null

	if [ -f ${defconfig} ]; then
		cp ${defconfig} ${build_path}/.config
		make O=${build_path} menuconfig
	else
		make O=${build_path} allnoconfig
	fi

	mv ${build_path}/.config ${defconfig}

	popd >> /dev/null
}

do_uclibc () {
	local src_path=$(do_uclibc_get_path src)
	local sysroot_path=$(do_toolchain_get_path sysroot)
	local defconfig=$(do_uclibc_get_path defconfig)
	local build_path=$(do_uclibc_get_path build)

	mkdir -p ${build_path}

	pushd ${src_path} >> /dev/null
	cp ${defconfig} ${build_path}/.config

	make \
		CROSS_COMPILE=arm-none-uclinux-uclibcgnueabi- \
		KERNEL_HEADERS=${sysroot_path}/usr/include \
		O=${build_path} \
		PREFIX=${sysroot_path} \
		-j ${core_count} \
		install

	popd >> /dev/null
}

# ------------------------------------------------------------------------------
# elf2flt

do_elf2flt_get_path () {
	case $1 in
		"src")
			echo ${elf2flt_src}
			;;
		"build")
			echo ${build_dir}/build_elf2flt
			;;
		"install")
			do_toolchain_get_path install
			;;
		*)
			echo /dev/null
			;;
	esac
}

do_elf2flt () {
	local src_path=$(do_elf2flt_get_path src)
	local build_path=$(do_elf2flt_get_path build)
	local install_path=$(do_elf2flt_get_path install)

	mkdir -p ${build_path}

	pushd ${build_path} >> /dev/null
	${src_path}/configure \
		--target=arm-none-uclinux-uclibcgnueabi \
		--prefix=${install_path} \
		--with-libbfd=$(do_binutils_get_path build)/bfd/libbfd.a \
		--with-bfd-include-dir=$(do_binutils_get_path build)/bfd \
		--with-libiberty=$(do_binutils_get_path build)/libiberty/libiberty.a \
		--with-binutils-include-dir=$(do_binutils_get_path src)/include \
		--disable-werror \
		--enable-shared \
		--disable-static \
		--disable-gtk-doc \
		--disable-gtk-doc-html \
		--disable-doc \
		--disable-docs \
		--disable-documentation \
		--disable-debug \
		--with-xmlto=no \
		--with-fop=no \
		--disable-nls \
		--disable-dependency-tracking \

	make -j ${core_count}
	make install

	popd >> /dev/null
}

# ------------------------------------------------------------------------------
# U-boot

do_uboot_get_path () {
	case $1 in
		"build")
			echo ${build_dir}/build_u-boot
			;;
		"src")
			echo ${uboot_src}
			;;
		"image")
			echo ${images}/u-boot.bin
			;;
		*)
			echo /dev/null
			;;
	esac
}

do_uboot_config () {
	local build_path=$(do_uboot_get_path build)
	local src_path=$(do_uboot_get_path src)
	local image_path=$(do_uboot_get_path image)

	mkdir -p ${build_path}

	pushd ${src_path} >> /dev/null
	make ARCH=arm CROSS_COMPILE=arm-none-uclinux-uclibcgnueabi- O=${build_path} stm32f446-stlinux_defconfig
	make ARCH=arm CROSS_COMPILE=arm-none-uclinux-uclibcgnueabi- O=${build_path} menuconfig
	make ARCH=arm CROSS_COMPILE=arm-none-uclinux-uclibcgnueabi- O=${build_path} savedefconfig
	mv ${build_path}/defconfig ${src_path}/configs/stm32f446-stlinux_defconfig
	popd >> /dev/null
}

do_uboot () {
	local build_path=$(do_uboot_get_path build)
	local src_path=$(do_uboot_get_path src)
	local image_path=$(do_uboot_get_path image)

	mkdir -p ${build_path}

	pushd ${src_path} >> /dev/null
	make ARCH=arm CROSS_COMPILE=arm-none-uclinux-uclibcgnueabi- O=${build_path} stm32f446-stlinux_defconfig
	make ARCH=arm CROSS_COMPILE=arm-none-uclinux-uclibcgnueabi- O=${build_path} -j ${core_count}
	cp ${build_path}/u-boot.bin ${image_path}
	popd >> /dev/null
}

# ------------------------------------------------------------------------------
# Linux

do_linux_get_path () {
	case $1 in
		"build")
			echo ${build_dir}/build_linux
			;;
		"src")
			echo ${linux_src}
			;;
		"image")
			echo ${images}/zImage
			;;
		"dtb")
			echo ${images}/stm32f446-stlinux.dtb
			;;
		*)
			echo /dev/null
			;;
	esac
}

do_linux_config () {
	local build_path=$(do_linux_get_path build)
	local src_path=$(do_linux_get_path src)

	mkdir -p ${build_path}

	pushd ${src_path} >> /dev/null
	make ARCH=arm O=${build_path} stm32f446-stlinux_defconfig
	make ARCH=arm O=${build_path} menuconfig
	make ARCH=arm O=${build_path} savedefconfig
	mv ${build_path}/defconfig ${src_path}/arch/arm/configs/stm32f446-stlinux_defconfig
	popd >> /dev/null
}

do_linux_headers () {
	local build_path=$(do_linux_get_path build)
	local src_path=$(do_linux_get_path src)
	local sysroot_path=$(do_toolchain_get_path sysroot)

	mkdir -p ${build_path}

	pushd ${src_path} >> /dev/null
	make ARCH=arm INSTALL_HDR_PATH=${sysroot_path}/usr -j ${core_count} headers_install
	make ARCH=arm mrproper
	popd >> /dev/null
}

do_linux_kernel () {
	local build_path=$(do_linux_get_path build)
	local src_path=$(do_linux_get_path src)
	local image_path=$(do_linux_get_path image)

	mkdir -p ${build_path}

	pushd ${src_path} >> /dev/null
	make ARCH=arm CROSS_COMPILE=arm-none-uclinux-uclibcgnueabi- O=${build_path} stm32f446-stlinux_defconfig
	make ARCH=arm CROSS_COMPILE=arm-none-uclinux-uclibcgnueabi- O=${build_path} -j ${core_count} zImage
	cp ${build_path}/arch/arm/boot/zImage ${image_path}
	popd >> /dev/null
}

do_linux_dtb () {
	local build_path=$(do_linux_get_path build)
	local src_path=$(do_linux_get_path src)
	local dtb_path=$(do_linux_get_path dtb)

	mkdir -p ${build_path}

	pushd ${src_path} >> /dev/null
	make ARCH=arm CROSS_COMPILE=arm-none-uclinux-uclibcgnueabi- O=${build_path} stm32f446-stlinux_defconfig
	make ARCH=arm CROSS_COMPILE=arm-none-uclinux-uclibcgnueabi- O=${build_path} -j ${core_count} stm32f446-stlinux.dtb
	cp ${build_path}/arch/arm/boot/dts/stm32f446-stlinux.dtb ${dtb_path}
	popd >> /dev/null
}

# ------------------------------------------------------------------------------
# busybox

do_busybox_get_path () {
	case $1 in
		"src")
			echo ${busybox_src}
			;;
		"install")
			echo ${staging}
			;;
		"defconfig")
			echo ${configs}/busybox.config
			;;
		*)
			echo /dev/null
			;;
	esac
}

do_busybox_config () {
	local src_path=$(do_busybox_get_path src)
	local defconfig=$(do_busybox_get_path defconfig)

	pushd ${src_path} >> /dev/null

	if [ -f ${defconfig} ]; then
		cp ${defconfig} ${src_path}/.config
		make menuconfig
	else
		make allnoconfig
	fi

	cp ${src_path}/.config ${defconfig}
	make mrproper

	popd >> /dev/null
}

do_busybox () {
	local src_path=$(do_busybox_get_path src)
	local install_path=$(do_busybox_get_path install)
	local defconfig=$(do_busybox_get_path defconfig)

	pushd ${src_path} >> /dev/null
	cp ${defconfig} ${src_path}/.config
	make CROSS_COMPILE=arm-none-uclinux-uclibcgnueabi- SKIP_STRIP=y -j ${core_count}
	make CROSS_COMPILE=arm-none-uclinux-uclibcgnueabi- SKIP_STRIP=y CONFIG_PREFIX=${install_path} install
	make mrproper
	popd >> /dev/null
}

# ------------------------------------------------------------------------------
# System skeleton

do_skeleton_get_path () {
	case $1 in
		"src")
			echo ${skeleton}
			;;
		"install")
			echo ${staging}
			;;
		*)
			echo /dev/null
			;;
	esac
}

do_skeleton () {
	local src_path=$(do_skeleton_get_path src)
	local install_path=$(do_skeleton_get_path install)

	cp -va ${src_path}/* ${install_path}/
	find ${install_path} -name ".git_placeholder" -exec rm {} \;
}

# ------------------------------------------------------------------------------
# Integration (CPIO archive)

do_cpio_get_path () {
	case $1 in
		"image")
			echo ${images}/clfs.uImage
			;;
		"gz_image")
			echo $(do_cpio_get_path cpio_image).gz
			;;
		"cpio_image")
			echo $(do_cpio_get_path image | sed "s/.uImage/.cpio/")
			;;
		"staging")
			echo ${staging}
			;;
		*)
			echo /dev/null
			;;
	esac
}

do_cpio_archive () {
	local image_cpio=$(do_cpio_get_path cpio_image)
	local image_gz=$(do_cpio_get_path gz_image)
	local image=$(do_cpio_get_path image)
	local staging=$(do_cpio_get_path staging)

	pushd ${staging} >> /dev/null
	find . | cpio -H newc -ov --owner root:root > ${image_cpio}
	pushd $(dirname ${image_cpio}) >> /dev/null
	gzip $(basename ${image_cpio})
	mkimage -A arm -O linux -T ramdisk -d ${image_gz} ${image}
	rm $(basename ${image_gz})
	popd >> /dev/null
	popd >> /dev/null
}

# ------------------------------------------------------------------------------
# Control logic

main () {
	verbose_echo "executing '${action}' command"

	case $action in
		"build")

			do_dir_structure

			case $target in
				headers)
					do_linux_headers
					;;

				binutils)
					do_binutils
					;;

				gcc_1)
					do_gcc_pass_1
					;;

				uclibc)
					do_uclibc
					;;

				gcc_2)
					do_gcc_pass_2
					;;

				elf2flt)
					do_elf2flt
					;;

				u-boot)
					do_uboot
					;;

				linux)
					do_linux_kernel
					;;

				dtb)
					do_linux_dtb
					;;

				busybox)
					do_busybox
					;;

				skeleton)
					do_skeleton
					;;

				cpio)
					do_cpio_archive
					;;

				toolchain)
					do_linux_headers
					do_binutils
					do_gcc_pass_1
					do_uclibc
					do_gcc_pass_2
					do_elf2flt
					;;

				rootfs)
					do_busybox
					do_skeleton
					do_cpio_archive
					;;

				all)
					do_linux_headers
					do_binutils
					do_gcc_pass_1
					do_uclibc
					do_gcc_pass_2
					do_elf2flt
					do_uboot
					do_linux_kernel
					do_linux_dtb
					do_busybox
					do_skeleton
					do_cpio_archive
					;;

				*)
					echo "Unknown target '${target}' to build!"
					exit 1
			esac
			;;

		"config")

			do_dir_structure

			case $target in
				uclibc)
					do_uclibc_config
					;;

				u-boot)
					do_uboot_config
					;;

				linux)
					do_linux_config
					;;

				busybox)
					do_busybox_config
					;;

				*)
					echo "Unknown target '${target}' to configure!"
					exit 1
			esac
			;;

		"clean")
			case $target in
				uclibc)
					rm -rf $(do_uclibc_get_path build)
					;;

				gcc)
					rm -rf $(do_gcc_get_path build)
					;;

				binutils)
					rm -rf $(do_binutils_get_path build)
					;;

				elf2flt)
					rm -rf $(do_elf2flt_get_path build)
					;;

				toolchain)
					rm -rf $(do_uclibc_get_path build)
					rm -rf $(do_gcc_get_path build)
					rm -rf $(do_binutils_get_path build)
					rm -rf $(do_elf2flt_get_path build)
					;;

				u-boot)
					rm -rf $(do_uboot_get_path build)
					rm -rf $(do_uboot_get_path image)
					;;

				linux)
					rm -rf $(do_linux_get_path build)
					rm -rf $(do_linux_get_path image)
					rm -rf $(do_linux_get_path dtb)
					;;

				busybox)
					pushd $(do_busybox_get_path src) >> /dev/null
					make mrproper
					popd >> /dev/null
					;;

				cpio)
					rm -rf $(do_cpio_get_path image)
					;;

				rootfs)
					rm -rf ${staging}/*
					rm -rf $(do_cpio_get_path image)
					pushd $(do_busybox_get_path src) >> /dev/null
					make mrproper
					popd >> /dev/null
					;;

				all)
					rm -rf ${build_dir}

					pushd $(do_busybox_get_path src) >> /dev/null
					make mrproper
					popd >> /dev/null

					;;

				*)
					echo "Unknown target '${target}' to cleanup!"
					exit 1
			esac
			;;

		"none")
			echo "Nothing to do. Exit now."
			exit
			;;

		*)
			echo "Unknown action '${action}'!"
			exit 1;
			;;
	esac

	echo "Done!"
}

# ------------------------------------------------------------------------------
# main execution

parse_args $@
do_update_path
main
