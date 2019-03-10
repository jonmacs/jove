/************************************************************************
 * This program is Copyright (C) 1986 by Jonathan Payne.  JOVE is       *
 * provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is *
 * included in all the files.                                           *
 ************************************************************************/

/* jove.h header file to be included by EVERYONE */

#include <setjmp.h>
#include <sys/types.h>

#ifndef TUNED
#   include "tune.h"
#endif

#define private	static

#ifndef BSD4_2
#   ifdef MENLO_JCL
#	ifndef EUNICE
#		define signal	sigset
#	endif
#   endif MENLO_JCL
#endif

#define EOF	-1
#define NULL	0
#define NIL	0

/* kinds of regular expression compiles */
#define NORM	0	/* nothing special */
#define OKAY_RE	1	/* allow regular expressions */
#define IN_CB	2	/* in curly brace; implies OKAY_RE */

/* return codes for command completion (all < 0 because >= 0 are
   legitimate offsets into array of strings */

#define AMBIGUOUS	-2	/* matches more than one at this point */
#define UNIQUE		-3	/* matches only one string */
#define ORIGINAL	-4	/* matches no strings at all! */
#define NULLSTRING	-5	/* just hit return without typing anything */

/* values for the `flags' argument to complete */
#define NOTHING		0	/* opposite of RET_STATE */
#define RET_STATE	1	/* return state when we hit return */
#define RCOMMAND	2	/* we are reading a joverc file */
#define CASEIND		4	/* map all to lower case */

#define DEFINE		01	/* defining this macro */
#define EXECUTE		02	/* executing this macro */
#define SAVE		04	/* this macro needs saving to a file */

#define	LBSIZE		BUFSIZ	/* same as a logical disk block */
#define FILESIZE	256

#define FORWARD		1
#define BACKWARD	-1

#define CTL(c)		('c' & 037)
#define META(c)		('c' | 0200)
#define RUBOUT		'\177'
#define LF		CTL(J)
#define CR		CTL(M)
#define BS		CTL(H)
#define ESC		'\033'

#define DoTimes(f, n)	exp_p = YES, exp = n, f
#define HALF(wp)	((wp->w_height - 1) / 2)
#define IsModified(b)	(b->b_modified)
#define SIZE(wp)	(wp->w_height - 1)
#define SavLine(a, b)	(a->l_dline = putline(b))
#define SetLine(line)	DotTo(line, 0)
#define bobp()		(firstp(curline) && bolp())
#define bolp()		(curchar == 0)
#define eobp()		(lastp(curline) && eolp())
#define eolp()		(linebuf[curchar] == '\0')
#define firstp(line)	(line == curbuf->b_first)
#define getDOT()	getline(curline->l_dline, linebuf)
#define isdirty(line)	(line->l_dline & DIRTY)
#define lastp(line)	(line == curbuf->b_last)
#define makedirty(line)	line->l_dline |= DIRTY
#define one_windp()	(fwind->w_next == fwind)

extern int	OkayAbort,	/* okay to abort redisplay */
		BufSize;

#define ARG_CMD		1
#define LINECMD		2
#define KILLCMD		3	/* so we can merge kills */
#define YANKCMD		4	/* so we can do ESC Y (yank-pop) */

/* Buffer type */

#define B_SCRATCH	1	/* for internal things, e.g. minibuffer ... */
#define B_FILE		2	/* normal file (We Auto-save these.) */
#define B_PROCESS	3	/* process output in this buffer */

/* Major modes */
#define FUNDAMENTAL	0	/* Fundamental mode */
#define TEXT		1	/* Text mode */
#define CMODE		2	/* C mode */
#ifdef LISP
#	define LISPMODE		3	/* Lisp mode */
#	define NMAJORS		4
#else
#	define NMAJORS	3
#endif

/* Minor Modes */
#define Indent		(1 << 0)	/* indent same as previous line after return */
#define ShowMatch	(1 << 1)	/* paren flash mode */
#define Fill		(1 << 2)	/* text fill mode */
#define OverWrite	(1 << 3)	/* over write mode */
#define Abbrev		(1 << 4)	/* abbrev mode */

#define BufMinorMode(b, x)	(b->b_minor & x)

#define MinorMode(x)	BufMinorMode(curbuf, x)
#define MajorMode(x)	(curbuf->b_major == x)
#define SetMajor(x)	((curbuf->b_major = x), UpdModLine++)

extern char	CharTable[NMAJORS][128];

/* setjmp/longjmp args for DoKeys() mainjmp */
#define FIRSTCALL	0
#define ERROR		1
#define COMPLAIN	2	/* do the error without a getDOT */
#define QUIT		3	/* leave this level of recursion */

#define QUIET		1	/* sure, why not? */

#define YES		1
#define NO		0
#define TRUE		1
#define FALSE		0
#define ON		1
#define OFF		0
#define YES_NODIGIT	2

extern char	*Mainbuf,
		*HomeDir,	/* home directory */
		key_strokes[],	/* strokes that make up current command */
		*Inputp;

extern int	HomeLen;	/* length of home directory */

extern char	NullStr[];

#ifdef VMUNIX
extern char	genbuf[LBSIZE],
		linebuf[LBSIZE],
		iobuff[LBSIZE];
#else
extern char	*genbuf,	/* scratch pad points at s_genbuf (see main()) */
		*linebuf,	/* points at s_linebuf */
		*iobuff;	/* for file reading ... points at s_iobuff */
#endif

extern int	InJoverc,
		Interactive;

#define	READ	0
#define	WRITE	1
extern int	errno;

extern jmp_buf	mainjmp;

#ifdef IPROCS
typedef struct process	Process;
#endif
typedef struct window	Window;
typedef struct position	Bufpos;
typedef struct mark	Mark;
typedef struct buffer	Buffer;
typedef struct line	Line;
typedef struct iobuf	IOBUF;
typedef struct data_obj {
	int	Type;
	char	*Name;
} data_obj;	/* points to cmd, macro, or variable */
typedef data_obj	*keymap[0200];

struct line {
	Line	*l_prev,		/* pointer to prev */
		*l_next;		/* pointer to next */
	disk_line	l_dline;	/* pointer to disk location */
};

#ifdef IPROCS
struct process {
	Process	*p_next;
#ifdef PIPEPROCS
	int	p_toproc,	/* read p_fromproc and write p_toproc */
		p_portpid,	/* Pid of child (the portsrv) */
		p_pid;		/* Pid of real child i.e. not portsrv */
#else
	int	p_fd,		/* File descriptor of ptyp? opened r/w */
		p_pid;		/* pid of child (the shell) */
#endif
	Buffer	*p_buffer;	/* Add output to end of this buffer */
	char	*p_name;	/* ... */
	char	p_state,	/* State */
		p_howdied,	/* Killed? or Exited? */
		p_reason,	/* If signaled, p_reason is the signal; else
				   it is the the exit code */
		p_eof;		/* Received EOF, so can be free'd up */
	Mark	*p_mark;	/* Where output left us. */
	data_obj
		*p_cmd;		/* Command to call when process dies */
};
#endif IPROCS

struct window {
	Window	*w_prev,	/* circular list */
		*w_next;
	Buffer	*w_bufp;	/* buffer associated with this window */
	Line	*w_top,		/* top line */
		*w_line;	/* current line */
	int	w_char,
		w_height,	/* window height */
		w_topnum,	/* line number of the topline */
		w_dotcol,	/* UpdWindow sets this ... */
		w_dotline,	/* ... and this */
		w_flags;
#define	W_TOPGONE	01
#define	W_CURGONE	02	/* topline (curline) of window has been deleted
				   since the last time a redisplay was called */
#define W_VISSPACE	04
#define W_NUMLINES	010
};

extern Window	*fwind,		/* first window in list */
		*curwind;	/* current window */

struct position {
	Line	*p_line;
	int	p_char;
};

struct mark {
	Line	*m_line;
	int	m_char;
	Mark	*m_next;	/* list of marks */
#define FLOATER	1
	char	m_floater;	/* FLOATERing mark? */
};

struct buffer {
	Buffer	*b_next;		/* next buffer in chain */
	char	*b_name,		/* buffer name */
		*b_fname;		/* file name associated with buffer */
	ino_t	b_ino;			/* inode of file name */
	time_t	b_mtime;		/* last modify time ...
					   to detect two people writing
					   to the same file */
	Line	*b_first,		/* pointer to first line in list */
		*b_dot,			/* current line */
		*b_last;		/* last line in list */
	int	b_char;			/* current character in line */

#define NMARKS	8			/* number of marks in the ring */

	Mark	*b_markring[NMARKS],	/* new marks are pushed saved here */
		*b_marks;		/* all the marks for this buffer */
	char	b_themark,		/* current mark (in b_markring) */
		b_type,			/* file, scratch, process, iprocess */
		b_ntbf,			/* needs to be found when we
					   first select? */
		b_modified;		/* is the buffer modified? */
	int	b_major,		/* major mode */
		b_minor;		/* and minor mode */
	keymap	*b_keybinds;		/* local bindings (if any) */
#ifdef IPROCS
	Process	*b_process;		/* process we're attached to */
#endif
};

struct macro {
	int	Type;		/* in this case a macro */
	char	*Name;		/* name is always second ... */
	int	m_len,		/* length of macro so we can use ^@ */
		m_buflen,	/* memory allocated for it */
		m_offset,	/* index into body for defining and running */
		m_flags,	/* defining/running this macro? */
		m_ntimes;	/* number of times to run this macro */
	char	*m_body;	/* actual body of the macro */
	struct macro
		*m_nextm;
};

struct variable {
	int	Type;		/* in this case a variable */
	char	*Name;		/* name is always second */
	int	*v_value,
		v_flags;
};

struct cmd {
	int	Type;
	char	*Name;
	int	(*c_proc)();
};

extern keymap	mainmap,	/* various key maps */
		pref1map,
		pref2map,
		miscmap;

extern data_obj	*LastCmd;	/* last command invoked */

extern char	*ProcFmt;

extern struct cmd	commands[];
extern struct macro	*macros;
extern struct variable	variables[];

extern struct macro
	*macstack[],
	KeyMacro;

#define FUNCTION	1
#define VARIABLE	2
#define MACRO		3
#define TYPEMASK	07
#define MAJOR_MODE	010
#define MINOR_MODE	020
#define DefMajor(x)	(FUNCTION|MAJOR_MODE|(x << 8))
#define DefMinor(x)	(FUNCTION|MINOR_MODE|(x << 8))

extern Buffer	*world,		/* first buffer */
		*curbuf;	/* pointer into world for current buffer */

#define curline	curbuf->b_dot
#define curchar curbuf->b_char

#define NUMKILLS	10	/* number of kills saved in the kill ring */

#define DIRTY		01	/* just needs updating for some reason */
#define MODELINE	02	/* this is a modeline */
#define L_MOD		04	/* this line has been modified internally */

struct scrimage {
	int	s_offset,	/* offset to start printing at */
		s_flags,	/* various flags */
		s_id,		/* which buffer line */
		s_vln;		/* Visible Line Number */
	Line	*s_lp;		/* so we can turn off red bit */
	Window	*s_window;	/* window that contains this line */
};

extern struct scrimage
	*DesiredScreen,		/* what we want */
	*PhysScreen;		/* what we got */

/* Variable flags (that can be set). */
#define V_BASE10	01	/* is integer in base 10 */
#define V_BASE8		02	/* is integer in base 8 */
#define V_BOOL		04	/* is a boolean */
#define V_STRING	010	/* is a string */
#define V_CHAR		020	/* is a character */
#define V_FILENAME	040	/* a file name (implies V_STRING) */
#define V_TYPEMASK	077	/* mask off the extra bits */
#define V_MODELINE	0100	/* update modeline */
#define V_CLRSCREEN	0200	/* clear and redraw screen */
#define V_TTY_RESET	0400	/* redo the tty modes */

extern int
	OKXonXoff,		/* disable start/stop characters */
	MetaKey,		/* this terminal has a meta key */
	VisBell,		/* use visible bell (if possible) */
	WrapScan,		/* make searches wrap */
	phystab,		/* terminal's tabstop settings */ 
	tabstop,		/* expand tabs to this number of spaces */
#ifdef BACKUPFILES
	BkupOnWrite,		/* make backup files when writing */
#endif
	RMargin,		/* right margin */
	LMargin,		/* left margin */
	ScrollStep,		/* how should we scroll */
	WtOnMk,			/* write files on compile-it command */
	EndWNewline,		/* end files with a blank line */
	MarkThresh,		/* moves greater than MarkThresh
				   will SetMark */
	PDelay,			/* paren flash delay in tenths of a second */
	CIndIncrmt,		/* how much each indentation level pushes
				   over in C mode */
	CreatMode,		/* default mode for creat'ing files */
	CaseIgnore,		/* case ignore search */
#ifdef ABBREV
	AutoCaseAbbrev,		/* automatically do case on abbreviations */
#endif
	MarksShouldFloat,	/* adjust marks on insertion/deletion */
	UseRE,			/* use regular expressions in search */
	SyncFreq,		/* how often to sync the file pointers */
	BriteMode,		/* make the mode line inverse? */
	OkayBadChars,		/* allow bad characters in files created
				   by JOVE */
	UpdFreq,		/* how often to update modeline */
	UseBuffers,		/* use buffers with Typeout() */
#ifdef BIFF
	BiffChk,		/* turn off/on biff with entering/exiting jove */
#endif
	MailInt,		/* mail check interval */
#ifdef ID_CHAR
	UseIC,			/* whether or not to use i/d char
				   processesing */
	SExitChar,		/* type this to stop i-search */
#endif
	IntChar,		/* ttysets this to generate QUIT */
	EWSize;			/* size to make the error window */

extern char
#ifdef IPROCS
	proc_prompt[80],	/* process prompt */
#endif
#ifdef F_COMPLETION
	BadExtensions[128],	/* extensions (e.g., ".o" to ignore) */
#endif
#ifdef CMT_FMT
	CmtFmt[80],
#endif
	ModeFmt[120],		/* mode line format string */
	Mailbox[128],		/* mailbox name */
	TmpFilePath[128],	/* directory/device to store tmp files */
	TagFile[128],		/* default tag file */
	Shell[128];		/* shell to use */

extern int
	exp,		/* argument count */
	exp_p,		/* argument count is supplied */

	TOabort,	/* flag set by Typeout() */
	io,		/* file descriptor for reading and writing files */
	errormsg,	/* last message was an error message
			   so don't erase the error before it
			   has been read */
	this_cmd,	/* ... */
	last_cmd,	/* last command ... to implement appending
			   to kill buffer */
	RecDepth,	/* recursion depth */
	InputPending,	/* nonzero if there is input waiting to
			   be processed */
 	killptr,	/* index into killbuf */
	CanScroll,	/* can this terminal scroll? */
	Crashing,	/* we are in the middle of crashing */
	Asking,		/* are we on read a string from the terminal? */
	inIOread;	/* so we know whether we can do a redisplay. */

extern char	Minibuf[LBSIZE];

#define curmark		(curbuf->b_markring[curbuf->b_themark])
#define b_curmark(b)	(b->b_markring[b->b_themark])

extern Line	*killbuf[NUMKILLS];	/* array of pointers to killed stuff */

#define MESG_SIZE 128
extern char	mesgbuf[MESG_SIZE];

struct screenline {
	char	*s_line,
		*s_length;
};

extern int
	LastKeyStruck;

extern int
	stackp,

	CapLine,	/* cursor line and cursor column */
	CapCol,

	UpdModLine,	/* whether we want to update the mode line */
	UpdMesg;	/* update the message line */

#define CATCH \
{\
	jmp_buf	sav_jmp; \
\
	push_env(sav_jmp); \
	if (setjmp(mainjmp) == 0) {

#define ONERROR \
	} else { \

#define ENDCATCH \
	} \
	pop_env(sav_jmp); \
}

extern int
	read(),
	write();
	getch();

extern time_t	time();
extern long	lseek();

extern disk_line
	putline();

extern data_obj
	*findcom(),
	*findvar(),
	*findmac();

extern Line
	*next_line(),
	*prev_line(),
	*nbufline(),
	*reg_delete(),
	*lastline(),
	*listput();

extern char
	*getsearch(),
	*pwd(),
	*itoa(),
	*get_time(),
	*copystr(),
	*basename(),
	*filename(),
	*IOerr(),
	*index(),
	*ask(),
	*do_ask(),
	*ask_buf(),
	*ask_file(),
	*lcontents(),
	*malloc(),
	*emalloc(),
	*mktemp(),
	*realloc(),
	*ltobuf(),
	*lbptr(),
	*rindex(),
	*getenv(),
	*tgoto(),
	*pr_name(),
	*sprint(),
	*StrIndex();

extern Bufpos
	*docompiled(),
	*dosearch(),
	*DoYank(),
	*c_indent(),
#ifdef LISP
	*lisp_indent(),
#endif
	*m_paren();

extern Mark
	*CurMark(),
	*MakeMark();

extern Window
	*windbp(),
	*div_wind();

extern data_obj
	**IsPrefix();

extern Buffer
	*do_find(),
	*do_select(),
	*mak_buf(),
	*buf_exists(),
	*file_exists();

struct cmd *
	FindCmd();
