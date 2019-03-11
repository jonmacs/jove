/************************************************************************
 * This program is Copyright (C) 1986-1994 by Jonathan Payne.  JOVE is  *
 * provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is *
 * included in all the files.                                           *
 ************************************************************************/

/***************************************************************************
 * This program performs all functions associated with mouse buttons being *
 * pressed and any movement of the mouse. These  procedures  will  only be *
 * called if JOVE is run under Jovetool or xJove.                          *
 ***************************************************************************/

#include "jove.h"

#ifdef MOUSE	/* the body is the rest of this file */

/* #include "argcount.h" */
#include "disp.h"
#include "termcap.h"
#include "misc.h"
#include "ask.h"
#include "delete.h"
#include "insert.h"	/* for lfreelist, in MouseLine */
#include "marks.h"
#include "move.h"
#include "wind.h"
#include "term.h"	/* for putpad */
#include "jctype.h"
#include "mouse.h"
#include "xjove/mousemsg.h"

bool XtermMouse = NO;	/* VAR: should we enable xterm mouse? */
private bool	xtMouseState = NO;	/* have we enabled the mouse? */

private const char
	xtMouseEnable[] = "\033[?1000h",	/* enable/disable mouse in xterm */
	xtMouseDisable[] = "\033[?1000l";

private int	but_state;	/* button state (and more) at mouse event */
private int	x_coord;	/* mouse x-coordinate, in pixels, origin 0 */
private int	y_coord;	/* mouse y-coordinate, in characters, origin 0 */
private int	font_width;	/* width of a character in pixels */
private Window	*oldwind;
private Mark	*oldpos;	/* Use a Mark so it gets adjusted if stuff is
				 * deleted */

#define	LMA_NONE	000
#define	LMA_COPY	001
#define	LMA_CUT		002
#define LMA_PASTE	010
private int last_mouse_act = LMA_NONE;

void
MouseOn()
{
	if (XtermMouse != xtMouseState) {
		putpad(XtermMouse? xtMouseEnable : xtMouseDisable, 1);
		xtMouseState = XtermMouse;
	}
}

void
MouseOff()
{
	if (xtMouseState) {
		putpad(xtMouseDisable, 1);
		xtMouseState = NO;
	}
}

/* Set cursor position to that of mouse pointer. */

private void
SetCursor()
{
	int	line_pos = in_window(curwind, curline);
	int	num_moves = y_coord - line_pos;

	if (num_moves > 0)
		line_move(FORWARD, num_moves, YES);
	if (num_moves < 0)
		line_move(BACKWARD, -num_moves, YES);

	curchar = how_far(curline,
		(x_coord + font_width/2) / font_width + PhysScreen[y_coord].s_offset
		- W_NUMWIDTH(curwind));
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
void (*p) ptrproto((void));
{
	if (BufMinorMode(curbuf, ReadOnly)) {
		rbell();
		message("[Buffer is read-only]");
		return NO;
	} else {
		(*p)();
		return YES;
	}
}

private bool	Slow;

/* Get next value in number sequence */

private int
NextValue()
{
	int val;

	(void) waitchar(&Slow);
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

#define	MPROTO_XTERM	0
#define	MPROTO_JOVETOOL	1

private bool
MouseParams(mproto)
int	mproto;
{
	/* The mouse commands will be invoked by hitting a mouse
	 * button but can also be invoked by typing the escape sequence
	 * CTRL-Xm so the following validation checks are necessary.
	 */

	static int	wind_pos;		/* reverse y-coordinate within window */
	static bool	button_held = NO;	/* true while button held down */
	static bool	mode_mode = NO;		/* true while doing modeline */
	bool	input_good = NO;

	Slow = NO;
	switch (mproto) {
	case MPROTO_XTERM:
		{
			ZXchar	cb = LastKeyStruck;
			ZXchar	cx = waitchar(&Slow);

			if (' ' <= cb && cb < ' '+32 && ' ' < cx && cx <= ' '+CO) {
				ZXchar	cy = waitchar(&Slow);

				if (' ' < cy && cy <= ' '+LI) {
					/* Encode button event to simulate xjove.
					 * There is no way to detect multiclicks.
					 * On a release event we aren't told which button
					 * is being released.
					 * Currently, we don't encode the meta key (but since
					 * it is part of the character used to select the command,
					 * it can have an affect).
					 */
					cb -= ' ';
					if ((cb & 03) == 03) {
						/* guess that released button is last depressed */
						but_state = (but_state & JT_BUTMASK) | JT_UPEVENT;
					} else {
						static const int	butcode[] = { JT_LEFT, JT_MIDDLE, JT_RIGHT };

						but_state = butcode[cb & 03] | JT_DOWNEVENT;
					}
					if (cb & 04)
						but_state |= JT_SHIFT;
#ifdef NEVER
					if (cb & 010)
						but_state |= JT_META;
#endif
					if (cb & 020)
						but_state |= JT_CONTROL;

					input_good = YES;
					x_coord = cx - '!';
					y_coord = cy -'!';
					font_width = 1;
				}
			}
		}
		break;
	case MPROTO_JOVETOOL:
		if (waitchar(&Slow) == '(') {
			but_state = NextValue();	/* Read in parameters */
			switch (waitchar(&Slow)) {
			case ' ':
				{
					int x = NextValue();

					if (waitchar(&Slow) == ' ') {
						int y = NextValue();

						if (waitchar(&Slow) == ' ') {
							font_width = NextValue();
							if (waitchar(&Slow) == ')'
							&& waitchar(&Slow) == '\r')
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
				input_good = (waitchar(&Slow) == '\r');
				break;
			}
		}
		break;
	}
	this_cmd = OTHER_CMD;	/* no longer gathering args */

	if (!input_good)
		complain("[mouse input of wrong format]");

	/* Note: at this point, button_held still reflects previous state */

	/* check: button_(was)_held iff currently event is UP or DRAG */
	if (button_held == ((but_state & (JT_UPEVENT | JT_DRAGEVENT))==0))
		complain("[Mouse events out of order]");

	/* update button_held to reflect new state */
	button_held = (but_state & (JT_UPEVENT|JT_DRAGEVENT)) != JT_UPEVENT;

	if ((but_state & JT_EVENTMASK) == JT_DOWNEVENT
		&& (but_state & JT_PASTEMASK) && (but_state & JT_CSMASK) == 0) {
		oldpos = MakeMark(curline, curchar);
	}

	if ((but_state & (JT_CLICKMASK | JT_EVENTMASK)) == JT_DOWNEVENT) {
		/* button down, and first click at that:
		 * - ok to changing window
		 * - ok to move in or out of modeline (scroll bar)
		 */
		oldwind = curwind;
		wind_pos = SelectWind((Window *) NULL);
		mode_mode = (wind_pos == 0);
	} else if (!mode_mode) {
		/* can only stay within the window, and cannot switch into modeline */
		wind_pos = SelectWind(oldwind);
	}

	if (mode_mode) {
		/* An event in mode_mode only scrolls, with no action (if it were a
		 * first down event, it might have caused a new window to be selected).
		 * Shifts and multiclicks indicate a confused user.
		 */
		ScrollToMouse();
		if (but_state & (JT_CSMASK | JT_CLICKMASK))
			complain("[You are just scrolling a window]");
	} else if ((but_state & JT_PASTEMASK) && (but_state & JT_CSMASK) == 0) {
		/* With JT_PASTE/CUT, window switching is allowed. */
		return YES;
	} else if (wind_pos <= 0) {
		/* ignore out-of-window events */
	} else if (curwind != oldwind) {
		/* Clicking within a different window (as opposed to its
		 * mode line) only switches to that window, with no action.
		 * Other shifts and multiclicks indicate a confused user.
		 */
		if (but_state & (JT_CSMASK | JT_CLICKMASK))
			complain("[You were just changing windows]");
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
		SetMark();
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

/* undo effect of preceding MouseCopyCut, if any */

private void
MouseUndo()
{
	if (last_mouse_act & LMA_PASTE) {
		ObeyProc(DelReg);	/* at old curwind/line/char */
		PopMark();
	}
	SelectWind((Window *)NULL);
	switch (last_mouse_act & ~LMA_PASTE) {
	case LMA_CUT:
		ToMark(CurMark());
		Yank();
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

/* Double clicking selects either an identifier (according to the current
 * major mode) or, if the current char is not part of an identifier, it
 * selects the gap between two identifiers. However, the selection is always
 * within one line. do_ident is called twice to locate the two ends of the
 * selected (un)identifier.  Note that the selection can be empty.
 */

void
xjMouseWord()
{
	if (MouseParams(MPROTO_JOVETOOL)) {
		bool	in_id;

		MouseUndo();
		if (eolp() && !bolp())
			curchar -= 1;
		in_id = jisident(linebuf[curchar]);
		while (!bolp() && jisident(linebuf[curchar-1]) == in_id)
			curchar -= 1;
		SetMark();
		while (!eolp() && jisident(linebuf[curchar]) == in_id)
			curchar += 1;
	}
}

void
xjMouseLine()
{
	if (MouseParams(MPROTO_JOVETOOL)) {
		MouseUndo();
		SetCursor();
		Bol();
		SetMark();
		NextLine();
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
		&& (mp->m_line != curline || mp->m_char != curchar)) {
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

void
xtMouseUp()
{
	if (MouseParams(MPROTO_XTERM)) {
		SetCursor();
		switch (but_state) {
		case JT_LEFT | JT_CONTROL:
			ObeyProc(Yank);
			break;
		case JT_MIDDLE | JT_CONTROL:
			CopyRegion();
			last_mouse_act = LMA_COPY;
			break;
		case JT_MIDDLE | JT_CONTROL | JT_SHIFT:
			last_mouse_act = ObeyProc(DelReg)? LMA_CUT : LMA_NONE;
			break;
		}
	}
}

#endif /* MOUSE */
