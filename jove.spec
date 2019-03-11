# Redhat Package Manager .spec file for JOVE
# To create SRPM and RPM, copy distribution .tgz to /usr/src/redhat/SOURCES
# and execute: rpm -ba jove.spec
# This .spec file has the JOVE version number embedded in it several
# places so it needs to be adjusted every time the version changes!

Summary: Jonathan's Own Version of Emacs
Name: jove
Version: 4.16.0.50
Release: 1
Copyright: Copyright (C) 1986-1999 by Jonathan Payne, freely redistributable
Packager: jovehacks@cs.toronto.edu
Group: Applications/Editors/Emacs
Source: ftp://ftp.cs.toronto.edu/pub/hugh/jove-dev/4.16.0.50.tgz
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
# Keep all three make commands consistent.
# Note: JOVEHOME must be the ultimate path since it will be compiled into JOVE.
# For older LINUX systems without UNIX98 PTYs (eg. Red Hat LINUX 5.2),
# set SYSDEFS=-DBSDPOSIX (this leaves a security hole).
make JOVEHOME=/usr \
	SYSDEFS="-DSYSVR4 -D_XOPEN_SOURCE=500 -DJLGBUFSIZ=12" \
	PORTSRVINST="" TROFF=groff TROFFPOST="" \
	all doc/jove.man doc/jove.man.ps

%install
# Keep all three make commands consistent.
# Note: JOVEHOME is a temporary home under $RPM_BUILD_ROOT/.
# This can be different from JOVEHOME for %build's make.
mkdir -p $RPM_BUILD_ROOT/usr/{bin,lib,man/man1}
make JOVEHOME=$RPM_BUILD_ROOT/usr \
	SYSDEFS="-DSYSVR4 -D_XOPEN_SOURCE=500 -DJLGBUFSIZ=12" \
	PORTSRVINST="" TROFF=groff TROFFPOST="" \
	install
# Note: later versions of RPM run a script that gzips man pages
# behind our back, but older versions don't.  This gzipping is defensive.
gzip $RPM_BUILD_ROOT/usr/man/man1/jove.1
gzip $RPM_BUILD_ROOT/usr/man/man1/teachjove.1
#gzip $RPM_BUILD_ROOT/usr/man/man1/xjove.1

%files
/usr/lib/jove/
/usr/bin/jove
/usr/bin/teachjove
%doc /usr/man/man1/jove.1.gz
%doc /usr/man/man1/teachjove.1.gz
#%doc /usr/man/man1/xjove.1.gz
%doc README doc/jove.man doc/jove.man.ps doc/jove.rc doc/example.rc doc/jove.qref

%clean
rm -rf $RPM_BUILD_ROOT
# Keep all three make commands consistent.
make JOVEHOME=$RPM_BUILD_ROOT/usr \
	SYSDEFS="-DSYSVR4 -D_XOPEN_SOURCE=500 -DJLGBUFSIZ=12" \
	PORTSRVINST="" TROFF=groff TROFFPOST="" \
	clean

%changelog
* Sun Feb 04 2001 <hugh@mimosa.com>
  - 4.16.0.50: survive covert gzipping; format manuals correctly
* Thu Nov 09 2000 Bennett Todd <bet@rahul.net>
  - 4.16.0.49: use RPM_BUILD_ROOT -- don't interfere with system install
* Wed Jul 12 2000 <hugh@mimosa.com>
  - 4.26.0.48: switched to using UNIX98 PTYs for security
* Wed Aug 18 1999 <hugh@mimosa.com>
  - Updated in prepartation for new release
* Tue Jul 28 1998 <markster@marko.net>
  - Updated to version 4.16.0.28
* Wed May 12 1997 <bet@fcmc.com>
  - Initial wrap
