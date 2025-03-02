/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

#ifdef HIGHLIGHTING

typedef void(*LEproc)(jbool);
#define LENULLPROC (LEproc)0

typedef struct LErange {
	unsigned
		start,	/* starting column for highlighting */
		width;	/* width of highlighting */
	LEproc	norm,
		high;
} *LineEffects;

#define	NOEFFECT	((LineEffects) NULL)
extern void US_effect(jbool);

#else /* !HIGHLIGHTING */

typedef jbool	LineEffects;	/* standout or not */
#define	NOEFFECT	NO

#endif /* !HIGHLIGHTING */

struct screenline {
	char
		*s_line,
		*s_roof;	/* character after last */
	LineEffects s_effects;
};

extern struct screenline
	*Screen,
	*Curline;

extern char *cursend;

extern int
	AbortCnt,

	CapLine,	/* cursor line and cursor column */
	CapCol;

extern jbool
	BufSwrite(int linenum),
	swrite(char *line, LineEffects hl, jbool abortable);

extern LineEffects
	WindowRange(Window *w);

extern void
	Placur(int line,int col),
	cl_eol(void),
	cl_scr(jbool doit),
	clrline(char *cp1, char *cp2),
	i_set(int nline, int ncol),
	make_scr(void),
	v_ins_line(int num, int top, int bottom),
	v_del_line(int num, int top, int bottom),
	SO_effect(jbool),
	SO_off(void);

#define	TABDIST(col)	 - (col)%tabstop	/* cols to next tabstop */

/* Variables: */

extern int	tabstop;		/* VAR: expand tabs to this number of spaces */
