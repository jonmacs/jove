/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

#define makedirty(line)	{ (line)->l_dline |= DDIRTY; }
#define isdirty(line)	((line)->l_dline & DDIRTY)

typedef DADDR jwid_t;	/* DADDR must be >= max(sizeof(daddr),sizeof(Buffer *)) */

struct scrimage {
	int	s_offset,	/* offset to start printing at */
		s_flags;	/* various flags */
	long	s_vln;		/* Visible Line Number */
	jwid_t	s_id;		/* unique identifier for line */
	LinePtr	s_lp;		/* line to display (if any) */
	Window	*s_window;	/* window that contains this line */
};

/* s_flags flags */
#define s_DIRTY		01		/* buffer line changed since last displayed */
#define s_MODELINE	02		/* this is a modeline */
#define s_L_MOD		04		/* line has been modified internally */

extern struct scrimage
	*DesiredScreen,		/* what we want */
	*PhysScreen;		/* what we got */

extern bool	UpdMesg;	/* update the message line */

extern bool
	chkmail proto((bool force));

extern int
	calc_pos proto((char *lp,int c_char));

#define MAX_TYPEOUT	MAXCOLS	/* maximum width of typout (in chars) */

extern void
	ChkWindows proto((LinePtr line1,LinePtr line2)),
	ChkWinLines proto((void)),
	DrawMesg proto((bool abortable)),
	message proto((const char *)),
	TOstart proto((const char *name)),
	TOstop proto((void)),
	Typeout proto((const char *, ...)),
	rbell proto((void)),
	redisplay proto((void));


#ifdef WINRESIZE
extern volatile bool
	ResizePending;	/* asynch request for screen resize */
# ifdef WIN32
extern void ResizeWindow proto((void));
# endif
#endif

extern bool
	DisabledRedisplay;

#ifdef ID_CHAR
extern bool
	IN_INSmode;

extern void
	INSmode proto((bool));
#endif /* ID_CHAR */


/* Variables: */

extern bool	BriteMode;		/* VAR: make the mode line inverse? */
#ifdef UNIX
extern int	MailInt;		/* VAR: mail check interval */
extern char	Mailbox[FILESIZE];	/* VAR: mailbox name */
#endif /* UNIX */
extern char	ModeFmt[MAXCOLS];	/* VAR: mode line format string */
extern bool	ScrollAll;		/* VAR: when current line scrolls, scroll whole window? */
extern int	ScrollWidth;		/* VAR: unit of horizontal scrolling */
extern bool	UseBuffers;		/* VAR: use buffers with Typeout() */
#ifdef ID_CHAR
extern bool	UseIC;			/* VAR: whether or not to use i/d char processesing */
#endif
extern bool	VisBell;		/* VAR: use visible bell (if possible) */
extern bool	MarkHighlighting;	/* VAR: highlight mark when visible */

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
