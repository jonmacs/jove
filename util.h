/***************************************************************************
 * This program is Copyright (C) 1986, 1987, 1988 by Jonathan Payne.  JOVE *
 * is provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is    *
 * included in all the files.                                              *
 ***************************************************************************/

/* some utility functions, as macros, to be included by jove.h */

extern char
	key_strokes[100],
	*keys_p;

extern int	ModCount;

#define init_strokes()	{ keys_p = key_strokes; *keys_p = '\0'; }
#define add_stroke(c)	{ \
	if (keys_p < &key_strokes[sizeof (key_strokes) - 1]) { \
		*keys_p++ = (c); \
		*keys_p = '\0'; \
	} \
}

#define IsModified(b)	((b)->b_modified)
#define SavLine(a, b)	((a)->l_dline = putline((b)))
#define SetLine(line)	DotTo((line), 0)
#define bobp()		(firstp(curline) && bolp())
#define bolp()		(curchar == 0)
#define eobp()		(lastp(curline) && eolp())
#define eolp()		(linebuf[curchar] == '\0')
#define firstp(line)	((line) == curbuf->b_first)
#define getDOT()	jgetline(curline->l_dline, linebuf)
#define lastp(line)	((line) == curbuf->b_last)

extern UnivPtr
	emalloc proto((size_t size)),
	erealloc proto((UnivPtr ptr, size_t size));

extern char
	*IOerr proto((char *err, char *file)),
	*StrIndex proto((int dir,char *buf,int charpos, int what)),
	*basename proto((char *f)),
	*copystr proto((char *str)),
	*filename proto((struct buffer *b)),
	*get_time proto((time_t *timep,char *buf,int from,int to)),
	*itoa proto((int num)),
	*lcontents proto((struct line *line)),
	*ltobuf proto((struct line *line,char *buf));

extern int
	LineDist proto((struct line *nextp,struct line *endp)),
	TwoBlank proto((void)),
	casecmp proto((const char *s1, const char *s2)),
	casencmp proto((const char *s1, const char *s2, size_t n)),
	inorder proto((struct line *nextp,int char1,struct line *endp,int char2)),
	length proto((struct line *line)),
	max proto((int a,int b)),
	min proto((int a,int b)),
	numcomp proto((char *s1,char *s2)),
	pnt_line proto((void));

extern bool
	blnkp proto((char *buf)),
	within_indent proto((void)),
	fixorder proto((struct line * *line1,int *char1,struct line * *line2,int *char2)),
	inlist proto((struct line *first,struct line *what)),
	sindex proto((char *pattern,char *string)),
	ModBufs proto((bool allp));

extern void
	DOTsave proto((struct position *buf)),
	DotTo proto((struct line *line,int col)),
	PushPntp proto((struct line *line)),
	SetDot proto((struct position *bp)),
	ToFirst proto((void)),
	ToLast proto((void)),
	ins_c proto((int c,char *buf,int atchar,int num,int max)),
	len_error proto((int flag)),
	linecopy proto((char *onto,int atchar,char *from)),
	make_argv proto((char * *argv,va_list ap)),
	modify proto((void)),
	null_ncpy proto((char *to, const char *from, size_t n)),
#ifdef	UNIX
	dopipe proto((int *p)),
	pipeclose proto((int *p)),
#endif
	pop_env proto((jmp_buf)),
	push_env proto((jmp_buf)),
	to_word proto((int dir)),
	unmodify proto((void));

extern UnivPtr
	freealloc proto((UnivPtr obj, size_t size));

/* Variables: */

extern int	MarkThresh;		/* moves greater than MarkThresh will SetMark */

/* externs.h:
 *	strerror (ifdef NO_STRERROR)
 */
