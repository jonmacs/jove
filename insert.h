/***************************************************************************
 * This program is Copyright (C) 1986, 1987, 1988 by Jonathan Payne.  JOVE *
 * is provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is    *
 * included in all the files.                                              *
 ***************************************************************************/

/* ??? Why, for example, are GCchunks, lfreelist, and lfreereg here? -- DHR */

extern Bufpos *DoYank proto((
	struct line *fline, int fchar,
	struct line *tline, int tchar,
	struct line *atline, int atchar,
	struct buffer *whatbuf));

extern void
	ins_str proto((char *str,int ok_nl)),
	LineInsert proto((int num)),
	open_lines proto((int n)),
	overwrite proto((int c, int n)),
	insert_c proto((int c,int n)),
	GCchunks proto((void)),
	lfreelist proto((struct line *first)),
	lfreereg proto((struct line *line1,struct line *line2)),
	n_indent proto((int goal));

#ifdef	ABBREV
extern void	MaybeAbbrevExpand proto((void));
#endif

extern  struct line
	*nbufline proto((void));

/* Commands: */

extern void
#ifdef	LISP
	AddSpecial proto((void)),	/* add lisp special form */
	GSexpr proto((void)),	/* Grind S Expression. */
#endif
	DoParen proto((void)),
	LineAI proto((void)),
	Newline proto((void)),
	OpenLine proto((void)),
	QuotChar proto((void)),
	SelfInsert proto((void)),
	Tab proto((void)),
	YankPop proto((void));


/* Variables: */

extern int	CIndIncrmt;	/* how much each indentation level pushes over in C mode */
extern int	PDelay;		/* paren flash delay in tenths of a second */
