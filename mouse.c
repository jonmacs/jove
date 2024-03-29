/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

/* This file contains all functions associated with mouse buttons being
 * pressed and any movement of the mouse (except on the Macintosh).
 * These procedures will only be called if JOVE is run under xterm,
 * xJove, or Jovetool.
 */

#include "jove.h"

#ifdef MOUSE	/* the body is the rest of this file */

/* #include "argcount.h" */
#include "commands.h"	/* for cmdproc_t */
#include "disp.h"
#include "misc.h"
#include "ask.h"
#include "chars.h"
#include "delete.h"
#include "fmt.h"
#include "insert.h"	/* for lfreelist, in MouseLine */
#include "marks.h"
#include "move.h"
#include "wind.h"
#include "term.h"		/* for M_SR */
#include "fp.h"		/* for putstr */
#include "jctype.h"
#include "mouse.h"
#include "xjove/mousemsg.h"

/* xterm control sequences, including those related to mousing, are described
 * in a file called ctlseqs.ms which is part of the X Distribution.
 *
 * We document some xterm bugs/features here: a good place
 * to capture the hard-won knowledge.
 *
 * - In Hilite mode, after a button 1 press (producing ^[[M xy), and after
 *   the program has replied with ^[f;sx;sy;fr;lrT (as required in Hilite
 *   mode), it treats the next character read as if it had been preceded by
 *   "^[[".  For example, if that next character happens to be A, B, C or D
 *   you therefore get a cursor movement.  See the #ifdef XTERMHLBUG below
 *   for the workaround.
 *
 * - mouse tracking and highlight tracking modes do not support modifiers
 *   other than CTRL, contrary to ctlseqs.ms.  See the #ifdef NEVER below.
 *
 * - mouse hilite tracking mode (MHTM) only seems to work for button 1.
 *
 * - MHTM does not seem to yield mouse-button-1-up events (but if
 *   a non-empty drag occurred, that will be reported)
 *
 * - The documentation specifies that the ^[ [ T x y x y x y message
 *   should only be generated by MHTM when the user has dragged outside
 *   the specified bounds.  It seems to also be generated:
 *   + by a multiclick (of button 1, of course)
 *   + by a drag to the left or up
 *   + when the button is released to the right of the end of a text line
 *     (but xterm's idea of this may well differ from JOVE's since JOVE
 *     optimizes screen output)
 *
 * - When the mouse is enabled, xterm interprets ^[[ ... T as an
 *   Initiate Hilite Mouse Tracking command.  Otherwise, recent
 *   versions interpret it as Parameterized Scroll Down.  Some recent
 *   termcap/terminfo xterm entries define the SR capability using
 *   this sequence.  But it doesn't work all the time!
 *   It is hard to say what the best fix would be:
 *   + get X to distinguish the two T commands by the number of parameters
 *   + correctly document the limitation and remove SR from the
 *     xterm termcap and terminfo datebases
 *
 *   In any case, the problem will be in the wild for some time
 *   so JOVE now has an ugly kludge to dodge the bug.
 *   KLUDGE: suppress M_SR (our name for SR) if in mouse mode and M_SR
 *   ends in "T".
 *
 *   First observed and analyzed in Fedora Core 4 in 2005 September.
 *
 * - xterm has gained more capabilities over the years.  Some misguided
 *   people of revised the termcap/terminfo entries for xterm to exploit
 *   new features.  This has unfortunate consequences when the xterm
 *   is on a different system from the termcap/terminfo database:
 *   if they don't match, JOVE will potentially curdle the screen.
 *   JOVE cannot work around this problem, the user must.
 */

bool	XtermMouse = NO;	/* VAR: should we enable xterm mouse? */
private bool	xtMouseState = NO;	/* have we enabled the mouse? */

/* sequences to enable/disable mouse hilite tracking in xterm */
private const char
	xtMouseEnable[] = "\033[?1001h",
	xtMouseDisable[] = "\033[?1001l";

private int	but_state;	/* button state (and more) at mouse event */
private int	x_coord;	/* mouse x-coordinate, in pixels, origin 0 */
private int	y_coord;	/* mouse y-coordinate, in characters, origin 0 */
private int		/* xterm drag range coordinates */
	startx, starty,
	endx, endy;
private int	font_width;	/* width of a character in pixels */
private Window	*oldwind;
private Mark	*oldpos = NULL;	/* Use a Mark so it is adjusted automatically */

#define LMA_NONE	0000
#define LMA_COPY	0001
#define LMA_CUT		0002
#define LMA_PASTE	0010
#define LMA_CHAR	0020
#define LMA_WORD	0030
#define LMA_LINE	0040
private int last_mouse_act = LMA_NONE;

private const char *saved_M_SR = NULL;	/* KLUDGE for xterm/termcap bug */

void
MouseOn()
{
	if (XtermMouse != xtMouseState) {
		/* KLUDGE for xterm/termcap bug */
		if (XtermMouse && M_SR != NULL) {
		    size_t len = strlen(M_SR);

		    saved_M_SR = M_SR;
		    if (len > 0 && M_SR[len-1] == 'T')
			M_SR = NULL;
		}
		/* end if KLUDGE */
		putstr(XtermMouse? xtMouseEnable : xtMouseDisable);
		xtMouseState = XtermMouse;
	}
}

void
MouseOff()
{
	if (xtMouseState) {
		putstr(xtMouseDisable);
		xtMouseState = NO;
		M_SR = saved_M_SR;	/* KLUDGE for xterm/termcap bug */
	}
}

/* Set cursor position to that of mouse pointer. */

private void
SetCursor()
{
	int	line_pos = in_window(curwind, curline);
	int	offset = PhysScreen[y_coord].s_offset;
	int	num_moves;

	if (line_pos < 0) {
		SetLine(curwind->w_top);
		line_pos = in_window(curwind, curline);
	}
	num_moves = y_coord - line_pos;
	if (num_moves > 0)
		line_move(FORWARD, num_moves, NO);
	if (num_moves < 0)
		line_move(BACKWARD, -num_moves, NO);

	curchar = how_far(curline,
		(x_coord + font_width/2) / font_width + offset
		- (W_NUMWIDTH(curwind) + SIWIDTH(offset)));
}

private void
ScrollToMouse()
{
	register int	lc;
	const int	width = (CO - 1 - (4 * SG)) * font_width;	/* must match size in ModeLine */
	int	top;

	/* This code ought to match the scroll-bar layout computed by
	 * WindowRange().
	 * - a click in the first column ought to force top-of-file
	 * - a click in the last column ought to force end-of-file
	 * - don't waste space by displaying less than a screenful at end-of-file.
	 * - After clicking at a given point in the scroll bar, the cursor
	 *   should be found to be exactly in the middle of the highlighted
	 *   region (except near the ends, of course).
	 */
	lc = LinesTo(curbuf->b_first, (LinePtr)NULL);
	top = 1 + (long)(x_coord-font_width) * (lc-2) / (width-2*font_width)
		- WSIZE(curwind)/2;
	if (top > lc - WSIZE(curwind))
		top = lc - WSIZE(curwind);
	if (top < 0)
		top = 0;

	SetTop(curwind, next_line(curbuf->b_first, top));
	if (in_window(curwind, curline) == -1)
		SetLine(next_line(curwind->w_top, WSIZE(curwind)/2));
}

private bool
ObeyProc(p)
cmdproc_t p;
{
	if (BufMinorMode(curbuf, BReadOnly)) {
		rbell();
		message("[Buffer is read-only]");
		return NO;
	} else {
		(*p)();
		return YES;
	}
}

/* Get next value in number sequence */

private int
NextValue()
{
	int val;

	(void) waitchar();
	Digit();
	val = arg_value();
	clr_arg_value();
	return val;
}

/* Select appropriate window */

private int
SelectWind(winforce)
Window	*winforce;	/* if non-null, must be within this window */
{
	register Window *wp = fwind;
	int	total_lines = wp->w_height;

	/* Find which window mouse pointer is in. */
	while (y_coord >= total_lines) {
		wp = wp->w_next;
		if (wp == fwind)
			return -1;

		total_lines += wp->w_height;
	}
	if (winforce != NULL && wp != winforce)
		return -1;

	SetWind(wp);		/* Set current window */
	return (total_lines - y_coord - 1);	/* Cursor pos within window */
}

/* get an origin-0 coordinate in funny representation used by xterm */

private int
xtGetCoord(upb)
int upb;
{
	ZXchar	c = waitchar();	/* coordinate */

	/* undo MetaKey if we think it was done */
	if (c == ESC && MetaKey)
		c = waitchar() | METABIT;

	/* It appears that mouse events near the extreme right hand edge of
	 * a window can give coordinates as large as LI.  Perhaps this is
	 * true for CO too.  So the range is inclusive.
	 */
	return '!' <= c && c - '!' <= upb? c - '!' : -1;
}

/* get some X Y pair from xterm; return indication of success */

private bool
xtGetXY(xp, yp)
int
	*xp,
	*yp;
{
	int x = xtGetCoord(CO);

	if (x != -1) {
		int y = xtGetCoord(LI);

		if (y != -1) {
			*xp = x;
			*yp = y;
			font_width = 1;
			return YES;
		}
	}
	return NO;
}

#define MPROTO_XTERM	0
#define MPROTO_XTDRAG	1
#define MPROTO_JOVETOOL	2

#define XtermProto(p)	((p) <= MPROTO_XTDRAG)

/* Format of command to xterm to start or stop mouse hilite tracking:
 * ^[ [ func ; startx ; starty ; firstrow ; lastrow T
 *
 * [1997 September] All xterm's up until now have a bug whereby
 * the character sent after this sequence is interpreted as if
 * it were preceded by an ESC.  We pad with an "x"  character to
 * be ignored.
 *
 * [1998 Sept 21]  XFree86 3.2's xterm fixes the original bug
 * so we have to refine our work-around: make sure that the
 * character to be ignored has no semantics (DEL).
 *
 * [2004 July 11] XFree86's xterm no longer ignores DEL!
 * This is an xterm bug.  A fix to xterm has been accepted
 * by XFree86 and been submitted to x.org.  Even so,
 * this bug will be in the field for a long time, so we
 * need to live with it.
 * The only character that seems to be reliably ignored
 * in XFree86-4.0.1's xterm is NUL.
 *
 * This fudge is intended to be harmless on xterms
 * with or without these bugs.
 */
#define XTERMHLBUG	1	/* always enable: we think that it is safe */

static void
hl_mode(hl_setting, startx, starty, endx, endy)
int	hl_setting, startx, starty, endx, endy;
{
	static const char	hl_fmt[] = "\033[%d;%d;%d;%d;%dT";
	char	buf[sizeof(hl_fmt) + 4*(5-2)];
	
	swritef(buf, sizeof(buf), hl_fmt, hl_setting, startx, starty, endx, endy);
	putstr(buf);
#ifdef XTERMHLBUG
	scr_putchar('\0');
#endif
}

private bool
MouseParams(mproto)
int	mproto;
{
	/* The mouse commands will be invoked by hitting a mouse
	 * button but can also be invoked by typing the escape sequence
	 * CTRL-Xm so the following validation checks are necessary.
	 */

	static int	wind_pos;		/* reverse y-coordinate within window */
	static bool	mode_mode = NO;		/* true while doing modeline */

	/* up_expected is an extended bool: it can be YES, NO, and -1!
	 * -1 signifies that we are in xterm mouse hilite tracking mode
	 * which appears to fail to yield an up event if it deems the
	 * event uninteresting (i.e. no motion).
	 */
	static int	up_expected = NO;	/* true while button held down */
	static int	estartx, estarty;	/* from last enable */

	bool	input_good = NO;

	/* This switch reads and decodes the control sequence.
	 * - input_good is set to YES if the sequence looks valid.
	 * - but_state, x_coord, and y_coord are changed to reflect the new state
	 * - if the event could follow an "initiate hilite tracking"
	 *   and up_expected is -1, the decoder should adjust up_expected.
	 */
	switch (mproto) {
	case MPROTO_XTERM:
		{
			ZXchar	cb = LastKeyStruck;

			switch (cb) {
			default:
				if (' ' <= cb && cb < ' '+040 && xtGetXY(&x_coord, &y_coord)) {
					/* ^[ [ M buttoninfo mousex mousey
					 * Button event with coordinates.
					 * On a release event we aren't told which button
					 * is being released.
					 */
					cb -= ' ';
					if ((cb & 03) == 03) {
						/* guess that released button is last depressed */
						but_state = (but_state & JT_BUTMASK) | JT_UPEVENT;
						/* We are welcome in xterm mouse hilite tracking mode */
						if (up_expected == -1)
							up_expected = YES;
					} else {
						static const int butcode[] = { JT_LEFT, JT_MIDDLE, JT_RIGHT };

						but_state = butcode[cb & 03] | JT_DOWNEVENT;
						/* We are welcome in xterm mouse hilite tracking mode */
						if (up_expected == -1)
							up_expected = NO;
					}
#ifdef NEVER	/* surprise: xterm won't generate these modifiers! */
					if (cb & 04)
						but_state |= JT_SHIFT;
					if (cb & 010)
						but_state |= JT_META;
#endif
					if (cb & 020)
						but_state |= JT_CONTROL;
					input_good = YES;
				}
				break;
			}
		}
		break;

	case MPROTO_XTDRAG:
		/* handle xterm hilite tracking drag event: ^[ [ t or ^[ [ T
		 * These are only generated for left-button acts.
		 * Each is some drag-like event.
		 */
		switch (LastKeyStruck) {
		case 'T':
			/* ^[ [ T startx starty endx endy mousex mousey
			 * returned on Left Button release beyond end-of-line,
			 * or (undocumented) a drag to left or up, or
			 * (undocumented) for a multi-click select.  These are
			 * transformed into three kinds of event, distinguished
			 * by JT_CLICK2 and JT_CLICK3.  The JT_CLICKn are used to
			 * indicate that it was a multi-click select (by
			 * eliminating the other cases).  Note that a double click
			 * beyond end-of-line (or on an empty line) is not reported
			 * even though a third such click is.  Oh the joys of
			 * relying on undocumented features!
			 */
			if (xtGetXY(&startx, &starty)
			&& xtGetXY(&endx, &endy)
			&& xtGetXY(&x_coord, &y_coord))
			{
				input_good = YES;
				if (((  endy != estarty || endx != estartx
				     || y_coord > estarty
				     || (y_coord == estarty && x_coord >= estartx)
				     )	/* it isn't a drag left or up */
				&&   (  endx != 0 || startx == endx
				     )	/* it isn't a drag across an e-o-l */
				    )
				||  (but_state & JT_CLICKMASK) == JT_CLICK2)
					/* it is clearly a third click */
				{
					/* a complex selection */
					if ((but_state & JT_CLICKMASK) == JT_CLICK2 || endx == 0)
						/* this is a third click:
						 * endx == 0 implies that we
						 * missed the second one
						 */
						but_state = JT_LEFT | JT_DRAGEVENT | JT_CLICK3;
					else
						but_state = JT_LEFT | JT_DRAGEVENT | JT_CLICK2;
				} else {
					/* A simple drag.
					 * Note: code is the same as for case 't' below.
					 */
					but_state = JT_LEFT | JT_DRAGEVENT;
				}
			}
			break;

		case 't':
			/* ^[ [ t mousex mousey
			 * returned on Left Button release within valid text.
			 * A simple drag.
			 * Note: code is duplicated above as part of case 'T'.
			 */
			if (xtGetXY(&x_coord, &y_coord)) {
				but_state = JT_LEFT | JT_DRAGEVENT;
				input_good = YES;
			}
			break;
		}
		/* We are welcome in xterm mouse hilite tracking mode */
		if (input_good && up_expected == -1)
			up_expected = YES;
		break;

	case MPROTO_JOVETOOL:
		if (waitchar() == '(') {
			but_state = NextValue();	/* Read in parameters */
			switch (waitchar()) {
			case ' ':
				{
					int x = NextValue();

					if (waitchar() == ' ') {
						int y = NextValue();

						if (waitchar() == ' ') {
							font_width = NextValue();
							if (waitchar() == ')'
							&& waitchar() == '\r')
							{
								input_good = YES;
								x_coord = x;
								y_coord = y;
							}
						}
					}
				}
				break;
			case ')':
				input_good = (waitchar() == '\r');
				break;
			}
		}
		break;
	}
	this_cmd = OTHER_CMD;	/* no longer gathering args */

	if (!input_good) {
		if (XtermProto(mproto) && but_state == (JT_LEFT | JT_DOWNEVENT)) {
			/* abort Hilite mode to prevent hangups */
			hl_mode(0,1,1,1,1);
		}
		complain("[mouse input of wrong format]");
		/* NOTREACHED */
	}

	/* Note: at this point, up_expected still reflects previous state.
	 * Unfortunately, xterm seems to elide some button-up events!
	 * For this reason, some cases above adjust up_expected to
	 * prevent the following sanity check from going off.
	 */

	/* check: button_(was)_held iff currently event is UP or DRAG */
	if (up_expected != ((but_state & JT_EVENTMASK)!=JT_DOWNEVENT)
	|| (up_expected == YES && last_cmd != MOUSE_CMD)) {
		if (XtermProto(mproto) && but_state == (JT_LEFT | JT_DOWNEVENT)) {
			/* abort Hilite mode to prevent hangups */
			hl_mode(0,1,1,1,1);
		}
		up_expected = NO;	/* resynch in neutral */
		complain("[Mouse events out of order]");
		/* NOTREACHED */
	}

	/* update up_expected to reflect new state */
	up_expected = XtermProto(mproto) && (but_state&JT_EVENTMASK) == JT_DRAGEVENT
		? -1 : (but_state & JT_EVENTMASK) != JT_UPEVENT;
	if (up_expected != NO)
		this_cmd = MOUSE_CMD;

	if ((but_state & JT_EVENTMASK) == JT_DOWNEVENT
	&& (but_state & JT_PASTEMASK) && (but_state & JT_CSMASK) == 0)
	{
		/* Hugh: ??? is this leak possible? -- checking code added as a probe. */
		/* CHL believes assert oldpos==NULL */
		if (oldpos != NULL) {
			complain("[internal error: mark leak from MouseParams]");
			/* NOTREACHED */
		}
		oldpos = MakeMark(curline, curchar);
	}

	/* UP events in xterm should do nothing in foreign windows */
	if (XtermProto(mproto)
	&& (but_state & JT_EVENTMASK) == JT_UPEVENT
	&& curwind != oldwind
	&& !mode_mode)
		return NO;

	if ((but_state & (JT_CLICKMASK | JT_EVENTMASK)) == JT_DOWNEVENT) {
		/* button down, and first click at that:
		 * - ok to changing window
		 * - ok to move in or out of modeline (scroll bar)
		 */
		oldwind = curwind;
		wind_pos = SelectWind((Window *) NULL);
		mode_mode = (wind_pos == 0);
		if (XtermProto(mproto) && but_state == (JT_LEFT | JT_DOWNEVENT)) {
			/* Initiate or abort mouse hilite tracking.  We use hilite
			 * tracking if we are not on the modeline.
			 * When hilite tracking is used, apparently the up event is
			 * elided if it would report no change in location.
			 */
			bool	use_hilite = !mode_mode;

			if (use_hilite)
				up_expected = -1;	/* half-expect an up */
			hl_mode(use_hilite,
				(estartx = x_coord)+1,
				(estarty = y_coord)+1,
				y_coord + wind_pos - curwind->w_height + 2,
				y_coord + wind_pos + 1);
		}
	} else if (!mode_mode) {
		/* can only stay within the window, and cannot switch into modeline */
		wind_pos = SelectWind(oldwind);
	}

	if (mode_mode) {
		/* An event in mode_mode only scrolls or resizes, with no
		 * action (unless it were a first down event, when it might
		 * have caused a new window to be selected).  Shifts and
		 * multiclicks indicate a confused user.
		 */
		if ((but_state & JT_BUTMASK) == JT_LEFT
		&& (but_state & JT_EVENTMASK) != JT_UPEVENT)
		{
			ScrollToMouse();
			if (but_state & (JT_CSMASK | JT_CLICKMASK)) {
				complain("[You are just scrolling a window]");
				/* NOTREACHED */
			}
		} else if ((but_state & JT_BUTMASK) == JT_MIDDLE
		&& (but_state & (JT_UPEVENT | JT_DRAGEVENT)) != 0)
		{
			oldwind = curwind;
			if ((wind_pos = SelectWind((Window *) NULL)) < 0)
				return NO;

			if (curwind == oldwind->w_next) {
				wind_pos -= curwind->w_height;
				SetWind(oldwind);
			}
			if (curwind == oldwind && curwind->w_next != fwind)
				WindSize(curwind->w_next, wind_pos);
			if (but_state & (JT_CSMASK | JT_CLICKMASK)) {
				complain("[You are just resizing a window]");
				/* NOTREACHED */
			}
		}
	} else if ((but_state & JT_PASTEMASK) && (but_state & JT_CSMASK) == 0) {
		/* With JT_PASTE/CUT, window switching is allowed. */
		return YES;
	} else if (curwind != oldwind) {
		/* Clicking within a different window (as opposed to its
		 * mode line) only switches to that window, with no action.
		 * Other shifts and multiclicks indicate a confused user.
		 */
		if (but_state & (JT_CSMASK | JT_CLICKMASK)) {
			complain("[You were just changing windows]");
			/* NOTREACHED */
		}
	} else if (wind_pos <= 0) {
		/* ignore out-of-window events */
	} else	{
		/* We've run the gauntlet.  Give the OK for action. */
		return YES;
	}
	last_mouse_act = LMA_NONE;
	if (oldpos != NULL) {
		SetWind(oldwind);
		DelMark(oldpos);
		oldpos = NULL;
	}
	return NO;
}

private void
MousePoint(mproto)
int	mproto;
{
	last_mouse_act = LMA_NONE;
	if (MouseParams(mproto))
		SetCursor();
}

void
xjMousePoint()
{
	MousePoint(MPROTO_JOVETOOL);
}

void
xtMousePoint()
{
	MousePoint(MPROTO_XTERM);
}

private void
MouseMark(mproto)
int	mproto;
{
	last_mouse_act = LMA_NONE;
	if (MouseParams(mproto)) {
		SetCursor();
		set_mark();
	}
}

void
xjMouseMark()
{
	MouseMark(MPROTO_JOVETOOL);
}

void
xtMouseMark()
{
	MouseMark(MPROTO_XTERM);
}

/* xtMouseYank is a reduced xtMousePointYank.
 * Although xtMousePointYank should be more useful,
 * xtMouseYank is more like XTerm's native behavior.
 */
void
xtMouseYank()
{
	last_mouse_act = LMA_NONE;
	if (MouseParams(MPROTO_XTERM)) {
		ObeyProc(Yank);
		this_cmd = MOUSE_CMD;
	}
}

void
xtMousePointYank()
{
	last_mouse_act = LMA_NONE;
	if (MouseParams(MPROTO_XTERM)) {
		SetCursor();
		ObeyProc(Yank);
		this_cmd = MOUSE_CMD;
	}
}

void
xtMouseCutPointYank()
{
	Mark *m;

	last_mouse_act = LMA_NONE;
	if (MouseParams(MPROTO_XTERM) && ObeyProc(set_mark)) {
		SetCursor();
		set_mark();
		m = curmark;
		PopMark();	/* pop our 2nd set_mark */
		PopMark();	/* pop our 1st set_mark */
		DelReg();	/* because we are (usually) about to
				   yank the same region again */
		ToMark(m);
		Yank();
		this_cmd = MOUSE_CMD;
	}
}

void
xtMouseNull()
{
	MouseParams(MPROTO_XTERM);
}

/* Double clicking selects either an identifier (according to the current
 * major mode) or, if the current char is not part of an identifier, it
 * selects the gap between two identifiers.  However, the selection is always
 * within one line.  startMouseWord and endMouseWord are called to locate the
 * two ends of the selected (un)identifier.  Note that the selection can be
 * empty.
 */
private void
startMouseWord()
{
	bool	in_id;

	if (eolp() && !bolp())
		curchar -= 1;
	in_id = jisident(linebuf[curchar]);
	while (!bolp() && jisident(linebuf[curchar-1]) == in_id)
		curchar -= 1;
}

private void
endMouseWord()
{
	bool	in_id;

	if (eolp() && !bolp())
		curchar -= 1;
	in_id = jisident(linebuf[curchar]);
	while (!eolp() && jisident(linebuf[curchar]) == in_id)
		curchar += 1;
}

private void
doMouseWord()
{
	startMouseWord();
	set_mark();
	endMouseWord();
}

private void
doMouseLine()
{
	Bol();
	set_mark();
	line_move(FORWARD, 1, NO);
}

/*
 * This command extends the region, at either end, in units of chars, words,
 * or lines depending on last_mouse_act (i.e. on whether the region was
 * selected by dragging, single or double clicking).  If used to shrink the
 * region, it is the point end (not the mark end) that gets moved.
 */
private bool
doMouseExtend()
{
	bool	region_forward, new_forward;

	if (last_mouse_act == LMA_NONE)
		return NO;	/* treat as xtMouseNull */

	region_forward = !inorder(curline, curchar, curmark->m_line, curmark->m_char);
	ExchPtMark();	/* for better effect when shrinking region */
	set_mark();
	SetCursor();
	new_forward = !inorder(curline, curchar, curmark->m_line, curmark->m_char);

	switch (last_mouse_act) {
	case LMA_CHAR:
		if (region_forward != new_forward) {
			PopMark();
			SetCursor();
		}
		break;
	case LMA_WORD:
		if (region_forward != new_forward) {
			PopMark();
			SetCursor();
		}
		if (new_forward)
			endMouseWord();
		else
			startMouseWord();
		break;
	case LMA_LINE:
		if (region_forward != new_forward) {
			PopMark();
			SetCursor();
		}
		Bol();
		if (new_forward)
			line_move(FORWARD, 1, NO);
		break;
	}
	return !(curmark->m_line==curline && curmark->m_char==curchar);
}

/* This command is intended to be bound to ^[ [ t and ^[ [ T, which
 * are the button UP events for the left button in Hilite mode.
 * It is not clear whether or not these codes should be produced
 * after a simple click, so the following code plays safe.
 *
 * Should a multiclick select what XTerm shows ("visual fidelity"),
 * thus using XTerm's definition of "word", or should it select based
 * on JOVE's version of "word"?  To select the latter, define XTJOVEWORD.
 *
 * This choice also affects how JOVE decides at which end of the selection
 * "point" should be placed.
 *
 * - When JOVE uses its own definition of word or line (they are of a piece)
 *   it selects the current unit, and then, if necessary, extends to envelop
 *   the current mouse position.  If no extension is needed the point will be
 *   at the right.  If extension is needed (usually because a drag was used)
 *   point will be at the far end of the extension.  This sounds complicated,
 *   but feels quite nice.
 *
 * - When JOVE uses XTerm's selection range, it places point at the end
 *   closest to the final mouse position.  This sounds simple, but it may
 *   be slightly surprising when no drag was done.
 */
void
xtMouseMarkDragPointCopy()
{
	if (MouseParams(MPROTO_XTDRAG)) {
		/* assert((but_state & JT_EVENTMASK) == JT_DRAGEVENT) */
		if ((but_state & JT_CLICKMASK) == 0) {
			/* simple (character-based) drag (from point to mouse) */
			set_mark();
			SetCursor();
			CopyRegion();
			last_mouse_act = LMA_CHAR;
#ifdef XTJOVEWORD
		} else if ((but_state & JT_CLICKMASK) == JT_CLICK2) {
			/* select based on JOVE's notion of words */
			doMouseWord();
			last_mouse_act = LMA_WORD;
			if (doMouseExtend())
				CopyRegion();

		} else /* ((but_state & JT_CLICKMASK) == JT_CLICK3) */ {
			/* select lines */
			if (last_mouse_act == LMA_WORD) {
				/* Second click was not missed.
				 * Avoid nasty mark and kill buildup.
				 */
				PopMark();
				DelKillRing();
			}
			doMouseLine();
			last_mouse_act = LMA_LINE;
			if (doMouseExtend())
				CopyRegion();
		}
#else
		} else {
			/* select based on xterm's selection: startxy through endxy
			 * Set point to whichever is closer to mouse position
			 * and set mark to farther.
			 */
			int		which_end;	/* distance difference: negative if closer to start */

			if (last_mouse_act != LMA_NONE) {
				/* avoid nasty mark and kill buildup */
				PopMark();
				DelKillRing();
			}
			which_end = (y_coord - starty) - (endy - y_coord);
			if (which_end == 0)
				which_end = (x_coord - startx) - (endx - x_coord);
			x_coord = startx;
			y_coord = starty;
			SetCursor();
			set_mark();
			x_coord = endx;
			y_coord = endy;
			SetCursor();
			if (which_end < 0)
				ExchPtMark();
			CopyRegion();
			last_mouse_act = last_mouse_act == LMA_WORD? LMA_LINE : LMA_WORD;
		}
#endif
	}
}

/* Extend the region to the pointed-at character, word or line
 * (depending on last_mouse_act).  Only valid after an event which
 * selects and copies a region (e.g. xtMouseMarkDragPointCop).
 */
void
xtMouseExtend()
{
	if (MouseParams(MPROTO_XTERM)) {
		if (doMouseExtend()) {
			DelKillRing();
			CopyRegion();
		}
	}
}

/* undo effect of preceding MouseCopyCut, if any */
private void
MouseUndo()
{
	if (last_mouse_act & LMA_PASTE) {
		ObeyProc(DelReg);	/* at old curwind/line/char */
		this_cmd = MOUSE_CMD;
		PopMark();
	}
	SelectWind((Window *)NULL);
	switch (last_mouse_act & ~LMA_PASTE) {
	case LMA_CUT:
		ToMark(CurMark());
		Yank();
		this_cmd = MOUSE_CMD;
		PopMark();
		/*FALLTHROUGH*/
	case LMA_COPY:
		/* remove the entry we previously put in the kill-ring */
		ToMark(CurMark());
		DelKillRing();
	}
	PopMark();
	last_mouse_act = LMA_NONE;
}

void
xjMouseWord()
{
	if (MouseParams(MPROTO_JOVETOOL)) {
		MouseUndo();
		doMouseWord();
	}
}

void
xjMouseLine()
{
	if (MouseParams(MPROTO_JOVETOOL)) {
		MouseUndo();
		SetCursor();
		doMouseLine();
	}
}

void
xjMouseYank()
{
	last_mouse_act = LMA_NONE;
	if (MouseParams(MPROTO_JOVETOOL)) {
		SetCursor();
		if (but_state & JT_CONTROL)
			ObeyProc(Yank);
	}
}

void
xjMouseCopyCut()
{
	register Mark	*mp = curmark;

	if (MouseParams(MPROTO_JOVETOOL)) {
		if (mp!=NULL
		&& (mp->m_line != curline || mp->m_char != curchar))
		{
			switch(but_state & (JT_CSMASK | JT_PASTEMASK)) {
			case JT_CONTROL:
				CopyRegion();
				last_mouse_act = LMA_COPY;
				break;
			case JT_CONTROL | JT_SHIFT:
				last_mouse_act = ObeyProc(DelReg)? LMA_CUT : LMA_NONE;
				break;
			case JT_PASTE:
				CopyRegion();
				SetWind(oldwind);
				ToMark(oldpos);
				ObeyProc(Yank);
				DelMark(oldpos);
				oldpos = NULL;
				last_mouse_act = LMA_COPY | LMA_PASTE;
				break;
			case JT_CUT:
				ObeyProc(DelReg);
				SetWind(oldwind);
				ToMark(oldpos);
				ObeyProc(Yank);
				DelMark(oldpos);
				oldpos = NULL;
				last_mouse_act = LMA_CUT | LMA_PASTE;
				break;
			/* default: ignore */
			}
		} else if (but_state & JT_PASTEMASK) {
			SetWind(oldwind);
			ToMark(oldpos);
			DelMark(oldpos);
			oldpos = NULL;
		}
	}
}

#endif /* MOUSE */
