/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

/* typedef struct window Window in jove.h */

struct window {
	Window
		*w_prev,	/* circular list */
		*w_next;
	Buffer	*w_bufp;	/* buffer associated with this window */
	LinePtr
		w_top,		/* top line */
		w_line;		/* current line */
	long	w_topnum;	/* buffer line number of the topline */
	int
		w_char,
		w_height,	/* window height (including mode line) */
		w_dotcol,	/* screen column of w_line (set by UpdWindow) */
		w_dotline,	/* screen line number of w_line (set by UpdWindow) */
		w_flags,
#define	W_TOPGONE	01
#define	W_CURGONE	02	/* topline (curline) of window has been deleted
				   since the last time a redisplay was called */
#define W_VISSPACE	04
#define W_NUMLINES	010
		w_LRscroll;	/* amount of LeftRight scrolling in window */
#ifdef MAC
	int	w_topline;	/* row number of top line in window */
	/* Note: "ControlHandle w_control" would require more includes */
	struct ControlRecord **w_control;	/* scroll bar for window */
#endif
};

#define W_NUMWIDTH(w)	(((w)->w_flags & W_NUMLINES)? 8 : 0)
#define SIWIDTH(off)	((off) != 0? 1 : 0)	/* width of shift indicator, if any */

extern Window
	*fwind,		/* first window in list */
	*curwind;	/* current window */

#define one_windp()	(fwind->w_next == fwind)
#define WSIZE(wp)	((wp)->w_height - 1)	/* window lines, without modeline */

extern int
	in_window(Window *windes,LinePtr line);

extern Window
	*div_wind(Window *wp,int n),
	*windbp(Buffer *bp);

extern void
	CalcWind(Window *w),
	CentWind(Window *w),
	SetTop(Window *w,LinePtr line),
	SetWind(Window *new),
	WindSize(Window *w,int inc),
	del_wind(Window *wp),
	pop_wind(const char *name, jbool clobber, int btype),
	tiewind(Window *w, Buffer *bp),	/* util.c!! */
	winit(void);

/* Commands: */

extern void
	DelCurWindow(void),
	GotoWind(void),
	GrowWindowCmd(void),
	NextWindow(void),
	OneWindow(void),
	PageNWind(void),
	PrevWindow(void),
	ScrollLeft(void),
	ScrollRight(void),
	ShrWindow(void),
	SplitWind(void),
	WNumLines(void),
	WVisSpace(void),
	WindFind(void);

/* Variables: */

extern int	ScrollStep;		/* VAR: how should we scroll */

#ifdef HIGHLIGHTING
extern jbool	ScrollBar;		/* VAR: whether the scrollbar is used */
#endif
