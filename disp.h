/***************************************************************************
 * This program is Copyright (C) 1986, 1987, 1988 by Jonathan Payne.  JOVE *
 * is provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is    *
 * included in all the files.                                              *
 ***************************************************************************/

#define DIRTY		((daddr) 01)	/* needs update for some reason */
#define MODELINE	02		/* this is a modeline */
#define L_MOD		04		/* line has been modified internally */

#define makedirty(line)	{ (line)->l_dline |= DIRTY; }
#define isdirty(line)	((line)->l_dline & DIRTY)

struct scrimage {
	int	s_offset,	/* offset to start printing at */
		s_flags,	/* various flags */
		s_vln;		/* Visible Line Number */
	daddr	s_id;		/* which buffer line */
	Line	*s_lp;		/* so we can turn off red bit */
	Window	*s_window;	/* window that contains this line */
};

extern struct scrimage
	*DesiredScreen,		/* what we want */
	*PhysScreen;		/* what we got */

extern bool	UpdMesg;	/* update the message line */

extern int
	chkmail proto((int force)),
	calc_pos proto((char *lp,int c_char));

extern void
	disp_opt_init proto((void)),
	ChkWindows proto((struct line *line1,struct line *line2)),
	ChkWinLines proto((void)),
	DrawMesg proto((bool abortable)),
	message proto((char *)),
	TOstart proto((char *name, bool auto_newline)),
	TOstop proto((void)),
	Typeout proto((char *, ...)),
	rbell proto((void)),
	redisplay proto((void));


extern int
	DisabledRedisplay;

#ifdef	ID_CHAR
extern bool
	IN_INSmode;

extern int
	IMlen;

extern void
	INSmode proto((bool));
#endif	/* ID_CHAR */


/* Variables: */

#ifdef	BIFF
extern bool	BiffChk;		/* turn off/on biff with entering/exiting jove */
#endif
extern bool	BriteMode;		/* make the mode line inverse? */
extern int	MailInt;		/* mail check interval */
#ifdef	UNIX
extern char	Mailbox[FILESIZE];	/* mailbox name */
#endif	/* UNIX */
extern char	ModeFmt[120];		/* mode line format string */
extern bool	ScrollAll;		/* we current line scrolls, scroll whole window? */
extern bool	UseBuffers;		/* use buffers with Typeout() */
#ifdef	ID_CHAR
extern bool	UseIC;			/* whether or not to use i/d char processesing */
#endif
extern bool	VisBell;		/* use visible bell (if possible) */

/* Commands: */
extern void
	Bow proto((void)),
	ClAndRedraw proto((void)),
	DownScroll proto((void)),
	Eow proto((void)),
	NextPage proto((void)),
	PrevPage proto((void)),
	RedrawDisplay proto((void)),
	UpScroll proto((void));

#ifdef	MSDOS
extern void
	PageScrollUp proto((void)),
	PageScrollDown proto((void));
#endif	/* MSDOS */
