/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

#ifdef MOUSE	/* the body is the rest of this file */

extern void
	MouseOn(void),
	MouseOff(void),

	xjMousePoint(void),
	xjMouseMark(void),
	xjMouseWord(void),
	xjMouseLine(void),
	xjMouseYank(void),
	xjMouseCopyCut(void),

	xtMouseYank(void),
	xtMousePointYank(void),
	xtMouseCutPointYank(void),
	xtMouseExtend(void),
	xtMouseMark(void),
	xtMouseMarkDragPointCopy(void),
	xtMouseNull(void),
	xtMousePoint(void);

extern jbool	XtermMouse;	/* VAR: should we enable xterm mouse? */

#endif /* MOUSE */
