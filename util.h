/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

/* some utility functions, as macros, to be included by jove.h */

extern int
	fnamecomp proto((UnivConstPtr, UnivConstPtr));	/* order file names */

#define IsModified(b)	((b)->b_modified)
#define SavLine(a, b)	((a)->l_dline = jputline((b)))
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
	*IOerr proto((const char *err, const char *file)),
	*copystr proto((const char *str)),
	*get_time proto((time_t *timep,char *buf,int from,int to)),
	*lcontents proto((LinePtr line)),
	*ltobuf proto((LinePtr line,char *buf));

extern const char
	*jbasename proto((const char *f)),
	*filename proto((const Buffer *b));

extern int
	inorder proto((LinePtr nextp,int char1,LinePtr endp,int char2)),
	LineDist proto((LinePtr nextp,LinePtr endp)),
	LinesTo proto((LinePtr from, LinePtr to)),
	length proto((LinePtr line)),
	max proto((int a,int b)),
	min proto((int a,int b)),
	numcomp proto((const char *s1, const char *s2)),
	numcompcase proto((const char *s1, const char *s2));

extern ZXchar
	DecodePair proto((ZXchar first, ZXchar second));

extern bool
	caseeqn proto((const char *s1, const char *s2, size_t n)),
	TwoBlank proto((void)),
	blnkp proto((char *buf)),
	within_indent proto((void)),
	fixorder proto((LinePtr  *line1,int *char1,LinePtr  *line2,int *char2)),
	inlist proto((LinePtr first,LinePtr what)),
	sindex proto((const char *pattern, const char *string)),
	ModBufs proto((bool allp));

extern void
	DOTsave proto((Bufpos *buf)),
	DotTo proto((LinePtr line,int col)),
	PushPntp proto((LinePtr line)),
	SetDot proto((Bufpos *bp)),
	ToFirst proto((void)),
	ToLast proto((void)),
	ins_c proto((DAPchar c,char *buf,int atchar,int num,int max)),
	len_error proto((int flag)),
	linecopy proto((char *onto,int atchar,char *from)),
	modify proto((void)),
	diverge proto((Buffer *buf, bool d)),
	null_ncpy proto((char *to, const char *from, size_t n)),
#ifdef UNIX
	dopipe proto((int *p)),
	pipeclose proto((int *p)),
#endif
	pop_env proto((jmp_buf)),
	push_env proto((jmp_buf)),
	to_word proto((int dir)),
	unmodify proto((void));

extern UnivPtr
	freealloc proto((UnivPtr obj, size_t size));

typedef struct {
    const char **e_data;
    bool e_malloced;
    int e_headroom;	/* remaining room in e_data array */
} Env;

extern void
	jputenv proto((Env *, const char *)),
	junsetenv proto((Env *, const char *));
	

/* copy a string into a possibly-too-small buffer
 * trunc* silently truncates a too-large string.
 * jam* complain if the string is too large.
 */
#define truncstr(buf, str)	truncstrsub(buf, str, sizeof(buf))
#define jamstr(buf, str)	jamstrsub(buf, str, sizeof(buf))
extern void
	truncstrsub proto((char *buf, const char *str, size_t bufsz)),
	jamstrsub proto((char *buf, const char *str, size_t bufsz));

/* Variables: */

extern int	MarkThresh;		/* VAR: moves greater than MarkThresh will SetMark */

/* externs.h:
 *	strerror (ifdef NO_STRERROR)
 */
