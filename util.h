/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

/* some utility functions, as macros, to be included by jove.h */

extern int
	fnamecomp(const void *, const void *);	/* order file names */

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

extern void
	*emalloc(size_t size),
	*erealloc(void *ptr, size_t size);

extern char
	*IOerr(const char *err, const char *file),
	*copystr(const char *str),
	*get_time(time_t *timep,char *buf,int from,int to),
	*lcontents(LinePtr line),
	*ltobuf(LinePtr line,char *buf);

extern const char
	*jbasename(const char *f),
	*filename(const Buffer *b);

extern long
	inorder(LinePtr nextp,int char1,LinePtr endp,int char2),
	LineDist(LinePtr nextp,LinePtr endp),
	LinesTo(LinePtr from, LinePtr to);

extern int
	length(LinePtr line),
	jmax(int a,int b),
	jmin(int a,int b),
	numcomp(const char *s1, const char *s2),
	numcompcase(const char *s1, const char *s2);

extern ZXchar
	DecodePair(ZXchar first, ZXchar second);

extern jbool
	caseeqn(const char *s1, const char *s2, size_t n),
	TwoBlank(void),
	blnkp(char *buf),
	within_indent(void),
	fixorder(LinePtr  *line1,int *char1,LinePtr  *line2,int *char2),
	inlist(LinePtr first,LinePtr what),
	sindex(const char *pattern, const char *string),
	ModBufs(jbool allp);

extern void
	DOTsave(Bufpos *buf),
	DotTo(LinePtr line,int col),
	PushPntp(LinePtr line),
	SetDot(const Bufpos *bp),
	ToFirst(void),
	ToLast(void),
	ins_c(DAPchar c,char *buf,int atchar,int num,int max),
	len_error(int flag) NEVER_RETURNS,
	linecopy(char *onto,int atchar,char *from),
	modify(void),
	diverge(Buffer *buf, jbool d),
	null_ncpy(char *to, const char *from, size_t n),
#ifdef UNIX
	dopipe(int p[2]),
	pipeclose(int p[2]),
#endif
	pop_env(jmp_buf),
	push_env(jmp_buf),
	to_word(int dir),
	unmodify(void);

extern void *
	freealloc(void *obj, size_t size);

#if defined(IPROCS) || defined(SUBSHELL)
# ifndef OWCDOS
/*
 * The Open Watcom C declaration of environ has some macro
 * decorations that do not match with this, so we skip this,
 * their declaration works and is available.
 * Linux only provides this in unistd.h if _GNU_SOURCE is defined,
 * which we do not.
 * *BSD and OpenIndiana (Solaris) and probably most other
 * systems do not provide it, since it is not part of POSIX,
 * but environ seems remarkably portable, nonetheless!
 */
extern char **environ;	/* <unistd.h> */
# endif

typedef struct {
	const char **e_data;
	jbool e_malloced;
	int e_headroom;	/* remaining room in e_data array */
} Env;

extern const char **
	jenvdata(Env *envp);
extern void
	jputenv(Env *envp, const char *def),
	junsetenv(Env *envp, const char *name);
#endif /* defined(IPROCS) */

/* copy a string into a possibly-too-small buffer
 * trunc* silently truncates a too-large string.
 * jam* complain if the string is too large.
 */
#define truncstr(buf, str)	truncstrsub(buf, str, sizeof(buf))
#define jamstr(buf, str)	jamstrsub(buf, str, sizeof(buf))
extern void
	truncstrsub(char *buf, const char *str, size_t bufsz),
	jamstrsub(char *buf, const char *str, size_t bufsz),
	jamstrcat(char *buf, const char *str, size_t bufsz);

/* Variables: */

extern int	MarkThresh;		/* VAR: moves greater than MarkThresh will SetMark */
