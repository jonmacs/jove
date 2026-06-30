/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

#include "jove.h"

#ifdef UNIX /* the body is the rest of the file */

#include "fp.h"

#include "chars.h"
#include "term.h"	/* ospeed */
#include "ttystate.h"


struct termios	sg[2];

#ifdef USE_TIOCSLTC
struct ltchars	ls[2];
#endif /* USE_TIOCSLTC */

/* Set tty to original (if !n) or JOVE (if n) modes.
 * This is designed to be idempotent: it can be called
 * several times with the same argument without damage.
 *
 * If STICKY_TTYSTATE is defined, the "original" state is sampled only once.
 * Normally, this is also sampled each time ttysetattr re-enters JOVE mode.
 * STICKY_TTYSTATE is useful if processes run while JOVE is suspended
 * cannot be trusted to leave the tty in a good state.
 * ConvexOS is currently an example: some of its programs turn on ISTRIP
 * without provocation.
 */

/*
 * Modern terminals (which are almost always terminal
 * emulators) are generally fine with ^S and ^Q, so follow GNU
 * Emacs lead and assume that by default.
 */
jbool	OKXonXoff = YES;	/* VAR: XON/XOFF can be used as ordinary chars */
ZXchar	IntChar = CTL(']');	/* VAR: ttysetattr sets this to generate SIGINT */

void
ttysetattr(jbool n)	/* `n' is also used as subscript! */
{
	static jbool	keep_saved = NO;

	if (!keep_saved) {
		/* Save the current tty settings:
		 * do the ioctls or whatever to fill in NO half
		 * of each appropriate tty state pair.
		 * NOTE: previously, the tty was not in JOVE mode.
		 * NOTE: the nested tangle of ifdefs is intended to follow
		 * the structure of the definitions in ttystate.c.
		 */
		(void) tcgetattr(0, &sg[NO]);

#ifdef USE_TIOCSLTC
		(void) ioctl(0, TIOCGLTC, &ls[NO]);
#endif /* USE_TIOCSLTC */

/* extract some info from results */

#ifdef TAB3
		TABS = (sg[NO].c_oflag & TABDLY) != TAB3;
#endif
		ospeed = cfgetospeed(&sg[NO]);

#ifdef STICKY_TTYSTATE
		/* keep saved copy of ttystate until JOVE quits */
		keep_saved = YES;
#endif
	}
#ifndef STICKY_TTYSTATE
	/* Keep saved copy of ttystate next time iff we are switching to JOVE mode.
	 * In other words, don't replace saved copy next time if we will
	 * be in JOVE mode -- the wrong mode to save.
	 */
	keep_saved = n;
#endif

	/* Fill in YES half of each appropriate tty state pair.
	 * They are filled in as late as possible so that each will
	 * reflect the latest settings of controling variables.
	 * NOTE: the nested tangle of ifdefs is intended to follow
	 * the structure of the definitions in ttystate.c.
	 */

	sg[YES] = sg[NO];

	if (OKXonXoff)
		sg[YES].c_iflag &= ~(IXON | IXOFF);
	sg[YES].c_iflag &= ~(INLCR|ICRNL|IGNCR | (MetaKey? ISTRIP : 0));
	sg[YES].c_lflag &= ~(ICANON|ECHO);
	sg[YES].c_oflag &= ~(OPOST);

	/* Set all those c_cc elements that we must.
	 * For peculiar systems, one might wish to predefine JVDISABLE
	 * in the configuration.  For example, on some unnamed
	 * versions of the Convex OS, it would be good to
	 * define it as (sg[YES].c_cc[VDISABLE]), saving a system call.
	 * Note that the only uses of JVDISABLE are in this block,
	 * so the macro may safely refer to things in this context.
	 */
	{
#ifndef JVDISABLE
# ifdef _POSIX_VDISABLE
#  define JVDISABLE	_POSIX_VDISABLE
# else /* !_POSIX_VDISABLE */
#  ifdef _PC_VDISABLE
		/* Cache the result of fpathconf to reduce the number of syscalls.
		 * We don't handle the error return (-1) because there isn't
		 * anything better to do with it.
		 */
	    cc_t	jvd = fpathconf(0, _PC_VDISABLE);
#   define JVDISABLE	jvd
#  else /* !_PC_VDISABLE */
#   define JVDISABLE	0
#  endif /* !_PC_VDISABLE */
# endif /* !_POSIX_VDISABLE */
#endif /* JVDISABLE */

		sg[YES].c_cc[VINTR] = IntChar;

#ifdef VQUIT
		sg[YES].c_cc[VQUIT] = JVDISABLE;
#endif
		/* VERASE, VKILL, VEOL2 irrelevant */
		/* Beware aliasing! VMIN is VEOF and VTIME is VEOL */
#ifdef VSWTCH
		sg[YES].c_cc[VSWTCH] = JVDISABLE;
#endif

		/* Under at least one system (SunOS 4.0), <termio.h>
		 * mistakenly defines the extra V symbols of <termios.h>
		 * without extending the c_cc array in struct termio
		 * to hold them!  This is why the following goo is doubly
		 * ifdefed.  It turns out that we don't use <termio.h>
		 * on SunOS 4.0, so the problem may be moot.
		 */

#ifdef VSUSP
		sg[YES].c_cc[VSUSP] = JVDISABLE;
#endif
#ifdef VDSUSP
		sg[YES].c_cc[VDSUSP] = JVDISABLE;
#endif
#ifdef VDISCARD
		/* ??? Under Solaris 2.1 needs VDISCARD disabled, or it will
		 * be processed by the tty driver, but not under SysVR4!
		 */
		sg[YES].c_cc[VDISCARD] = JVDISABLE;	/* flush output */
#endif
#ifdef VLNEXT
		sg[YES].c_cc[VLNEXT] = JVDISABLE;	/* literal next char */
#endif

		sg[YES].c_cc[VMIN] = 1;
		sg[YES].c_cc[VTIME] = 1;
	}

#ifdef USE_TIOCSLTC
	ls[YES] = ls[NO];
	ls[YES].t_suspc = (char) -1;
	ls[YES].t_dsuspc = (char) -1;
	ls[YES].t_flushc = (char) -1;
	ls[YES].t_lnextc = (char) -1;
#endif /* USE_TIOCSLTC */

	/* Set tty state according to appropriate entry of each state pair.
	 * NOTE: the nested tangle of ifdefs is intended to follow
	 * the structure of the definitions in ttystate.c.
	 */

	do {} while (tcsetattr(0, TCSADRAIN, &sg[n]) < 0 && errno == EINTR);

#ifdef USE_TIOCSLTC
	(void) ioctl(0, TIOCSLTC, &ls[n]);
#endif /* USE_TIOCSLTC */

}

/* Determine the number of characters to buffer at each baud rate.  The
 * lower the number, the quicker the response when new input arrives.  Of
 * course the lower the number, the more prone the program is to stop in
 * output.  Decide what matters most to you. This sets ScrBufSize to the right
 * number or chars, and initializes `jstdout'.
 */
void
settout(void)
{
	int	speed_chars;

	static const struct {
		unsigned int bsize;
		unsigned int brate;
	} speeds[] = {

#ifdef B0
		{ 1, B0 },
#endif
#ifdef B50
		{ 1, B50 },
#endif
#ifdef B75
		{ 1, B75 },
#endif
#ifdef B110
		{ 1, B110 },
#endif
#ifdef B134
		{ 1, B134 },
#endif
#ifdef B150
		{ 1, B150 },
#endif
#ifdef B200
		{ 1, B200 },
#endif
#ifdef B300
		{ 2, B300 },
#endif
#ifdef B600
		{ 4, B600 },
#endif
#ifdef B900
		{ 6, B900 },
#endif
#ifdef B1200
		{ 8, B1200 },
#endif
#ifdef B1800
		{ 16, B1800 },
#endif
#ifdef B2400
		{ 32, B2400 },
#endif
#ifdef B3600
		{ 64, B3600 },
#endif
#ifdef B4800
		{ 128, B4800 },
#endif
#ifdef B7200
		{ 256, B7200 },
#endif
#ifdef B9600
		{ 256, B9600 },
#endif
#ifdef EXTA
		{ 512, EXTA },
#endif
#ifdef B19200
		{ 512, B19200 },
#endif
#ifdef EXTB
		{ 1024, EXTB },
#endif
#ifdef B38400
		{ 1024, B38400 },
#endif
#ifdef EXT
		{ 1024, EXT }
#endif
};
	int i;
	for (i = 0; ; i++) {
		if (i == elemsof(speeds)) {
			speed_chars = 512;
			ospeed = B9600;	/* XXX */
			break;
		}
		if (speeds[i].brate == (unsigned short) ospeed) {
			speed_chars = speeds[i].bsize;
			break;
		}
	}

	flushscreen();		/* flush the one character buffer */
	ScrBufSize = jmin(MAXTTYBUF, speed_chars * jmax(LI / 24, 1));
#ifndef NO_JSTDOUT
	jstdout = fd_open("/dev/tty", F_WRITE|F_LOCKED, 1, (char *)NULL, ScrBufSize);
#endif
}

void
ttsize(void)
{
	/* ??? We really ought to wait until the screen is big enough:
	 * at least three lines high (one line each for buffer, mode,
	 * and message) and at least twelve columns wide (eight for
	 * line number, one for content, two for overflow indicators,
	 * and one blank at end).
	 */
#ifdef TIOCGWINSZ
	struct winsize win;

	if (ioctl(0, TIOCGWINSZ, &win) == 0
	    && win.ws_col >= 12
	    && win.ws_row >= 3)
	{
		CO = jmin(win.ws_col, MAXCOLS);
		LI = win.ws_row;
	}
#else /* !TIOCGWINSZ */
# ifdef BTL_BLIT
	struct jwinsize jwin;

	if (ioctl(0, JWINSIZE, &jwin) == 0
	    && jwin.bytesx >= 12
	    && jwin.bytesy >= 3)
	{
		CO = jmin(jwin.bytesx, MAXCOLS);
		LI = jwin.bytesy;
	}
# endif /* BTL_BLIT */
#endif /* !TIOCGWINSZ */
	ILI = LI - 1;
}

#endif /* UNIX */
