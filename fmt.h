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
	format(char *buf, size_t len, const char *fmt, va_list ap),
	PPchar(ZXchar c, char *cp),
	add_mess(const char *, ...),
	f_mess(const char *, ...),
	fwritef(File *, const char *, ...),
	writef(const char *, ...),
	s_mess(const char *, ...),
	swritef(char *, size_t, const char *, ...);

extern char
	*sprint(const char *, ...);

extern jbool jdebug;
extern const char *jdpath;
extern void jdprintf(const char *, ...);

#define jdbg if (!jdebug) ; else jdprintf
