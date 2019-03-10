/*************************************************************************
 * This program is copyright (C) 1985, 1986 by Jonathan Payne.  It is    *
 * provided to you without charge for use only on a licensed Unix        *
 * system.  You may copy JOVE provided that this notice is included with *
 * the copy.  You may not sell copies of this program or versions        *
 * modified for use on microcomputer systems, unless the copies are      *
 * included with a Unix system distribution and the source is provided.  *
 *************************************************************************/

/* Termcap definitions */

extern char
	*UP,	/* Scroll reverse, or up */
	*CS,	/* If on vt100 */
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
	*M_IC,	/* Insert char with arg */
	*M_DC,	/* Delete char with arg */
	*M_AL,	/* Insert line with arg */
	*M_DL,	/* Delete line with arg */
	*SF,	/* Scroll forward */
	*SR,	/* Scroll reverse */
	*SP,	/* Send cursor position */
#ifdef LSRHS
	*RS,	/* Reverse video start. */
	*RE,	/* Reverse video end. */
#endif
	*VB;	/* Visible bell */

extern int
	LI,		/* Number of lines. */
	ILI,		/* Number of internal lines. */
	CO,		/* Number of columns. */

	UL,		/* Underscores don't replace chars already on screen */
	MI,		/* Okay to move while in insert mode */
	SG,		/* Number of magic cookies left by SO and SE */

	TABS,		/* Whether we are in tabs mode */
	UPlen,		/* Length of the UP string */
	HOlen,		/* Length of Home string */
	LLlen;		/* Length of lower string */

extern char
	PC,
	*BC;	/* Back space */

extern int ospeed;
