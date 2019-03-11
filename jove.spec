# Redhat Package Manager .spec file for JOVE
# To create SRPM and RPM: rpmbuild -ta distribution.tgz
#
# Beware: .spec file processing is counterintuitve in several ways.
# For example, macros are expanded even in comments!

# This version number must be kept in sync with version.h.
%define jversion 4.16.0.64

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

# Older Red Hat (eg. 5.2):
#define configflags -C jove%{jversion} SYSDEFS="-DBSDPOSIX -DJLGBUFSIZ=12 -DIPROC_TERM='\\"TERM=vanilla\\"'" TROFF=groff TROFFPOST=""
# Recent Red Hat (eg. 6.0 - Fedora Core 1):
%define configflags -C jove%{jversion} SYSDEFS="-DSYSVR4 -D_XOPEN_SOURCE=500 -DJLGBUFSIZ=12 -DIPROC_TERM='\\"TERM=vanilla\\"'" TROFF=groff TROFFPOST=""
# To use libcurses instead of termcap (eg. Mandrake 9.2):
#define configflags -C jove%{jversion} SYSDEFS="-DSYSVR4 -D_XOPEN_SOURCE=500 -DJLGBUFSIZ=12 -DIPROC_TERM='\\"TERM=vanilla\\"'" TERMCAPLIB="-lncurses" TROFF=groff TROFFPOST=""

Summary: Jonathan's Own Version of Emacs
Name: jove
Version: %{jversion}
Release: 1
Copyright: Copyright (C) 1986-2002 by Jonathan Payne, freely redistributable
Packager: jovehacks@cs.toronto.edu
Group: Applications/Editors/Emacs
Source: ftp://ftp.cs.toronto.edu/pub/hugh/jove-dev/jove%{jversion}.tgz
BuildRoot: /var/tmp/%{name}-rpmroot
%description

Jove is a compact, powerful Emacs-style text-editor. It provides the common
emacs keyboard bindings, together with a reasonable assortment of the most
popular advanced features (e.g. interactive shell windows, compile-it,
language specific modes) while weighing in with CPU, memory, and disk
requirements comparable to vi(1).

%prep
%setup -c

%build
# Keep all three make commands consistent, except for JOVEHOME and targets.
# JOVEHOME must be the ultimate path since it will be compiled into JOVE.
make JOVEHOME=/usr %{configflags} all doc/jove.man doc/jove.man.ps

%install
# Keep all three make commands consistent, except for JOVEHOME and targets.
# JOVEHOME is a temporary home under $RPM_BUILD_ROOT/.
# This can be different from JOVEHOME for the build phase's make.
mkdir -p $RPM_BUILD_ROOT/usr/bin
mkdir -p $RPM_BUILD_ROOT/usr/lib
mkdir -p $RPM_BUILD_ROOT/usr/man/man1
make JOVEHOME=$RPM_BUILD_ROOT/usr %{configflags} install

# although we build jovetool.1 and xjove.1, we don't install them
rm $RPM_BUILD_ROOT/usr/man/man1/jovetool.1
rm $RPM_BUILD_ROOT/usr/man/man1/xjove.1

%files
/usr/lib/jove/
/usr/bin/jove
/usr/bin/teachjove
# Note: later versions of Red Hat RPM run a script that gzips man pages behind
# our back; Mandrake bzip2s them.  The wildcard lets us roll with the punch.
%doc /usr/man/man1/jove.1*
%doc /usr/man/man1/teachjove.1*
#doc /usr/man/man1/jovetool.1*
#doc /usr/man/man1/xjove.1*
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
make JOVEHOME=$RPM_BUILD_ROOT/usr %{configflags} clean

%changelog
