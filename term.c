/************************************************************************
 * This program is Copyright (C) 1986 by Jonathan Payne.  JOVE is       *
 * provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is *
 * included in all the files.                                           *
 ************************************************************************/

#include "jove.h"
#include <ctype.h>
#include <errno.h>
#ifndef MSDOS
#ifdef SYSV
#   include <termio.h>
#else
#   include <sgtty.h>
#endif /* SYSV */
#endif /* MSDOS */

#ifdef IPROCS
#   include <signal.h>
#endif

#define _TERM
#include "termcap.h"

/* Termcap definitions */

#ifndef IBMPC
char	*CS,
	*SO,
	*SE,
	*CM,
	*CL,
	*CE,
	*HO,
	*AL,
	*DL,
	*VS,
	*VE,
	*KS,
	*KE,
	*TI,
	*TE,
	*IC,
	*DC,
	*IM,
	*EI,
	*LL,
	*M_IC,	/* Insert char with arg */
	*M_DC,	/* Delete char with arg */
	*M_AL,	/* Insert line with arg */
	*M_DL,	/* Delete line with arg */
	*SF,	/* Scroll forward */
	*SR,
	*SP,	/* Send Cursor Position */
	*VB,
	*BL,
	*IP,	/* insert pad after character inserted */
	*lPC,
	*NL;
#endif

int	LI,
	ILI,	/* Internal lines, i.e., 23 of LI is 24. */
	CO,

	UL,
	MI,
	SG,	/* number of magic cookies left by SO and SE */
	XS,	/* whether standout is braindamaged */

	TABS,
	UPlen,
	HOlen,
	LLlen;

extern char	PC,
		*BC,
		*UP;

#ifdef notdef
	/*
	 * Are you sure about this one Jon?  On the SYSV system I tried this
	 * on I got a multiple definition of PC because it was already
	 * defined in -ltermcap.  Similarly for BC and UP ...
	 */
#ifdef SYSVR2 /* release 2, at least */
char	PC;
#else
extern char	PC;
#endif /* SYSVR2 */
#endif

#ifndef IBMPC
static char	tspace[256];

/* The ordering of ts and meas must agree !! */
static char	*ts="vsvealdlspcssosecmclcehoupbcicimdceillsfsrvbksketiteALDLICDCpcipblnl";
static char	**meas[] = {
	&VS, &VE, &AL, &DL, &SP, &CS, &SO, &SE,
	&CM, &CL, &CE, &HO, &UP, &BC, &IC, &IM,
	&DC, &EI, &LL, &SF, &SR, &VB, &KS, &KE,
	&TI, &TE, &M_AL, &M_DL, &M_IC, &M_DC,
	&lPC, &IP, &BL, &NL, 0
};

static
gets(buf)
char	*buf;
{
	buf[read(0, buf, 12) - 1] = 0;
}	

/* VARARGS1 */

static
TermError(fmt, a)
char	*fmt;
{
	printf(fmt, a);
	_exit(1);
}

getTERM()
{
	char	*getenv(), *tgetstr() ;
	char	termbuf[13],
		*termname = 0,
		*termp = tspace,
		tbuff[2048];	/* Good grief! */
	int	i;

	termname = getenv("TERM");
	if (termname == 0) {
		putstr("Enter terminal name: ");
		gets(termbuf);
		if (termbuf[0] == 0)
			TermError(NullStr);

		termname = termbuf;
	}

	if (tgetent(tbuff, termname) < 1)
		TermError("[\"%s\" unknown terminal type?]", termname);

	if ((CO = tgetnum("co")) == -1)
		TermError("columns?");
	else if (CO > MAXCOLS)
		CO = MAXCOLS;

	if ((LI = tgetnum("li")) == -1)
		TermError("lines?");

	if ((SG = tgetnum("sg")) == -1)
		SG = 0;			/* Used for mode line only */

	if ((XS = tgetflag("xs")) == -1)
		XS = 0;			/* Used for mode line only */

	for (i = 0; meas[i]; i++) {
		*(meas[i]) = (char *) tgetstr(ts, &termp);
		ts += 2;
	}
	if (lPC)
		PC = *lPC;
	if (XS)
		SO = SE = 0;

	if (CS && !SR)
		CS = SR = SF = 0;

	if (CS && !SF)
		SF = "\n";

	if (IM && (*IM == 0))
		IM = 0;
	else
		MI = tgetflag("mi");

	UL = tgetflag("ul");

	if (NL == 0)
		NL = "\n";
	else {			/* strip stupid padding information */
		while (isdigit(*NL))
			NL += 1;
		if (*NL == '*')
			NL += 1;
	}

	if (BL == 0)
		BL = "\007";

#ifdef ID_CHAR
	disp_opt_init();
#endif
	if (CanScroll = ((AL && DL) || CS))
		IDline_setup(termname);
}

#else

InitCM()
{
}

int EGA;

getTERM()
{
	char	*getenv(), *tgetstr() ;
	char	*termname;
    void	init_43(), init_term();
	unsigned char lpp(), chpl();

	if (getenv("EGA") || (!stricmp(getenv("TERM"), "EGA"))) {
	   termname = "ega";
	   init_43();
	   EGA = 1;
	}
	else {
	   termname = "ibmpc";
	   init_term();
	   EGA = 0;
	}

	CO = chpl();
	LI = lpp();

	SG = 0;			/* Used for mode line only */
	XS = 0;			/* Used for mode line only */

	CanScroll = 1;
}

#endif /* IBMPC */

