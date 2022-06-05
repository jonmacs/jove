#!/bin/sh
# A hackish autoconfig that gets the Jove build flags right on 
# most modern systems.
# Falls back to JTC (Jove built-in termcap for vt1xx) to avoid
# external dependencies.
u=`uname | tr -d -c '[a-zA-Z0-9_]'`
o="-Os -Wall -Werror -pedantic"
cf="-D$u $o"
lf=
case "$u" in
CYGWIN*) cf="-DCYGWIN $o";;
*BSD|DragonFly)	lf="-ltermcap -lutil";;
SunOS)	cf="-D$u -O"; lf="-ltermcap";;
GNU|Linux)
	if pkg-config ncurses > /dev/null 2>&1; then
		cf="$cf `pkg-config --cflags ncurses`"
		lf="$lf `pkg-config --libs ncurses`"
	else
		cf="$cf -DJTC"
	fi
	lf="$lf -lutil"
	;;
*)	cf="-D$u -DJTC -O";;
esac
case "$1" in
--cflags)	echo "$cf";;
--libs)		echo "$lf";;
*)	echo "Usage: $0 [--cflags] [--libs]" >&2; exit 1;;
esac
