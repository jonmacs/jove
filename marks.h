/***************************************************************************
 * This program is Copyright (C) 1986, 1987, 1988 by Jonathan Payne.  JOVE *
 * is provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is    *
 * included in all the files.                                              *
 ***************************************************************************/

extern void
	AllMarkSet proto((struct buffer *b,struct line *line,int col)),
	DFixMarks proto((struct line *line1,int char1,struct line *line2,int char2)),
	DelMark proto((struct mark *m)),
	IFixMarks proto((struct line *line1, int char1, struct line *line2, int char2)),
	MarkSet proto((struct mark *m, struct line *line, int column)),
	ToMark proto((struct mark *m)),
	flush_marks proto((Buffer *)),
	do_set_mark proto((struct line *l, int c)),
	set_mark proto((void));

extern Mark
	*CurMark proto((void)),
	*MakeMark proto((struct line *line,int column,int type));

/* Commands: */

extern void
	PopMark proto((void)),
	PtToMark proto((void)),
	SetMark proto((void));

/* Variables: */

extern bool	MarksShouldFloat;	/* adjust marks on insertion/deletion */
