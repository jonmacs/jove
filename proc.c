/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

#include "jove.h"

#include "jctype.h"
#include "fp.h"
#include "re.h"
#include "disp.h"
#include "sysprocs.h"
#include "ask.h"
#include "delete.h"
#include "extend.h"
#include "fmt.h"
#include "insert.h"
#ifdef IPROCS
# include "iproc.h"
#endif
#include "marks.h"
#include "misc.h"
#include "move.h"
#include "proc.h"
#include "wind.h"

#include <signal.h>

#ifdef MSDOS_PROCS
# include <io.h>
# ifndef MSC51
#  include <sys/stat.h>	/* for S_IWRITE and S_IREAD */
# endif
# include <process.h>
#endif /* WIN32 */

#ifdef POSIX_SIGS
# define SIGINTMASK_DECL	sigset_t sigintmask;
# define SIGINTMASK_INIT()	{ sigemptyset(&sigintmask); sigaddset(&sigintmask, SIGINT); }
# define SIGINT_BLOCK()	sigprocmask(SIG_BLOCK, &sigintmask, (sigset_t *)NULL)
# define SIGINT_UNBLOCK()	sigprocmask(SIG_UNBLOCK, &sigintmask, (sigset_t *)NULL)
#else /* !POSIX_SIGS */
# ifdef USE_SIGSET
#  define SIGINT_BLOCK()	sighold(SIGINT)
#  define SIGINT_UNBLOCK()	sigrelse(SIGINT)
# else /* !USE_SIGSET */
#  ifdef BSD_SIGS
#   define SIGINT_BLOCK()	sigsetmask(sigmask(SIGINT))
#   define SIGINT_UNBLOCK()	sigsetmask(0)
#  endif /* BSD_SIGS */
# endif /* !USE_SIGSET */
#endif /* !POSIX_SIGS */

/* This disgusting RE search string parses output from the GREP
 * family, from the pdp11 compiler, pcc, and lint.  Jay (HACK)
 * Fenlasen changed this to work for the lint errors.
 */
char	ErrFmtStr[256] = "^\\{\"\\|\\}\\([^:\"( \t]*\\)\\{\"\\, line \\|:\\|(\\} *\\([0-9][0-9]*\\)[:)]\
\\|::  *\\([^(]*\\)(\\([0-9]*\\))$\
\\|( \\([^(]*\\)(\\([0-9]*\\)) ),";	/* VAR: format string for parse errors */

#ifdef SPELL
char	SpellCmdFmt[FILESIZE] = SPELL;	/* VAR: command to use for spell check */
#endif

struct error {
	Buffer		*er_buf;	/* Buffer error is in */
	LinePtr		er_mess,	/* Actual error message */
			er_text;	/* Actual error */
	int		er_char;	/* char pos of error */
	struct error	*er_prev,	/* List of errors */
			*er_next;
};

private struct error	*cur_error = NULL,
		*errorlist = NULL;

/* Eliminate any error records that contain dangling references to Lines.
 * We only eliminate error structs when either referent is recycled.
 * If it deleted, we keep it (dormant) in case it will be pasted back
 * into the same buffer.
 */
void
ChkErrorLines()
{
	register struct error	*e;
	struct error	*prev = NULL;

	for (e = errorlist; e != NULL; ) {
		struct error	*next = e->er_next;

		if (e->er_mess->l_dline == NULL_DADDR
		|| e->er_text->l_dline == NULL_DADDR)
		{
			/* dangling reference: delete */
			if (prev == NULL)
				errorlist = next;
			else
				prev->er_next = next;
			if (next != NULL)
				next->er_prev = prev;
			if (cur_error == e)
				cur_error = next;
			free((UnivPtr)e);
		} else {
			prev = e;
		}
		e = next;
	}
}

/* Add an error to the end of the list of errors.  This is used for
 * parse-{C,LINT}-errors and for the spell-buffer command
 */
private struct error *
AddError(laste, errline, buf, line, charpos)
struct error	*laste;
LinePtr	errline,
	line;
Buffer	*buf;
int	charpos;
{
	struct error	*new = (struct error *) emalloc(sizeof *new);

	new->er_prev = laste;
	if (laste == NULL) {
		/* first time: free up old errors */
		if (errorlist != NULL)
			ErrFree();
		cur_error = errorlist = new;
	} else {
		laste->er_next = new;
	}
	new->er_next = NULL;
	new->er_buf = buf;
	new->er_text = line;
	new->er_char = charpos;
	new->er_mess = errline;

	return new;
}

void
get_FL_info(fname, lineno)
char	*fname,
	*lineno;
{
	putmatch(1, fname, (size_t)FILESIZE);
	putmatch(2, lineno, (size_t)FILESIZE);

	/* if error had lineno before fname, switch the two */
	if (!jisdigit(lineno[0]) && jisdigit(fname[0])) {
	    putmatch(1, lineno, (size_t)FILESIZE);
	    putmatch(2, fname, (size_t)FILESIZE);
	}
}

/* Free up all the errors */

void
ErrFree()
{
	cur_error = NULL;
	while (errorlist != NULL) {
		struct error	*nel = errorlist->er_next;

		free((UnivPtr) errorlist);
		errorlist = nel;
	}
}

/* Parse errors of the form specified in ErrFmtStr in the current
 * buffer.  Do a show error of the first error.  This is neat because this
 * will work for any kind of output that prints a file name and a line
 * number on the same line.
 */
void
ErrParse()
{
	struct RE_block	re_blk;
	Bufpos	*bp;
	char	fname[FILESIZE],
		lineno[FILESIZE];
	long	lnum,
		last_lnum = -1;
	struct error	*ep = NULL;
	Buffer	*buf,
		*lastb = NULL;
	LinePtr	err_line = NULL;	/* avoid uninitialized complaint from gcc -W */

	ErrFree();		/* This is important! */
	ToFirst();
	perr_buf = curbuf;
	REcompile(ErrFmtStr, YES, &re_blk);
	/* Find a line with a number on it. */
	while ((bp = docompiled(FORWARD, &re_blk)) != NULL) {
		SetDot(bp);
		get_FL_info(fname, lineno);
		buf = do_find((Window *)NULL, fname, YES, YES);
		(void) chr_to_long(lineno, 10, NO, &lnum);
		if (buf != lastb) {
			lastb = buf;
			last_lnum = 1;		/* new file */
			err_line = buf->b_first;
		} else if (lnum == last_lnum)	/* one error per line is nicer */
			continue;
		err_line = next_line(err_line, lnum - last_lnum);
		ep = AddError(ep, curline, buf, err_line, 0);
		last_lnum = lnum;
	}
	if (cur_error != NULL)
		ShowErr();
}

private void
NeedErrors()
{
	if (cur_error == NULL) {
		complain("No errors!");
		/* NOTREACHED */
	}
}

private bool
ErrorHasReferents()
{
	return inlist(cur_error->er_buf->b_first, cur_error->er_text)
		&& inlist(perr_buf->b_first, cur_error->er_mess);
}

/* Go the the next error, if there is one.  Put the error buffer in
 * one window and the buffer with the error in another window.
 * It checks to make sure that the error actually exists.
 */
private void
ToError(forward)
bool	forward;
{
	register struct error	*e = cur_error;
	int	num = arg_value();

	NeedErrors();
	if ((forward? e->er_next : e->er_prev) == NULL) {
		s_mess("You're at the %s error.", forward ? "last" : "first");
	} else {
		while (--num >= 0 || !ErrorHasReferents()) {
			e = forward ? e->er_next : e->er_prev;
			if (e == NULL)
				break;

			cur_error = e;
		}
		ShowErr();
	}
}

void
NextError()
{
	ToError(YES);
}

void
PrevError()
{
	ToError(NO);
}

int	EWSize = 20;	/* VAR: percentage of screen to make the error window */

private void
set_wsize(wsize)
int	wsize;
{
	wsize = (LI * wsize) / 100;
	if (wsize >= 1 && !one_windp())
		WindSize(curwind, wsize - (curwind->w_height - 1));
}

/* Show the current error, i.e. put the line containing the error message
 * in one window, and the buffer containing the actual error in another
 * window.
 */
void
ShowErr()
{
	Window	*err_wind,
		*buf_wind;

	NeedErrors();
	if (!ErrorHasReferents()) {
		rbell();
		return;
	}
	err_wind = windbp(perr_buf);
	buf_wind = windbp(cur_error->er_buf);

	if (err_wind == NULL) {
		if (buf_wind != NULL) {
			SetWind(buf_wind);
			pop_wind(perr_buf->b_name, NO, -1);
			err_wind = curwind;
		} else {
			pop_wind(perr_buf->b_name, NO, -1);
			err_wind = curwind;
			pop_wind(cur_error->er_buf->b_name, NO, -1);
			buf_wind = curwind;
		}
	} else if (buf_wind == NULL) {
		SetWind(err_wind);
		pop_wind(cur_error->er_buf->b_name, NO, -1);
		buf_wind = curwind;
	}

	/* Put the current error message at the top of its Window */
	SetWind(err_wind);
	SetLine(cur_error->er_mess);
	SetTop(curwind, (curwind->w_line = cur_error->er_mess));
	set_wsize(EWSize);

	/* now go to the the line with the error in the other window */
	SetWind(buf_wind);
	DotTo(cur_error->er_text, cur_error->er_char);
}

char	ShcomBuf[LBSIZE];

/* Make a buffer name given the command `command', i.e. "fgrep -n foo *.c"
 * will return the buffer name "fgrep".
 */
char *
MakeName(command)
char	*command;
{
	static char	bnm[FILESIZE];
	register char	*cp = bnm,
			c;
	const char	*bp;

	do {
		c = *command++;
	} while (jiswhite(c));
	*cp++ = '['; /* avoid potential clash with script or program names */
	while (cp < &bnm[sizeof(bnm) - 1] && c != '\0' && !jiswhite(c)) {
		*cp++ = c;
		c = *command++;
	}
	*cp = '\0';
	if ((bp = jbasename(&bnm[1])) != &bnm[1]) {
		/* bp overlaps bnm, cannot use strcpy */
		cp = &bnm[1];
		do ; while ((*cp++ = *bp++) != '\0');
		cp--; /* make sure cp points at '\0' */
	}
	if (cp > &bnm[1] && cp < &bnm[sizeof(bnm)-1]) {
		*cp++ = ']';
		*cp = '\0';
	}
	return bnm;
}

#ifdef SUBSHELL	/* the body is the rest of this file */

/* Run make, first writing all the modified buffers (if the WtOnMk flag is
 * on), parse the errors, and go the first error.
 */
bool	WtOnMk = YES;		/* VAR: write files on compile-it command */
bool	WrapProcessLines = NO;	/* VAR: wrap process lines at CO-1 chars */

private void
	DoShell proto((char *, char *)),
	com_finish proto((wait_status_t, char *));

private char	make_cmd[LBSIZE] = "make";

void
MakeErrors()
{
	Window	*old = curwind;

	if (WtOnMk)
		put_bufs(NO);

	/* When we're not doing make or cc (i.e., the last command
	 * was probably a grep or something) and the user just types
	 * ^X ^E, he probably (possibly, hopefully, usually (in my
	 * case)) doesn't want to do the grep again but rather wants
	 * to do a make again; so we ring the bell and insert the
	 * default command and let the person decide.
	 */
	if (is_an_arg()
	|| !(sindex("make", make_cmd) || sindex("cc", make_cmd)))
	{
		if (!is_an_arg())
			rbell();
		/* insert the default for the user (Kludge: only if Inputp is free) */
		if (Inputp == NULL)
			Inputp = make_cmd;
		jamstr(make_cmd, ask(make_cmd, "Compilation command: "));
	}
	com_finish(UnixToBuf(UTB_DISP|UTB_CLOBBER|UTB_ERRWIN|UTB_SH,
		MakeName(make_cmd), (char *)NULL, make_cmd), make_cmd);

	ErrParse();

	if (!cur_error)
		SetWind(old);
}

# ifdef SPELL

private void
SpelParse(bname)
const char	*bname;
{
	Buffer	*buftospel,
		*wordsb;
	char	wordspel[100];
	Bufpos	*bp;
	struct error	*ep = NULL;

	ErrFree();		/* This is important! */

	buftospel = curbuf;
	wordsb = buf_exists(bname);
	if (wordsb == NULL) {
		complain("Buffer %s is gone!", bname);
		/* NOTREACHED */
	}
	perr_buf = wordsb;	/* This is important (buffer containing
				   error messages) */
	SetBuf(wordsb);
	ToFirst();
	f_mess("Finding misspelled words ... ");
	while (!lastp(curline)) {
		swritef(wordspel, sizeof(wordspel), "\\<%s\\>", linebuf);
		SetBuf(buftospel);
		ToFirst();
		while ((bp = dosearch(wordspel, FORWARD, NO)) != NULL) {
			SetDot(bp);
			ep = AddError(ep, wordsb->b_dot, buftospel,
					  curline, curchar);
		}
		SetBuf(wordsb);
		line_move(FORWARD, 1, NO);
	}
	add_mess("Done.");

	/* undo buffer switches that ought not to be reflected in current window */
	SetBuf(curwind->w_bufp);

	ShowErr();
}

void
SpelBuffer()
{
	static const char	Spell[] = "Spell";
	const char *cp;
	char	com[100];
	Buffer	*savebp = curbuf;

	if (curbuf->b_fname == NULL) {
		complain("no file name");
		/* NOTREACHED */
	}
	if ((cp = strchr(SpellCmdFmt, '%')) == NULL ||
	    cp[1] != 's' || strchr(cp+2, '%') != NULL) {
		complain("spell-command-format needs one %%s with no other format characters");
		/* NOTREACHED */
	}
	if (IsModified(curbuf))
		SaveFile();
	swritef(com, sizeof(com), SpellCmdFmt, curbuf->b_fname);
	(void) UnixToBuf(UTB_DISP|UTB_CLOBBER|UTB_ERRWIN|UTB_SH,
		Spell, (char *)NULL, com);
	message("[Delete the irrelevant words and then type ^X ^C]");
	ToFirst();
	Recur();
	if (!valid_bp(savebp)) {
		complain("Buffer gone!");
		/* NOTREACHED */
	}
	SetBuf(savebp);
	SpelParse(Spell);
}

void
SpelWords()
{
	Buffer	*wordsb = curbuf;
	const char	*buftospel = ask_buf((Buffer *)NULL, ALLOW_OLD | ALLOW_INDEX);

	SetBuf(do_select(curwind, buftospel));
	SpelParse(wordsb->b_name);
}

# endif /* SPELL */

void
ShToBuf()
{
	char	bnm[128],
		cmd[LBSIZE];

	jamstr(bnm, ask((char *)NULL, "Buffer: "));
	jamstr(cmd, ask(ShcomBuf, "Command: "));
	DoShell(bnm, cmd);
}

void
ShellCom()
{
	jamstr(ShcomBuf, ask(ShcomBuf, ProcFmt));
	DoShell(MakeName(ShcomBuf), ShcomBuf);
}

void
ShNoBuf()
{
	jamstr(ShcomBuf, ask(ShcomBuf, ProcFmt));
	com_finish(UnixToBuf(UTB_SH|UTB_FILEARG, (char *)NULL, (char *)NULL,
		ShcomBuf), ShcomBuf);
}

void
Shtypeout()
{
	wait_status_t	status;

	jamstr(ShcomBuf, ask(ShcomBuf, ProcFmt));
	status = UnixToBuf(UTB_DISP|UTB_SH|UTB_FILEARG, (char *)NULL, (char *)NULL,
		ShcomBuf);
#ifdef MSDOS_PROCS
	if (status < 0)
		Typeout("[%s: not executed %d]", ShcomBuf, status);
	else if (status > 0)
		Typeout("[%s: exited with %d]", ShcomBuf, status);
	else if (!is_an_arg())
		Typeout("[%s: completed successfully]", ShcomBuf);
#else /* !MSDOS_PROCS */
	if (WIFSIGNALED(status))
		Typeout("[%s: terminated by signal %d]", ShcomBuf, WTERMSIG(status));
	else if (WIFEXITED(status) && WEXITSTATUS(status)!=0)
		Typeout("[%s: exited with %d]", ShcomBuf, WEXITSTATUS(status));
	else if (!is_an_arg())
		Typeout("[%s: completed successfully]", ShcomBuf);
#endif /* !MSDOS_PROCS */
	TOstop();
}

/* Run the shell command into `bnm'.  Empty the buffer except when we
 * give a numeric argument, in which case it inserts the output at the
 * current position in the buffer.
 */
private void
DoShell(bnm, command)
char	*bnm,
	*command;
{
	Window	*savewp = curwind;

	com_finish(UnixToBuf(
		(is_an_arg()
			? UTB_DISP|UTB_SH|UTB_FILEARG
			: UTB_DISP|UTB_CLOBBER|UTB_SH|UTB_FILEARG),
		bnm, (char *)NULL, command), command);
	SetWind(savewp);
}

private void
com_finish(status, cmd)
wait_status_t	status;
char	*cmd;
{
#ifdef MSDOS_PROCS
	if (status < 0)
		s_mess("[%s: not executed %d]", cmd, status);
	else if (status > 0)
		s_mess("[%s: exited with %d]", cmd, status);
	else
		s_mess("[%s: completed successfully]", cmd);
#else /* !MSDOS_PROCS */
	if (WIFSIGNALED(status))
		s_mess("[%s: terminated by signal %d]", cmd, WTERMSIG(status));
	else if (WIFEXITED(status) && WEXITSTATUS(status)!=0)
		s_mess("[%s: exited with %d]", cmd, WEXITSTATUS(status));
	else
		s_mess("[%s: completed successfully]", cmd);
#endif /* !MSDOS_PROCS */
}

#ifndef MSDOS_PROCS

/* pid of any outstanding non-iproc process.
 * Note: since there is only room for one pid, there can be no more than
 * one running non-iproc process.
 */
pid_t	ChildPid;

void
dowait(status)
wait_status_t	*status;	/* may be NULL */
{
# ifdef IPROCS
	while (DeadPid != ChildPid) {
		wait_status_t	w;
		pid_t	rpid = wait(&w);

		if (rpid == -1) {
			if (errno == ECHILD) {
				/* fudge what we hope is a bland value */
				byte_zero((UnivPtr)&DeadStatus, sizeof(wait_status_t));
				break;
			}
		} else {
			kill_off(rpid, w);
		}
	}
	DeadPid = 0;
	if (status != NULL)
		*status = DeadStatus;
# else
	wait_status_t	w;

	for (;;) {
		pid_t	rpid = wait(&w);

		if (rpid == -1) {
			if (errno == ECHILD) {
				/* fudge what we hope is a bland value */
				byte_zero((UnivPtr)&w, sizeof(wait_status_t));
				break;
			}
		} else if (rpid == ChildPid) {
			break;
		}
	}
	if (status != NULL)
		*status = w;
# endif
	ChildPid = 0;
}

#endif /* !MSDOS_PROCS */

/* Environment management for subshells */
private Env proc_env;
private char PEnvExpBuf[LBSIZE];
private char PEnvUnsetBuf[LBSIZE];

void
ProcEnvExport()
{
	jamstr(PEnvExpBuf, ask(PEnvExpBuf, ProcFmt));
	jputenv(&proc_env, PEnvExpBuf);
}

void
ProcEnvShow()
{
	const char **p;
	TOstart("subshell environment");
	for (p = jenvdata(&proc_env); *p; p++) {
	    Typeout("%s", *p);
	}
	TOstop();
}

void
ProcEnvUnset()
{
	jamstr(PEnvUnsetBuf, ask(PEnvUnsetBuf, ProcFmt));
	junsetenv(&proc_env, PEnvUnsetBuf);
}

/* Run the command cmd.  Output to the buffer named bnm (if not
 * NULL), first erasing bnm (if UTB_DISP and UTB_CLOBBER), and
 * redisplay (if UTB_DISP).  Leaves bnm as the current buffer and
 * leaves any windows it creates lying around.  It's up to the
 * caller to fix everything up after we're done.  (Usually there's
 * nothing to fix up.)
 *
 * If bnm is non-NULL, the process output goes to that buffer.
 * Furthermore, if UTB_DISP, the buffer is displayed in a window.
 * If not UTB_DISP, the buffer is not given a window (of course it
 * might already have one).  If UTB_DISP and UTB_CLOBBER, the buffer
 * is emptied initially.  If UTB_DISP and UTB_ERRWIN, that window's
 * size is as specified by the variable error-window-size.
 *
 * If bnm is NULL, the process output does not go to a buffer.  In this
 * case, if UTB_DISP, it is displayed using Typeout; if not UTB_DISP,
 * the output is discarded.
 *
 * Only if UTB_DISP and bnm is non-NULL are UTB_ERRWIN and
 * UTB_CLOBBER meaningful.
 */
wait_status_t
UnixToBuf(flags, bnm, InFName, cmd)
	int	flags;	/* bunch of booleans: see UTB_* in proc.h */
	const char	*bnm;	/* buffer name (NULL means none) */
	const char	*InFName;	/* name of file for process stdin (NULL means none) */
	const char	*cmd;	/* command to run */
{
#ifndef MSDOS_PROCS
	int	p[2];
	wait_status_t	status;
	SIGHANDLERTYPE	old_int;
	char	bfn[FILESIZE];	/* buffer file name */
	char	bln[10];	/* buffer line number (big enough for any line number) */
	char	bc[6];	/* buffer column (big enough for any column) */
#else /* MSDOS_PROCS */
	char	cmdbuf[129];
	int	status;
	char	pipename[FILESIZE];
#endif /* MSDOS_PROCS */
	bool	eof;
	char	*argv[9];	/* worst case: /bin/sh sh -cf "echo $1" $1 $1 $2 $3 NULL */
	char	**ap = argv;
	File	*fp;
#ifdef SIGINTMASK_DECL
	SIGINTMASK_DECL

	SIGINTMASK_INIT();
#endif

	jdbg("UnixToBuf flags 0x%x \"%s\"\n", flags, cmd);
	SlowCmd += 1;;
	if (flags & UTB_SH) {
			*ap++ = Shell;
			*ap++ = (char *)jbasename(Shell);	/* lose const (but it's safe) */
			*ap++ = ShFlags;
			*ap++ = (char *)cmd;	/* lose const (but it's safe) */
#ifdef MSDOS_PROCS
			/* Kludge alert!
			 * UNIX-like DOS shells and command.com-like DOS shells
			 * seem to differ seem to differ on two points:
			 * - UNIX-like shells use "-" to start flags whereas
			 *   command.com-like shells use "/".
			 * - UNIX-like shells seem to require that the argument to
			 *   $SHELL -c be quoted to cause it to be taken as a single argument.
			 *   command.com-like shells seem to automatically use the rest
			 *   of the arguments.  This is not an issue under real UNIX
			 *   since arguments are passed already broken down.
			 *
			 * E.g., your shell comand: echo foo
			 *         jove runs: command /c echo foo     OK
			 *         jove runs: sh -c echo foo          Oops! sh just runs "echo"
			 *         jove runs: sh -c "echo foo"        Ah, now I get it.
			 *
			 * We use the first character of ShFlags to distinguish
			 * which kind of shell we are dealing with!
			 */
			if (ShFlags[0] == '-') {
				swritef(cmdbuf, sizeof(cmdbuf), "\"%s\"", cmd);
				ap[-1] = cmdbuf;
				/* ??? can we usefully jam in a copy or two of current filename? */
			}
#else /* !MSDOS_PROCS */
			/* Two copies of the file name are passed to the shell:
			 * The Cshell uses the first as a definition of $1.
			 * Most versions of the Bourne shell use the second as a
			 * definition of $1.  (Unfortunately, these same versions
			 * of the Bourne shell take the first as their own name
			 * for error reporting.)
			 */
			if ((flags & UTB_FILEARG) && curbuf->b_fname != NULL) {
				strcpy(bfn, pr_name(curbuf->b_fname, NO));
				*ap++ = bfn;
				*ap++ = bfn;
				{
					int ln = 1;	/* origin 1 */
					LinePtr lp;

					for (lp = curbuf->b_first; lp != curline; lp = lp->l_next)
						ln++;
					swritef(bln, sizeof(bln), "%d", ln);
					*ap++ = bln;
				}
				swritef(bc, sizeof(bc), "%d", curchar + 1);	/* origin 1 */
				*ap++ = bc;
			}
#endif /* !MSDOS_PROCS */
	} else {
		*ap++ = (char *)cmd;	/* lose const (but it's safe) */
		*ap++ = (char *)jbasename(cmd);	/* lose const (but it's safe) */
	}
	*ap++ = NULL;

	if (access(argv[0], J_X_OK) != 0) {
		complain("[Couldn't access %s: %s]", argv[0], strerror(errno));
		/* NOTREACHED */
	}
	if (flags & UTB_DISP) {
		if (bnm != NULL) {
			if (flags & UTB_CLOBBER) {
				isprocbuf(bnm);
				pop_wind(bnm, YES, B_PROCESS);
			} else {
				pop_wind(bnm, NO, B_FILE);
			}
			set_wsize(flags & UTB_ERRWIN? EWSize : 0);
			message("Starting up...");
			redisplay();
		} else {
			TOstart(argv[0]);
			Typeout("Starting up...");
			TOstart(argv[0]);	/* overwrite "Starting up..." */
		}
	}
	/* Now I will attempt to describe how I deal with signals during
	 * the execution of the shell command.  My desire was to be able
	 * to interrupt the shell command AS SOON AS the window pops up.
	 * So, if we have SIGINT_BLOCK (i.e., a modern signal mechanism)
	 * I hold SIGINT, meaning if we interrupt now, we will eventually
	 * see the interrupt, but not before we are ready for it.  We
	 * fork, the child releases the interrupt, it then sees the
	 * interrupt, and so exits.  Meanwhile the parent ignores the
	 * signal, so if there was a pending one, it's now lost.
	 *
	 * Without SIGINT_BLOCK, the best behavior you can expect is that
	 * when you type ^] too soon after the window pops up, it may
	 * be ignored.  The behavior BEFORE was that it would interrupt
	 * JOVE and then you would have to continue JOVE and wait a
	 * little while longer before trying again.  Now that is fixed,
	 * in that you just have to type it twice.
	 */

#ifndef MSDOS_PROCS
	dopipe(p);

# ifdef SIGINT_BLOCK
	SIGINT_BLOCK();
# else
	old_int = setsighandler(SIGINT, SIG_IGN),
# endif

# ifdef USE_VFORK
	/*
	 * There are several other forks in Jove, but for
	 * machines lacking MMUs, where vfork might be
	 * significantly more performant, it is nice to have
	 * SUBSHELL capability using pipes (MSDOS_PROCS use a
	 * temporary file for subshell output and spawn).
	 * moraes Note from 2020 4.17: vfork has not been tested in a
	 * long time, it is quite possible/likely that it does
	 * not even work (I think that nothing between here
	 * and the exec damages parent state, but using vfork
	 * is a deal with the devil)
	 */
	ChildPid = vfork();
# else
	ChildPid = fork();
# endif

	if (ChildPid == -1) {
		int	fork_errno = errno;

		pipeclose(p);
# ifdef SIGINT_UNBLOCK
		SIGINT_UNBLOCK();
# else
		(void) setsighandler(SIGINT, old_int),
# endif
		complain("[Fork failed: %s]", strerror(fork_errno));
		/* NOTREACHED */
	}
	if (ChildPid == 0) {
		const char	*a;	/* action name (for error message) */
		(void) setsighandler(SIGINT, SIG_DFL);
#  ifdef SIGINT_UNBLOCK
		SIGINT_UNBLOCK();
#  endif
		if (!((a = "close 0", close(0)) == 0
		&& (a = "open", open(InFName==NULL? "/dev/null" : InFName, O_RDONLY | O_BINARY)) == 0
		&& (a = "close 1", close(1)) == 0
		&& (a = "dup 1", dup(p[1])) == 1
		&& (a = "close 2", close(2)) == 0
		&& (a = "dup 2", dup(p[1])) == 2)) {
			raw_complain("% in setup for child failed: %s", a, strerror(errno));
			_exit(1);
		}

		pipeclose(p);
		jcloseall();
		execve(argv[0], &argv[1], (char **) jenvdata(&proc_env));
		raw_complain("Execl failed: %s", strerror(errno));
		_exit(1);
	}
# ifdef SIGINT_BLOCK
	old_int = setsighandler(SIGINT, SIG_IGN);	/* got to do this eventually */
# endif
	(void) close(p[1]);
	fp = fd_open(argv[1], F_READ, p[0], iobuff, LBSIZE);
#else /* MSDOS_PROCS*/
	{
		int	oldi = dup(0),
			oldo = dup(1),
			olde = dup(2);
		bool	InFailure = NO;
		int	ph;

		PathCat(pipename, sizeof(pipename), TmpDir, "jpXXXXXX");
		ph = MakeTemp(pipename, "cannot make tempfile \"%s\" for filter");
		jdbg("created temp \"%s\"\n", pipename);

		if (dup2(ph, 1) < 0 || dup2(ph, 2) < 0) {
			jdbg("dup2 of temp file fd failed: %d %s\n", errno,
			     strerror(errno));
			InFailure = YES;
		}
		(void) close(ph);
		(void) close(0);

		if (!InFailure)
			InFailure = InFName != NULL && open(InFName, O_RDONLY | O_BINARY) < 0;
		jdbg("InFailure %d InFName \"%s\"\n", InFailure,
		     InFName == NULL ? "(NULL)" : InFName);
		if (!InFailure) {
			if (jdebug) {
				int vk = 0;
				while (argv[vk]) {
					jdprintf("%d: \"%s\"\n", vk, argv[vk]);
					vk++;
				}
			}
			status = spawnve(0, argv[0], &argv[1],
					 jenvdata(&proc_env));
			jdbg("status %d spawnve \"%s\"\n", status, argv[0]);
		}
		jdbg("restoring fds 0 1 2 from %d %d %d\n", oldi, oldo, olde);
		(void) dup2(oldi, 0);
		(void) dup2(oldo, 1);
		(void) dup2(olde, 2);
		(void) close(oldi);
		(void) close(oldo);
		(void) close(olde);

		if (InFailure) {
			complain("[filter input failed]");
			/* NOTREACHED */
		}
		if (status < 0)
			s_mess("[Spawn failed %d]", errno);
		jdbg("opening \"%s\" from \"%s\"\n", pipename, argv[1]);
		ph = open(pipename, O_RDONLY | O_BINARY | O_CLOEXEC);
		if (ph < 0) {
			complain("[cannot reopen pipe %s: %s]", pipename, strerror(errno));
			/* NOTREACHED */
		}
		fp = fd_open(argv[1], F_READ, ph, iobuff, LBSIZE);
	}

#endif /* MSDOS_PROCS */

	do {
		int	wrap_col = WrapProcessLines ? CO-1 : LBSIZE;
#ifdef UNIX
		InSlowRead = YES;
#endif
		eof = f_gets(fp, genbuf, (size_t)LBSIZE);
#ifdef UNIX
		InSlowRead = NO;
#endif
		if (bnm != NULL) {
			ins_str_wrap(genbuf, YES, wrap_col);
			if (!eof)
				LineInsert(1);
			if ((flags & UTB_DISP) && fp->f_cnt <= 0) {
				message("Chugging along...");
				redisplay();
			}
		} else if (flags & UTB_DISP)
			Typeout("%s", genbuf);
	} while (!eof);
	if (flags & UTB_DISP)
		DrawMesg(NO);
	close_file(fp);
#ifndef MSDOS_PROCS
	dowait(&status);
# ifdef SIGINT_UNBLOCK
	SIGINT_UNBLOCK();
# endif
	(void) setsighandler(SIGINT, old_int);
#else /* MSDOS_PROCS */
	unlink(pipename);
	getCWD();
# ifdef WINRESIZE
	ResizePending = YES;	/* In case subproc did a MODE command or something */
# endif
#endif /* MSDOS_PROCS */
	SlowCmd -= 1;;
	return status;
}

/* Send the current region to CMD and insert the output from the
 * command into OUT_BUF.
 */
private void
RegToUnix(outbuf, cmd, wrap)
Buffer	*outbuf;
char	*cmd;
bool	wrap;
{
	Mark	*m = CurMark();
	static char	tname[FILESIZE];
	Window	*save_wind = curwind;
	volatile wait_status_t	status;
	volatile bool	err = NO;
	bool	old_wrap = WrapProcessLines;
	File	*volatile fp;
	jmp_buf	sav_jmp;

	PathCat(tname, sizeof(tname), TmpDir, "jfXXXXXX");

	/* safe version of mktemp */
	close(MakeTemp(tname, "can't create tempfile \"%s\" for filter"));

	fp = open_file(tname, iobuff, F_WRITE, YES);
	push_env(sav_jmp);
	if (setjmp(mainjmp) == 0) {
		WrapProcessLines = wrap;
		putreg(fp, m->m_line, m->m_char, curline, curchar, YES);
		DelReg();
		f_close(fp);
		status = UnixToBuf(UTB_SH|UTB_FILEARG, outbuf->b_name, tname, cmd);
	} else {
		f_close(fp);
		err = YES;
	}
	pop_env(sav_jmp);
	WrapProcessLines = old_wrap;

	(void) unlink(tname);
	SetWind(save_wind);
	if (!err)
		com_finish(status, cmd);
}

void
FilterRegion()
{
	static char FltComBuf[LBSIZE];

	jamstr(FltComBuf, ask(FltComBuf, ": %f (through command) "));
	RegToUnix(curbuf, FltComBuf, NO);
	this_cmd = UNDOABLECMD;
}

void
isprocbuf(bnm)
const char	*bnm;
{
	Buffer	*bp;

	if ((bp = buf_exists(bnm)) != NULL && bp->b_type != B_PROCESS)
		confirm("Over-write buffer %s? ", bnm);
}

#endif /* SUBSHELL */
