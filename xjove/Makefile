CIFLAGS = -m'some reason for change' -u4.14.10.n -q

# For SunView:
# DEFINES=SUNVIEW
DEFINES=

# for Solaris 2.x:
SYSDEFS=-DSUNOS5
LDFLAGS= -g -i -L$$OPENWINHOME/lib -R $$OPENWINHOME/lib

# for Sun0s4.x:
# LDFLAGS=-L$$OPENWINHOME/lib
# SYSDEFS=-DSUNOS41

CFLAGS = -g -I.. $(SYSDEFS) $(DEFINES)
XCC=$(CC) -I$$OPENWINHOME/include

BINDIR = /usr/local/bin
GROUP = -g bin
INSTALL = /usr/ucb/install

# REMEMBER: when adding to these lists, adjust BAKUPSXJ in ../Makefile

C_SRC = jovetool.c xjove.c jovewindows.c xjovewindows.c jovemenu.c \
	exts.h jovewindows.h mousemsg.h xjovewindows.h
ICONS = jove.icon jove.cursor copy.cursor cut.cursor paste.cursor
MISC = Makefile README
BACKUPS = $(C_SRC) $(ICONS) $(MISC)

all:		xjove

install:	installxjove

.filelist: $(BACKUPS)
	@ls $(BACKUPS) >.filelist

installxjove:	xjove
		$(INSTALL) $(GROUP) -c -s -m 755 xjove $(BINDIR)/xjove

installjovetool:	jovetool
		$(INSTALL) $(GROUP) -c -s -m 755 jovetool $(BINDIR)/jovetool

jovetool:	jovetool.o jovewindows.o
		$(CC) -o jovetool jovetool.o jovewindows.o -lsuntool -lsunwindow -lpixrect

jovetool.o:	jovetool.c mousemsg.h jove.icon jove.cursor cut.cursor paste.cursor

jovewindows.o:	jovewindows.c jovemenu.c jovewindows.h ../tune.h

xjove:	xjove.o xjovewindows.o
		$(CC) $(LDFLAGS) -o xjove xjove.o xjovewindows.o -lxview -lX11 -lolgx

xjove.o:	xjove.c mousemsg.h jove.icon jove.cursor cut.cursor paste.cursor
		$(XCC) $(CFLAGS) -c xjove.c

xjovewindows.o:	xjovewindows.c jovemenu.c xjovewindows.h ../tune.h
		$(XCC) $(CFLAGS) -c xjovewindows.c

ciall:
	ci $(CIFLAGS) $(BACKUPS)

clean:
		rm -f a.out core *.o jovetool xjove '#'*~ .filelist

clobber:	clean
