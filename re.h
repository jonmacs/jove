/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

#define NALTS		16	/* number of alternate search strings */
#define COMPSIZE	512

/* kinds of regular expression compiles */
#define NORM	0	/* nothing special */
#define OKAY_RE	1	/* allow regular expressions */
#define IN_CB	2	/* in curly brace; implies OKAY_RE */

struct RE_block {
	char
		r_compbuf[COMPSIZE],
		*r_alternates[NALTS],
		r_lbuf[LBSIZE];
	int
		r_nparens;
	ZXchar
		r_firstc;
	jbool
		r_anchored;
};

extern char
	rep_search[128],	/* replace search string */
	rep_str[128];		/* contains replacement string */

extern int
	REbom,		/* beginning and end columns of match */
	REeom,
	REdelta;	/* increase in line length due to last re_dosub */

extern jbool
	okay_wrap;	/* Do a wrap search ... not when we're
			   parsing errors ... */

extern jbool
	re_lindex(LinePtr line, int offset, int dir,
		struct RE_block *re_blk, jbool lbuf_okay, int crater),
	LookingAt(const char *pattern,char *buf,int offset),
	look_at(char *expr);

extern Bufpos
	*docompiled(int dir, struct RE_block *re_blk),
	*dosearch(const char *pattern, int dir, jbool re);

extern void
	REcompile(const char *pattern, jbool re, struct RE_block *re_blk),
	putmatch(int which, char *buf,size_t size),
	re_dosub(struct RE_block *re_blk, char *tobuf, jbool delp),
	RErecur(void);

/* Variables: */

extern jbool
	CaseIgnore,		/* VAR: ignore case in search */
	WrapScan;		/* VAR: make searches wrap */
