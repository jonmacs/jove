/***************************************************************************
 * This program is Copyright (C) 1986, 1987, 1988 by Jonathan Payne.  JOVE *
 * is provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is    *
 * included in all the files.                                              *
 ***************************************************************************/

#include "jove.h"
#include "fp.h"
#include "disp.h"
#include "ctype.h"
#include "fmt.h"
#include "term.h"
#include <errno.h>

#ifndef	MAC	/* most of the file... */

#ifdef TERMIO
# include <termio.h>
#endif
#ifdef TERMIOS
# include <termios.h>
#endif
#ifdef SGTTY
# include <sgtty.h>
#endif	/* SGTTY */

#ifdef	IPROCS
# include <signal.h>
#endif

/* Termcap definitions */

#include "termcap.h"

extern int	UNMACRO(tgetent) proto((char */*buf*/, const char */*name*/));
extern int	UNMACRO(tgetflag) proto((const char */*id*/));
extern int	UNMACRO(tgetnum) proto((const char */*id*/));
extern char	*UNMACRO(tgetstr) proto((const char */*id*/, char **/*area*/));
extern void	UNMACRO(tputs) proto((const char *, int, void (*) proto((int))));

#ifndef	IBMPC
char
	*CS,	/* change scrolling region */
	*SO,	/* Start standout */
	*SE,	/* End standout */
	*CM,	/* The cursor motion string */
	*CL,	/* Clear screen */
	*CE,	/* Clear to end of line */
	*HO,	/* Home cursor */
	*AL,	/* Addline (insert line) */
	*DL,	/* Delete line */
	*VS,	/* Visual start */
	*VE,	/* Visual end */
	*KS,	/* Keypad mode start */
	*KE,	/* Keypad mode end */
	*TI,	/* Cursor addressing start */
	*TE,	/* Cursor addressing end */
	*IC,	/* Insert char */
	*DC,	/* Delete char */
	*IM,	/* Insert mode */
	*EI,	/* End insert mode */
	*LL,	/* Last line, first column */
	*SF,	/* Scroll forward (defaults to \n) */
	*SR,	/* Scroll reverse */
	*SP,	/* Send cursor position */
	*VB,	/* visible bell */
	*BL,	/* audible bell (defaults to BEL) */
	*IP,	/* insert pad after character inserted */
	*lPC,	/* pad character (as a string!) */
	*NL,	/* newline character (defaults to \n) */
	*DO,	/* down one line (defaults to NL capability) */
	*M_IC,	/* Insert char with arg */
	*M_DC,	/* Delete char with arg */
	*M_AL,	/* Insert line with arg */
	*M_DL,	/* Delete line with arg */
	*M_SF,	/* Scroll forward with arg */
	*M_SR;	/* Scroll back with arg */
#endif

int
	LI,		/* number of lines */
	ILI,		/* number of internal lines */
	CO,		/* number of columns */

	SG,		/* number of magic cookies left by SO and SE */
	UPlen,		/* length of the UP string */
	HOlen,		/* length of Home string */
	LLlen;		/* length of lower string */

bool
	Hazeltine,		/* Hazeltine tilde kludge */
	MI,		/* okay to move while in insert mode */
	UL,		/* underscores don't replace chars already on screen */
	NP,		/* there is No Pad character */
	TABS;		/* whether we are in tabs mode */

#ifdef	DEFINE_PC_BC_UP_OSPEED
	/* This is needed for HP-UX, possibly for other SYSVR2 systems */
char
	PC,		/* pad character, as a char (set from lPC; defaults to NUL) */
	*BC,	/* back space (defaults to BS) */
	*UP;	/* Scroll reverse, or up */

short	ospeed;
#endif /* HPUX */

#ifndef	IBMPC

bool	CanScroll = NO;

private char	tspace[256];

/* The ordering of ts and meas must agree !! */
private const char	ts[] =
	"vsvealdlspcssosecmclcehoupbcicimdceillsfsrvbksketitepcipblnldoALDLICDCSFSR";
private char	**const meas[] = {
	&VS, &VE, &AL, &DL, &SP, &CS, &SO, &SE,
	&CM, &CL, &CE, &HO, &UP, &BC, &IC, &IM,
	&DC, &EI, &LL, &SF, &SR, &VB, &KS, &KE,
	&TI, &TE, &lPC, &IP, &BL, &NL, &DO,
	&M_AL, &M_DL, &M_IC, &M_DC, &M_SF, &M_SR,
	NULL
};

private void
TermError()
{
	flushscreen();
	_exit(1);
}

void
getTERM()
{
	char	termbuf[13],
		*termname = NULL,
		*termp = tspace,
		tbuff[2048];	/* Good grief! */
	const char	*tsp = ts;
	int	i;

	termname = getenv("TERM");
	if (termname == NULL || *termname == '\0'
	|| strcmp(termname, "dumb") == 0
	|| strcmp(termname, "unknown") == 0
	|| strcmp(termname, "network") == 0)
	{
		putstr("Enter terminal type (e.g, vt100): ");
		flushscreen();
		termbuf[read(0, (UnivPtr) termbuf, sizeof(termbuf)) - 1] = '\0';
		if (termbuf[0] == '\0')
			TermError();

		termname = termbuf;
	}

	if (tgetent(tbuff, termname) < 1) {
		writef("[\"%s\" unknown terminal type?]", termname);
		TermError();
	}

	/* get numeric capabilities */

	if ((CO = tgetnum("co")) == -1) {
wimperr:
		writef("You can't run JOVE on a %s terminal.\n", termname);
		TermError();
		/*NOTREACHED*/
	}
	if (CO > MAXCOLS)
		CO = MAXCOLS;

	if ((LI = tgetnum("li")) == -1)
		goto wimperr;

	if ((SG = tgetnum("sg")) == -1)
		SG = 0;			/* Used for mode line only */

	/* get string capabilities */

	for (i = 0; meas[i]; i++) {
		static char	nm[3] = "xx";

		nm[0] = *tsp++;
		nm[1] = *tsp++;
		*(meas[i]) = (char *) tgetstr(nm, &termp);
		if (termp > tspace + sizeof(tspace))
			goto wimperr;
	}
	if (lPC)
		PC = *lPC;	/* convert lPC string attribute to char PC */

	/* get boolean capabilities */

	Hazeltine = tgetflag("hz")==YES;	/* Hazeltine tilde kludge */
	NP = tgetflag("NP")==YES;	/* there is No Pad character */
	MI = tgetflag("mi")==YES;	/* okay to move while in insert mode */
	UL = tgetflag("ul")==YES;	/* underscores don't replace chars already on screen */

	/* adjust capabilities */

	if (tgetflag("km") == YES)		/* has meta-key */
		MetaKey = YES;

	if (tgetflag("xs") == YES)
		SO = SE = NULL;	/* don't use braindamaged standout mode */

	if (CS && !(SR || M_SR))
		CS = NULL;	/* don't use scrolling region without way of scrolling */

	if (CS && !SF)
		SF = "\n";	/* default SF */

	if (IM && (*IM == '\0'))
		IM = NULL;	/* If IM is empty, supress.  ??? why would this happen? */

	if (NL == NULL)
		NL = "\n";	/* default NL to NL (naturally!) */
	/* strip stupid padding information */
	while (jisdigit(*NL))
		NL += 1;
	if (*NL == '*')
		NL += 1;

	if (!DO)
		DO = NL;	/* default down one line to NL */

	if (BL == NULL)
		BL = "\007";	/* default bell to BEL */

	if (BC == NULL)
		BC = "\b";	/* default back space to BS */

#ifdef	ID_CHAR
	disp_opt_init();
#endif
	CanScroll = (AL != NULL && DL != NULL) || CS != NULL;
}

#else	/* IBMPC */

#include "pcscr.h"

extern char
	*getenv(),
	*tgetstr();

extern void
	init_43(),
	init_term();

bool	CanScroll = YES;

void
InitCM()
{
}

bool EGA;

void
getTERM()
{
	char	*termname;

	if (getenv("EGA") != NULL || (!stricmp(getenv("TERM"), "EGA"))) {
	   termname = "ega";
	   init_43();
	   EGA = YES;
	} else {
	   termname = "ibmpc";
	   init_term();
	   EGA = NO;
	}

	CO = chpl();
	LI = lpp();
	SG = 0;			/* Used for mode line only */
}

#endif	/* IBMPC */

#else	/* MAC */
int	LI,
	ILI,	/* Internal lines, i.e., 23 of LI is 24. */
	CO,
	SG;

bool	CanScroll = YES;

bool	TABS;

void getTERM()
{
	SG = 0;
}

#endif	/* MAC */

/* put a string with padding */

#ifndef	IBMPC
private void
tputc(c)
int	c;
{
	jputchar(c);
}
#endif	/* IBMPC */

#ifndef	MAC
void
putpad(str, lines)
char	*str;
int	lines;
{
#ifndef	IBMPC
	if (str)
		tputs(str, lines, tputc);
#else	/* IBMPC */
	write_emif(str);
#endif	/* IBMPC */
}

/* Put multi-unit or multiple single-unit strings, as appropriate. */

void
putmulti(ss, ms, num, lines)
char
	*ss,	/* single line */
	*ms;	/* multiline */
int
	num,	/* number of iterations */
	lines;	/* lines affected (for padding) */
{
	if (ms && (num > 1 || !ss)) {
		/* use the multi string */
#ifndef	IBMPC
		tputs(targ1(ms, num), lines, tputc);
#else	/* IBMPC */
		/* This code is only a guess: I don't know if any M_* termcap
		 * attributes are defined for the PC.  If they are not used,
		 * this routine is not called.  Perhaps this routine should
		 * simply abort.
		 */
		char	buf[16];	/* hope that this is long enough */

		swritef(buf, sizeof(buf), ms, num);	/* hope only %d appears in ms */
		write_em(buf);
#endif	/* IBMPC */
	} else {
		/* repeatedly use single string */
		while (num--)
			putpad(ss, lines);
	}
}

#endif	/* !MAC */

/* Determine the number of characters to buffer at each baud rate.  The
   lower the number, the quicker the response when new input arrives.  Of
   course the lower the number, the more prone the program is to stop in
   output.  Decide what matters most to you. This sets BufSize to the right
   number or chars, and initializes `stdout'.  */

void
settout(ttbuf)
char	*ttbuf;
{
#ifdef	UNIX
	int	speed_chars;

	static const struct {
		unsigned int bsize;
		unsigned int brate;
	} speeds[] = {

# ifdef B0
		{ 1, B0 },
# endif
# ifdef B50
		{ 1, B50 },
# endif
# ifdef B75
		{ 1, B75 },
# endif
# ifdef B110
		{ 1, B110 },
# endif
# ifdef B134
		{ 1, B134 },
# endif
# ifdef B150
		{ 1, B150 },
# endif
# ifdef B200
		{ 1, B200 },
# endif
# ifdef B300
		{ 2, B300 },
# endif
# ifdef B600
		{ 4, B600 },
# endif
# ifdef B900
		{ 6, B900 },
# endif
# ifdef B1200
		{ 8, B1200 },
# endif
# ifdef B1800
		{ 16, B1800 },
# endif
# ifdef B2400
		{ 32, B2400 },
# endif
# ifdef B3600
		{ 64, B3600 },
# endif
# ifdef B4800
		{ 128, B4800 },
# endif
# ifdef B7200
		{ 256, B7200 },
# endif
# ifdef B9600
		{ 256, B9600 },
# endif
# ifdef EXTA
		{ 512, EXTA },
# endif
# ifdef B19200
		{ 512, B19200 },
# endif
# ifdef EXTB
		{ 1024, EXTB },
# endif
# ifdef B38400
		{ 1024, B38400 },
# endif
# ifdef EXT
		{ 1024, EXT }
# endif
};
	int i;
	for (i = 0; ; i++) {
		if (i == sizeof(speeds) / sizeof(speeds[0])) {
			speed_chars = 512;
			ospeed = B9600;	/* XXX */
			break;
		}
		if (speeds[i].brate == (unsigned short) ospeed) {
			speed_chars = speeds[i].bsize;
			break;
		}
	}
			
#else
	int	speed_chars = 256;
#endif

	flushscreen();		/* flush the one character buffer */
	BufSize = min(MAXTTYBUF, speed_chars * max(LI / 24, 1));
	stdout = fd_open("/dev/tty", F_WRITE|F_LOCKED, 1, ttbuf, BufSize);
}
