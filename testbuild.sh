#!/bin/sh
# Automated builds with various configs, should work on 
# range of Linux, SunOS (OpenIndiana), FreeBSD.

: ${TB_MACH=$(uname -m)}
: ${TB_OS=$(uname)}
: ${TB_REV=$(uname -r)}
: ${TB_NODE=$(uname -n)}
: ${TB_OPTFLAGS=-O}

dist=DIST/$TB_OS-$TB_MACH-$TB_NODE
if test ! -d $dist; then mkdir -p $dist; fi

(echo TB_MACH=$TB_MACH TB_OS=$TB_OS TB_REV=$TB_REV
echo
for f in /etc/*-release; do if test -e "$f"; then echo ">>> $f"; cat "$f"; echo; fi; done
) | tee $dist/TBINFO

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
		$SUDO env DEBIAN_FRONTEND=noninteractive apt-get install --no-install-recommends -y gcc make pkg-config ncurses-dev groff exuberant-ctags zip dpkg-dev bzip2
		case "$TB_MACH" in
		x86_64) 
			$SUDO env DEBIAN_FRONTEND=noninteractive apt-get install --no-install-recommends -y mingw-w64
			;;
		esac
		# similar OPTFLAGS to what Cord uses for debian packaging
		TB_OPTFLAGS="-g -O2 -ffile-prefix-map=$PWD=. -fstack-protector-strong -Wformat -Werror=format-security -Wdate-time -D_FORTIFY_SOURCE=2"
	elif type apk 2> /dev/null; then
		$SUDO apk update && apk add gcc make pkgconfig musl-dev ncurses-dev groff ctags zip
	elif type yum 2> /dev/null; then
		$SUDO yum install -y make gcc ncurses-devel groff ctags zip rpm-build
	elif type brew 2> /dev/null; then
		brew install make ctags zip ncurses
	elif type pacman 2> /dev/null; then
		$SUDO pacman -Sy --noconfirm gcc make pkgconf ncurses groff ctags zip
	elif type nix-shell 2> /dev/null; then
		$SUDO nix-shell -p gnumake
	else
		# not a packaging system we know, hope it already has minimal testbuild needs
		type cc make mktemp
	fi
	;;
esac

# directory for artifacts
make .version
ver=$(cat .version)

# note: older *BSD, OSX, Solaris require a template
td=$(mktemp -d "${TMPDIR:-/tmp}/jvt.XXXXXXXX")
dd="DESTDIR=$td"
u=$(uname)
o=${TB_OPTFLAGS}
e=
d=
x=
d=-DTERMIOS
t=-ltermcap
lib=
jv=NPROCESSORS_ONLN
# Some logic below is replicated in/from jmake.sh, needed here because
# of the extra non-typical builds that are tested.
case $TB_OS in
SunOS)  x="NROFF=nroff TROFF=troff"
        d="-DREALSTDC -DPOSIX_PROCS"
        case "$TB_REV" in
        5.0) d="-DSYSVR4 -DGRANTPT_BUG";; # really, just do not run this version, it was so so so problematic!
        5.*) ;;
        *) echo "$0: Not set up for testing $TB_OS $TB_REV" >&2; exit 1;;
        esac
        xi=/usr/gnu/bin/install
        if test ! -x $xi; then
            xi=cp
        fi
        x="$x XINSTALL=$xi TINSTALL=$xi"
        ;;
GNU|Linux)  t=-lncurses
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
# Try using either old-style SYSDEFS, OPTFLAGS and TERMCAPLIB, EXTRALIBS
# or new-style CPPFLAGS, CFLAGS, LDLIBS
make clean &&
./jmake.sh $j $dd/t10-$TB_OS install &&
make clean &&
make $j OPTFLAGS="$o" SYSDEFS="-DPIPEPROCS $d" TERMCAPLIB=$t EXTRALIBS= $x $dd/t20-pipeprocs install &&
make clean &&
make $j CFLAGS="$o" CPPFLAGS="-DBSDPOSIX $d" LDLIBS=$t $x $dd/t30-bsdposix install &&
make clean &&
make $j CFLAGS="$o" CPPFLAGS="-DBAREBONES -DSMALL -DJTC $d" LDLIBS= $x $dd/t60-small install &&
make clean &&
case "$lib" in
GLIBC)
    make $j OPTFLAGS="$o" SYSDEFS="-DXLINUX" TERMCAPLIB=$t EXTRALIBS= $x $dd/t40-xlinux install &&
    make clean &&
    make $j OPTFLAGS="$o" SYSDEFS="-DTERMINFO -DUSE_VFORK" TERMCAPLIB=$t $x $dd/t50-vfork &&
    make clean
    ;;
esac &&
make $x tgz && mv jove-*.tgz $dist &&
if type zip 2> /dev/null; then
	make $x zip && 
	mv jove*s.zip $dist
fi &&
if test -e /etc/redhat-release; then
	# build and perhaps install RPM
	make rpm &&
	case "${1-}" in *install) rpm -i $HOME/rpmbuild/RPMS/x86_64/jove-[4-9]*.rpm;; esac &&
	mv $HOME/rpmbuild/*RPMS/x86_64/jove-[4-9]*.rpm $dist
elif test -e /etc/alpine-release; then
	# build a statically compiled version
	r=jove-$ver-static &&
	make SYSDEFS="-D$TB_OS -DJTC" OPTFLAGS="-Os -static" &&
	if test ! -d $dist/$r; then mkdir $dist/$r; fi &&
	strip jjove recover teachjove &&
	mv jjove $dist/$r/jove &&
	mv recover teachjove $dist/$r &&
	cp -pr README paths.h doc $dist/$r/ &&
	tar -c -j -v -f $dist/$r.tar.bz2 -C $dist $r &&
	rm -r $dist/$r
elif type i686-w64-mingw32-gcc 2> /dev/null ; then
	# build a cross-compiled version for Windows
	r=jove-$ver-mingw &&
	make CC=i686-w64-mingw32-gcc CPPFLAGS="-DMINGW" LOCALCC=cc XEXT=.exe EXTRAOBJS="win32.o jjove.coff" LDLIBS=-lcomdlg32 &&
	if test ! -d $dist/$r; then mkdir $dist/$r; fi &&
	mv jjove.exe $dist/$r/jove.exe &&
	mv recover.exe teachjove.exe $dist/$r &&
	cp -pr README paths.h doc $dist/$r &&
	(cd $dist && zip -rm $r.zip $r) &&
	make XEXT=.exe clean
fi &&
tar -c -j -v -f $dist/$ver-builds.tar.bz2 -C "$td" .
rm -rf $td
