/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

extern void
	AllMarkReset(Buffer *b,LinePtr line),
	DFixMarks(LinePtr line1,int char1,LinePtr line2,int char2),
	DelMark(Mark *m),
	IFixMarks(LinePtr line1, int char1, LinePtr line2, int char2),
	MarkSet(Mark *m, LinePtr line, int column),
	ToMark(Mark *m),
	flush_marks(Buffer *),
	do_set_mark(LinePtr l, int c),
	set_mark(void);

extern Mark
	*CurMark(void),
	*MakeMark(LinePtr line,int column);

/* Commands: */

extern void
	PopMark(void),
	ExchPtMark(void),
	SetMark(void);
