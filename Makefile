###########################################################################
# This program is Copyright (C) 1986, 1987, 1988 by Jonathan Payne.  JOVE #
# is provided to you without charge, and with no warranty.  You may give  #
# away copies of JOVE, including sources, provided that this notice is    #
# included in all the files.                                              #
###########################################################################

# TMPDIR is where the tmp files get stored, usually /tmp or /tmp/jove.  If
# your system does not remove subdirectories of /tmp on reboot (lots do
# remove them these days) then it makes sense to make TMPDIR be /tmp/jove.
# But if you want to recover buffers on system crashes, you should create a
# directory that doesn't get cleaned upon reboot, and use that instead.
# You would probably want to clean out that directory periodically with
# /etc/cron.
# RECDIR is the directory in which RECOVER looks for JOVE's tempfiles.
#
# JOVEHOME is the directory in which pieces of JOVE are kept.  It is only used
#	in the default definitions of SHAREDIR, LIBDIR, BINDIR, and MANDIR.
# SHAREDIR is for online documentation, and the system-wide .joverc file.
# LIBDIR is for the PORTSRV and RECOVER programs.
# BINDIR is where to put the executables JOVE and TEACHJOVE.
# MANDIR is where the manual pages go for JOVE, RECOVER and TEACHJOVE.
# MANEXT is the extension for the man pages, e.g., jove.1 or jove.l or jove.m.
# DFLTSHELL is the default shell invoked by JOVE and TEACHJOVE.
#
# If they don't exist, this makefile will try to create the directories
# LIBDIR and SHAREDIR.  All others must already exist.

SHELL = /bin/sh
TMPDIR = /tmp
RECDIR = /usr/preserve

JOVEHOME = /local
SHAREDIR = $(JOVEHOME)/lib/jove
LIBDIR = $(JOVEHOME)/lib/jove
BINDIR = $(JOVEHOME)/bin
MANDIR = $(JOVEHOME)/man/man$(MANEXT)
MANEXT = 1
DFLTSHELL = /bin/csh

# to install executable files
XINSTALL=cp
#XINSTALL=install -c -m 755 # -s
# to install text files
TINSTALL=cp
#TINSTALL=install -c -m 644

PROG = jove
VERSION = 4.14

# These should all just be right if the above ones are.
JOVE = $(BINDIR)/jove
TEACHJOVE = $(BINDIR)/teachjove
RECOVER = $(LIBDIR)/recover
PORTSRV = $(LIBDIR)/portsrv
JOVERC = $(SHAREDIR)/jove.rc
CMDS.DOC = $(SHAREDIR)/cmds.doc
TEACH-JOVE = $(SHAREDIR)/teach-jove
JOVEM = $(MANDIR)/jove.$(MANEXT)
TEACHJOVEM = $(MANDIR)/teachjove.$(MANEXT)

# Select optimization level (flags passed to compiling and linking steps).
# On most systems, -g for debugging, -O for optimization.

OPTFLAGS = -g

# Select the right libraries for your system.
#	2.10BSD:LIBS = -ltermcap
#	v7:	LIBS = -ltermcap
#	4.1BSD:	LIBS = -ltermcap -ljobs
#	4.2BSD:	LIBS = -ltermcap
#	4.3BSD:	LIBS = -ltermcap
#	SysV Rel. 2: LIBS = -lcurses
#	SCO Xenix: LIBS = -ltermcap -lx
#	SCO: LIBS = -lcurses
#	AIX on the R6000s: LIBS = -lcurses
#	MIPS: LIBS = -ltermcap

LIBS = -ltermcap

#	2.10BSD:LDFLAGS =
#	v7:	LDFLAGS =
#	4.1BSD:	LDFLAGS =
#	4.2BSD:	LDFLAGS =
#	4.3BSD:	LDFLAGS =
#	SysV Rel. 2: LDFLAGS = -Ml
#	SCO Xenix: LDFLAGS = -Ml -F 3000
#	SCO Unix: LDFLAGS = 
#
# To optimize the use of the address spaces, add to the LDFLAGS:
#	PDP-11 with separate I&D: -i
#	PDP-11 without separate I&D: -n


LDFLAGS =

# define a symbol for your OS if it hasn't got one.  See sysdep.h.
# Jove has very few defaults, you will almost certainly need to define
# *something*.
#
#	BSDI, 386BSD, BSD4.4		SYSDEFS=-DBSDPOSIX
#	BSD4.2,4.3			SYSDEFS=-DBSD4
# 	Apple A/UX on macIIs 		SYSDEFS=-DA_UX
#	SunOS3.x			SYSDEFS=-DSUNOS3
#	SunOS4.0*			SYSDEFS=-DSUNOS40
#	SunOS4.1*			SYSDEFS=-DSUNOS41
#	SunOS5.0/Solaris 2.0		SYSDEFS=-DSUNOS5
#	Ultrix 4.2			SYSDEFS=-DBSDPOSIX
#	MIPS RiscOS4.x			SYSDEFS=-systype bsd43 -DBSD4
#	Irix 3.3 onwards		SYSDEFS=-DIRIX4
#	SCO Unix			SYSDEFS=-DSCO
#	IBM RS6000s			SYSDEFS=-DAIX -D_BSD -D_BSD_INCLUDES -D_NO_PROTO
#	Sys V Release 4			SYSDEFS=-DSYSVR4
#	Sys III, Sys V R 2,3		SYSDEFS=-DSYSV
#
# Some of the MIPS based Ultrix (upto 4.2 at least), RiscOS and Irix (upto
# 3.3 at least) also need -DMIPS_CC_BUG.
#
# Add -DUSE_EXIT if you're profiling or using purify (this causes Jove
# to exit using exit(), instead of _exit()).
#
# You can just say 'make SYSDEFS=-Dwhatever' on these systems.

SYSDEFS = 

# for SCO Xenix, set
#	MEMFLAGS = -Mle
#	CFLAGS = -LARGE -O -F 3000 -K -Mle  (say -Mle2 for an 80286)

CFLAGS = $(OPTFLAGS) $(SYSDEFS)

# For SVR4 (/usr/ucb/cc will NOT work because of setjmp.h).
# CC = /usr/bin/cc

# Load invocation of cc.
# LDCC = purify $(CC)

LDCC = $(CC)

# For cross compiling Jove, set CC to the cross compiler, and LOCALCC
# to the local C compiler. LOCALCC will be used for compiling setmaps,
# which is run as part of the compilation to generate the keymaps.
# Set LOCALCFLAGS and LOCALLDFLAGS appropriately too. For Xenix, note
# that LOCALCFLAGS must be set to $(MEMFLAGS)

LOCALCC = $(CC)
LOCALCFLAGS = $(CFLAGS)	# $(MEMFLAGS)
LOCALLDFLAGS = $(LDFLAGS)

BASESEG = commands.o keys.o argcount.o ask.o buf.o ctype.o delete.o \
	  disp.o insert.o io.o jove.o loadavg.o marks.o misc.o re.o \
	  screen.o ttystate.o tune.o util.o vars.o list.o keymaps.o
OVLAY1 = abbrev.o rec.o paragraph.o fmt.o
OVLAY2 = c.o wind.o fp.o move.o
OVLAY3 = extend.o macros.o
OVLAY4 = iproc.o reapp.o
OVLAY5 = proc.o scandir.o term.o case.o

OBJECTS = $(BASESEG) $(OVLAY1) $(OVLAY2) $(OVLAY3) $(OVLAY4) $(OVLAY5)

C_SRC = commands.c commands.tab abbrev.c argcount.c ask.c buf.c c.c case.c ctype.c \
	delete.c disp.c extend.c fp.c fmt.c insert.c io.c iproc.c \
	jove.c list.c loadavg.c macros.c marks.c misc.c move.c paragraph.c \
	proc.c re.c reapp.c rec.c scandir.c screen.c term.c ttystate.c util.c \
	vars.c vars.tab wind.c getch.c mac.c keymaps.c pcscr.c

SOURCES = $(C_SRC) portsrv.c recover.c setmaps.c teachjove.c

HEADERS = abbrev.h argcount.h ask.h buf.h c.h case.h chars.h commands.h \
	ctype.h dataobj.h delete.h disp.h extend.h externs.h \
	fmt.h fp.h getch.h insert.h io.h iproc.h jove.h \
	keymaps.h list.h loadavg.h mac.h macros.h marks.h \
	misc.h move.h paragraph.h pcscr.h proc.h \
	re.h reapp.h rec.h scandir.h screen.h select.h \
	sysdep.h sysprocs.h temp.h term.h termcap.h ttystate.h \
	tune.h util.h vars.h version.h wind.h

DOCS1 =	doc/example.rc doc/jove.1 doc/jove.2 doc/jove.3 \
	doc/jove.4 doc/jove.5 doc/jove.nr doc/system.rc \
	doc/teach-jove doc/teachjove.nr doc/README doc/jove.qref
DOCS2 = doc/cmds.doc.nr
DOCS3 = doc/joveman doc/cmds.doc doc/manpage
DOCS = $(DOCS1) $(DOCS2)

MISC = Makefile Makefile.dos Makefile.zor \
	tune.dos tune.template \
	README Readme.dos Readme.mac sysdep.doc tune.doc \
	iproc-pipes.c iproc-ptys.c

SUPPORT = teachjove.c recover.c setmaps.c portsrv.c keys.txt \
	macvert.c menumaps.txt mjovers.Hqx

BACKUPS = $(HEADERS) $(C_SRC) $(DOCS) $(SUPPORT) $(MISC)

all:	sdate xjove recover teachjove portsrv macvert doc/cmds.doc edate

sdate:
	@echo "**** make started at `date` ****"

edate:
	@echo "**** make completed at `date` ****"

xjove:	$(OBJECTS)
	$(LDCC) $(LDFLAGS) $(OPTFLAGS) -o xjove $(OBJECTS) $(LIBS)
	@-size xjove

gjove:	$(OBJECTS)
	ld -X /lib/gcrt0.o -o gjove $(OBJECTS) -lc $(LIBS)
	@-size gjove

ovjove:	$(OBJECTS)
	ld $(LDFLAGS) $(OPTFLAGS) -X /lib/crt0.o \
		-Z $(OVLAY1) \
		-Z $(OVLAY2) \
		-Z $(OVLAY3) \
		-Z $(OVLAY4) \
		-Z $(OVLAY5) \
		-Y $(BASESEG) \
		-o xjove $(LIBS) -lc
	@-size xjove

portsrv:	portsrv.o
	$(CC) $(LDFLAGS) $(OPTFLAGS) -o portsrv portsrv.o $(LIBS)

recover:	recover.o tune.o
	$(CC) $(LDFLAGS) $(OPTFLAGS) -o recover recover.o tune.o $(LIBS)

recover.o:	recover.c rectune.h rec.h temp.h

teachjove:	teachjove.o
	$(CC) $(LDFLAGS) $(OPTFLAGS) -o teachjove teachjove.o $(LIBS)

teachjove.o:	teachjove.c /usr/include/sys/types.h /usr/include/sys/file.h
	$(CC) -c $(CFLAGS) -DTEACHJOVE=\"$(TEACH-JOVE)\" teachjove.c

# don't optimize setmaps.c because it produces bad code in some places
# for some reason

setmaps:	setmaps.o
	$(LOCALCC) $(LOCALLDFLAGS) -o setmaps setmaps.o

setmaps.o:	setmaps.c commands.tab vars.tab
	$(LOCALCC) $(LOCALCFLAGS) -c setmaps.c

# Ignore warning messages from setmaps:
# it doesn't understand ifdefs.

keys.c:	setmaps keys.txt
	./setmaps < keys.txt > keys.c

keys.o:	keys.c jove.h

tune.c: Makefile tune.template
	-rm -f tune.c
	echo "/* Changes should be made in Makefile, not to this file! */" > tune.c
	echo "" >> tune.c
	sed -e 's;<TMPDIR>;$(TMPDIR);' \
	     -e 's;<LIBDIR>;$(LIBDIR);' \
	     -e 's;<SHAREDIR>;$(SHAREDIR);' \
	     -e 's;<BINDIR>;$(BINDIR);' \
	     -e 's;<SHELL>;$(DFLTSHELL);' tune.template >> tune.c

rectune.h: Makefile
	-rm -f rectune.h
	@echo "/* Changes should be made in Makefile, not to this file! */" > rectune.h
	@echo "" >> rectune.h
	@echo \#define TMP_DIR \"$(TMPDIR)\" >> rectune.h
	@echo \#define REC_DIR \"$(RECDIR)\" >> rectune.h

iproc.o: iproc-ptys.c iproc-pipes.c iproc.c
	$(CC) $(CFLAGS) -c iproc.c

macvert:	macvert.c
	$(CC) $(CFLAGS) -o macvert macvert.c

# install doesn't work for Xenix (no install program)

install: $(LIBDIR) $(SHAREDIR) \
	 $(TEACH-JOVE) $(CMDS.DOC) $(JOVERC) \
	 $(PORTSRV) $(RECOVER) $(JOVE) $(TEACHJOVE) $(JOVEM) \
	 $(RECOVERM) $(TEACHJOVEM)
	@echo See the README about changes to /etc/rc or /etc/rc.local
	@echo so that the system recovers jove files on reboot after a crash

$(LIBDIR)::
	[ -d $(LIBDIR) ] || mkdir $(LIBDIR)

$(SHAREDIR)::
	[ -d $(SHAREDIR) ] || mkdir $(SHAREDIR)

$(TEACH-JOVE): doc/teach-jove
	$(TINSTALL) doc/teach-jove $(TEACH-JOVE)

doc/cmds.doc:	doc/cmds.doc.nr doc/jove.4 doc/jove.5
	nroff doc/cmds.doc.nr doc/jove.4 doc/jove.5 > doc/cmds.doc

$(CMDS.DOC): doc/cmds.doc
	$(TINSTALL) doc/cmds.doc $(CMDS.DOC)

$(JOVERC): doc/system.rc
	$(TINSTALL) doc/system.rc $(JOVERC)

$(PORTSRV): portsrv
	$(XINSTALL) portsrv $(PORTSRV)

$(RECOVER): recover
	$(XINSTALL) recover $(RECOVER)

$(JOVE): xjove
	$(XINSTALL) xjove $(JOVE)

$(TEACHJOVE): teachjove
	$(XINSTALL) teachjove $(TEACHJOVE)

$(JOVEM): doc/jove.nr
	@sed -e 's;<TMPDIR>;$(TMPDIR);' \
	     -e 's;<LIBDIR>;$(LIBDIR);' \
	     -e 's;<SHAREDIR>;$(SHAREDIR);' \
	     -e 's;<SHELL>;$(DFLTSHELL);' doc/jove.nr > /tmp/jove.nr
	$(TINSTALL) /tmp/jove.nr $(JOVEM)
	rm /tmp/jove.nr

$(TEACHJOVEM): doc/teachjove.nr
	@sed -e 's;<TMPDIR>;$(TMPDIR);' \
	     -e 's;<LIBDIR>;$(LIBDIR);' \
	     -e 's;<SHAREDIR>;$(SHAREDIR);' \
	     -e 's;<SHELL>;$(DFLTSHELL);' doc/teachjove.nr > /tmp/teachjove.nr
	$(TINSTALL) /tmp/teachjove.nr $(TEACHJOVEM)
	rm /tmp/teachjove.nr

echo:
	@echo $(C-FILES) $(HEADERS)

lint:
	lint -n $(C_SRC) tune.c keys.c
	@echo Done

tags:	$(C_SRC) $(HEADERS) iproc-ptys.c
	ctags -w $(C_SRC) $(HEADERS) iproc-ptys.c

ciall:
	ci $(BACKUPS)

coall:
	co $(BACKUPS)

jove.shar:
	shar $(BACKUPS) > jove.shar

tar:
	@tar cvf - `find . -type f -print | \
		egrep -v '(,v|\.o|xjove|portsrv|setmaps|~)$$' | \
		sort`

backup: $(BACKUPS)
	tar cf backup $(BACKUPS)

tape-backup:
	tar cf /dev/rst8 $(BACKUPS)

srcdownload:
	kermit -s $(SUPPORT) $(MISC) $(HEADERS) $(C_SRC)

docdownload:
	kermit -s $(DOCS1) $(DOCS3)
	kermit -s doc/cmds.doc.nr -a cmdsdoc.nr

touch:
	touch $(OBJECTS)

clean:
	rm -f a.out core *.o keys.c tune.c xjove portsrv recover setmaps \
	teachjove macvert rectune.h \#* *~ make.log doc/cmds.doc \
	xjove.pure_* tags ID

clobber: clean
	rm -f *.orig *.rej

# This version only works under 4.3BSD
depend:
	@echo '"make depend" only works under 4.3BSD'
	sed -e '/^# DO NOT DELETE THIS LINE/q' Makefile >Makefile.new
	for i in ${SOURCES} ; do \
		cc -M ${CFLAGS} $$i | \
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

# DO NOT DELETE THIS LINE -- "make depend" uses it
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see "make depend" above
