#!/bin/sh -eux
# NOTE: Jove does not use autotools/configure.  This is 
# a minimal convenience wrapper that crudely auto-detects the OS on 
# a few of the most common modern (2020+) open-source
# operating systems by using uname (which can be overridden for 
# testing or cross-build).  It sets appropriate variables
# (CC, SYSDEFS, OPTFLAGS, LDLIBS, LDFLAGS) for make.
# All command-line arguments given to jmake.sh are
# passed directly onward to make.
# For distro maintainers or sysadmins with complex or multi-target
# build environments, or those who just prefer full control of config, 
# just ignore this script, invoke make directly with
# the flags you want (see last line for examples.
u=${JMAKE_UNAME-`uname | tr -d -c '[a-zA-Z0-9_]'`}
cc=${CC-cc}
sysdefs="-D$u"	# see sysdep.h for symbols to define for porting Jove to various systems
# most modern compilers are gcc-compatible (even if called cc)
optflags=${CFLAGS-"-g -Os -Wall -Werror -pedantic"}
ldlibs=
ldflags=	# special link flags, usually none needed
extra=		# older UN*X (e.g Solaris, SunOS, etc, might need these)
case "$u" in
CYGWIN*)
	sysdefs="-DCYGWIN"
	;;
*BSD|DragonFly)
	# openpty on BSD requires libutil
	ldlibs="-ltermcap -lutil"
	;;
SunOS)	optflags=-O # generally not gcc
	ldlibs="-ltermcap"
	extra="NROFF=nroff TROFF=troff"	
        xi=/usr/gnu/bin/install
        if test ! -x $xi; then
            xi=cp
        fi
        extra="$extra XINSTALL=$xi TINSTALL=$xi"
	;;
GNU|Linux)
	if dpkg-buildflags > /dev/null 2>&1; then
		sysdefs="$sysdefs `dpkg-buildflags --get CPPFLAGS`"
		optflags="$optflags `dpkg-buildflags --get CFLAGS`"
		ldflags="$ldflags `dpkg-buildflags --get LDFLAGS`"
	fi
	jtc=y
	# see if we can find ncurses installed via pkgconf or pkg-config
	# generally results in no cflags, but some libs
        for pkc in pkgconf pkg-config; do
		if $pkc --version > /dev/null 2>&1; then
			if $pkc ncurses > /dev/null 2>&1; then
				optflags="$optflags `$pkc --cflags ncurses`"
				ldlibs="$ldlibs `$pkc --libs ncurses`"
				jtc=n
			fi
			break
		fi
	done
	if test x$jtc = xy -a -e /etc/gentoo-release; then
		ldlibs="$ldlibs -ltinfo"
		jtc=n
	fi
	# if no ncurses, use built-in VT1xx code
	case "$jtc" in y) sysdefs="$sysdefs -DJTC";; esac
	# best to use openpty on Linux, so need libutil
	ldlibs="$ldlibs -lutil"
	;;
*)	# for unknown platforms, we try fairly generic, builtin vt1xx
	sysdefs="-DBSDPOSIX -DJTC"; optflags="-O"
	;;
esac
exec make ${JMAKE_OPTS-} CC="$cc" SYSDEFS="$sysdefs" OPTFLAGS="$optflags" LDLIBS="$ldlibs" LDFLAGS="$ldflags" $extra "$@"
