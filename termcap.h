/************************************************************************
 * This program is Copyright (C) 1986-1994 by Jonathan Payne.  JOVE is  *
 * provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is *
 * included in all the files.                                           *
 ************************************************************************/

#define MAXCOLS		256	/* maximum number of columns */

#ifdef TERMCAP

/* termcap definitions */

extern char
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
	*SF,	/* Scroll forward (defaults to \n) */
	*SR,	/* Scroll reverse */
	*VB,	/* visible bell */
	*BL,	/* audible bell (defaults to BEL) */
	*lPC,	/* pad character (as a string!) */
	*NL,	/* newline character (defaults to \n) */
	*DO,	/* down one line (defaults to \n capability) */
	*M_AL,	/* Insert line with arg */
	*M_DL,	/* Delete line with arg */
	*M_SF,	/* Scroll forward with arg */
	*M_SR;	/* Scroll back with arg */

# define	INFINITY	1000	/* cost too high to afford */

extern int
	UPlen,		/* length of the UP string */
	HOlen,		/* length of Home string */
	LLlen,		/* length of last line string */

	phystab,	/* ("it") terminal's tabstop settings */
	UG;		/* number of magic cookies left by US and UE */

extern bool
	Hazeltine,		/* Hazeltine tilde kludge */
	UL,		/* underscores don't replace chars already on screen */
	NP;		/* there is No Pad character */

extern char
	PC,		/* pad character, as a char (set from lPC; defaults to NUL) */
	*BC,	/* back space (defaults to BS) */
	*UP;	/* Scroll reverse, or up */

extern short	ospeed;

# ifdef ID_CHAR

extern char
	*IC,	/* Insert char */
	*DC,	/* Delete char */
	*IM,	/* Insert mode */
	*EI,	/* End insert mode */
	*IP,	/* insert pad after character inserted */
	*M_IC,	/* Insert char with arg */
	*M_DC;	/* Delete char with arg */

extern bool	UseIC;	/* VAR: whether or not to use i/d char processesing */

extern int
	IMlen,	/* length of insert mode */
	EIlen,	/* length of end insert mode string */
	IClen,	/* length of insert char */
	DClen,	/* length of delete char */
	MIClen,	/* length of insert char with arg */
	MDClen,	/* length of delete char with arg */
	CElen;	/* length of clear to end of line */

extern bool
	MI;		/* okay to move while in insert mode */

# endif /* ID_CHAR */

#endif /* TERMCAP */

extern int
	SG,		/* number of magic cookies left by SO and SE */
	LI,		/* number of lines */
	ILI,		/* number of internal lines */
	CO;		/* number of columns (CO <= MAXCOLS) */

extern bool
	TABS;		/* terminal supports tabs */
