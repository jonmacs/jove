/*************************************************************************
 * This program is copyright (C) 1985, 1986 by Jonathan Payne.  It is    *
 * provided to you without charge for use only on a licensed Unix        *
 * system.  You may copy JOVE provided that this notice is included with *
 * the copy.  You may not sell copies of this program or versions        *
 * modified for use on microcomputer systems, unless the copies are      *
 * included with a Unix system distribution and the source is provided.  *
 *************************************************************************/

/* Contains commands for C mode.  Paren matching routines are in here. */

#include "jove.h"
#include "ctype.h"

Bufpos *
m_paren(orig, stop)
char	orig;		/* One we are on */
Line	*stop;
{
	char	*origs = "(){}[]<>",
		*matches = ")(}{][><",
		matcher;
	int	which,
		forward,
		count = 0;
	char	c;
	Bufpos	savedot,
		lastdot,
		*bp;
	static Bufpos	ret;
	char	REstr[10],
		REbuf[30],
		*REalts[4];
	extern Line	*RElimit;

	which = index(origs, orig) - origs;
	forward = (which % 2) == 0;
	matcher = matches[which];
	ignore(sprintf(REstr, "[\\%c\\%c]", orig, matcher));
	DOTsave(&savedot);
	REcompile(REstr, 1, REbuf, REalts);

	while (count >= 0) {
		RElimit = stop;		/* Set his EVERY time. */
		bp = docompiled(forward ? FORWARD : BACKWARD, REbuf, REalts);
		if (bp == 0)
			break;
		SetDot(bp);
		DOTsave(&lastdot);
		c = linebuf[curchar - forward];	/* Get it? */
		if (c == matcher)
			count -= NotInQuotes(linebuf, curchar - forward);
		else if (c == orig)
			count += NotInQuotes(linebuf, curchar - forward);
	}
	ret.p_line = curline;
	ret.p_char = curchar;
	SetDot(&savedot);
	if (count >= 0)
		return 0;
	return &ret;
}

static
do_expr(dir)
{
	char	c;

	exp = 1;
	if (dir == BACKWARD)
		BackChar();
	c = linebuf[curchar];
	for (;;) {
		if ((ismword(c)) ||
		    (index("[({", c) != 0 && dir == FORWARD) ||
		    (index("})]", c) != 0 && dir == BACKWARD))
			break;
		if (dir == BACKWARD) {
			if (bobp())
				return;
			BackChar();
		} else {
			if (eobp())
				return;
			ForChar();
		}
		c = linebuf[curchar];
	}

	if (((c == '(' || c == '{') && dir == FORWARD) ||
	    ((c == ')' || c == '}') && dir == BACKWARD)) {
		FindMatch(dir);
	} else {
	    WITH_TABLE(curbuf->b_major)

		(dir == FORWARD) ? ForWord() : BackWord();

	    END_TABLE();
	}
}

Fparen()
{
	register int	num = exp;

	if (exp < 0) {
		exp = -exp;
		Bparen();
	}
	while (--num >= 0)
		do_expr(FORWARD);
}

Bparen()
{
	register int	num = exp;

	if (exp < 0) {
		exp = -exp;
		Fparen();
	}
	while (--num >= 0)
		do_expr(BACKWARD);
}

/* Move to the matching brace or paren depending on the current position
   in the buffer. */

FindMatch(dir)
{
	Bufpos	*bp;
	char	c = linebuf[curchar];

	if ((index("){}(", linebuf[curchar]) == 0) ||
	    !NotInQuotes(linebuf, curchar))
		complain((char *) 0);
	if (dir == FORWARD)
		ForChar();
	bp = m_paren(c, (dir == FORWARD) ? curbuf->b_last : curbuf->b_first);
	if (bp)
		SetDot(bp);
	else
		complain("[No match]");
}

/* Make sure character at c_char is not surrounded by double
   or single quotes. */

NotInQuotes(buf, pos)
register char	*buf;
{
	char	quotchar = 0,
		c,
		*quots;
	register int	i;

	if (MajorMode(CMODE))
		quots = "\"'";
#ifdef LISP
	else if (MajorMode(LISPMODE))
		quots = "\"";
#endif
	else
		return 1;	/* Not in quotes */

	for (i = 0; i < pos && buf[i]; i++) {
		if ((c = buf[i]) == '\\') {
			if (++i == pos)
				return 0;	/* The char is BackSlash'd */
			continue;
		}
		if (!index(quots, c))
			continue;
		if (quotchar == 0)
			quotchar = c;
		else if (c == quotchar)
			quotchar = 0;	/* Terminated string */
	}
	return (quotchar == 0);
}

Bufpos *
c_indent()
{
	Bufpos	*bp,
		save;
	int	indent;

	DOTsave(&save);
	bp = m_paren('}', curbuf->b_first);
	if (bp == 0)
		return 0;
	SetDot(bp);
	ToIndent();
	indent = calc_pos(linebuf, curchar);
	SetDot(&save);
	n_indent(indent);

	return bp;
}

#ifdef CMT_FMT

char	CmtFmt[80] = "/*%n%! * %c%!%n */";

Comment()
{
	FillComment(CmtFmt);
}

/* Strip leading and trailing white space.  Skip over any imbedded '\r's. */

static
strip_c(from, to)
char	*from,
	*to;
{
	register char	*fr_p = from,
			*to_p = to,
			c;

	while (c = *fr_p) {
		if (c == ' ' || c == '\t' || c == '\r')
			fr_p++;
		else
			break;
	}
	while (c = *fr_p) {
		if (c != '\r')
			*to_p++ = c;
		fr_p++;
	}
	while (--to_p >= to)
		if (*to_p != ' ' && *to_p != '\t')
			break;
	*++to_p = '\0';
}

static char	open_c[20],	/* the open comment format string */
		open_pat[20],	/* the search pattern for open comment */
		l_header[20],	/* the prefix for each comment line */
		l_trailer[20],	/* the suffix ... */
		close_c[20],
		close_pat[20];

static char	*comment_body[] = {
 	open_c,
	l_header,
	l_trailer,
	close_c
};
					
static int	nlflags;

/* Fill in the data structures above from the format string.  Don't return
   if there's trouble. */

static
parse_cmt_fmt(str)
char	*str;
{
	register char	*fmtp = str;
	register char	**c_body = comment_body,
			*body_p = *c_body;
	int	c,
	 	newlines = 1;

	/* pick apart the comment string */
	while (c = *fmtp++) {
		if (c != '%') {
			*body_p++ = c;
			continue;
		}
		switch(c = *fmtp++) {
		case 'n':
			if (newlines == 2 || newlines == 3)
				complain("%n not allowed in line header or trailer: %s",
				  fmtp - 2);
			nlflags += newlines;
			*body_p++ = '\r';
			break;
		case 't':
			*body_p++ = '\t';
			break;
		case '%':
			*body_p++ = '%';
			break;
		case '!':
		case 'c':
			newlines++;
			*body_p++ = '\0';
			body_p = *++c_body;
			break;
		default:
			complain("[Unknown comment escape: %%%c]", c);
			/* VARARGS */
			break;
		}
	}
	*body_p = '\0';
	/* make search patterns */
	strip_c(open_c, open_pat);
	strip_c(close_c, close_pat);
}

#define NL_IN_OPEN_C  ((nlflags % 4) == 1)
#define NL_IN_CLOSE_C (nlflags >= 4)

FillComment(format)
char	*format;
{
	int	saveRMargin,
		indent_pos,
		close_at_dot = 0,
		slen,
		header_len,
		trailer_len;
	register char	*cp;
	static char	inside_err[] = "[Must be between %s and %s to re-format]";
	Bufpos	open_c_pt,
		close_c_pt,
		tmp_bp,
		*match_o,
		*match_c;
	Mark	*entry_mark,
		*open_c_mark,
		*savedot;

	parse_cmt_fmt(format);
	/* figure out if we're "inside" a comment */
 	if ((match_o = dosearch(open_pat, BACKWARD, 0)) == 0)
		/* VARARGS */
		complain("No opening %s to match to.", open_pat);
	open_c_pt = *match_o;
	if ((match_c = dosearch(close_pat, BACKWARD, NO)) != 0 &&
	    inorder(open_c_pt.p_line, open_c_pt.p_char,
		    match_c->p_line, match_c->p_char))
	  	complain(inside_err, open_pat, close_pat);
	if ((match_o = dosearch(open_pat, FORWARD, NO)) != 0) {
		tmp_bp = *match_o;
		match_o = &tmp_bp;
	} 
	if ((match_c = dosearch(close_pat, FORWARD, 0)) != (Bufpos *) 0)
		close_c_pt = *match_c;

	/* Here's where we figure out whether to format from dot or from
	   the close comment.  Note that we've already searched backwards to
	   find the open comment symbol for the comment we are formatting.
	   The open symbol mentioned below refers to the possible existence
	   of the next comment.  There are 5 cases:
		1) no open or close symbol		==> dot
		2) open, but no close symbol		==> dot
		3) close, but no open			==> close
		4) open, close are inorder		==> dot
		5) open, close are not inorder		==> close */


	if (match_o == (Bufpos *) 0) {
		if (match_c == (Bufpos *) 0)
			close_at_dot++;
	} else if (match_c == (Bufpos *) 0)
		close_at_dot++;
	else if (inorder(match_o->p_line, match_o->p_char,
		 match_c->p_line, match_c->p_char))
		close_at_dot++;

	if (close_at_dot) {
		close_c_pt.p_line = curline;
		close_c_pt.p_char = curchar;
	} else {
		SetDot(match_c);
	}
	SetDot(&open_c_pt);
	open_c_mark = MakeMark(curline, curchar, FLOATER);
	indent_pos = calc_pos(linebuf, curchar);
	/* search for a close comment; delete it if it exits */
	SetDot(&close_c_pt);
	if (close_at_dot == 0) {
		slen = strlen(close_pat);
		while (slen--)
			DelPChar();
	}
	entry_mark = MakeMark(curline, curchar, FLOATER);
	ToMark(open_c_mark);
	/* always separate the comment body from anything preceeding it */
	LineInsert();
	DelWtSpace();
	Bol();
	for (cp = open_c; *cp; cp++) {
		if (*cp == '\r') {
			if (!eolp())
				LineInsert();
			else
				line_move(FORWARD, NO);
		} else if (*cp == ' ' || *cp == '\t') {
			if (linebuf[curchar] != *cp)
				Insert(*cp);
		} else
			/* Since we matched the open comment string on this
			   line, we don't need to worry about crossing line
			   boundaries. */
			curchar++;
	}
	savedot = MakeMark(curline, curchar, FLOATER);

	/* We need to strip the line header pattern of leading white space
	   since we need to match the line after all of its leading
	   whitespace is gone. */
	for (cp = l_header; *cp && (isspace(*cp)); cp++)
		;
	header_len = strlen(cp);
	trailer_len = strlen(l_trailer);

	/* Strip each comment line of the open and close comment strings
	   before reformatting it. */

	do {
		Bol();
		DelWtSpace();
		if (header_len && !strncmp(linebuf, cp, header_len))
			DoTimes(DelNChar(), header_len);
		if (trailer_len) {
			Eol();
			if ((curchar > trailer_len) &&
			    (!strncmp(&linebuf[curchar - trailer_len],
				      l_trailer, trailer_len)))
				DoTimes(DelPChar(), trailer_len);
		}
		if (curline->l_next != 0)
			line_move(FORWARD, NO);
		else
			break;
	} while (curline != entry_mark->m_line->l_next);

	DoSetMark(savedot->m_line, savedot->m_char);
	ToMark(entry_mark);
	saveRMargin = RMargin;
	RMargin = saveRMargin - strlen(l_header) -
		  strlen(l_trailer) - indent_pos + 2;
	/* do not use the left margin */
	exp_p = 0;
	do_rfill();
	RMargin = saveRMargin;
	/* get back to the start of the comment */
	PopMark(); 
	do {
		if (curline == open_c_mark->m_line->l_next) {
			;
		} else {
			Bol();
			n_indent(indent_pos);
			ins_str(l_header, NO);
		}
		Eol();
		if (!NL_IN_CLOSE_C && (curline == entry_mark->m_line))
			;
		else
			ins_str(l_trailer, NO);
		if (curline->l_next != 0)
			line_move(FORWARD, NO);
		else 
			break;
	} while (curline != entry_mark->m_line->l_next);
	/* handle the close comment symbol */
	if (curline == entry_mark->m_line->l_next) {
		line_move(BACKWARD, NO);
		Eol();
	}
	DelWtSpace();
	/* if the addition of the close symbol would cause the line to be
	   too long, put the close symbol on the next line. */
	if (strlen(close_c) + calc_pos(linebuf, curchar) > RMargin) {
		LineInsert();
		n_indent(indent_pos);
	}
	for (cp = close_c; *cp; cp++) {
		if (*cp == '\r') {
			LineInsert();
			n_indent(indent_pos);
		} else
			Insert(*cp);
	}
	ToMark(open_c_mark);
	Eol();
	exp_p = 0;
	DelNChar();
}

#endif CMT_FMT

