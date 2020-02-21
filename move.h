/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

extern void
	b_char proto((long n)),
	f_char proto((long n)),
	f_word proto((long num)),
	line_move proto((int dir, long n, bool line_cmd));

extern int
	how_far proto((LinePtr line,int col));

/* Commands: */

extern void
	BackChar proto((void)),
	BackWord proto((void)),
	Bof proto((void)),
	Bol proto((void)),
	Bos proto((void)),
	Eof proto((void)),
	Eol proto((void)),
	Eos proto((void)),
	ForChar proto((void)),
	ForWord proto((void)),
	ForIdent proto((void)),
	NextLine proto((void)),
	PrevLine proto((void));
