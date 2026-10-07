/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

extern void
	b_char(long n),
	char_motion(long n, jbool whole),
	f_char(long n),
	f_word(long num),
	line_move(int dir, long n, jbool line_cmd);

extern int
	how_far(LinePtr line,int col),
	next_char(const char *line, int i),
	prev_char(const char *line, int i);

/* Commands: */

extern void
	BackChar(void),
	BackWord(void),
	Bof(void),
	Bol(void),
	Bos(void),
	Eof(void),
	Eol(void),
	Eos(void),
	ForChar(void),
	ForWord(void),
	ForIdent(void),
	NextLine(void),
	PrevLine(void);
