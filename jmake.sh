#!/bin/sh
# A minimal wrapper that auto-detects the OS on most modern 
# systems, and sets appropriate variables (compile flags etc)
# for make. For those who hate automatic transmission, just
# ignore this script and figure out the flags you want
# to give make yourself, as before.
# All command-line arguments are passed to make.
u=`uname | tr -d -c '[a-zA-Z0-9_]'`
cppflags="-D$u"	# see sysdep.h for symbols to define for porting Jove to various systems
cc=gcc
ldlibs=
ldflags=	# special link flags, usually none needed
case "$u" in
CYGWIN*)
	cppflags="-DCYGWIN"
	;;
*BSD|DragonFly)
	# openpty on BSD requires libutil
	ldlibs="-ltermcap -lutil"
	;;
SunOS)	cc=cc; ldlibs="-ltermcap";;
GNU|Linux)
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
	# if no ncurses, use built-in VT1xx code
	case "$jtc" in y) cppflags="$cppflags -DJTC";; esac
	# best to use openpty on Linux, so need libutil
	ldlibs="$ldlibs -lutil"
	;;
*)	cc=cc; cppflags="$cppflags -DJTC";;
esac
# default cflags (optimization, link) depend on compiler, prefer gcc-compatible
if test x$cc = xgcc -o x$cc = xclang; then
	cflags=${cflags-"-g -Os -Wall -Werror -pedantic"}
else
	cflags=${cflags-"-O"}  # unknown compiler, just try optimization
fi
exec make CPPFLAGS="$cppflags" CFLAGS="$cflags" LDLIBS="$ldlibs" LDFLAGS="$ldflags" "$@"
