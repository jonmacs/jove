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
	bool
		r_anchored;
};

extern char
	rep_search[128],	/* replace search string */
	rep_str[128];		/* contains replacement string */

extern int
	REbom,		/* beginning and end columns of match */
	REeom,
	REdelta;	/* increase in line length due to last re_dosub */

extern bool	okay_wrap;	/* Do a wrap search ... not when we're
				   parsing errors ... */

extern bool
	re_lindex proto((LinePtr line, int offset, int dir,
		struct RE_block *re_blk, bool lbuf_okay, int crater)),
	LookingAt proto((const char *pattern,char *buf,int offset)),
	look_at proto((char *expr));

extern Bufpos
	*docompiled proto((int dir, struct RE_block *re_blk)),
	*dosearch proto((const char *pattern, int dir, bool re));

extern void
	REcompile proto((const char *pattern, bool re, struct RE_block *re_blk)),
	putmatch proto((int which, char *buf,size_t size)),
	re_dosub proto((struct RE_block *re_blk, char *tobuf, bool delp)),
	RErecur proto((void));

/* Variables: */

extern bool
	CaseIgnore,		/* VAR: ignore case in search */
	WrapScan;		/* VAR: make searches wrap */
