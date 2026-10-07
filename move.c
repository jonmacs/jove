/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

#include "jove.h"
#include "re.h"
#include "chars.h"
#include "jctype.h"
#include "disp.h"
#include "move.h"
#include "screen.h"	/* for tabstop */

private int	line_pos;

/* next_char and prev_char return the offset of the character after
 * and before offset i in line.  In UTF-8 mode a well-formed multibyte
 * sequence is one character; otherwise each byte is.
 */
int
next_char(const char *line, int i)
{
#ifdef USE_UTF8
	ScrChar	cp;
	int	len,
		width;

	if (UTF8Mode && (len = utf8_decode(line + i, &cp, &width)) != 0)
		return i + len;
#endif
	return i + 1;
}

int
prev_char(const char *line, int i)
{
#ifdef USE_UTF8
	if (UTF8Mode) {
		int	j;

		for (j = i - 1; j >= 0 && j >= i - 4; j--)
			if ((ZXC(line[j]) & 0xC0) != 0x80)
				return next_char(line, j) == i? j : i - 1;
	}
#endif
	return i - 1;
}

/* Move n characters forward (backward if n is negative).  If whole is
 * NO, a character is a byte, which is what callers that count with
 * strlen want.
 */
void
char_motion(long n, jbool whole)
{
	for (; n > 0; n--) {
		if (eolp()) {			/* Go to the next Line */
			if (curline->l_next == NULL)
				break;

			SetLine(curline->l_next);
		} else {
			curchar = whole? next_char(linebuf, curchar) : curchar + 1;
		}
	}
	for (; n < 0; n++) {
		if (bolp()) {
			if (curline->l_prev == NULL)
				break;

			SetLine(curline->l_prev);
			Eol();
		} else {
			curchar = whole? prev_char(linebuf, curchar) : curchar - 1;
		}
	}
}

void
f_char(long n)
{
	char_motion(n, NO);
}

void
b_char(long n)
{
	char_motion(-n, NO);
}

void
ForChar(void)
{
	char_motion(arg_value(), YES);
}

void
BackChar(void)
{
	char_motion(-arg_value(), YES);
}

void
NextLine(void)
{
	if ((curline == curbuf->b_last) && eolp()) {
		complain(NullStr);
		/* NOTREACHED */
	}
	line_move(FORWARD, arg_value(), YES);
}

void
PrevLine(void)
{
	if ((curline == curbuf->b_first) && bolp()) {
		complain(NullStr);
		/* NOTREACHED */
	}
	line_move(BACKWARD, arg_value(), YES);
}

/* moves to a different line in DIR; LINE_CMD says whether this is
 * being called from NextLine() or PrevLine(), in which case it tries
 * to line up the column with the column of the current line
 */
void
line_move(int dir, long n, jbool line_cmd)
{
	LinePtr	(*proc)(LinePtr, long) =
		(dir == FORWARD) ? next_line : prev_line;
	LinePtr	line;

	line = (*proc)(curline, n);
	if (line == curline) {
		if (dir == FORWARD)
			Eol();
		else
			Bol();
		return;
	}

	if (line_cmd) {
		this_cmd = LINECMD;
		if (last_cmd != LINECMD)
			line_pos = calc_pos(linebuf, curchar);
	}
	SetLine(line);		/* curline is in linebuf now */
	if (line_cmd)
		curchar = how_far(curline, line_pos);
}

/* how_far returns what cur_char should be to be at or beyond col
 * screen columns in to the line.
 *
 * Note: if col indicates a position in the middle of a Tab or other
 * extended character, the result corresponds to that character
 * (as if col had indicated its start).
 *
 */

int
how_far(LinePtr line, int col)
{
	char	*lp = lcontents(line);
	int	i = 0,
		pos = 0,
		len;

	while (lp[i] != '\0') {
		pos += char_cols(lp + i, pos, &len);
		if (pos > col)
			break;
		i += len;
	}
	return i;
}

void
Bol(void)
{
	curchar = 0;
}

void
Eol(void)
{
	curchar = length(curline);
}

void
Eof(void)
{
	PushPntp(curbuf->b_last);
	ToLast();
}

void
Bof(void)
{
	PushPntp(curbuf->b_first);
	ToFirst();
}

/* Move forward (if dir > 0) or backward (if dir < 0) a sentence.  Deals
 * with all the kludgery involved with paragraphs, and moving backwards
 * is particularly yucky.
 */
private void
to_sent(int dir)
{
	for (;;) {
		Bufpos
			old,	/* where we started */
			*new;	/* where dosearch stopped */

		DOTsave(&old);

		new = dosearch(
			"^[ \t]*$\\|[?.!]\\{''\\|[\"')\\]]\\|\\}\\{$\\|[ \t]\\}",
			dir, YES);
		if (new == NULL) {
			if (dir == BACKWARD)
				ToFirst();
			else
				ToLast();
			break;
		}
		SetDot(new);
		if (dir < 0) {
			to_word(FORWARD);
			if ((old.p_line != curline || old.p_char > curchar)
			&& (!inorder(new->p_line, new->p_char, old.p_line, old.p_char)
			  || !inorder(old.p_line, old.p_char, curline, curchar)))
				break;

			SetDot(new);
		} else {
			if (blnkp(linebuf)) {
				Bol();
				b_char(1);
				if (old.p_line != curline || old.p_char < curchar)
					break;

				to_word(FORWARD);	/* Oh brother this is painful */
			} else {
				curchar = REbom + 1;	/* Just after the [?.!] */
				if (LookingAt("''\\|[\"')\\]]", linebuf, curchar))
					curchar = REeom;
				break;
			}
		}
	}
}

void
Bos(void)
{
	int	num = arg_value();

	if (num < 0) {
		negate_arg();
		Eos();
	} else {
		while (--num >= 0) {
			to_sent(BACKWARD);
			if (bobp())
				break;
		}
	}
}

void
Eos(void)
{
	int	num = arg_value();

	if (num < 0) {
		negate_arg();
		Bos();
	} else {
		while (--num >= 0) {
			to_sent(FORWARD);
			if (eobp())
				break;
		}
	}
}

void
f_word(long num)
{
	if (num < 0) {
		while (++num <= 0) {
			to_word(BACKWARD);
			while (!bolp() && jisword(linebuf[curchar - 1]))
				curchar -= 1;
			if (bobp())
				break;
		}
	} else {
		while (--num >= 0) {
			char	c;

			to_word(FORWARD);
			while ((c = linebuf[curchar]) != '\0' && jisword(c))
				curchar += 1;
			if (eobp())
				break;
		}
	}
	/* ??? why is the following necessary? -- DHR */
	this_cmd = OTHER_CMD;	/* Semi kludge to stop some unfavorable behavior */
}

void
ForWord(void)
{
	f_word(arg_value());
}

void
BackWord(void)
{
	f_word(-arg_value());
}
