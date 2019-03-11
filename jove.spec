# Redhat Package Manager .spec file for JOVE
# To create SRPM and RPM: rpmbuild -ta distribution.tgz
#
# Beware: .spec file processing is counterintuitve in several ways.
# For example, macros are expanded even in comments!

# This version number must be kept in sync with version.h.
%define jversion 4.16.0.73

# configflags: flags passed to each make to configure for LINUX.
# The choices are explained in Makefile and sysdep.doc.
#
# For older LINUX systems without UNIX98 PTYs (eg. Red Hat LINUX 5.2),
# in SYSDEFS, replace -DSYSVR4 -D_XOPEN_SOURCE=500 with -DBSDPOSIX.
# (This leaves a security hole).
#
# On some LINUX systems, the termcap library or database are broken or missing:
# on those, use the fatter libcurses by adding TERMCAPLIB=-lcurses
# inside the SYSDEFS string.
# Note: Mandrake 8.2 and 9.2 (at least) have a bug in their termcap database
# description of xterm.  Use libcurses to avoid the bug.
# See <http://bugs.mandrakelinux.com/query.php?bug=233>

# Pre 1999 Red Hat Linux (eg. RHL5.2):
#define configflags -C jove%{jversion} SYSDEFS="-DBSDPOSIX -DIPROC_TERM='\\"TERM=vanilla\\"'" NROFF="nroff -Tascii" TROFF=groff TROFFPOST=""
# 1999 or later Red Hat (eg. RHL6.0, Fedora Core 1, RHEL, CentOS):
%define configflags -C jove%{jversion} SYSDEFS="-DSYSVR4 -D_XOPEN_SOURCE=500 -DIPROC_TERM='\\"TERM=vanilla\\"'" NROFF="nroff -Tascii" TROFF=groff TROFFPOST=""
# To use libcurses instead of termcap (eg. Mandrake 9.2):
#define configflags -C jove%{jversion} SYSDEFS="-DSYSVR4 -D_XOPEN_SOURCE=500 -DIPROC_TERM='\\"TERM=vanilla\\"'" TERMCAPLIB="-lncurses" NROFF="nroff -Tascii" TROFF=groff TROFFPOST=""

Summary: Jonathan's Own Version of Emacs
Name: jove
Version: %{jversion}
Release: 1%{?dist}
# Older RPM uses Copyright tag instead of License tag
License: BSD
#Packager: jovehacks@cs.toronto.edu
Group: Applications/Editors
URL: ftp://ftp.cs.toronto.edu/pub/hugh/jove-dev/
Source: ftp://ftp.cs.toronto.edu/pub/hugh/jove-dev/jove%{jversion}.tgz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
BuildRequires: ncurses-devel groff

%description

Jove is a compact, powerful Emacs-style text-editor. It provides the common
emacs keyboard bindings, together with a reasonable assortment of the most
popular advanced features (e.g. interactive shell windows, compile-it,
language specific modes) while weighing in with CPU, memory, and disk
requirements comparable to vi(1).

%prep
%setup -c -q

%build
# Keep all three make commands consistent, except for JOVEHOME and targets.
# JOVEHOME must be the ultimate path since it will be compiled into JOVE.
make JOVEHOME=/usr LIBDIR=%{_libdir}/jove SHAREDIR=%{_datadir}/jove MANDIR=%{_mandir}/man1 OPTFLAGS="%{optflags}" %{configflags} all doc/jove.man doc/jove.man.ps

%install
# Keep all three make commands consistent, except for JOVEHOME and targets.
# JOVEHOME is a temporary home under $RPM_BUILD_ROOT/.
# This can be different from JOVEHOME for the build phase's make.
mkdir -p $RPM_BUILD_ROOT%{_bindir}
mkdir -p $RPM_BUILD_ROOT%{_libdir}
mkdir -p $RPM_BUILD_ROOT%{_datadir}
mkdir -p $RPM_BUILD_ROOT%{_mandir}/man1
make JOVEHOME=$RPM_BUILD_ROOT/usr LIBDIR=$RPM_BUILD_ROOT%{_libdir}/jove SHAREDIR=$RPM_BUILD_ROOT%{_datadir}/jove MANDIR=$RPM_BUILD_ROOT%{_mandir}/man1 OPTFLAGS="%{optflags}" %{configflags} install

# although we build jovetool.1 and xjove.1, we don't install them
rm $RPM_BUILD_ROOT%{_mandir}/man1/jovetool.1
rm $RPM_BUILD_ROOT%{_mandir}/man1/xjove.1

%files
%defattr(-, root, root, -)
%{_libdir}/jove/
%{_datadir}/jove/
%{_bindir}/jove
%{_bindir}/teachjove
# Note: later versions of Red Hat RPM run a script that gzips man pages behind
# our back; Mandrake bzip2s them.  The wildcard lets us roll with the punch.
%doc %{_mandir}/man1/jove.1*
%doc %{_mandir}/man1/teachjove.1*
#doc %{_mandir}/man1/jovetool.1*
#doc %{_mandir}/man1/xjove.1*
%doc jove%{jversion}/README
%doc jove%{jversion}/doc/jove.man
%doc jove%{jversion}/doc/jove.man.ps
%doc jove%{jversion}/doc/jove.rc
%doc jove%{jversion}/doc/example.rc
%doc jove%{jversion}/doc/jove.qref

%clean
rm -rf $RPM_BUILD_ROOT
# Keep all three make commands consistent, except for JOVEHOME and targets.
# JOVEHOME is a temporary home under $RPM_BUILD_ROOT/.
#make JOVEHOME=$RPM_BUILD_ROOT/usr LIBDIR=$RPM_BUILD_ROOT%{_libdir}/jove SHAREDIR=$RPM_BUILD_ROOT%{_datadir}/jove MANDIR=$RPM_BUILD_ROOT%{_mandir}/man1 OPTFLAGS="%{optflags}" %{configflags} clean

%changelog
* Sun Jul 11 2010 D. Hugh Redelmeier 4.16.0.73
- added NROFF="nroff -Tascii" to Makefile and jove.spec to force groff to use ASCII
- spelling corrections [Cord Beermann]
- remove -lolgx from xjove link [Cord Beermann]
- improve recover's email Subject [Cord Beermann]

* Mon May 24 2010 D. Hugh Redelmeier 4.16.0.72
- eliminate strcpy and byte_copy calls with overlapping source and destination
- fix setmaps.c misuse of fprintf

* Sun May 16 2010 D. Hugh Redelmeier 4.16.0.71
- add new variable display-default-filenames (Casey Leedom)
- eliminate most GCC warnings; improve handling of some errors
- allow for Linux/glibc elimination of I_PUSH (pseudo TTY STREAMS)
- improve jove.spec for Red Hat packaging
- delete obsolete command process-dbx-output
- delete obsolete variables allow-bad-filenames, display-bad-filenames, internal-tabstop
- add bindings for more xterm function key variants
