/***************************************************************************
 * This program is Copyright (C) 1986, 1987, 1988 by Jonathan Payne.  JOVE *
 * is provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is    *
 * included in all the files.                                              *
 ***************************************************************************/

extern bool
	Asking,		/* are we on read a string from the terminal? */
	InRealAsk;	/* are we currently executing real_ask()? */

extern int
	AskingWidth;	/* width of question being asked */

extern char
	Minibuf[LBSIZE];

extern char
	*ask_file proto((const char *prmt, char *def, char *buf)),
	*ask proto((char *, char *, ...)),
	*do_ask proto((char *, bool (*) proto((int)), char *, const char *, ...));

extern bool	yes_or_no_p proto((char *, ...));

extern void	minib_add proto((char *str, bool movedown));

/* Variables: */

extern int	AbortChar;		/* cancels command input */
#ifdef	F_COMPLETION
extern char	BadExtensions[128];	/* extensions (e.g., ".o" to ignore) */
extern bool	DispBadFs;		/* display filenames with bad extensions? */
#endif
extern bool	DoEVexpand;		/* treat $foo as environment variable */
