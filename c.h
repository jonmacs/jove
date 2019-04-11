/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

extern Bufpos
	*c_indent proto((bool brace)),
	*m_paren proto((DAPchar p_type, int dir, bool can_mismatch, bool can_stop));

extern void	mp_error proto((void));

/* Variables: */

extern int	CArgIndent;		/* VAR: how to indent arguments to C functions */
#ifdef CMT_FMT
extern char	CmtFmt[80];		/* VAR: comment format */
#endif

/* Commands: */
extern void
	BList proto((void)),
	BSexpr proto((void)),
	BUpList proto((void)),
#ifdef CMT_FMT
	FillComment proto((void)),
#endif
	FDownList proto((void)),
	FList proto((void)),
	FSexpr proto((void)),
	LRShift proto((void)),
	RRShift proto((void));
