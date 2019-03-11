/************************************************************************
 * This program is Copyright (C) 1986-1994 by Jonathan Payne.  JOVE is  *
 * provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is *
 * included in all the files.                                           *
 ************************************************************************/

#include "jove.h"
#include "fp.h"
#include "disp.h"
#include "jctype.h"
#include "fmt.h"
#include "term.h"

#ifndef MAC

# ifdef TERMIO
#  include <termio.h>
# endif
# ifdef TERMIOS
#  include <termios.h>
# endif
# ifdef SGTTY
#  include <sgtty.h>
# endif /* SGTTY */

/* Termcap definitions */

# include "termcap.h"

# ifdef TERMCAP

extern int	UNMACRO(tgetent) proto((char */*buf*/, const char */*name*/));
extern int	UNMACRO(tgetflag) proto((const char */*id*/));
extern int	UNMACRO(tgetnum) proto((const char */*id*/));
extern char	*UNMACRO(tgetstr) proto((const char */*id*/, char **/*area*/));
extern void	UNMACRO(tputs) proto((const char *, int, void (*) proto((int))));

char
	*CS,	/* change scrolling region */
	*SO,	/* Start standout */
	*SE,	/* End standout */
	*US,	/* Start underlining */
	*UE,	/* End underlining */
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
	*LL,	/* Last line, first column */
	*SF = "\n",	/* Scroll forward (defaults to \n) */
	*SR,	/* Scroll reverse */
	*VB,	/* visible bell */
	*BL = "\007",	/* audible bell (defaults to BEL) */
	*lPC,	/* pad character (as a string!) */
	*NL = "\n",	/* newline character (defaults to \n) */
	*DO = "\n",	/* down one line (defaults to \n capability) */
	*M_AL,	/* Insert line with arg */
	*M_DL,	/* Delete line with arg */
	*M_SF,	/* Scroll forward with arg */
	*M_SR;	/* Scroll back with arg */

int
	UPlen = INFINITY,		/* length of the UP string */
	HOlen = INFINITY,		/* length of Home string */
	LLlen = INFINITY,		/* length of last line string */

	phystab = 8,	/* ("it") terminal's tabstop settings */
	UG;		/* number of magic cookies left by US and UE */

bool
	Hazeltine,	/* Hazeltine tilde kludge */
	UL,		/* underscores don't replace chars already on screen */
	NP;		/* there is No Pad character */

#  ifdef DEFINE_PC_BC_UP_OSPEED
	/* This is needed for HP-UX, possibly for other SYSVR2 systems */
char
	PC,		/* pad character, as a char (set from lPC; defaults to NUL) */
	*BC,	/* back space (defaults to BS) */
	*UP;	/* Scroll reverse, or up */

short	ospeed;
#  endif /* DEFINE_PC_BC_UP_OSPEED */

#  ifdef ID_CHAR

char
	*IC,	/* Insert char */
	*DC,	/* Delete char */
	*IM,	/* Insert mode */
	*EI,	/* End insert mode */
	*IP,	/* insert pad after character inserted */
	*M_IC,	/* Insert char with arg */
	*M_DC;	/* Delete char with arg */

bool	UseIC = NO;	/* VAR: whether or not to use i/d char processesing */

int
	IMlen = 0,	/* length of insert mode */
	EIlen = 0,	/* length of end insert mode string */
	IClen = 0,	/* length of insert char */
	DClen = 0,	/* length of delete char */
	MIClen = INFINITY,	/* length of insert char with arg */
	MDClen = INFINITY,	/* length of delete char with arg */
	CElen = 0;	/* length of clear to end of line */

bool
	MI;		/* okay to move while in insert mode */

#  endif /* ID_CHAR */

# endif /* TERMCAP */

int
	SG = 0,		/* number of magic cookies left by SO and SE */
	LI,		/* number of lines */
	ILI,		/* number of internal lines */
	CO;		/* number of columns (CO <= MAXCOLS) */

bool
	TABS;		/* terminal supports tabs */

# ifdef TERMCAP

bool	CanScroll = NO;

private char	tspace[256];

/* The ordering of ts and meas must agree !! */
private const char	ts[] =
#  ifdef ID_CHAR
/*                   |                   |                   | |             */
"vsvealdlcssoseusuecmclcehoupbcllsfsrvbksketitepcblnldoALDLSFSRicimdceiipICDC";
#  else
"vsvealdlcssoseusuecmclcehoupbcllsfsrvbksketitepcblnldoALDLSFSR";
#  endif

private char	**const meas[] = {
	&VS, &VE, &AL, &DL, &CS, &SO, &SE, &US, &UE, &CM,
	&CL, &CE, &HO, &UP, &BC, &LL, &SF, &SR, &VB, &KS,
	&KE, &TI, &TE, &lPC, &BL, &NL, &DO, &M_AL, &M_DL, &M_SF,
	&M_SR,
#  ifdef ID_CHAR
	&IC, &IM, &DC, &EI, &IP, &M_IC, &M_DC,
#  endif
	NULL
};

struct CapLen {
	char	**cap_var;
	int	*len_var;
};

private const struct CapLen	CapLenTab[] = {
	{ &HO,	&HOlen },
	{ &LL,	&LLlen },
	{ &UP,	&UPlen },
#  ifdef ID_CHAR
	{ &IM,	&IMlen },
	{ &EI,	&EIlen },
	{ &IC,	&IClen },
	{ &DC,	&DClen },
	{ &M_IC,	&MIClen },
	{ &M_DC,	&MDClen },
	{ &CE,	&CElen },
#  endif
	{ NULL,	NULL }
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

	if ((phystab = tgetnum("it")) == -1 || phystab <= 0)
		phystab = 8;

	if ((SG = tgetnum("sg")) == -1)
		SG = 0;			/* Used for mode line only */

	if ((UG = tgetnum("ug")) == -1)
		UG = 0;			/* We shan't bother if != 0 */

	/* get string capabilities */
	{
		const char	*tsp = ts;
		char	**const *measp;

		for (measp = meas; *measp != NULL; measp++) {
			static char	nm[3] = "xx";
			char	*val;

			nm[0] = *tsp++;
			nm[1] = *tsp++;
			val = tgetstr(nm, &termp);
			if (val != NULL)
				**measp = val;
			if (termp > tspace + sizeof(tspace))
				goto wimperr;
		}
	}
	if (lPC)
		PC = *lPC;	/* convert lPC string attribute to char PC */

	/* get boolean capabilities */

	Hazeltine = tgetflag("hz")==YES;	/* Hazeltine tilde kludge */
	NP = tgetflag("NP")==YES;	/* there is No Pad character */
	UL = tgetflag("ul")==YES;	/* underscores don't replace chars already on screen */

	/* adjust capabilities */

#if defined(USE_CTYPE) && !defined(NO_SETLOCALE)
	if (tgetflag("km") == YES && strcmp(LcCtype, "C") == 0)
					/* has meta-key and default locale*/
#else
	if (tgetflag("km") == YES)	/* has meta-key */
#endif
		MetaKey = YES;

	if (tgetflag("xs") == YES) {
		SO = SE = NULL;	/* don't use braindamaged standout mode */
		SG = 0;
	}

	if (SR == NULL && M_SR == NULL)
		CS = NULL;	/* don't use scrolling region without way of reverse scrolling */

	/* Note: the way termcap/terminfo is defined, we must use *both*
	 * IC and IM to insert, but normally only one will be defined.
	 * See terminfo(5), under the heading "Insert/Delete Character".
	 * Because of this, IM might be defined as a null string.
	 */
#  ifdef ID_CHAR
	if (IM != NULL && *IM == '\0')
		IM = NULL;	/* If IM is empty, supress. */

	UseIC = (IC != NULL || IM != NULL || M_IC != NULL);
	MI = tgetflag("mi")==YES;	/* okay to move while in insert mode */
#  endif /* ID_CHAR */

	/* strip stupid padding information */
	while ('0' <= *NL && *NL <= '9')
		NL += 1;
	if (*NL == '*')
		NL += 1;

	if (BC == NULL)
		BC = "\b";	/* default back space to BS */

	CanScroll = (AL != NULL && DL != NULL) || CS != NULL;

	/* calculate lengths */
	{
		static const struct CapLen	*p;

		for (p = CapLenTab; p->cap_var != NULL; p++)
			if (*p->cap_var != NULL)
				*p->len_var = strlen(*p->cap_var);
	}
}

/* Put multi-unit or multiple single-unit strings, as appropriate. */

private void
tputc(c)
char	c;
{
	scr_putchar(c);
}

void
putmulti(ss, ms, num, lines)
const char
	*ss,	/* single line */
	*ms;	/* multiline */
int
	num,	/* number of iterations */
	lines;	/* lines affected (for padding) */
{
	if (ms && (num > 1 || !ss)) {
		/* use the multi string */
		tputs(targ1(ms, num), lines, tputc);
	} else {
		/* repeatedly use single string */
		while (num--)
			putpad(ss, lines);
	}
}

/* put a string with padding */

void
putpad(str, lines)
const char	*str;
int	lines;
{
	if (str != NULL)
		tputs(str, lines, tputc);
}

# endif /* TERMCAP */

# ifdef IBMPC

#  include "msgetch.h"
#  include "pcscr.h"

bool	CanScroll = YES;

void
getTERM()
{
	/* Check if 101- or 102-key keyboard is installed.
	 * This test is apparently unreliable, so we allow override.
	 * Courtesy of Ralph Brown's interrupt list.
	 */
	char _far *kbd_stat_byte3 = (char _far *)0x00400096UL;
	enhanced_keybrd = (0x10 & *kbd_stat_byte3) != 0;

	pcSetTerm();
}

# endif /* IBMPC */

#else /* MAC */

int	LI,
	ILI,	/* Internal lines, i.e., 23 if LI is 24. */
	CO,
	SG = 0;

bool	CanScroll = YES;

bool	TABS;

void getTERM()
{
}

#endif /* MAC */

/* Determine the number of characters to buffer at each baud rate.  The
   lower the number, the quicker the response when new input arrives.  Of
   course the lower the number, the more prone the program is to stop in
   output.  Decide what matters most to you. This sets ScrBufSize to the right
   number or chars, and initializes `jstdout'.  */

void
settout()
{
#ifdef UNIX
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

#else /* !UNIX */
	int	speed_chars = 256;
#endif /* !UNIX */

	flushscreen();		/* flush the one character buffer */
	ScrBufSize = min(MAXTTYBUF, speed_chars * max(LI / 24, 1));
#ifndef IBMPC
	jstdout = fd_open("/dev/tty", F_WRITE|F_LOCKED, 1, (char *)NULL, ScrBufSize);
#endif
}
