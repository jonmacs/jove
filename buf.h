/***************************************************************************
 * This program is Copyright (C) 1986, 1987, 1988 by Jonathan Payne.  JOVE *
 * is provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is    *
 * included in all the files.                                              *
 ***************************************************************************/

/* maximum length of a line (including '\0').  Currently cannot
   be larger than a logical disk block. */
#define	LBSIZE		JBUFSIZ

/* buffer types */
#define B_SCRATCH	1	/* for internal things, e.g. minibuffer ... */
#define B_FILE		2	/* normal file (we auto-save these.) */
#define B_PROCESS	3	/* unix process output in this buffer */

/* major modes */
#define FUNDAMENTAL	0	/* Fundamental mode */
#define TEXT		1	/* Text mode */
#define CMODE		2	/* C mode */
#ifdef	LISP
# define LISPMODE	3	/* Lisp mode */
# define NMAJORS	4
#else
# define NMAJORS	3
#endif

#define MajorMode(x)	(curbuf->b_major == (x))
#define SetMajor(x)	{ curbuf->b_major = (x); UpdModLine = YES; }

/* minor modes */
#define Indent		(1 << 0)	/* indent same as previous line after return */
#define ShowMatch	(1 << 1)	/* paren flash mode */
#define Fill		(1 << 2)	/* text fill mode */
#define OverWrite	(1 << 3)	/* over write mode */
#define Abbrev		(1 << 4)	/* abbrev mode */
#define ReadOnly	(1 << 5)	/* buffer is read only */

#define BufMinorMode(b, x)	(((b)->b_minor & (x)) != 0)
#define MinorMode(x)		BufMinorMode(curbuf, (x))

/* global line scratch buffers */
#ifdef	pdp11
extern char	*genbuf,	/* scratch pad points at s_genbuf (see main()) */
		*linebuf,	/* points at s_linebuf */
		*iobuff;	/* for file reading ... points at s_iobuff */
#else
extern char	genbuf[LBSIZE],
		linebuf[LBSIZE],
		iobuff[LBSIZE];
#endif

/* typedef struct line Line in jove.h */

struct line {
	Line	*l_prev,		/* pointer to prev */
		*l_next;		/* pointer to next */
	daddr	l_dline;		/* pointer to disk location */
};

/* typedef struct mark Mark in jove.h */

struct mark {
	Line	*m_line;
	int	m_char;
	Mark	*m_next;	/* list of marks */
#define M_FIXED		00
#define M_FLOATER	01
#define M_BIG_DELETE	02
	char	m_flags;	/* FLOATERing mark? */
};

/* typedef struct buffer Buffer in jove.h */

#ifdef ZORTECH
/* FUDGE FUDGE FUDGE */
typedef short	dev_t;
typedef short	ino_t;
#endif

struct buffer {
#ifdef	MAC
	int Type;		/* kludge... to look like a data_obj */
	char *Name;		/* Name will not be used */
#endif
	Buffer	*b_next;		/* next buffer in chain */
	char	*b_name,		/* buffer name */
		*b_fname;		/* file name associated with buffer */
	dev_t	b_dev;			/* device of file name. */
	ino_t	b_ino;			/* inode of file name */
	time_t	b_mtime;		/* last modify time ...
					   to detect two people writing
					   to the same file */
	Line	*b_first,		/* pointer to first line in list */
		*b_dot,			/* current line */
		*b_last;		/* last line in list */
	int	b_char;			/* current character in line */

#define NMARKS	8			/* number of marks in the ring */

	Mark	*b_markring[NMARKS],	/* new marks are pushed here */
#define b_curmark(b)	((b)->b_markring[(b)->b_themark])
#define curmark		b_curmark(curbuf)
		*b_marks;		/* all the marks for this buffer */
	char	b_themark,		/* current mark (in b_markring) */
		b_type,			/* file, scratch, process, iprocess */
		b_ntbf,			/* (bool) needs to be found when we
					   first select? */
		b_modified;		/* (bool) is the buffer modified? */
	int	b_major,		/* major mode */
		b_minor;		/* and minor mode */
	struct keymap	*b_map;		/* local bindings (if any) */
#ifdef	IPROCS
	Process	*b_process;		/* process we're attached to */
#endif
};

extern Buffer
	*world,		/* first buffer */
	*curbuf,	/* pointer into world for current buffer */
	*lastbuf,	/* Last buffer we were in so we have a default
			   buffer during a select buffer. */
	*perr_buf;	/* Buffer with error messages */

#define curline	(curbuf->b_dot)
#define curchar (curbuf->b_char)

/* typedef struct position Bufpos in jove.h */

struct position {
	Line	*p_line;
	int	p_char;
};

extern bool
	valid_bp proto((Buffer	*bp));

extern Buffer
	*buf_exists proto((char *name)),
	*do_find proto((struct window *w, char *fname, bool force)),
	*do_select proto((struct window *w,char *name)),
	*file_exists proto((char *name));

extern char
	*ask_buf proto((struct buffer *def));

#ifdef	USE_PROTOTYPES
struct macro;	/* forward declaration preventing prototype scoping */
#endif	/* USE_PROTOTYPES */

extern void
	TogMinor proto((int bit)),
	initlist proto((struct buffer *b)),
	setfname proto((struct buffer *b,char *name)),
	set_ino proto((struct buffer *b)),
	SetABuf proto((struct buffer *b)),
	SetBuf proto((struct buffer *newbuf)),
	buf_init proto((void));

extern  struct line
	*lastline proto((struct line *lp)),
	*listput proto((struct buffer *buf,struct line *after)),
	*next_line proto((struct line *line,int num)),
	*prev_line proto((struct line *line,int num));

/* Commands: */

extern void
	BufErase proto((void)),
	BufKill proto((void)),
	BufList proto((void)),
	BufSelect proto((void)),
	FindFile proto((void)),
	KillSome proto((void)),
	ReNamBuf proto((void));

#ifdef	MSDOS
extern void
	Buf1Select proto((void)),
	Buf2Select proto((void)),
	Buf3Select proto((void)),
	Buf4Select proto((void)),
	Buf5Select proto((void)),
	Buf6Select proto((void)),
	Buf7Select proto((void)),
	Buf8Select proto((void)),
	Buf9Select proto((void)),
	Buf10Select proto((void));
#endif	/* MSDOS */
