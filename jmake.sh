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
	ldlibs="-ltermcap -lutil"
	;;
SunOS)	cc=cc; ldlibs="-ltermcap";;
GNU|Linux)
	if dpkg-buildflags > /dev/null 2>&1; then
		cppflags="$cppflags `dpkg-buildflags --get CPPFLAGS`"
		cflags="$cflags `dpkg-buildflags --get CFLAGS`"
		ldflags="$ldflags `dpkg-buildflags --get LDFLAGS`"
	fi	
	if pkg-config ncurses > /dev/null 2>&1; then
		cppflags="$cppflags `pkg-config --cflags ncurses`"
		ldlibs="$ldlibs `pkg-config --libs ncurses`"
	else
		cppflags="$cppflags -DJTC"
	fi
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
