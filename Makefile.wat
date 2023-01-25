##########################################################################
# This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    #
# provided by Jonathan and Jovehacks without charge and without          #
# warranty.  You may copy, modify, and/or distribute JOVE, provided that #
# this notice is included in all the source files and documentation.     #
##########################################################################

# wmake -f makefile.wat [<targets>]

# Makefile for Watcom C 10.0
#
# - supported targets:
#   + jjove.exe (build JOVE, but don't install it)
#   + jovedosx.zip (build executable JOVE kit)
#   + clean
#
# - to install, do the following:
#   + copy jjove.exe where you wish
#   + copy doc/cmds.doc to <SHAREDIR>/cmds.doc
#   + optional: copy some jove rc (none supplied) file to <SHAREDIR>/jove.rc

# Watcom Quirks:
# - "&" is used instead of "\" as the continuation character
# - .SYMBOLIC is needed it a rule doesn't create the target (eg. "clean")
# - .AUTODEPEND picks up dependencies recorded in .obj files
# - stack size is specified using -k
# - "*" at the start of a command will work around >128 char args.
#   For some reason, I couldn't get this to work, so I used *.obj
#   in the link step -- a bit fragile!
# - To get wild-card processing of command-line args, we must
#   link in wildargv.obj (which we must build -- see below).

# ===================================================================
# Jove configuration: default JOVE paths.
# Note that all these can be set from environment variables as well;
# see README.DOS for details.
#
# TMPDIR is where the tmp files get stored, usually /tmp or /usr/tmp.
# RECDIR is the directory in which RECOVER looks for JOVE's tempfiles.
# LIBDIR is for the PORTSRV and RECOVER programs.
# SHAREDIR is for online documentation, and the system-wide jove.rc file.
## BINDIR is where to put the executables JOVE
# DFLTSHELL is the default shell invoked by JOVE.

TMPDIR = c:/tmp
RECDIR = c:/tmp
# BINDIR = c:/jove
# LIBDIR and SHAREDIR are relative to executable directory
LIBDIR = 
SHAREDIR = doc
DFLTSHELL = command

# Compiler:

CC = wcc

# Watcom Compiler Flags:
#
# -d2		full symbolic debugging information
# -fo=<file_name> set object or preprocessor output file name
# -i=<directory>	another include directory
# -m{s,m,c,l,h}	memory model (Small,Medium,Compact,Large,Huge)
# -w<number>	set warning level number
# -wx		generate all warnings
# -zq		operate quietly (diagnostics are not suppressed)
# -os		optimize for size over speed
# -ot		optimize for speed over size
# -s 		do not add stack overflow check code
#
# Same as UNIX:
# -d<name>[=text] precompilation #define name [text]
# NOTE: quotes around the macro body in a -D are actually taken as part
# of that body!!

# -ms (small mode) cannot be used (Jove is about 15K over the 64K code limit,
# even with -DBAREBONES, and OPTFLAGS="-os -s")
# -mm (medium mode) builds with very few I/O buffers, and 512-byte max line
# length. Might run out of tmp file space or heap memory if editing many
# files.
# -ml (large mode) is recommended, has more buffers (so should be faster
# for normal use), 2K max line length, takes advantage of all heap memory
# so can keep a very large number of files open.
# 640K ought to be enough for anyone!
MODEL = -ml
OPTFLAGS = -os	# optimize for size over speed
CFLAGS = $(MODEL) $(OPTFLAGS) -wx -zq -dOWCDOS=1

# Linker:

LD = wcl

# Watcom wcl (Watcom Compile/Link) Link Flags:
#
# -fe=<file_name> set .exec output file name
# -fm 		generate .map file
# -kN		allocate N bytes for stack
# -x		make case of names significant
# N for stack may need to be lowered if linking complains that
# DGROUP is too large (reduce it by the number of bytes in the 
# complaint)

STACKSIZE = 11500
LDFLAGS = $(CFLAGS) -x -k$(STACKSIZE)

# ===================================================================
# Implicit rules.

.c.obj:	.AUTODEPEND
	$(CC) $(CFLAGS) $<

.obj.exe:
	$(LD) $(LDFLAGS) $<


OBJECTS = keys.obj commands.obj abbrev.obj ask.obj buf.obj c.obj &
	case.obj jctype.obj delete.obj extend.obj argcount.obj insert.obj &
	io.obj jove.obj macros.obj marks.obj misc.obj mouse.obj move.obj &
	para.obj proc.obj re.obj reapp.obj rec.obj scandir.obj list.obj &
	keymaps.obj util.obj vars.obj wind.obj fmt.obj disp.obj term.obj &
	fp.obj screen.obj msgetch.obj ibmpcdos.obj

HEADERS = abbrev.h argcount.h ask.h buf.h c.h case.h chars.h commands.h &
	jctype.h dataobj.h delete.h disp.h extend.h externs.h &
	fmt.h fp.h insert.h io.h iproc.h jove.h &
	keymaps.h list.h mac.h macros.h marks.h &
	misc.h mouse.h move.h para.h proc.h &
	re.h reapp.h rec.h scandir.h screen.h &
	sysdep.h sysprocs.h temp.h term.h ttystate.h &
	tune.h util.h vars.h version.h wind.h

all: jjove.exe recover.exe

#
# For this reason, we can only force the building of paths.h
# by adding it to the dependencies for explicit targets.
# In the hope that it is built soon enough, we put it at the front.
# To avoid over-long cmd line, we use *.obj for jjove link,
# which means we have to ensure recover or setmaps use one-step
# compile-and-link and do not leave .obj lying around.

#	* $(LD) $(LDFLAGS) -fe=$* $(OBJECTS)
jjove.exe:	paths.h $(OBJECTS) wildargv.obj
	-del recover.obj
	-del setmaps.obj
	$(LD) $(LDFLAGS) -fm -fe=$* *.obj

recover.exe:	recover.c
	wcl $(CFLAGS) $(LDFLAGS) $< -fe=$@ $<

# Complains about an overly long command
# $(OBJECTS):	$(HEADERS)

jovedosx.zip:	paths.h jjove.exe
	-del jovedosx.zip
	-del jove.exe
	rename jjove.exe jove.exe
	pkzip -aP jovedosx.zip jove.exe recover.exe doc\*.* paths.h README.dos changelg.txt

# Note that quotes are not stripped by the shell that will
# execute the recipe for paths.h

paths.h:	Makefile.wat
	@echo Making <<paths.h
/* Changes should be made in Makefile.wat, not to this file! */

#define TMPDIR "$(TMPDIR)"
#define RECDIR "$(RECDIR)"
#define LIBDIR "$(LIBDIR)"
#define SHAREDIR "$(SHAREDIR)"
#define TEACHJOVE "teachjov.txt"
#define DFLTSHELL "$(DFLTSHELL)"
<<KEEP

setmaps.exe:	setmaps.c commands.tab
	wcl $(CFLAGS) $(LDFLAGS) setmaps.c -fe=$@

keys.c:	setmaps.exe keys.txt
	setmaps < keys.txt > keys.c

# Note: it may be necessary to manually copy the source file from
# the distribution CDROM to the installation.  On the CDROM, the
# file's path is \watcom\src\startup\wildargv.c.
# Or you can get it from 
# http://perforce.openwatcom.org:4000/@md=d&cd=//depot/openwatcom/bld/clib/startup/c/&c=WFs@//depot/openwatcom/bld/clib/startup/c/wildargv.c?ac=22
# (e.g. version 3 seems to compile with OpenWatcom 1.9)
# The latest versions at github need newer compilers than 1.9.0
# https://github.com/open-watcom/open-watcom-v2/master/bld/clib/startup/c/wildargv.c
# At least with some versions (fixed in the github version), wildargv.c does not accept tabs as
# argument delimiters.  This should be fixed.
# Change line 82 from:
#	    while( *p == ' ' ) ++p;	/* skip over blanks */
# to:
#	    while( *p == ' ' || *p == '\t' ) ++p;	/* skip over blanks */
# Change line 103 from:
#		    if( *p == ' ' ) break;
# to:
#		    if( *p == ' ' || *p == '\t' ) break;

#WILDSRC=$(%WATCOM)\src\startup\wildargv.c
WILDSRC=wildargv.c
wildargv.obj:	$(WILDSRC)
	$(CC) $(CFLAGS) $(WILDSRC)

clean:	.SYMBOLIC
	-del *.obj
	-del *.exe
	-del *.bak
	-del *.map
	-del keys.c
