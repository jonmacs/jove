/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

/* ??? Why, for example, are GCchunks, lfreelist, and lfreereg here? -- DHR */

extern Bufpos *DoYank(
	LinePtr fline, int fchar,
	LinePtr tline, int tchar,
	LinePtr atline, int atchar,
	Buffer *whatbuf);

extern void
	ins_str(const char *str),
	ins_str_wrap(const char *str, jbool ok_nl, int wrap_off),
	LineInsert(long num),
	open_lines(long n),
	overwrite(DAPchar c, int n),
	insert_c(DAPchar c,int n),
	GCchunks(void),
	lfreelist(LinePtr first),
	lfreereg(LinePtr line1,LinePtr line2),
	n_indent(int goal);

#ifdef ABBREV
extern void	MaybeAbbrevExpand(void);
#endif

extern LinePtr
	nbufline(void);

/* Commands: */

extern void
#ifdef LISP
	AddSpecial(void),	/* add lisp special form */
	GSexpr(void),	/* Grind S Expression. */
#endif
	DoParen(void),
	LineAI(void),
	Newline(void),
	OpenLine(void),
	QuotChar(void),
	SelfInsert(void),
	Tab(void),
	YankPop(void);


/* Variables: */

extern int	CIndIncrmt;	/* VAR: how much each indentation level pushes over in C mode */
extern int	PDelay;		/* VAR: paren flash delay in tenths of a second */
