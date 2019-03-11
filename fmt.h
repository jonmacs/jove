/***************************************************************************
 * This program is Copyright (C) 1986, 1987, 1988 by Jonathan Payne.  JOVE *
 * is provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is    *
 * included in all the files.                                              *
 ***************************************************************************/

#define MESG_SIZE 128
extern char	mesgbuf[MESG_SIZE];

extern void
	format proto((char *buf, size_t len, const char *fmt, va_list ap)),
	add_mess proto((const char *, ...)),
	f_mess proto((const char *, ...)),
	fwritef proto((struct FileStruct *, const char *, ...)),
	writef proto((const char *, ...)),
	s_mess proto((const char *, ...)),
	swritef proto((char *, size_t, const char *, ...));

extern char
	*sprint proto((const char *, ...));
