/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

#ifdef MOUSE	/* the body is the rest of this file */

extern void
	MouseOn proto((void)),
	MouseOff proto((void)),

	xjMousePoint proto((void)),
	xjMouseMark proto((void)),
	xjMouseWord proto((void)),
	xjMouseLine proto((void)),
	xjMouseYank proto((void)),
	xjMouseCopyCut proto((void)),

	xtMouseYank proto((void)),
	xtMousePointYank proto((void)),
	xtMouseCutPointYank proto((void)),
	xtMouseExtend proto((void)),
	xtMouseMark proto((void)),
	xtMouseMarkDragPointCopy proto((void)),
	xtMouseNull proto((void)),
	xtMousePoint proto((void));

extern bool	XtermMouse;	/* VAR: should we enable xterm mouse? */

#endif /* MOUSE */
