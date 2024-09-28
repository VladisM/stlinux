#!/bin/bash
#
# Bc. Vladislav Mlejnecký <v.mlejnecky@seznam.cz> (2023)
#
# Script for building linux based system for STM32F446 based device.

# TODO: build busybox out of the tree

set -e
set +h

top=$(pwd)

# ------------------------------------------------------------------------------
# Configuration

tmp_dir=${top}/build
sys_dir=${top}/system

build_dir=${tmp_dir}/build
images=${tmp_dir}/images
tools=${tmp_dir}/tools
sysroot=${tmp_dir}/sysroot
staging=${tmp_dir}/staging
downloads=${tmp_dir}/downloads
sources=${tmp_dir}/sources

configs=${sys_dir}/configs
skeleton=${sys_dir}/skeleton
patches=${sys_dir}/patches

# ------------------------------------------------------------------------------
# misc

do_dir_structure () {
	mkdir -p ${build_dir}
	mkdir -p ${images}
	mkdir -p ${tools}
	mkdir -p ${sysroot}
	mkdir -p ${staging}
	mkdir -p ${downloads}
	mkdir -p ${sources}

	# creating sysroot
	if [ ! -L ${sysroot}/usr ]; then
		ln -sf . ${sysroot}/usr
	fi
}

do_update_path () {
	export PATH=$(do_toolchain_get_path bin):$PATH
}

verbose_echo () {
	if [ $beVerbose = 1 ]; then
		echo $1
	fi
}

# ------------------------------------------------------------------------------
# Parsing arguments

action="none"
target="none"
core_count=1
beVerbose=0

replacement_uboot=""
replacement_linux=""

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

		Replacing components:
		  --use-u-boot <path>    Use this source for u-boot instead.
		  --use-linux <path>     Use this source for linux kernel instead.

		Targets to build:
		  headers,binutils,gcc_1,uclibc,gcc_2,elf2flt,toolchain,u-boot,linux,
		  dtb,busybox,skeleton,cpio,rootfs,all

		Targets to config:
		  uclibc,u-boot,linux,busybox

		Targets to clean:
		  binutils,uclibc,gcc,elf2flt,toolchain,u-boot,linux,dtb,busybox,
		  rootfs,cpio,all

		Metatargets:
		  toolchnain: headers, bintuils, gcc_1, uclibc, gcc_2, elf2flt
		  rootfs: busybox, skeleton, cpio
		  all: toolchain, u-boot, linux, dtb, rootfs

		Example:
		  * Build whole system, all output files will be in '$(realpath --relative-to=$(pwd) ${images})'.
		      \$ $0 --build all -j 8
		  * Rebuild toolchain
		      \$ $0 --clean toolchain
		      \$ $0 --build toolchain -j 8
	EOF
}

parse_args () {
	VALID_ARGS=$(getopt -o "j:hv" -l help,build:,config:,clean:,use-u-boot:,use-linux: -- "$@")
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

			--use-u-boot)
				replacement_uboot=$(realpath $2)
				shift 2
				;;

			--use-linux)
				replacement_linux=$(realpath $2)
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

# ------------------------------------------------------------------------------
# Toolchain integration

do_toolchain_get_path () {
	local component="$1"

	declare -A paths=(
		["bin"]="${tools}/toolchain/bin/"
		["install"]="${tools}/toolchain/"
		["sysroot"]="${sysroot}"
	)

	echo "${paths[$component]:-/dev/null}"
}

# ------------------------------------------------------------------------------
# Binutils

do_binutils_get_path () {
	local component="$1"

	declare -A paths=(
		["url"]=https://sourceware.org/pub/binutils/releases/binutils-2.38.tar.gz
		["archive"]="${downloads}/binutils-2.38.tar.gz"
		["source"]="${sources}/binutils-2.38"
		["build"]="${build_dir}/binutils"
		["install"]="$(do_toolchain_get_path "install")"
	)

	echo "${paths[$component]:-/dev/null}"
}

do_binutils_download () {
	local url=$(do_binutils_get_path url)
	local archive=$(do_binutils_get_path archive)

	if [ ! -f ${archive} ]; then
		wget ${url} -O ${archive}
	fi
}

do_binutils_unpack () {
	local archive=$(do_binutils_get_path archive)
	local source_path=$(do_binutils_get_path source)

	if [ ! -d ${source_path} ]; then
		tar -xvf ${archive} -C ${sources}
	fi
}

do_binutils_build () {
	local build_path=$(do_binutils_get_path build)
	local source_path=$(do_binutils_get_path source)
	local install_path=$(do_binutils_get_path install)
	local sysroot_path=$(do_toolchain_get_path sysroot)

	mkdir -p ${build_path}

	pushd ${build_path} >> /dev/null
	${source_path}/configure \
		--prefix=${install_path} \
		--target="arm-none-uclinux-uclibcgnueabi" \
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

do_binutils () {
	local action=$1

	case $action in
		"build")
			do_binutils_download
			do_binutils_unpack
			do_binutils_build
			;;

		*)
			echo "Unknown action '${action}' for bintuils!"
			exit 1
			;;
	esac
}

# ------------------------------------------------------------------------------
# MPFR (dependency for GCC)

do_mpfr_get_path () {
	local component="$1"

	declare -A paths=(
		["url"]="https://ftp.gnu.org/gnu/mpfr/mpfr-4.2.1.tar.xz"
		["archive"]="${downloads}/mpfr-4.2.1.tar.xz"
		["source"]="${sources}/mpfr-4.2.1"
	)

	echo "${paths[$component]:-/dev/null}"
}

do_mpfr_download () {
	local url=$(do_mpfr_get_path url)
	local archive=$(do_mpfr_get_path archive)

	if [ ! -f ${archive} ]; then
		wget ${url} -O ${archive}
	fi
}

do_mpfr_unpack () {
	local archive=$(do_mpfr_get_path archive)
	local source_path=$(do_mpfr_get_path source)

	if [ ! -d ${source_path} ]; then
		tar -xvf ${archive} -C ${sources}
	fi
}

# ------------------------------------------------------------------------------
# GMP (dependency for GCC)

do_gmp_get_path () {
	local component="$1"

	declare -A paths=(
		["url"]="https://ftp.gnu.org/gnu/gmp/gmp-6.3.0.tar.xz"
		["archive"]="${downloads}/gmp-6.3.0.tar.xz"
		["source"]="${sources}/gmp-6.3.0"
	)

	echo "${paths[$component]:-/dev/null}"
}

do_gmp_download () {
	local url=$(do_gmp_get_path url)
	local archive=$(do_gmp_get_path archive)

	if [ ! -f ${archive} ]; then
		wget ${url} -O ${archive}
	fi
}

do_gmp_unpack () {
	local archive=$(do_gmp_get_path archive)
	local source_path=$(do_gmp_get_path source)

	if [ ! -d ${source_path} ]; then
		tar -xvf ${archive} -C ${sources}
	fi
}

# ------------------------------------------------------------------------------
# MPC (dependency for GCC)

do_mpc_get_path () {
	local component="$1"

	declare -A paths=(
		["url"]="https://ftp.gnu.org/gnu/mpc/mpc-1.3.1.tar.gz"
		["archive"]="${downloads}/mpc-1.3.1.tar.gz"
		["source"]="${sources}/mpc-1.3.1"
	)

	echo "${paths[$component]:-/dev/null}"
}

do_mpc_download () {
	local url=$(do_mpc_get_path url)
	local archive=$(do_mpc_get_path archive)

	if [ ! -f ${archive} ]; then
		wget ${url} -O ${archive}
	fi
}

do_mpc_unpack () {
	local archive=$(do_mpc_get_path archive)
	local source_path=$(do_mpc_get_path source)

	if [ ! -d ${source_path} ]; then
		tar -xvf ${archive} -C ${sources}
	fi
}

# ------------------------------------------------------------------------------
# GCC

do_gcc_get_path () {
	local component="$1"

	declare -A paths=(
		["url"]="https://gcc.gnu.org/pub/gcc/releases/gcc-11.5.0/gcc-11.5.0.tar.gz"
		["archive"]="${downloads}/gcc-11.5.0.tar.gz"
		["source"]="${sources}/gcc-11.5.0"
		["build"]="${build_dir}/gcc"
		["install"]="$(do_toolchain_get_path "install")"
	)

	echo "${paths[$component]:-/dev/null}"
}

do_gcc_download () {
	local url=$(do_gcc_get_path url)
	local archive=$(do_gcc_get_path archive)

	if [ ! -f ${archive} ]; then
		wget ${url} -O ${archive}
	fi

	do_mpfr_download
	do_gmp_download
	do_mpc_download
}

do_gcc_unpack () {
	local archive=$(do_gcc_get_path archive)
	local source_path=$(do_gcc_get_path source)

	if [ ! -d ${source_path} ]; then
		tar -xvf ${archive} -C ${sources}
	fi

	do_mpfr_unpack
	do_gmp_unpack
	do_mpc_unpack

	pushd ${source_path} >> /dev/null
	ln -sf $(do_mpfr_get_path source) mpfr
	ln -sf $(do_gmp_get_path source) gmp
	ln -sf $(do_mpc_get_path source) mpc
	popd >> /dev/null
}

do_gcc_pass_1 () {
	local build_path=$(do_gcc_get_path build)/pass_1
	local source_path=$(do_gcc_get_path source)
	local install_path=$(do_gcc_get_path install)
	local sysroot_path=$(do_toolchain_get_path sysroot)

	mkdir -p ${build_path}

	pushd ${build_path} >> /dev/null

	${source_path}/configure \
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
	local source_path=$(do_gcc_get_path source)
	local install_path=$(do_gcc_get_path install)
	local sysroot_path=$(do_toolchain_get_path sysroot)

	mkdir -p ${build_path}

	pushd ${build_path} >> /dev/null

	${source_path}/configure \
		--prefix=${install_path} \
		--target="arm-none-uclinux-uclibcgnueabi" \
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

do_gcc () {
	local action=$1

	case $action in
		"build_pass_1")
			do_gcc_download
			do_gcc_unpack
			do_gcc_pass_1
			;;

		"build_pass_2")
			do_gcc_download
			do_gcc_unpack
			do_gcc_pass_2
			;;

		*)
			echo "Unknown action '${action}' for gcc!"
			exit 1
			;;
	esac
}

# ------------------------------------------------------------------------------
# uclibc

do_uclibc_get_path () {
	local component="$1"

	declare -A paths=(
		["url"]="https://downloads.uclibc-ng.org/releases/1.0.43/uClibc-ng-1.0.43.tar.gz"
		["archive"]="${downloads}/uClibc-ng-1.0.43.tar.gz"
		["source"]="${sources}/uClibc-ng-1.0.43"
		["patch"]="${patches}/uclibc"
		["build"]="${build_dir}/uclibc"
		["defconfig"]="${configs}/uclibc.config"
	)

	echo "${paths[$component]:-/dev/null}"
}

do_uclibc_download () {
	local url=$(do_uclibc_get_path url)
	local archive=$(do_uclibc_get_path archive)

	if [ ! -f ${archive} ]; then
		wget ${url} -O ${archive}
	fi
}

do_uclibc_unpack () {
	local archive=$(do_uclibc_get_path archive)
	local source_path=$(do_uclibc_get_path source)

	if [ ! -d ${source_path} ]; then
		tar -xvf ${archive} -C ${sources}
		do_uclibc_patch
	fi
}

do_uclibc_patch () {
	local source_path=$(do_uclibc_get_path source)
	local patch_path=$(do_uclibc_get_path patch)

	if [ ! -f ${source_path} ]; then
		pushd ${source_path}
		for patch in $(ls "${patch_path}"/*.patch | sort); do
			patch -Np1 -i "$patch"
		done
		popd
	fi
}

do_uclibc_config () {
	local source_path=$(do_uclibc_get_path source)
	local defconfig=$(do_uclibc_get_path defconfig)
	local build_path=$(do_uclibc_get_path build)

	pushd ${source_path} >> /dev/null

	if [ -f ${defconfig} ]; then
		cp ${defconfig} ${build_path}/.config
		make O=${build_path} menuconfig
	else
		make O=${build_path} allnoconfig
	fi

	mv ${build_path}/.config ${defconfig}

	popd >> /dev/null
}

do_uclibc_build () {
	local source_path=$(do_uclibc_get_path source)
	local sysroot_path=$(do_toolchain_get_path sysroot)
	local defconfig=$(do_uclibc_get_path defconfig)
	local build_path=$(do_uclibc_get_path build)

	mkdir -p ${build_path}

	pushd ${source_path} >> /dev/null
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

do_uclibc () {
	local action=$1

	case $action in
		"config")
			do_uclibc_download
			do_uclibc_unpack
			do_uclibc_config
			;;

		"build")
			do_uclibc_download
			do_uclibc_unpack
			do_uclibc_build
			;;

		*)
			echo "Unknown action '${action}' for uclibc!"
			exit 1
			;;
	esac
}

# ------------------------------------------------------------------------------
# elf2flt

do_elf2flt_get_path () {
	local component="$1"

	declare -A paths=(
		["url"]="https://github.com/uclinux-dev/elf2flt/archive/refs/tags/v2023.04.tar.gz"
		["archive"]="${downloads}/elf2flt-2023.04.tar.gz"
		["source"]="${sources}/elf2flt-2023.04"
		["build"]="${build_dir}/elf2flt"
		["install"]="$(do_toolchain_get_path install)"
	)

	echo "${paths[$component]:-/dev/null}"
}

do_elf2flt_download () {
	local url=$(do_elf2flt_get_path url)
	local archive=$(do_elf2flt_get_path archive)

	if [ ! -f ${archive} ]; then
		wget ${url} -O ${archive}
	fi
}

do_elf2flt_unpack () {
	local archive=$(do_elf2flt_get_path archive)
	local source_path=$(do_elf2flt_get_path source)

	if [ ! -d ${source_path} ]; then
		tar -xvf ${archive} -C ${sources}
	fi
}

do_elf2flt_build () {
	local source_path=$(do_elf2flt_get_path source)
	local build_path=$(do_elf2flt_get_path build)
	local install_path=$(do_elf2flt_get_path install)

	mkdir -p ${build_path}

	pushd ${build_path} >> /dev/null

	${source_path}/configure \
		--target=arm-none-uclinux-uclibcgnueabi \
		--prefix=${install_path} \
		--with-libbfd=$(do_binutils_get_path build)/bfd/libbfd.a \
		--with-bfd-include-dir=$(do_binutils_get_path build)/bfd \
		--with-libiberty=$(do_binutils_get_path build)/libiberty/libiberty.a \
		--with-binutils-include-dir=$(do_binutils_get_path source)/include \
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

do_elf2flt () {
	local action=$1

	case $action in
		"build")
			do_elf2flt_download
			do_elf2flt_unpack
			do_elf2flt_build
			;;

		*)
			echo "Unknown action '${action}' for elf2flt!"
			exit 1
			;;
	esac
}

# ------------------------------------------------------------------------------
# U-boot

do_uboot_handle_source_path () {
	if [ -z ${replacement_uboot} ]; then
		echo "${sources}/u-boot-v2024.07"
	else
		echo ${replacement_uboot}
	fi
}

do_uboot_handle_patch_path () {
	if [ -z ${replacement_uboot} ]; then
		echo "${patches}/u-boot"
	else
		echo ""
	fi
}

do_uboot_get_path () {
	local component="$1"

	declare -A paths=(
		["url"]="https://source.denx.de/u-boot/u-boot/-/archive/v2024.07/u-boot-v2024.07.tar.gz"
		["archive"]="${downloads}/u-boot-v2024.07.tar.gz"
		["source"]=$(do_uboot_handle_source_path)
		["patch"]=$(do_uboot_handle_patch_path)
		["build"]="${build_dir}/u-boot"
		["image"]="${images}/u-boot.bin"
	)

	echo "${paths[$component]:-/dev/null}"
}

do_uboot_download () {
	local url=$(do_uboot_get_path url)
	local archive=$(do_uboot_get_path archive)

	if [ -z ${replacement_uboot} ]; then
		if [ ! -f ${archive} ]; then
			wget ${url} -O ${archive}
		fi
	fi
}

do_uboot_unpack () {
	local archive=$(do_uboot_get_path archive)
	local source_path=$(do_uboot_get_path source)

	if [ -z ${replacement_uboot} ]; then
		if [ ! -d ${source_path} ]; then
			tar -xvf ${archive} -C ${sources}
			do_uboot_patch
		fi
	fi
}

do_uboot_patch () {
	local source_path=$(do_uboot_get_path source)
	local patch_path=$(do_uboot_get_path patch)

	if [ -z ${replacement_uboot} ]; then
		if [ ! -f ${source_path} ]; then
			pushd ${source_path}
			for patch in $(ls "${patch_path}"/*.patch | sort); do
				patch -Np1 -i "$patch"
			done
			popd
		fi
	fi
}

do_uboot_config () {
	local build_path=$(do_uboot_get_path build)
	local source_path=$(do_uboot_get_path source)
	local image_path=$(do_uboot_get_path image)

	mkdir -p ${build_path}

	pushd ${source_path} >> /dev/null

	make ARCH=arm CROSS_COMPILE=arm-none-uclinux-uclibcgnueabi- O=${build_path} stm32f446-stlinux_defconfig
	make ARCH=arm CROSS_COMPILE=arm-none-uclinux-uclibcgnueabi- O=${build_path} menuconfig
	make ARCH=arm CROSS_COMPILE=arm-none-uclinux-uclibcgnueabi- O=${build_path} savedefconfig
	mv ${build_path}/defconfig ${source_path}/configs/stm32f446-stlinux_defconfig

	popd >> /dev/null
}

do_uboot_build () {
	local build_path=$(do_uboot_get_path build)
	local source_path=$(do_uboot_get_path source)
	local image_path=$(do_uboot_get_path image)

	mkdir -p ${build_path}

	pushd ${source_path} >> /dev/null

	make ARCH=arm CROSS_COMPILE=arm-none-uclinux-uclibcgnueabi- O=${build_path} stm32f446-stlinux_defconfig
	make ARCH=arm CROSS_COMPILE=arm-none-uclinux-uclibcgnueabi- O=${build_path} -j ${core_count}
	cp ${build_path}/u-boot.bin ${image_path}

	popd >> /dev/null
}

do_uboot () {
	local action=$1

	case $action in
		"config")
			do_uboot_download
			do_uboot_unpack
			do_uboot_config
			;;

		"build")
			do_uboot_download
			do_uboot_unpack
			do_uboot_build
			;;

		*)
			echo "Unknown action '${action}' for uboot!"
			exit 1
			;;
	esac
}

# ------------------------------------------------------------------------------
# Linux

do_linux_handle_source_path () {
	if [ -z ${replacement_linux} ]; then
		echo "${sources}/linux-6.11"
	else
		echo ${replacement_linux}
	fi
}

do_linux_handle_patch_path () {
	if [ -z ${replacement_linux} ]; then
		echo "${patches}/linux"
	else
		echo ""
	fi
}

do_linux_get_path () {
	local component="$1"

	declare -A paths=(
		["url"]="https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.11.tar.gz"
		["archive"]="${downloads}/linux-6.11.tar.gz"
		["source"]=$(do_linux_handle_source_path)
		["patch"]=$(do_linux_handle_patch_path)
		["build"]="${build_dir}/linux"
		["image"]="${images}/zImage"
		["dtb"]="${images}/stm32f446-stlinux.dtb"
	)

	echo "${paths[$component]:-/dev/null}"
}

do_linux_download () {
	local url=$(do_linux_get_path url)
	local archive=$(do_linux_get_path archive)

	if [ -z ${replacement_linux} ]; then
		if [ ! -f ${archive} ]; then
			wget ${url} -O ${archive}
		fi
	fi
}

do_linux_unpack () {
	local archive=$(do_linux_get_path archive)
	local source_path=$(do_linux_get_path source)

	if [ -z ${replacement_linux} ]; then
		if [ ! -d ${source_path} ]; then
			tar -xvf ${archive} -C ${sources}
			do_linux_patch
		fi
	fi
}

do_linux_patch () {
	local source_path=$(do_linux_get_path source)
	local patch_path=$(do_linux_get_path patch)

	if [ -z ${replacement_linux} ]; then
		if [ ! -f ${source_path} ]; then
			pushd ${source_path}
			for patch in $(ls "${patch_path}"/*.patch | sort); do
				patch -Np1 -i "$patch"
			done
			popd
		fi
	fi
}

do_linux_config () {
	local build_path=$(do_linux_get_path build)
	local source_path=$(do_linux_get_path source)

	mkdir -p ${build_path}

	pushd ${source_path} >> /dev/null

	make ARCH=arm O=${build_path} stm32f446-stlinux_defconfig
	make ARCH=arm O=${build_path} menuconfig
	make ARCH=arm O=${build_path} savedefconfig
	mv ${build_path}/defconfig ${source_path}/arch/arm/configs/stm32f446-stlinux_defconfig

	popd >> /dev/null
}

do_linux_headers () {
	local build_path=$(do_linux_get_path build)
	local source_path=$(do_linux_get_path source)
	local sysroot_path=$(do_toolchain_get_path sysroot)

	mkdir -p ${build_path}

	pushd ${source_path} >> /dev/null

	make ARCH=arm O=${build_path} stm32f446-stlinux_defconfig
	make ARCH=arm O=${build_path} INSTALL_HDR_PATH=${sysroot_path}/usr -j ${core_count} headers_install
	make ARCH=arm O=${build_path} mrproper

	popd >> /dev/null
}

do_linux_kernel () {
	local build_path=$(do_linux_get_path build)
	local source_path=$(do_linux_get_path source)
	local image_path=$(do_linux_get_path image)

	mkdir -p ${build_path}

	pushd ${source_path} >> /dev/null

	make ARCH=arm CROSS_COMPILE=arm-none-uclinux-uclibcgnueabi- O=${build_path} stm32f446-stlinux_defconfig
	make ARCH=arm CROSS_COMPILE=arm-none-uclinux-uclibcgnueabi- O=${build_path} -j ${core_count} zImage
	cp ${build_path}/arch/arm/boot/zImage ${image_path}

	popd >> /dev/null
}

do_linux_dtb () {
	local build_path=$(do_linux_get_path build)
	local source_path=$(do_linux_get_path source)
	local dtb_path=$(do_linux_get_path dtb)

	mkdir -p ${build_path}

	pushd ${source_path} >> /dev/null

	make ARCH=arm CROSS_COMPILE=arm-none-uclinux-uclibcgnueabi- O=${build_path} stm32f446-stlinux_defconfig
	make ARCH=arm CROSS_COMPILE=arm-none-uclinux-uclibcgnueabi- O=${build_path} -j ${core_count} st/stm32f446-stlinux.dtb
	cp ${build_path}/arch/arm/boot/dts/st/stm32f446-stlinux.dtb ${dtb_path}

	popd >> /dev/null
}

do_linux () {
	local action=$1

	case $action in
		"config")
			do_linux_download
			do_linux_unpack
			do_linux_config
			;;

		"headers")
			do_linux_download
			do_linux_unpack
			do_linux_headers
			;;

		"kernel")
			do_linux_download
			do_linux_unpack
			do_linux_kernel
			;;

		"dtb")
			do_linux_download
			do_linux_unpack
			do_linux_dtb
			;;

		*)
			echo "Unknown action '${action}' for uboot!"
			exit 1
			;;
	esac
}

# ------------------------------------------------------------------------------
# busybox

do_busybox_get_path () {
	local component="$1"

	declare -A paths=(
		["url"]="https://busybox.net/downloads/busybox-1.36.1.tar.bz2"
		["archive"]="${downloads}/busybox-1.36.1.tar.bz2"
		["source"]="${sources}/busybox-1.36.1"
		["patch"]="${patches}/busybox"
		["install"]="${staging}"
		["defconfig"]="${configs}/busybox.config"
	)

	echo "${paths[$component]:-/dev/null}"
}

do_busybox_download () {
	local url=$(do_busybox_get_path url)
	local archive=$(do_busybox_get_path archive)

	if [ ! -f ${archive} ]; then
		wget ${url} -O ${archive}
	fi
}

do_busybox_unpack () {
	local archive=$(do_busybox_get_path archive)
	local source_path=$(do_busybox_get_path source)

	if [ ! -d ${source_path} ]; then
		tar -xvf ${archive} -C ${sources}
		do_busybox_patch
	fi
}

do_busybox_patch () {
	local source_path=$(do_busybox_get_path source)
	local patch_path=$(do_busybox_get_path patch)

	if [ ! -f ${source_path} ]; then
		pushd ${source_path}
		for patch in $(ls "${patch_path}"/*.patch | sort); do
			patch -Np1 -i "$patch"
		done
		popd
	fi
}

do_busybox_config () {
	local source_path=$(do_busybox_get_path source)
	local defconfig=$(do_busybox_get_path defconfig)

	pushd ${source_path} >> /dev/null

	if [ -f ${defconfig} ]; then
		cp ${defconfig} ${source_path}/.config
		make menuconfig
	else
		make allnoconfig
	fi

	cp ${source_path}/.config ${defconfig}
	make mrproper

	popd >> /dev/null
}

do_busybox_build () {
	local source_path=$(do_busybox_get_path source)
	local install_path=$(do_busybox_get_path install)
	local defconfig=$(do_busybox_get_path defconfig)

	pushd ${source_path} >> /dev/null

	cp ${defconfig} ${source_path}/.config
	make CROSS_COMPILE=arm-none-uclinux-uclibcgnueabi- SKIP_STRIP=y -j ${core_count}
	make CROSS_COMPILE=arm-none-uclinux-uclibcgnueabi- SKIP_STRIP=y CONFIG_PREFIX=${install_path} install
	make mrproper

	popd >> /dev/null
}

do_busybox () {
	local action=$1

	case $action in
		"config")
			do_busybox_download
			do_busybox_unpack
			do_busybox_config
			;;

		"build")
			do_busybox_download
			do_busybox_unpack
			do_busybox_build
			;;

		*)
			echo "Unknown action '${action}' for busybox!"
			exit 1
			;;
	esac
}

# ------------------------------------------------------------------------------
# System skeleton

do_skeleton_get_path () {
	local component="$1"

	declare -A paths=(
		["source"]="${skeleton}"
		["install"]="${staging}"
	)

	echo "${paths[$component]:-/dev/null}"
}

do_skeleton () {
	local source_path=$(do_skeleton_get_path source)
	local install_path=$(do_skeleton_get_path install)

	cp -va ${source_path}/* ${install_path}/
	find ${install_path} -name ".git_placeholder" -exec rm {} \;
}

# ------------------------------------------------------------------------------
# Integration (CPIO archive)

do_cpio_get_path () {
	local component="$1"

	declare -A paths=(
		["image"]="${images}/clfs.uImage"
		["gz_image"]="${images}/clfs.cpio.gz"
		["cpio_image"]="${images}/clfs.cpio"
		["staging"]="${staging}"
	)

	echo "${paths[$component]:-/dev/null}"
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
					do_linux headers
					;;

				binutils)
					do_binutils ${action}
					;;

				gcc_1)
					do_gcc build_pass_1
					;;

				uclibc)
					do_uclibc ${action}
					;;

				gcc_2)
					do_gcc build_pass_2
					;;

				elf2flt)
					do_elf2flt ${action}
					;;

				u-boot)
					do_uboot ${action}
					;;

				linux)
					do_linux kernel
					;;

				dtb)
					do_linux dtb
					;;

				busybox)
					do_busybox ${action}
					;;

				skeleton)
					do_skeleton
					;;

				cpio)
					do_cpio_archive
					;;

				toolchain)
					do_linux headers
					do_binutils ${action}
					do_gcc build_pass_1
					do_uclibc ${action}
					do_gcc build_pass_2
					do_elf2flt ${action}
					;;

				rootfs)
					do_busybox ${action}
					do_skeleton
					do_cpio_archive
					;;

				all)
					do_linux headers
					do_binutils ${action}
					do_gcc build_pass_1
					do_uclibc ${action}
					do_gcc build_pass_2
					do_elf2flt ${action}
					do_uboot ${action}
					do_linux kernel
					do_linux dtb
					do_busybox ${action}
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
					do_uclibc ${action}
					;;

				u-boot)
					do_uboot ${action}
					;;

				linux)
					do_linux ${action}
					;;

				busybox)
					do_busybox ${action}
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
					if [ -d $(do_busybox_get_path source) ]; then
						pushd $(do_busybox_get_path source) >> /dev/null
						make mrproper
						popd >> /dev/null
					fi
					;;

				cpio)
					rm -rf $(do_cpio_get_path image)
					;;

				rootfs)
					rm -rf ${staging}/*
					rm -rf $(do_cpio_get_path image)
					if [ -d $(do_busybox_get_path source) ]; then
						pushd $(do_busybox_get_path source) >> /dev/null
						make mrproper
						popd >> /dev/null
					fi
					;;

				all)
					rm -rf ${tmp_dir}
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
