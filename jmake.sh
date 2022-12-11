#!/bin/sh -eux
# NOTE: Jove does not use autotools/configure.  This is 
# a minimal convenience wrapper that crudely auto-detects the OS on 
# a few of the most common modern (2020+) open-source
# operating systems by using uname (which can be overridden for 
# testing or cross-build).  It sets appropriate variables
# (CC, CPPFLAGS, CFLAGS, LDLIBS, LDFLAGS) for make.
# All command-line arguments given to jmake.sh are
# passed directly onward to make.
# For distro maintainers or sysadmins with complex or multi-target
# build environments, or those who just prefer full control of config, 
# just ignore this script, invoke make directly with
# the flags you want (see last line for examples.
u=${JMAKE_UNAME-`uname | tr -d -c '[a-zA-Z0-9_]'`}
cc=${CC-cc}
cppflags="-D$u"	# see sysdep.h for symbols to define for porting Jove to various systems
cflags=
ldlibs=
ldflags=	# special link flags, usually none needed
extra=		# older UN*X (e.g Solaris, SunOS, etc, might need these)
case "$u" in
CYGWIN*)
	cc=${CC-gcc}
	cppflags="-DCYGWIN" # CYGWIN uname not legal cpp name
	;;
*BSD|DragonFly)
	# openpty on BSD requires libutil
	ldlibs="-ltermcap -lutil"
	;;
SunOS)	ldlibs="-ltermcap"
	extra="NROFF=nroff TROFF=troff"	
        xi=/usr/gnu/bin/install
        if test ! -x $xi; then
            xi=cp
        fi
        extra="$extra XINSTALL=$xi TINSTALL=$xi"
	;;
GNU|Linux)
	cc=${CC-gcc}
	if dpkg-buildflags > /dev/null 2>&1; then
		cppflags="$cppflags `dpkg-buildflags --get CPPFLAGS`"
		cflags="$cflags `dpkg-buildflags --get CFLAGS`"
		ldflags="$ldflags `dpkg-buildflags --get LDFLAGS`"
	fi
	jtc=y
	# see if we can find ncurses installed via pkgconf or pkg-config
        for pkc in pkgconf pkg-config; do
		if $pkc --version > /dev/null 2>&1; then
			if $pkc ncurses > /dev/null 2>&1; then
				cppflags="$cppflags `$pkc --cflags ncurses`"
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
	case "$jtc" in y) cppflags="$cppflags -DJTC";; esac
	# best to use openpty on Linux, so need libutil
	ldlibs="$ldlibs -lutil"
	;;
*)	# for unknown platforms, we try fairly generic, builtin vt1xx
	cc=${CC-cc}; cppflags="-DBSDPOSIX -DJTC"
	;;
esac
# default cflags (optimization, link) depend on compiler, prefer gcc-compatible
case "$cc" in
*gcc*|*clang*)
	cflags=${cflags-"-g -Os -Wall -Werror -pedantic"};;
*)
	cflags=${cflags-"-O"}  # unknown compiler, just try vanilla optimization
	;;
esac
exec make CC="$cc" CPPFLAGS="$cppflags" CFLAGS="$cflags" LDLIBS="$ldlibs" LDFLAGS="$ldflags" $extra "$@"
