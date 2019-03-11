/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

#define MESG_SIZE MAXCOLS
extern char	mesgbuf[MESG_SIZE];

#define PPWIDTH	5	/* maximum width of PPchar output: "\\000\0" */

extern void
	format proto((char *buf, size_t len, const char *fmt, va_list ap)),
	PPchar proto((ZXchar c, char *cp)),
	add_mess proto((const char *, ...)),
	f_mess proto((const char *, ...)),
	fwritef proto((File *, const char *, ...)),
	writef proto((const char *, ...)),
	s_mess proto((const char *, ...)),
	swritef proto((char *, size_t, const char *, ...));

extern char
	*sprint proto((const char *, ...));
