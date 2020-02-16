##########################################################################
# This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    #
# provided by Jonathan and Jovehacks without charge and without          #
# warranty.  You may copy, modify, and/or distribute JOVE, provided that #
# this notice is included in all the source files and documentation.     #
##########################################################################

# SHELL for this Makefile (csh won't work!)
SHELL = /bin/sh

# JOVEHOME is the directory in which pieces of JOVE are kept.  It is only used
#	in the default definitions of JSHAREDIR, JLIBDIR, JBINDIR, and JMANDIR.
# JSHAREDIR is for online documentation, and the distributed standard system-wide
# jove.rc file with some common
# JLIBDIR is for the PORTSRV and RECOVER programs.
# JBINDIR is where to put the executables JOVE and TEACHJOVE.
# XEXT is the extension for executables (empty for UNIX; .exe for CYGWIN)
# JMANDIR is where the manual pages go for JOVE, RECOVER and TEACHJOVE.
# MANEXT is the extension for the man pages, e.g., jove.1 or jove.l or jove.m.
#	Must not be "nr".
#
# If they don't exist, this makefile will try to create all *DIR directories
# (prefixed with $DESTDIR, as most packaging systems desire)
# All others must already exist.

JOVEHOME = /usr/local
JSHAREDIR = $(JOVEHOME)/share/jove
DSHAREDIR = $(DESTDIR)$(JSHAREDIR)
JLIBDIR = $(JOVEHOME)/lib/jove
DLIBDIR = $(DESTDIR)$(JLIBDIR)
JBINDIR = $(JOVEHOME)/bin
DBINDIR = $(DESTDIR)$(JBINDIR)
XEXT=
JMANDIR = $(JOVEHOME)/man/man$(MANEXT)
DMANDIR = $(DESTDIR)$(JMANDIR)
MANEXT = 1

# Install permission for SHAREDIR, LIBDIR, BINDIR
DPERM = 755
XPERM = 755

# JTMPDIR is where the tmp files get stored, usually /tmp, /var/tmp, or
# /usr/tmp.  If you wish to be able to recover buffers after a system
# crash, this needs to be a directory that isn't cleaned out on reboot.
# You would probably want to clean out that directory periodically with
# /etc/cron.
# JRECDIR is the directory in which RECOVER looks for JOVE's tempfiles
# (in case the system startup salvages tempfiles by moving them,
# which is probably a good idea).

JETCDIR = /etc/jove
DETCDIR = $(DESTDIR)$(JETCDIR)
JTMPDIR = /var/tmp
JRECDIR = /var/lib/jove/preserve
DRECDIR = $(DESTDIR)$(JRECDIR)

# Install permission for DRECDIR
RECPERM = 1777

# place to copy source tarball for rpmbuild
RPMHOME = $(HOME)/rpmbuild/SOURCES

# DFLTSHELL is the default shell invoked by JOVE and TEACHJOVE.
DFLTSHELL = /bin/sh

# The install commands of BSD and System V differ in unpleasant ways:
# -c: copy (BSD); -c dir: destination directory (SysV)
# -s: strip (BSD); -s: suppress messages (SysV)
# Also, the destination specification is very different.
# The result is that the System V install command must not be used.
# If you know that /bin/install is the BSD program, you can use it.
# "cp" will work reasonably well, but be aware that any links continue
# referencing the old file with new contents.

INSTALLFLAGS = # -g bin -o root

# XINSTALL to install executable files, prefer install since it is independent
# of user umask
# Linux/modern BSD/CYGWIN
XINSTALL=install $(INSTALLFLAGS) -m 755
TINSTALL=install $(INSTALLFLAGS) -m 444
# SysV-derivatives, or non-BSD Unix (V7, etc)
#XINSTALL=cp
#TINSTALL=cp
# 4BSD, SunOS
#XINSTALL=/usr/ucb/install $(INSTALLFLAGS) -c -m 755 # -s
#TINSTALL=/usr/ucb/install $(INSTALLFLAGS) -c -m 644

# These should all just be right if the above ones are.
# You will confuse JOVE if you move anything from LIBDIR or SHAREDIR.

JOVE = $(DBINDIR)/jove$(XEXT)
TEACHJOVE = $(DBINDIR)/teachjove$(XEXT)
RECOVER = $(DLIBDIR)/recover$(XEXT)
PORTSRV = $(DLIBDIR)/portsrv$(XEXT)
JOVERC = $(DSHAREDIR)/jove.rc
TERMSDIR = $(DSHAREDIR)
CMDS.DOC = $(DSHAREDIR)/cmds.doc
TEACH-JOVE = $(DSHAREDIR)/teach-jove
JOVEM = $(DMANDIR)/jove.$(MANEXT)
TEACHJOVEM = $(DMANDIR)/teachjove.$(MANEXT)
XJOVEM = $(DMANDIR)/xjove.$(MANEXT)
JOVETOOLM = $(DMANDIR)/jovetool.$(MANEXT)

# SYSDEFS: specify system characteristics.
# The default is the system uname, which describes a number of modern
# systems (but not Solaris or Cygwin).  If this isn't suitable for your system,
# you will need to change it.  You may need to define a new symbol for
# your OS if we haven't created a suitable one.  See sysdep.h.
# The OS and version recently tested with a specific SYSDEFS are below.
# For most systems below, if they have select() (i.e. any *n*x variant
# since the late 1980s), you can add -DJTC to the SYSDEFS and TERMCAPLIB=
# to skip the curses/termcap dependency and only support ansi/vt[12345]xx/xterm
# terminals (i.e any practical terminals since the late 1980s!)
#
#	Cygwin 3.1.2			SYSDEFS=-DCYGWIN_JTC TERMCAPLIB= # builtin vt100/xterm/rxvt etc support, no need for curses dependency
#	Cygwin 3.1.2 (curses)		SYSDEFS=-DCYGWIN TERMCAPLIB=-lncursesw
#	Darwin aka MacOS X		SYSDEFS=-DDarwin
#	FreeBSD 12.1			SYSDEFS=-DFreeBSD EXTRALIBS=-lutil
#	Linux (modern, UNIX98 PTYS)	SYSDEFS=-DLinux  # some have -ltermcap
#	Linux (modern, UNIX98 PTYS)	SYSDEFS=-DLinux TERMCAPLIB=-lncurses
#	Linux (modern, UNIX98 PTYS)	SYSDEFS=-DLinux TERMCAPLIB=-lncursesw
#	Linux (modern, glibc pty.h)	SYSDEFS=-DGLIBCPTY EXTRALIBS=-lutil TERMCAPLIB=...
#	MacOS X aka Darwin 		SYSDEFS=-DDarwin
#	NetBSD 8.1			SYSDEFS=-DNetBSD EXTRALIBS=-lutil
#	OpenBSD 6.6			SYSDEFS=-DOpenBSD EXTRALIBS=-lutil
#
# The following used to work circa Jove 4.16 in the 1990s and have probably not
# been tested in the 21st century, they may still work but some of this
# ancient support may be deleted from Jove at some point, we welcome
# any recent success stories from jove builders/packagers to refresh
# or maintain these. Pretty please!
# 
#	Apple A/UX on macIIs		SYSDEFS=-DA_UX
#	BSD4.2,4.3			SYSDEFS=-DBSD4
#	BSDI, 386BSD, BSD4.4		SYSDEFS=-DBSDPOSIX
#	Consensys V4			SYSDEFS="-DSYSVR4 -DGRANTPT_BUG"
#	Compaq Tru64 UNIX V4.0g, 5.1	SYSDEFS=-DSYSVR4
#	DEC OSF R1.3MK			SYSDEFS=-DSYSVR4
#	DEC OSF/1 V1.3			SYSDEFS="-DBSDPOSIX -DNO_TIOCREMOTE -DNO_TIOCSIGNAL"
#	DEC OSF/1 V2.0 and later	SYSDEFS=-DSYSVR4
#	DEC Ultrix 4.2			SYSDEFS=-DBSDPOSIX
#	DEC Ultrix 4.3			SYSDEFS="-DBSDPOSIX -DJVDISABLE=255"
#	Digital UNIX V4.0 and later	SYSDEFS="-DSYSVR4 -DGRANTPT_BUG"
#	DG AViiON 5.3R4			SYSDEFS="-DSYSVR4 -DBSD_SIGS"
#	HP/UX 8 or 9			SYSDEFS="-DHPUX -Ac"
#	HP/UX 11 (-Ac redundant)	SYSDEFS=-DHPUX
#	IBM AIX 3.2			SYSDEFS=-DAIX3_2
#	IBM AIX 4.2, 5.2		SYSDEFS="-DAIX4_2" TERMCAPLIB="-lcurses -ls"
#	Irix 3.3-4.0.5			SYSDEFS="-DIRIX -DIRIX4"
#	Irix 5.0 onwards		SYSDEFS="-DIRIX -prototypes"
#	Linux (older, eg. RedHat 4, 5)	SYSDEFS=-DBSDPOSIX
#	MIPS RiscOS4.x			SYSDEFS="-systype bsd43 -DBSD4"
#	SCO Unix			SYSDEFS=-DSCO
#	SunOS3.x			SYSDEFS=-DSUNOS3
#	SunOS4.0*			SYSDEFS=-DSUNOS40
#	SunOS4.1*			SYSDEFS=-DSUNOS41
#	SunOS5.0 (Solaris 2.0)		SYSDEFS="-DSYSVR4 -DGRANTPT_BUG"
#	SunOS5.[12345678] (Solaris)	SYSDEFS=-DSYSVR4
#	Sys III, Sys V R 2,3		SYSDEFS=-DSYSV PORTSRVINST='$(PORTSRV)'
#	Sys V Release 4.0		SYSDEFS="-DSYSVR4 -DGRANTPT_BUG"
#	Sys V Release 4.x		SYSDEFS=-DSYSVR4
#
# Some systems based on System V release 4 have a bug affecting interactive
# processes.  This bug can be worked around by defining GRANTPT_BUG.
# Read the explanation of GRANTPT_BUG in sysdep.doc.
#
# Some of the MIPS based Ultrix (up to 4.2 at least), RiscOS and Irix (up to
# 3.3 at least) also need -DMIPS_CC_BUG.
#
# Some old versions of the HPUX C compiler have a bug in handling forward
# struct tag declarations.  Using the -Ac flag in place of -Ae will avoid
# this problem (and reduce the compiler's error checking, unfortunately).
#
# Add -DUSE_EXIT if you're profiling or using purify (this causes Jove
# to exit using exit(), instead of _exit()).
# Add -DDEBUGCRASH if you want to not trap on SEGV, BUS, etc, useful
# running jove under a debugger (gdb, dbx)

SYSDEFS = -DLinux

# Select optimization level (flags passed to compiling and linking steps).
# On most systems: -g for debugging, -O for optimization.
# -Os produces smaller binaries with gcc, should not hurt performance
# On the official Sun ANSI C compiler and the standard System V Release 4
# compiler, adding -Xa -v will increase compiler checking.
# On DEC OSF/1 and Digital UNIX VV4.0, add -std1 to enable ANSI C features
# and perhaps -g3 for more debugging info with optimization.

OPTFLAGS = -O # -g -Wall -pedantic

# For making dependencies under BSD systems
DEPENDFLAG = -M
# or, using the official Sun ANSI C compiler
#	DEPENDFLAG = -xM

# Flags for Library to provide termcap functions.
# Some systems have dropped termcap: use -lcurses (fatter!) or -lncurses
#	Cygwin32: TERMCAPLIB = -L/usr/local/lib -lcurses
#	SysV Rel. 2: TERMCAPLIB = -lcurses
#	SCO UNIX: TERMCAPLIB = -lcurses
#	AIX on the R6000s: TERMCAPLIB = -lcurses -ltermcap -ls
#	OpenSuSE: TERMCAPLIB = -lncurses
# Jove comes with a simplified termcap substitute that only supports
# ANSI X.3/VT[1-5]xx or compatible terminal emulators like xterm, rxvt, etc,
# which is almost certainly all that is necessary on modern machines.
# To use it, define -DJTC and leave TERMCAPLIB unset

TERMCAPLIB = -ltermcap

# Extra libraries flags needed by oddball systems.
# Some BSD systems using openpty need its library.
#	4.1BSD:	EXTRALIBS = -ljobs
#	FreeBSD 4.2: EXTRALIBS = -lutil
#	FreeBSD 4.2, NetBSD 1.5, OpenBSD 2.x:  EXTRALIBS = -lutil

EXTRALIBS =

# Flags of linker (LDFLAGS)
# Most systems do not need any flags.
# Known exceptions:
#	SysV Rel. 2: LDFLAGS = -Ml
#	SCO Xenix: LDFLAGS = -Ml -F 3000
#	AIX Unix: LDFLAGS = -bloadmap:$@.map # only if loadmap
#
# To optimize the use of the address spaces, add to the LDFLAGS:
#	PDP-11 with separate I&D: -i
#	PDP-11 without separate I&D: -n

LDFLAGS =

# for SCO Xenix, set
#	MEMFLAGS = -Mle
#	CFLAGS = -LARGE -O -F 3000 -K -Mle  (say -Mle2 for an 80286)

CFLAGS = $(OPTFLAGS) $(SYSDEFS)

# For SYSVR4 (/usr/ucb/cc will NOT work because of setjmp.h):
#	CC = /usr/bin/cc
# To use the SunPro compiler under SunOS 4.n:
#	CC = acc
# To use the official Sun compiler under Solaris 2.n:
#	CC = /opt/SUNWspro/bin/cc
# For DG AViiON, expect compile errors unless you use the GNU C compiler:
#	CC=gcc

# Load invocation of cc.
# to use Purify(TM): LDCC = purify $(CC)

LDCC = $(CC)

# For cross compiling Jove, set CC to the cross compiler, and LOCALCC
# to the local C compiler. LOCALCC will be used for compiling setmaps,
# which is run as part of the compilation to generate the keymaps.
# Set LOCALCFLAGS and LOCALLDFLAGS appropriately too. For Xenix, note
# that LOCALCFLAGS must be set to $(MEMFLAGS)

LOCALCC = $(CC)
LOCALCFLAGS = $(CFLAGS)	# $(MEMFLAGS)
LOCALLDFLAGS = $(LDFLAGS)
LOCALEXTRALIBS = $(EXTRALIBS)

# Objects are grouped into overlays for the benefit of (at least) 2.xBSD.

BASESEG = commands.o keys.o argcount.o ask.o buf.o jctype.o delete.o \
	  disp.o insert.o io.o jove.o marks.o misc.o re.o \
	  screen.o termcap.o unix.o util.o vars.o list.o keymaps.o \
	  mouse.o jtc.o
OVLAY1 = abbrev.o rec.o paragraph.o fmt.o
OVLAY2 = c.o wind.o fp.o move.o
OVLAY3 = extend.o macros.o
OVLAY4 = iproc.o reapp.o
OVLAY5 = proc.o scandir.o term.o case.o

OBJECTS = $(BASESEG) $(OVLAY1) $(OVLAY2) $(OVLAY3) $(OVLAY4) $(OVLAY5)

# These NROFF, TROFF and TROFFPOST settings work with groff.
# Classic Unix and derivatives used to have ditroff for which, use:
#	NROFF = nroff
#	TROFF = troff
#	TROFFPOST = | /usr/lib/lp/postscript/dpost -
NROFF = nroff -Tascii
TROFF = groff
TROFFPOST = 

MANUALS = $(JOVEM) $(TEACHJOVEM)

C_SRC = commands.c commands.tab abbrev.c argcount.c ask.c buf.c c.c case.c jctype.c \
	delete.c disp.c extend.c fp.c fmt.c insert.c io.c iproc.c \
	jove.c jtc.c list.c macros.c marks.c misc.c move.c paragraph.c \
	proc.c re.c reapp.c rec.c scandir.c screen.c term.c termcap.c unix.c \
	util.c vars.c vars.tab wind.c msgetch.c mac.c keymaps.c ibmpcdos.c \
	mouse.c win32.c

SOURCES = $(C_SRC) portsrv.c recover.c setmaps.c teachjove.c

HEADERS = abbrev.h argcount.h ask.h buf.h c.h case.h chars.h commands.h \
	jctype.h dataobj.h delete.h disp.h extend.h externs.h \
	fmt.h fp.h insert.h io.h iproc.h jove.h \
	keymaps.h list.h mac.h macros.h marks.h \
	misc.h mouse.h move.h paragraph.h proc.h \
	re.h reapp.h rec.h recover.h scandir.h screen.h \
	select.h sysdep.h sysprocs.h temp.h term.h ttystate.h \
	tune.h util.h vars.h version.h wind.h

DOCTERMS =	doc/jove.rc.sun doc/keychart.sun \
	doc/jove.rc.sun-cmd doc/keychart.sun-cmd \
	doc/jove.rc.vt100 doc/keychart.vt100 \
	doc/jove.rc.wyse doc/keychart.wyse \
	doc/jove.rc.xterm doc/keychart.xterm \
	doc/jove.rc.xterm-256color doc/keychart.xterm-256color \
	doc/jove.rc.z29 doc/keychart.z29 \
	doc/jove.rc.3022 doc/keychart.3022 \
	doc/keychart. \
	doc/XTermresource

# formatted docs, we ship these in the distrib to avoid groff dependency
# and for non-Unix/Linux platforms
FDOCS = doc/cmds.doc doc/jove.man doc/jove.man.ps doc/jove.doc

# files we generate that we also ship in distrib for platforms sans sed
GEN = 	jove.spec doc/jove.rc doc/jove.$(MANEXT) \
	doc/teachjove.$(MANEXT) doc/jovetool.$(MANEXT)

DOCS =	doc/README doc/teach-jove doc/jove.qref \
	doc/intro.nr doc/cmds.macros.nr doc/cmds.nr doc/contents.nr \
	doc/jove.nr doc/teachjove.nr doc/xjove.nr doc/jovetool.nr \
	doc/jove.rc.in doc/example.rc $(DOCTERMS) $(FDOCS) $(GEN)

MISC =	Makefile Makefile.bcc Makefile.msc Makefile.wat Makefile.zor \
	README README.dos README.mac README.w32 README.cyg \
	sysdep.doc tune.doc style.doc jspec.in

SUPPORT = teachjove.c recover.c setmaps.c portsrv.c keys.txt \
	menumaps.txt mjovers.Hqx jjove.ico jjove.rc

BACKUPS = $(HEADERS) $(C_SRC) $(SUPPORT) $(MISC)

# all: default target.
# Builds everything that "install" needs.
all:	jjove$(XEXT) recover$(XEXT) teachjove$(XEXT) portsrv$(XEXT) $(FDOCS) \
	$(GEN)

jjove$(XEXT):	$(OBJECTS)
	$(LDCC) $(LDFLAGS) $(OPTFLAGS) -o jjove$(XEXT) $(OBJECTS) $(TERMCAPLIB) $(EXTRALIBS)
	@-size jjove$(XEXT)

# For 2.xBSD: link jove as a set of overlays.  Not tested recently.

ovjove:	$(OBJECTS)
	ld $(LDFLAGS) $(OPTFLAGS) -X /lib/crt0.o \
		-Z $(OVLAY1) \
		-Z $(OVLAY2) \
		-Z $(OVLAY3) \
		-Z $(OVLAY4) \
		-Z $(OVLAY5) \
		-Y $(BASESEG) \
		-o jjove$(XEXT)  $(TERMCAPLIB) $(EXTRALIBS) -lc
	@-size jjove$(XEXT)

# portsrv is only needed if IPROCS are implemented using PIPEPROCS
# (modern systems use PTYPROCS).
# Making PORTSRVINST null suppresses installing portsrv.

# PORTSRVINST=$(PORTSRV)
PORTSRVINST=

portsrv$(XEXT):	portsrv.o
	$(LDCC) $(LDFLAGS) $(OPTFLAGS) -o portsrv$(XEXT) portsrv.o $(EXTRALIBS)

recover$(XEXT):	recover.o
	$(LDCC) $(LDFLAGS) $(OPTFLAGS) -o recover$(XEXT) recover.o $(EXTRALIBS)

teachjove$(XEXT):	teachjove.o
	$(LDCC) $(LDFLAGS) $(OPTFLAGS) -o teachjove$(XEXT) teachjove.o $(EXTRALIBS)

# don't optimize setmaps.c because it produces bad code in some places
# for some reason

setmaps$(XEXT):	setmaps.o
	$(LOCALCC) $(LOCALLDFLAGS) -o setmaps$(XEXT) setmaps.o $(LOCALEXTRALIBS)

setmaps.o:	setmaps.c
	$(LOCALCC) $(LOCALCFLAGS) -c setmaps.c

keys.c:	setmaps$(XEXT) keys.txt
	@-rm -f keys.c
	./setmaps$(XEXT) < keys.txt > keys.c

keys.o:	keys.c tune.h sysdep.h jove.h keymaps.h dataobj.h commands.h

.ALWAYS:

.version: .ALWAYS
	@-rm -f version.h.tmp
	@sed -n 's/# *define  *jversion[ \t]*"\([0-9\\.]*\)".*/\1/p' version.h > .version.tmp; \
	if ! cmp -s .version.tmp .version; then mv .version.tmp .version; else rm .version.tmp; fi

jove.spec: .version .ALWAYS
	@-rm -f jspec.tmp
	@v=`sed 's/_.*//' .version`; sed "s,__VERSION__,$$v,g" jspec.in > jspec.tmp; \
	if ! cmp -s jove.spec jspec.tmp; then mv jspec.tmp jove.spec; else rm jspec.tmp; fi

paths.h: .ALWAYS
	@-rm -f paths.tmp
	@echo "/* Changes should be made in Makefile, not to this file! */" > paths.tmp
	@echo "" >> paths.tmp
	@echo \#define TMPDIR \"$(JTMPDIR)\" >> paths.tmp
	@echo \#define RECDIR \"$(JRECDIR)\" >> paths.tmp
	@echo \#define LIBDIR \"$(JLIBDIR)\" >> paths.tmp
	@echo \#define SHAREDIR \"$(JSHAREDIR)\" >> paths.tmp
	@echo \#define DFLTSHELL \"$(DFLTSHELL)\" >> paths.tmp
	if ! cmp -s paths.h paths.tmp; then mv paths.tmp paths.h; else rm paths.tmp; fi

makexjove:
	( cd xjove ; make CC="$(CC)" OPTFLAGS="$(OPTFLAGS)" SYSDEFS="$(SYSDEFS)" $(TOOLMAKEEXTRAS) xjove )

installxjove:  $(XJOVEM)
	( cd xjove ; make CC="$(CC)" OPTFLAGS="$(OPTFLAGS)" SYSDEFS="$(SYSDEFS)" XINSTALL="$(XINSTALL)" BINDIR="$(DBINDIR)" INSTALLFLAGS="$(INSTALLFLAGS)" $(TOOLMAKEEXTRAS) installxjove )

makejovetool:
	( cd xjove ; make CC="$(CC)" OPTFLAGS="$(OPTFLAGS)" SYSDEFS="$(SYSDEFS)" DEFINES=-DSUNVIEW $(TOOLMAKEEXTRAS) jovetool )

installjovetool: $(JOVETOOLM)
	( cd xjove ; make CC="$(CC)" OPTFLAGS="$(OPTFLAGS)" SYSDEFS="$(SYSDEFS)" DEFINES=-DSUNVIEW XINSTALL="$(XINSTALL)" BINDIR="$(DBINDIR)" INSTALLFLAGS="$(INSTALLFLAGS)" $(TOOLMAKEEXTRAS) installjovetool )

# Note: everything needed by "install" should be built by "all".
# Thus, if "all" is done first, "install" can be invoked with
# JOVEHOME pointing at a playpen where files are to be marshalled.
# This property is fragile.
install: $(DLIBDIR) $(DSHAREDIR) $(DBINDIR) $(DMANDIR) $(DRECDIR) $(DETCDIR) \
	 $(TEACH-JOVE) $(CMDS.DOC) $(TERMSDIR)docs $(JOVERC) \
	 $(PORTSRVINST) $(RECOVER) $(JOVE) $(TEACHJOVE) $(MANUALS)
	@echo See the README about changes to /etc/rc or /etc/rc.local
	@echo so that the system recovers jove files on reboot after a crash

$(DBINDIR)::
	if test ! -e $(DBINDIR); then mkdir -p $(DBINDIR) && chmod $(DPERM) $(DBINDIR); fi

$(DLIBDIR)::
	if test ! -e $(DLIBDIR); then mkdir -p $(DLIBDIR) && chmod $(DPERM) $(DLIBDIR); fi

$(DSHAREDIR)::
	if test ! -e $(DSHAREDIR); then mkdir -p $(DSHAREDIR) && chmod $(DPERM) $(DSHAREDIR); fi

$(DETCDIR)::
	if test ! -e $(DETCDIR); then mkdir -p $(DETCDIR) && chmod $(DPERM) $(DETCDIR); fi

$(DMANDIR)::
	if test ! -e $(DMANDIR); then mkdir -p $(DMANDIR) && chmod $(DPERM) $(DMANDIR); fi

$(DRECDIR)::
	if test ! -e $(DRECDIR); then mkdir -p $(DRECDIR) && chmod $(RECPERM) $(DRECDIR); fi

# first run of rpmbuild will mkdir other sibling directories in RPMHOME, but 
# we need this before we run rpmbuild, so RPMHOME might not even exist.
$(RPMHOME)::
	if test ! -e $(RPMHOME); then mkdir -p $(RPMHOME) && chmod $(DPERM) $(RPMHOME); fi

$(TEACH-JOVE): doc/teach-jove
	$(TINSTALL) doc/teach-jove $(TEACH-JOVE)

doc/cmds.doc:	doc/cmds.macros.nr doc/cmds.nr
	@-rm -f doc/cmds.doc
	LANG=C $(NROFF) doc/cmds.macros.nr doc/cmds.nr > doc/cmds.doc

doc/jove.man:	doc/intro.nr doc/cmds.nr
	@-rm -f doc/jove.man
	( LANG=C; export LANG; cd doc; tbl intro.nr | $(NROFF) -ms - cmds.nr >jove.man )

doc/jove.man.ps:
	@-rm -f doc/doc/jove.man.ps
	( LANG=C; export LANG; cd doc; tbl intro.nr | $(TROFF) -ms - cmds.nr contents.nr $(TROFFPOST) >jove.man.ps )

$(CMDS.DOC): doc/cmds.doc
	$(TINSTALL) doc/cmds.doc $(CMDS.DOC)

doc/jove.rc: doc/jove.rc.in
	@-rm -f doc/jove.rc.tmp
	sed "s,__ETCDIR__,$(JETCDIR)," doc/jove.rc.in > doc/jove.rc.tmp
	if ! cmp -s doc/jove.rc.tmp doc/jove.rc; then mv doc/jove.rc.tmp doc/jove.rc; else rm doc/jove.rc.tmp; fi

$(JOVERC): doc/jove.rc
	$(TINSTALL) doc/jove.rc $(JOVERC)

$(TERMSDIR)docs: $(DOCTERMS)
	$(TINSTALL) $(DOCTERMS) $(TERMSDIR)

$(PORTSRV): portsrv$(XEXT)
	$(XINSTALL) portsrv$(XEXT) $(PORTSRV)

$(RECOVER): recover$(XEXT)
	$(XINSTALL) recover$(XEXT) $(RECOVER)

$(JOVE): jjove$(XEXT)
	$(XINSTALL) jjove$(XEXT) $(JOVE)

$(TEACHJOVE): teachjove$(XEXT)
	$(XINSTALL) teachjove$(XEXT) $(TEACHJOVE)

doc/jove.$(MANEXT): doc/jove.nr
	@-rm -f doc/jove.$(MANEXT)
	sed -e 's;<TMPDIR>;$(JTMPDIR);' \
	     -e 's;<LIBDIR>;$(JLIBDIR);' \
	     -e 's;<SHAREDIR>;$(JSHAREDIR);' \
	     -e 's;<SHELL>;$(DFLTSHELL);' doc/jove.nr > doc/jove.$(MANEXT)

$(JOVEM): $(DMANDIR) doc/jove.$(MANEXT)
	$(TINSTALL) doc/jove.$(MANEXT) $(JOVEM)

# doc/jove.doc is the formatted manpage (only needed by DOS)
# Building it should be like building $(JOVEM) except that we
# don't know what to substitute for <TMPDIR> etc. because they
# are not fixed yet, and because we must do the formatting.

doc/jove.doc: doc/jove.nr
	@-rm -f doc/jove.doc
	LANG=C $(NROFF) -man doc/jove.nr >doc/jove.doc

doc/teachjove.$(MANEXT): doc/teachjove.nr
	@-rm -f doc/teachjove.$(MANEXT)
	sed -e 's;<TMPDIR>;$(JTMPDIR);' \
	     -e 's;<LIBDIR>;$(JLIBDIR);' \
	     -e 's;<SHAREDIR>;$(JSHAREDIR);' \
	     -e 's;<SHELL>;$(DFLTSHELL);' doc/teachjove.nr > doc/teachjove.$(MANEXT)

$(TEACHJOVEM): $(DMANDIR) doc/teachjove.$(MANEXT)
	$(TINSTALL) doc/teachjove.$(MANEXT) $(TEACHJOVEM)

$(XJOVEM): $(DMANDIR) doc/xjove.nr
	$(TINSTALL) doc/xjove.nr $(XJOVEM)

doc/jovetool.$(MANEXT): doc/jovetool.nr
	@-rm -f doc/jovetool.$(MANEXT)
	sed -e 's;<MANDIR>;$(MANDIR);' \
	     -e 's;<MANEXT>;$(MANEXT);' doc/jovetool.nr > doc/jovetool.$(MANEXT)

$(JOVETOOLM): $(DMANDIR) doc/jovetool.$(MANEXT)
	$(TINSTALL) doc/jovetool.$(MANEXT) $(JOVETOOLM)

echo:
	@echo $(C-FILES) $(HEADERS)

# note: $(C_SRC) contains commands.tab and vars.tab
# These should not be linted, but they will probably be ignored.

lint: keys.c
	lint $(SYSDEFS) $(C_SRC) keys.c
	lint $(SYSDEFS) portsrv.c
	lint $(SYSDEFS) recover.c
	lint $(SYSDEFS) setmaps.c
	lint $(SYSDEFS) teachjove.c
	@echo Done

# CTAGSFLAGS = -N --format=1 # fishy options required for Exuberant Ctags
# since this is inconvenient to specify, you can use target extags instead.
CTAGSFLAGS = -w

tags:	$(C_SRC) $(HEADERS)
	ctags $(CTAGSFLAGS) $(C_SRC) $(HEADERS)

extags:	$(C_SRC) $(HEADERS)
	ctags -N --format=1 $(C_SRC) $(HEADERS)


# .filelist is a trick to get around a make limit:
# the list of files is too long to fit in a command generated by make
# The actual contents of the file depend only on Makefile, but by
# adding extra dependencies, dependants of .filelist can have shorter
# dependency lists.  Note: since we have no list of xjove files,
# we alway force a make of xjove/.filelist.  This forces .filelist
# to be rebuilt every time it is needed.

.filelist:	$(BACKUPS) $(DOCS) .xjfilelist
	@-rm -f .filelist
	@ls $(BACKUPS) >.filelist
	@ls $(DOCS) >>.filelist
	@sed -e 's=^=xjove/=' xjove/.filelist >>.filelist

.xjfilelist:
	@( cd xjove ; make .filelist )

# override CIFLAGS with something like:
# CIFLAGS="-m'some reason for change' -u4.14.10.n -q"

ciall:	.filelist
	ci $(CIFLAGS) `cat .filelist`

coall:	.filelist
	co $(BACKUPS) `cat .filelist`

jove.shar:	.filelist
	@-rm -f jove.shar
	shar .filelist > jove.shar

backup.Z: .filelist
	-rm -f backup backup.Z
	tar cf backup `cat .filelist`
	compress backup

backup.tgz: .filelist
	# GNU tar only: z
	-rm -f backup.tgz
	tar czf backup.tgz `cat .filelist`

tape-backup:	.filelist
	tar cf /dev/rst8 `cat .filelist`

# Build a distribution: a gzipped tar file with a name "jove-<version>.tgz"
# The tar will unpack into a directory with the name jove-<version>
# Beware: old files with these names will be blown away.
distrib:	.filelist
	set -u ; set -e ; \
	BN=jove-`cat .version` ; \
	rm -rf $$BN $$BN.tgz* ; \
	mkdir $$BN ; \
	tar cf - `cat .filelist` | ( cd $$BN ; tar xf - ) ; \
	tar czf $$BN.tgz $$BN ; \
	rm -rf $$BN ; \
	ls -l $$BN.tgz

rpm: distrib $(RPMHOME)
	# rpmbuild really wants the source tarball to have
	# the basename of Source0 in the rpm spec, and be
	# in the rpmbuild SOURCES directory.
	rpmsrc=`sed -n '/Source0/s,.*/,,p' jove.spec`; \
	BN=jove-`cat .version` ; \
	cp $$BN.tgz $(RPMHOME)/$$rpmsrc; \
	rpmbuild -ta $(RPMHOME)/$$rpmsrc

# create a distribution and a separate PGP signature for it
signeddistrib-pgp:	distrib
	BN=jove-`cat .version` ; \
	pgp -sba $$BN.tgz; \
	chmod a+r $$BN.tgz.asc

signeddistrib-gpg:	distrib
	BN=jove-`cat .version` ; \
	gpg -sba $$BN.tgz; \
	chmod a+r $$BN.tgz.asc

# System V sum can be made to match BSD with a -r flag.
# To get this effect, override with SUM = sum -r
SUM = sum

checksum:	.filelist
	$(SUM) `cat .filelist`

# MSDOS isn't a full-fledged development environment.
# Preparing a distribution for MSDOS involves discarding some things
# and pre-building others.  All should have \n converted to CR LF.
# From SUPPORT: only setmaps.c and keys.txt [would like teachjove.c, recover.c]
# From MISC: all but Makefile and README.mac
# Preformatted documentation. [would like a joverc]
# tags

DOSSRC = $(HEADERS) $(C_SRC) setmaps.c keys.txt \
	Makefile.bcc Makefile.msc Makefile.wat Makefile.zor \
	README README.dos README.w32 README.cyg sysdep.doc tune.doc style.doc \
	jjove.ico jjove.rc \
	doc/cmds.doc doc/jove.man doc/jove.doc tags

jovedoss.zip:	$(DOSSRC) jjove.ico
	-rm -f jovedoss.zip
	zip -k jovedoss.zip jjove.ico -l $(DOSSRC)

touch:
	touch $(OBJECTS)

clean:
	rm -f a.out core *.o keys.c jjove$(XEXT) portsrv$(XEXT) recover$(XEXT) \
		setmaps$(XEXT) teachjove$(XEXT) make.log *.map \#* *~ *.tmp \
		$(DIST).tgz jjove.pure_* tags ID \
		.filelist xjove/.filelist .version

cleanall: clean
	( cd xjove ; make clean )

clobber: clean
	rm -f paths.h $(FDOCS) $(GEN) *.orig *.rej
	( cd xjove ; make clobber )

# This version only works under 4.3BSD
dependbsd:
	@echo '"make depend" only works under 4.3BSD'
	@-rm -f Makefile.new
	sed -e '/^# DO NOT DELETE THIS LINE/q' Makefile >Makefile.new
	for i in ${SOURCES} ; do \
		$(CC) ${CFLAGS} ${DEPENDFLAG} $$i | \
		awk ' /[/]usr[/]include/ { next } \
			{ if ($$1 != prev) \
		    { if (rec != "") print rec; rec = $$0; prev = $$1; } \
		    else { if (length(rec $$2) > 78) { print rec; rec = $$0; } \
		    else rec = rec " " $$2 } } \
		    END { print rec } ' >>Makefile.new; \
	done
	echo '# DEPENDENCIES MUST END AT END OF FILE' >>Makefile.new
	echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY' >>Makefile.new
	echo '# see "make depend" above' >>Makefile.new
	@echo 'New makefile is in "Makefile.new".  Move it to "Makefile".'

# This version should work with any UNIX
# It records all dependencies, including ones that are #ifdef'ed out.
# It assumes that only jove.h and tune.h include other headers

depend:
	@sed -e '/^# DO NOT DELETE THIS LINE/q' Makefile >Makefile.new
	for i in tune.h jove.h ${SOURCES} ; do \
		( ( echo "$$i:"; sed -n -e 's/^#[ 	]*include[ 	]*"\([^"]*\)".*/\1/p' $$i ) | \
		sed -e 's/^jove\.h$$/$$(JOVE_H)/' -e 's/^tune\.h$$/$$(TUNE_H)/' \
			-e 's/^jove\.h:$$/JOVE_H = jove.h/' -e 's/^tune\.h:$$/TUNE_H = tune.h/' \
			-e 's/\.c:$$/.o:/' | \
		tr "\012" "\040" ; echo ) | sed -e 's/ $$//' -e '/:$$/d' >>Makefile.new ; \
	done
	@echo '# DEPENDENCIES MUST END AT END OF FILE' >>Makefile.new
	@echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY' >>Makefile.new
	@echo '# see "make depend" above' >>Makefile.new
	@if cmp -s Makefile Makefile.new ; \
		then echo '*** Makefile is already up to date' ; \
		else echo '*** New makefile is in "Makefile.new".  Move it to "Makefile".' ; \
	fi

# DO NOT DELETE THIS LINE -- "make depend" uses it
TUNE_H = tune.h sysdep.h
JOVE_H = jove.h $(TUNE_H) buf.h io.h dataobj.h keymaps.h argcount.h util.h externs.h
commands.o: $(JOVE_H) jctype.h extend.h macros.h mouse.h abbrev.h c.h case.h commands.h delete.h disp.h insert.h sysprocs.h iproc.h marks.h misc.h move.h paragraph.h proc.h reapp.h wind.h commands.tab
abbrev.o: $(JOVE_H) fp.h jctype.h abbrev.h ask.h commands.h delete.h insert.h disp.h fmt.h move.h wind.h
argcount.o: $(JOVE_H) jctype.h
ask.o: $(JOVE_H) jctype.h chars.h disp.h fp.h scandir.h screen.h ask.h delete.h insert.h extend.h fmt.h marks.h move.h mac.h
buf.o: $(JOVE_H) jctype.h disp.h ask.h extend.h fmt.h insert.h macros.h marks.h move.h sysprocs.h proc.h wind.h fp.h iproc.h mac.h
c.o: $(JOVE_H) re.h c.h jctype.h disp.h delete.h insert.h fmt.h marks.h misc.h move.h paragraph.h
case.o: $(JOVE_H) disp.h case.h jctype.h marks.h move.h
jctype.o: $(JOVE_H) jctype.h
delete.o: $(JOVE_H) jctype.h disp.h delete.h insert.h marks.h move.h
disp.o: $(JOVE_H) jctype.h chars.h fp.h disp.h ask.h extend.h fmt.h insert.h sysprocs.h iproc.h move.h macros.h screen.h term.h wind.h mac.h
extend.o: $(JOVE_H) fp.h jctype.h chars.h commands.h disp.h re.h ask.h extend.h fmt.h insert.h move.h sysprocs.h proc.h vars.h mac.h
fp.o: $(JOVE_H) fp.h jctype.h disp.h fmt.h mac.h
fmt.o: $(JOVE_H) chars.h fp.h jctype.h disp.h extend.h fmt.h mac.h
insert.o: $(JOVE_H) jctype.h list.h chars.h disp.h abbrev.h ask.h c.h delete.h insert.h fmt.h macros.h marks.h misc.h move.h paragraph.h screen.h sysprocs.h proc.h wind.h re.h
io.o: $(JOVE_H) list.h fp.h jctype.h disp.h ask.h fmt.h insert.h marks.h sysprocs.h proc.h wind.h rec.h mac.h re.h temp.h
iproc.o: $(JOVE_H) re.h jctype.h disp.h fp.h sysprocs.h iproc.h ask.h extend.h fmt.h insert.h marks.h move.h proc.h wind.h select.h ttystate.h
jove.o: $(JOVE_H) fp.h jctype.h chars.h disp.h re.h reapp.h sysprocs.h rec.h ask.h extend.h fmt.h macros.h marks.h mouse.h paths.h proc.h screen.h term.h wind.h iproc.h select.h mac.h
list.o: $(JOVE_H) list.h
macros.o: $(JOVE_H) jctype.h fp.h chars.h disp.h ask.h commands.h macros.h extend.h fmt.h
marks.o: $(JOVE_H) fmt.h marks.h disp.h
misc.o: $(JOVE_H) jctype.h disp.h ask.h c.h delete.h insert.h extend.h fmt.h marks.h misc.h move.h paragraph.h
move.o: $(JOVE_H) re.h chars.h jctype.h disp.h move.h screen.h
paragraph.o: $(JOVE_H) jctype.h disp.h delete.h insert.h fmt.h marks.h misc.h move.h paragraph.h re.h
proc.o: $(JOVE_H) jctype.h fp.h re.h disp.h sysprocs.h ask.h delete.h extend.h fmt.h insert.h iproc.h marks.h misc.h move.h proc.h wind.h
re.o: $(JOVE_H) re.h jctype.h ask.h disp.h fmt.h marks.h
reapp.o: $(JOVE_H) fp.h re.h jctype.h chars.h disp.h ask.h extend.h fmt.h marks.h reapp.h wind.h mac.h
rec.o: $(JOVE_H) fp.h sysprocs.h rec.h fmt.h recover.h
scandir.o: $(JOVE_H) scandir.h
screen.o: $(JOVE_H) fp.h chars.h jctype.h disp.h extend.h fmt.h term.h mac.h screen.h wind.h
term.o: $(JOVE_H) term.h fp.h
termcap.o: $(JOVE_H) term.h disp.h fmt.h fp.h jctype.h screen.h
unix.o: $(JOVE_H) fp.h chars.h term.h ttystate.h util.h
util.o: $(JOVE_H) jctype.h disp.h fp.h ask.h chars.h fmt.h insert.h macros.h marks.h move.h rec.h mac.h
vars.o: $(JOVE_H) extend.h vars.h abbrev.h ask.h c.h jctype.h disp.h insert.h sysprocs.h iproc.h mac.h mouse.h paragraph.h proc.h re.h reapp.h rec.h screen.h term.h ttystate.h wind.h vars.tab
wind.o: $(JOVE_H) chars.h disp.h ask.h extend.h commands.h mac.h reapp.h wind.h screen.h
msgetch.o: $(JOVE_H) chars.h disp.h
mac.o: $(TUNE_H) $(JOVE_H) mac.h ask.h chars.h disp.h extend.h fp.h commands.h fmt.h marks.h misc.h move.h screen.h scandir.h term.h vars.h wind.h
keymaps.o: $(JOVE_H) list.h fp.h jctype.h chars.h disp.h re.h ask.h commands.h macros.h extend.h fmt.h screen.h vars.h sysprocs.h iproc.h
ibmpcdos.o: $(JOVE_H) fp.h chars.h screen.h term.h
mouse.o: $(JOVE_H) disp.h misc.h ask.h chars.h delete.h fmt.h insert.h marks.h move.h wind.h term.h jctype.h mouse.h xjove/mousemsg.h fp.h
win32.o: $(JOVE_H) fp.h chars.h screen.h disp.h
portsrv.o: $(JOVE_H) sysprocs.h iproc.h
recover.o: $(JOVE_H) temp.h sysprocs.h rec.h paths.h recover.h scandir.c jctype.h
setmaps.o: $(JOVE_H) chars.h commands.h vars.h commands.tab vars.tab
teachjove.o: $(TUNE_H) paths.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see "make depend" above
