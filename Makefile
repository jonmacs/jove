########################################################################
# This program is Copyright (C) 1986 by Jonathan Payne.  JOVE is       #
# provided to you without charge, and with no warranty.  You may give  #
# away copies of JOVE, including sources, provided that this notice is #
# included in all the files.                                           #
########################################################################

# TMPDIR is where the tmp files get stored, usually /tmp or /tmp/jove.  If
# your system does not remove subdirectories of /tmp on reboot (lots do
# remove them these days) then it makes sense to make TMPDIR be /tmp/jove.
# But if you want to recover buffers on system crashes, you should create a
# directory that doesn't get clearned upon reboot, and use that instead.
# You would probably want to clean out that directory periodically with
# /etc/cron.  LIBDIR is for online documentation, the PORTSRV process,
# RECOVER, and the system-wide .joverc file.  BINDIR is where to put the
# executables JOVE and TEACHJOVE.  MANDIR is where the manual pages go for
# JOVE, RECOVER and TEACHJOVE.  MANEXT is the extension for the man pages,
# e.g., jove.1 or jove.l or jove.m.

DESTDIR =
TMPDIR = /tmp
LIBDIR = /u/jpayne/lib/jove
BINDIR = /u/jpayne/bin
MANDIR = /u/jpayne/lib/manl
MANEXT = l
SHELL = /bin/csh

# These should all just be right if the above ones are.
JOVE = $(DESTDIR)$(BINDIR)/jove
TEACHJOVE = $(DESTDIR)$(BINDIR)/teachjove
RECOVER = $(DESTDIR)$(LIBDIR)/recover
PORTSRV = $(DESTDIR)$(LIBDIR)/portsrv
JOVERC = $(DESTDIR)$(LIBDIR)/.joverc
CMDS.DOC = $(DESTDIR)$(LIBDIR)/cmds.doc
TEACH-JOVE = $(DESTDIR)$(LIBDIR)/teach-jove
JOVEM = $(DESTDIR)$(MANDIR)/jove.$(MANEXT)
TEACHJOVEM = $(DESTDIR)$(MANDIR)/teachjove.$(MANEXT)

# Select the right libraries for your system.
#	2.10BSD:LIBS = -ltermcap
#	v7:	LIBS = -ltermcap
#	4.1BSD:	LIBS = -ltermcap -ljobs
#	4.2BSD:	LIBS = -ltermcap
#	4.3BSD:	LIBS = -ltermcap
#	SysV Rel. 2: LIBS = -lcurses

LIBS = -ltermcap

# If you are not VMUNIX (vax running Berkeley Version 4), you must specify
# the -i flags (split I/D space) and maybe the -x option (for adb to work).
#	2.10BSD:LDFLAGS =
#	v7:	LDFLAGS =
#	4.1BSD:	LDFLAGS =
#	4.2BSD:	LDFLAGS =
#	4.3BSD:	LDFLAGS =
#	SysV Rel. 2: LDFLAGS = -Ml
#
# SEPFLAG should be:
#	not on a PDP-11:		SEPFLAG =
#	PDP-11 with separate I&D:	SEPFLAG = -i
#	PDP-11 without separate I&D:	SEPFLAG = -n

LDFLAGS = 
SEPFLAG =

CFLAGS = -O

BASESEG = funcdefs.o keymaps.o argcount.o ask.o buf.o ctype.o delete.o \
	  disp.o insert.o io.o jove.o malloc.o marks.o misc.o re.o \
	  screen.o table.o tune.o util.o vars.o version.o
OVLAY1 = abbrev.o rec.o paragraph.o fmt.o
OVLAY2 = c.o wind.o fp.o move.o
OVLAY3 = extend.o macros.o
OVLAY4 = iproc.o re1.o
OVLAY5 = proc.o scandir.o term.o case.o

OBJECTS = $(BASESEG) $(OVLAY1) $(OVLAY2) $(OVLAY3) $(OVLAY4) $(OVLAY5)

C_SRC = funcdefs.c abbrev.c argcount.c ask.c buf.c c.c case.c ctype.c \
	delete.c disp.c extend.c fp.c fmt.c insert.c io.c iproc.c \
	jove.c macros.c malloc.c marks.c misc.c move.c paragraph.c \
	proc.c re.c re1.c rec.c scandir.c screen.c table.c term.c util.c \
	vars.c version.c wind.c

ASM_SRC = break.asm getch.asm

SOURCES = $(C_SRC) portsrv.c recover.c setmaps.c teachjove.c

HEADERS = ctype.h io.h jove.h re.h rec.h table.h temp.h termcap.h tune.h

DOCS =	doc/cmds.doc.nr doc/example.rc doc/jove.1 doc/jove.2 doc/jove.3 \
	doc/jove.4 doc/jove.5 doc/jove.nr doc/system.rc \
	doc/teach-jove doc/teachjove.nr doc/README

MISC = Makefile Ovmakefile Makefile.dos pcjove.lnk tune.dos tune.template \
	README Readme.dos iproc-pipes.c iproc-ptys.c

SUPPORT = teachjove.c recover.c setmaps.c portsrv.c keymaps.txt

BACKUPS = $(HEADERS) $(C_SRC) $(ASM_SRC) $(DOCS) $(SUPPORT) $(MISC)

all:	sdate xjove recover teachjove portsrv edate

sdate:
	@echo "**** make started at `date` ****"

edate:
	@echo "**** make completed at `date` ****"

xjove:	$(OBJECTS)
	$(CC) $(LDFLAGS) -o xjove $(OBJECTS) $(LIBS)
	@-size xjove

gjove:	$(OBJECTS)
	ld -X /lib/gcrt0.o -o gjove $(OBJECTS) -lc $(LIBS)
	@-size gjove

ovjove:	$(OBJECTS)
	ld $(SEPFLAG) $(LDFLAGS) -X /lib/crt0.o \
		-Z $(OVLAY1) \
		-Z $(OVLAY2) \
		-Z $(OVLAY3) \
		-Z $(OVLAY4) \
		-Z $(OVLAY5) \
		-Y $(BASESEG) \
		-o xjove $(LIBS) -lc; \
	@-size xjove

portsrv:	portsrv.o
	cc -o portsrv -n portsrv.o $(LIBS)

recover:	recover.o tune.o rec.h temp.h
	cc -o recover -n recover.o tune.o $(LIBS)

teachjove:	teachjove.o
	cc -o teachjove -n teachjove.o $(LIBS)

setmaps:	setmaps.o funcdefs.c
	cc -o setmaps setmaps.o

teachjove.o:	teachjove.c /usr/include/sys/types.h /usr/include/sys/file.h
	cc -c $(CFLAGS) -DTEACHJOVE=\"$(TEACH-JOVE)\" teachjove.c

# don't optimize setmaps.c because it produces bad code on the sun2
#
setmaps.o:	funcdefs.c keymaps.txt
	cc -c setmaps.c

# ignore error messages from setmaps
# it doesn't understand ifdefs

keymaps.c:	setmaps keymaps.txt
	-setmaps < keymaps.txt > keymaps.c

keymaps.o:	keymaps.c jove.h

tune.c: Makefile tune.template
	@echo "/* Changes should be made in Makefile, not to this file! */" > tune.c
	@echo "" >> tune.c
	@sed -e 's;TMPDIR;$(TMPDIR);' \
	     -e 's;LIBDIR;$(LIBDIR);' \
	     -e 's;BINDIR;$(BINDIR);' \
	     -e 's;SHELL;$(SHELL);' tune.template >> tune.c

install: $(DESTDIR)$(LIBDIR) $(TEACH-JOVE) $(CMDS.DOC) $(JOVERC) \
	 $(PORTSRV) $(RECOVER) $(JOVE) $(TEACHJOVE) $(JOVEM) \
	 $(RECOVERM) $(TEACHJOVEM)

$(DESTDIR)$(LIBDIR):
	-mkdir $(DESTDIR)$(LIBDIR)

$(TEACH-JOVE): doc/teach-jove
	install -c -m 644 doc/teach-jove $(TEACH-JOVE)

doc/cmds.doc:	doc/cmds.doc.nr doc/jove.4 doc/jove.5
	nroff doc/cmds.doc.nr doc/jove.4 doc/jove.5 > doc/cmds.doc

$(CMDS.DOC): doc/cmds.doc
	install -c -m 644 doc/cmds.doc $(CMDS.DOC)

$(JOVERC): doc/system.rc
	install -c -m 644 doc/system.rc $(JOVERC)

$(PORTSRV): portsrv
	install -c -s -m 755 portsrv $(PORTSRV)

$(RECOVER): recover
	install -c -s -m 755 recover $(RECOVER)

$(JOVE): xjove
	install -c -m 755 xjove $(JOVE)

$(TEACHJOVE): teachjove
	install -c -s -m 755 teachjove $(TEACHJOVE)

$(JOVEM): doc/jove.nr
	@sed -e 's;TMPDIR;$(TMPDIR);' \
	     -e 's;LIBDIR;$(LIBDIR);' \
	     -e 's;SHELL;$(SHELL);' doc/jove.nr > /tmp/jove.nr
	install -m 644 /tmp/jove.nr $(JOVEM)

$(TEACHJOVEM): doc/teachjove.nr
	@sed -e 's;TMPDIR;$(TMPDIR);' \
	     -e 's;LIBDIR;$(LIBDIR);' \
	     -e 's;SHELL;$(SHELL);' doc/teachjove.nr > /tmp/teachjove.nr
	install -m 644 /tmp/teachjove.nr $(TEACHJOVEM)

echo:
	@echo $(C-FILES) $(HEADERS)

lint:
	lint -n $(C_SRC) tune.c keymaps.c
	@echo Done

tags:
	ctags -w $(C_SRC) $(HEADERS) iproc-ptys.c

ciall:
	ci $(BACKUPS)

coall:
	co $(BACKUPS)

jove.shar:
	shar $(BACKUPS) > jove.shar

backup: $(BACKUPS)
	tar cf backup $(BACKUPS)

tape-backup:
	tar cbf 20 /dev/rmt0 $(BACKUPS)

rtape-backup:
	rtar cbf 20 sen:/dev/rmt0 $(BACKUPS)

touch:
	touch $(OBJECTS)

clean:
	rm -f a.out core *.o keymaps.c tune.c xjove portsrv recover setmaps \
	teachjove

# This version only works under 4.3BSD
#depend:
#	for i in ${SOURCES} ; do \
#		cc -M ${CFLAGS} $$i | awk ' { if ($$1 != prev) \
#		    { if (rec != "") print rec; rec = $$0; prev = $$1; } \
#		    else { if (length(rec $$2) > 78) { print rec; rec = $$0; } \
#		    else rec = rec " " $$2 } } \
#		    END { print rec } ' >> makedep; done
#	echo '/^# DO NOT DELETE THIS LINE/+2,$$d' >eddep
#	echo '$$r makedep' >>eddep
#	echo 'w' >>eddep
#	cp Makefile Makefile.bak
#	ed - Makefile < eddep
#	rm eddep makedep
#	echo '# DEPENDENCIES MUST END AT END OF FILE' >> Makefile
#	echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY' >> Makefile
#	echo '# see make depend above' >> Makefile
#
## DO NOT DELETE THIS LINE -- make depend uses it
