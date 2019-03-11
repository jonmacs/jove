/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

extern void
	skip_wht_space proto((void));

/* Commands: */

extern void
	ChrToOct proto((void)),
	GoLine proto((void)),
	KillBos proto((void)),
	KillEOL proto((void)),
	KillEos proto((void)),
	KillExpr proto((void)),
	Leave proto((void)),
	NotModified proto((void)),
	SetLMargin proto((void)),
	SetRMargin proto((void)),
	StrLength proto((void)),
	ToIndent proto((void)),
	TransChar proto((void)),
	TransLines proto((void)),
	Yank proto((void)),
	prCTIME proto((void));
