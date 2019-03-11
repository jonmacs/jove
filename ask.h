/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

extern bool
	Asking,		/* are we on read a string from the terminal? */
	InRealAsk;	/* are we currently executing real_ask()? */

extern int
	AskingWidth;	/* width of question being asked */

extern char
	Minibuf[LBSIZE];

#define NULL_ASK_EXT	((bool (*) ptrproto((ZXchar))) NULL)

/* ask for file or directory (only different under MSFILESYSTEM) */
#ifdef MSFILESYSTEM
extern char
	*ask_file proto((const char *prmt, char *def, char *buf)),
	*ask_dir proto((const char *prmt, char *def, char *buf));
#else
extern char
	*ask_ford proto((const char *prmt, char *def, char *buf));
# define ask_file(prmt, def, buf)	ask_ford(prmt, def, buf)
# define ask_dir(prmt, def, buf)	ask_ford(prmt, def, buf)
#endif

extern const char
	*ask proto((const char *, const char *, ...)),
	*do_ask proto((const char *, bool (*) ptrproto((ZXchar)), const char *, const char *, ...));

extern bool	yes_or_no_p proto((const char *, ...));

extern void	minib_add proto((char *str, bool movedown));

/* Variables: */

extern ZXchar	AbortChar;	/* VAR: cancels command input */
#ifdef F_COMPLETION
extern bool	DispBadFs;		/* VAR: display filenames with bad extensions? */
extern bool	DispDefFs;		/* VAR: display default filenames in prompt? */
extern char	BadExtensions[128];	/* VAR: extensions (e.g. ".o") to ignore */
# ifndef MAC	/* no environment in MacOS */
extern bool	DoEVexpand;		/* VAR: should we expand evironment variables? */
# endif
#endif
extern bool	OneKeyConfirmation;		/* VAR: single y or n keystroke sufficient? */
