#!/bin/sh
# A hackish autoconfig that gets the Jove build flags right on 
# most modern systems.
# Falls back to JTC (Jove built-in termcap for vt1xx) to avoid
# external dependencies.
u=`uname | tr -d -c '[a-zA-Z0-9_]'`
sysdefs="-D$u"	# see sysdep.h for symbols to define for porting Jove to various systems
cppflags=	# generally for -I or -D, usually none needed
cflags="-Os -Wall -Werror -pedantic"	# typically optimization and warning flags
ldlibs=		# often -ltermcap or -lncurses, sometimes with -lutil
ldflags=	# special link flags, usually none needed
case "$u" in
CYGWIN*) sysdefs="-DCYGWIN";;
*BSD|DragonFly)	ldlibs="-ltermcap -lutil";;
SunOS)	cflags="-O"; ldlibs="-ltermcap";;
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
		sysdefs="$sysdefs -DJTC"
	fi
	ldlibs="$ldlibs -lutil"
	;;
*)	sysdefs="$sysdefs -DJTC"; cflags="-O";;
esac
case "$1" in
--sysdefs)	echo "$sysdefs";;
--cppflags)	echo "$cppflags";;
--cflags)	echo "$cflags";;
--ldlibs)	echo "$ldlibs";;
--ldflags)	echo "$ldflags";;
*)	echo "Usage: $0 [--sysdefs] [--cppflags] [--cflags] [--ldlibs] [--ldflags]" >&2; exit 1;;
esac
