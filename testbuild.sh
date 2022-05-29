#!/bin/sh
# Automated builds with various configs, should work on 
# range of Linux, SunOS (OpenIndiana), FreeBSD.

set -eux
# note: older *BSD, OSX, Solaris require a template
td=$(mktemp -d "${TMPDIR:-/tmp}/jvt.XXXXXXXX")
dd="DESTDIR=$td"
u=$(uname)
o=-O
e=
d=
x=
d=-DTERMIOS
t=-ltermcap
lib=
jv=NPROCESSORS_ONLN
case $u in
SunOS)  x="NROFF=nroff TROFF=troff"
        d="-DREALSTDC -DPOSIX_PROCS"
        ur=$(uname -r)
        case "$ur" in
        5.0) d="-DSYSVR4 -DGRANTPT_BUG";;
        5.*) ;;
        *) echo "$0: Not set up for testing $u $ur" >&2; exit 1;;
        esac
        xi=/usr/gnu/bin/install
        if test ! -x $xi; then
            xi=cp
        fi
        x="$x XINSTALL=$xi TINSTALL=$xi"
        ;;
Linux)  t=-lncurses
        if test -e /etc/alpine-release; then
            d="$d -DUSE_GETCWD"; lib=MUSL;
        elif test -e /etc/gentoo-release; then
            t="-ltinfo"; lib=GLIBC;
        else
            lib=GLIBC;
        fi
        jv=_NPROCESSORS_ONLN
        ;;
esac
if test -x /usr/bin/getconf; then j=-j$(getconf $jv); else j=; fi
# other than the first "make install" with $u, some of the others
# (e.g. PIPEPROCS, TERMINFO) may produce warnings (e.g. no declaration for
# ioctl) because we are just testing somewhat abnormal non-POSIX sysdefs.
make clean &&
make $j $x $dd/t10-$u install &&
make clean &&
make $j OPTFLAGS="$o" SYSDEFS="-DPIPEPROCS $d" TERMCAPLIB=$t $x $dd/t20-pipeprocs install &&
make clean &&
make $j OPTFLAGS="$o" SYSDEFS="-DBSDPOSIX $d" TERMCAPLIB=$t $x $dd/t30-bsdposix install &&
make clean &&
make $j OPTFLAGS="$o" SYSDEFS="-DBAREBONES -DSMALL -DJTC $d" TERMCAPLIB= $x $dd/t60-small install &&
make clean &&
case "$lib" in
GLIBC)
    make $j OPTFLAGS="$o" SYSDEFS="-DGLIBCPTY" TERMCAPLIB=$t EXTRALIBS=-lutil $x $dd/t40-glibcpty install &&
    make clean &&
    make $j OPTFLAGS="$o" SYSDEFS="-DTERMINFO -DUSE_VFORK" TERMCAPLIB=$t $x $dd/t50-vfork &&
    make clean
    ;;
esac &&
make $x tgz &&
if type zip 2> /dev/null; then make $x zip; fi &&
ls -s $(find $td | grep /bin/) &&
rm -rf $td &&
set +eu
