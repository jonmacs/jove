/***************************************************************************
 * This program is Copyright (C) 1986, 1987, 1988 by Jonathan Payne.  JOVE *
 * is provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is    *
 * included in all the files.                                              *
 ***************************************************************************/

#ifdef	IBMPC

typedef unsigned char	BYTE;
typedef unsigned int	WORD;

extern unsigned char	CHPL;

extern void
	write_em proto((char *s)),
	write_emc proto((char *s, int n)),
	write_emif proto((char *s));

extern void
	pcUnsetTerm proto((void)),
	pcResetTerm proto((void));

extern void
	/*near*/ normfun proto((char)),
	/*near*/ scr_win proto((int, BYTE, BYTE, BYTE, BYTE)),
	/*near*/ clr_page(),
	/*near*/ clr_eoln();

extern unsigned char
	chpl proto((void)),
	lpp proto((void));

/* Variables: */

extern int
	Fgcolor,
	Bgcolor,
	Mdcolor;

#endif	/* IBMPC */
