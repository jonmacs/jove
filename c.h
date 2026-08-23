/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

extern Bufpos
	*c_indent(jbool brace),
	*m_paren(DAPchar p_type, int dir, jbool can_mismatch, jbool can_stop);

extern void	mp_error(void);

/* Variables: */

extern int	CArgIndent;		/* VAR: how to indent arguments to C functions */
#ifdef CMT_FMT
extern char	CmtFmt[80];		/* VAR: comment format */
#endif

/* Commands: */
extern void
	BList(void),
	BSexpr(void),
	BUpList(void),
#ifdef CMT_FMT
	FillComment(void),
#endif
	FDownList(void),
	FList(void),
	FSexpr(void),
	LRShift(void),
	RRShift(void);
