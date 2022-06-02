#!/bin/sh
# Automated builds with various configs, should work on 
# range of Linux, SunOS (OpenIndiana), FreeBSD.

: ${TB_MACH=$(uname -m)}
: ${TB_OS=$(uname)}
: ${TB_REV=$(uname -r)}

echo TB_MACH=$TB_MACH TB_OS=$TB_OS TB_REV=$TB_REV
echo
for f in /etc/*-release; do if test -r "$f"; then echo ">>> $f"; cat "$f"; echo; fi; done

set -eux
case $# in
0)	;;
*)	case "$1" in
	--preinstall) 
		SUDO=
		;;
	--sudo-preinstall)
		SUDO=sudo
		;;
	*) 	echo "Usage: $0 --preinstall|--sudo-preinstall" >&2
		exit 1
		;;
	esac
	if type apt-get 2> /dev/null; then
		$SUDO apt-get update
		$SUDO env DEBIAN_FRONTEND=noninteractive apt-get install --no-install-recommends -y gcc make pkg-config ncurses-dev exuberant-ctags zip groff
		case "$TB_MACH" in
		x86_64) 
			$SUDO env DEBIAN_FRONTEND=noninteractive apt-get install --no-install-recommends -y mingw-w64
			;;
		esac
	elif type apk 2> /dev/null; then
		$SUDO apk update && apk add gcc make pkgconfig musl-dev ncurses-dev ctags zip groff 
	elif type yum 2> /dev/null; then
		$SUDO yum install -y make gcc ncurses-devel zip groff ctags rpm-build
	elif type brew 2> /dev/null; then
		$SUDO brew install make ctags zip ncurses
	elif type pacman 2> /dev/null; then
		$SUDO pacman -Sy gcc make pkgconf ncurses
	elif type nix-shell 2> /dev/null; then
		$SUDO nix-shell -p gnumake
	else
		# not a packaging system we know, hope it already has minimal testbuild needs
		type cc make mktemp
	fi
	;;
esac

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
case $TB_OS in
SunOS)  x="NROFF=nroff TROFF=troff"
        d="-DREALSTDC -DPOSIX_PROCS"
        case "$TB_REV" in
        5.0) d="-DSYSVR4 -DGRANTPT_BUG";;
        5.*) ;;
        *) echo "$0: Not set up for testing $TB_OS $TB_REV" >&2; exit 1;;
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
# other than the first "make install" with $TB_OS, some of the others
# (e.g. PIPEPROCS, TERMINFO) may produce warnings (e.g. no declaration for
# ioctl) because we are just testing somewhat abnormal non-POSIX sysdefs.
make clean &&
make $j $x $dd/t10-$TB_OS install &&
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
make $x tgz && if test -d DIST; then mv jove-*.tgz DIST; fi &&
if type zip 2> /dev/null; then
	make $x zip && 
	if test -d DIST; then
		mv jove*s.zip DIST
	fi
fi &&
if test -e /etc/redhat-release -a -d DIST; then
	make rpm &&
	if test DIST; then
		rpm -i $HOME/rpmbuild/RPMS/x86_64/jove-[4-9]*.rpm &&
		mv $HOME/rpmbuild/*RPMS/x86_64/jove-[4-9]*.rpm DIST
	fi
elif test -e /etc/alpine-release -a -d DIST; then
	make .version &&
	u=$(uname) &&
	r=$(cat .version)-$(uname)-$(uname -m)-static &&
	make SYSDEFS="-D$TB_OS -DJTC" TERMCAPLIB= OPTFLAGS="-Os -static" &&
	strip jjove recover teachjove &&
	mv jjove DIST/jove-$r/jove &&
	mv recover teachjove $DIST/jove-$r &&
	cp -pr README paths.h doc DIST/jove-$r/ &&
	tar -c -j -v -f DIST/jove-$r.tar.bz2 DIST/jove-$r
elif type i686-w64-mingw32-gcc  2> /dev/null && test -e /etc/debian_version -a -d DIST; then
	make CC=i686-w64-mingw32-gcc SYSDEFS="-DMINGW" LOCALCC=gcc TERMCAPLIB= XEXT=.exe EXTRAOBJS="win32.o jjove.coff" EXTRALIBS=-lcomdlg32 &&
	make XEXT=.exe clean
fi &&
ls -s $(find $td | grep /bin/) &&
rm -rf $td &&
set +eu
