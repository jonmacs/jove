/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

#include "jove.h"

#ifdef IPROCS	/* the body is the rest of this file */

#include <signal.h>

#include "re.h"
#include "jctype.h"
#include "disp.h"
#include "fp.h"
#include "sysprocs.h"
#include "iproc.h"
#include "ask.h"
#include "extend.h"
#include "fmt.h"
#include "insert.h"
#include "marks.h"
#include "move.h"
#include "proc.h"
#include "wind.h"

#ifdef USE_KILLPG
# ifndef FULL_UNISTD
extern int	UNMACRO(killpg) proto((int /*pgrp*/, int /*sig*/));
# endif
#else /* !USE_KILLPG */
#define killpg(pid, sig)	kill(-(pid), (sig))
#endif /* USE_KILLPG */

struct process {
	Process	p_next;
#ifdef PIPEPROCS
	int	p_toproc;	/* write end of pipe to process */
	pid_t	p_portpid,	/* pid of direct child (the portsrv) */
		p_pid;		/* pid of real child i.e. not portsrv */
	bool	p_portlive;	/* is portsrv still live? */
#else
	int	p_fd;		/* file descriptor of pty? opened r/w */
# define	p_portpid	p_pid	/* pid of direct child (the shell) */
	pid_t	p_pid;		/* pid of child (the shell) */
#endif
	Buffer	*p_buffer;	/* add output to end of this buffer */
	char	*p_name;	/* ... */
	int	p_io_state;	/* state of communication with child and descendents */
#		define IO_NEW	0	/* brand new, process has not yet sent output */
#		define IO_RUNNING	1	/* just running */
#		define IO_EOFED	2	/* we have sent EOF, or have received it */
	int	p_child_state;	/* state of child process, as disclosed by SIGCHLD/wait */
#		define C_LIVE	0	/* no news is good news */
#		define C_STOPPED	1
#		define C_EXITED	2
#		define C_KILLED	3
	int	p_reason;	/* If killed, p_reason is the signal;
				   if exited, it is the the exit code */
	Mark	*p_mark;	/* where output left us */
};

private void
	proc_rec proto((Process, char *, size_t)),
	proc_close proto ((Process)),
	SendData proto((bool)),
	obituary proto((register Process child, wait_status_t w));

private bool
	proc_kill proto((Process, int));

#define child_dead(p)	((p)->p_child_state >= C_EXITED)
#define io_eofed(p)	((p)->p_io_state == IO_EOFED)

private bool
dead(p)
Process	p;
{
	return p == NULL || (io_eofed(p) && child_dead(p));
}


#define proc_cmd(p)	((p)->p_name)

private Process	procs = NULL;

private Process
proc_pid(pid)
pid_t	pid;
{
	register Process	p;

	for (p = procs; ; p = p->p_next)
		if (p == NULL || p->p_portpid == pid)
			return p;
}

private const char *
proc_bufname(p)
Process	p;
{
	Buffer	*b = p->p_buffer;

	return (b != NULL) ? b->b_name : "<Deleted>";
}

void
ProcKill()
{
	(void) proc_kill(
		buf_exists(ask_buf(curbuf, ALLOW_OLD | ALLOW_INDEX))->b_process,
		SIGKILL);
}

private void
make_argv(argv, ap)
register char	*argv[];
va_list	ap;
{
	register int	i = 0;

	argv[i++] = va_arg(ap, char *);
	argv[i++] = (char *)jbasename(argv[0]);	/* lose const (but it's safe) */
	do {} while ((argv[i++] = va_arg(ap, char *)) != NULL);
}

/* environment manipulation for interactive processes */

private Env iproc_env;
private char IEnvExpBuf[LBSIZE];
private char IEnvUnsetBuf[LBSIZE];

/* Erase misleading knowledge of the terminal type from the environment.
 * The value of variable TERMCAP has two interpretations:
 * If it starts with /, it is a path to the database.
 * If it does not, it is the database entry.
 * Only happens in child.
 */
private void
set_process_env()
{
	static const char tcn[] = "TERMCAP";
	const char *tc = getenv(tcn);

	if (tc != NULL && tc[0] != '/')
		junsetenv(&iproc_env, tcn);
	environ = (char **) jenvdata(&iproc_env); /* avoid gcc warning */
}

void
IprocEnvExport()
{
	jamstr(IEnvExpBuf, ask(IEnvExpBuf, ProcFmt));
	jputenv(&iproc_env, IEnvExpBuf);
}

void
IprocEnvShow()
{
	const char **p;
	TOstart("i-shell environment");
	for (p = jenvdata(&iproc_env); *p; p++) {
	    Typeout("%s", *p);
	}
	TOstop();
}

void
IprocEnvUnset()
{
	jamstr(IEnvUnsetBuf, ask(IEnvUnsetBuf, ProcFmt));
	junsetenv(&iproc_env, IEnvUnsetBuf);
}

/* There are two very different implementation techniques: pipes and ptys.
 * The following two chunks of code implement the same operations using
 * the two techniques: the first uses pipes and the second uses ptys.
 */


#include "ttystate.h"
#ifdef PIPEPROCS

char	Portsrv[FILESIZE];	/* path to portsrv program (in LibDir) */

int	NumProcs = 0;

File	*ProcInput;
private int	ProcOutput = -1;

pid_t	kbd_pid = -1;

void
read_pipe_proc(pid, nbytes)
pid_t	pid;
register int	nbytes;
{
	register Process	p = proc_pid(pid);

	if (p == NULL) {
		writef("\riproc: unknown pid (%d)", (int)pid);
	} else if (p->p_io_state == IO_NEW) {
		/* first message: pid of real child, not of portsrv */
		pid_t	rpid;

		(void) f_readn(ProcInput, (char *) &rpid, sizeof(pid_t));
		p->p_pid = rpid;
		p->p_io_state = IO_RUNNING;
		UpdModLine = YES;
	} else if (nbytes == -1) {
		/* okay to clean up this process */
		wait_status_t	status;

		(void) f_readn(ProcInput, (char *) &status, sizeof(status));
		/* Reap portsrv process, if it still remains.
		 * Note that any status for this process is uninteresting:
		 * the interesting status came (just now) by pipe.
		 */
		while (p->p_portlive) {
			wait_status_t	w;
			pid_t	rpid = wait(&w);

			if (rpid == -1) {
				if (errno == ECHILD) {
					/* oops: no children, not even portsrv */
					p->p_portlive = NO;
				}
			} else {
				kill_off(rpid, w);
			}
		}
		proc_close(p);
		obituary(p, status);
	} else {
		/* regular data */
		while (nbytes > 0) {
			char	ibuf[512+1];	/* NOTE: room for added NUL */
			size_t n = f_readn(ProcInput, ibuf,
				(size_t)jmin((int)(sizeof ibuf) - 1, nbytes));

			nbytes -= n;
			proc_rec(p, ibuf, n);
		}
	}
}

void
ProcInt()
{
	(void) proc_kill(curbuf->b_process, SIGINT);
}

void
ProcQuit()
{
	(void) proc_kill(curbuf->b_process, SIGQUIT);
}

private void
proc_close(p)
Process	p;
{
	if (p->p_toproc >= 0) {
		jdbg("closing proc fd %d pid %d", p->p_toproc, p->p_pid);
		(void) close(p->p_toproc);
		p->p_toproc = -1;	/* writes will fail */
		NumProcs -= 1;
		p->p_io_state = IO_EOFED;	/* process output EOF is tied to reaping */
		UpdModLine = YES;
	}
}

private void
proc_write(p, buf, nbytes)
Process	p;
char	*buf;
size_t	nbytes;
{
	if (p->p_toproc >= 0) {
		while (nbytes != 0) {
			SSIZE_T	wr = write(p->p_toproc, (UnivConstPtr)buf, nbytes);

			if (wr >= 0) {
				nbytes -= wr;
				buf += wr;
			} else if (errno != EINTR) {
				complain("[error writing to iproc: %d %s]", errno, strerror(errno));
			}
		}
	}
}


# ifdef STDARGS
private void
proc_strt(char *bufname, bool clobber, char *procname, ...)
# else
private /*VARARGS3*/ void
proc_strt(bufname, clobber, procname, va_alist)
	char	*bufname;
	bool	clobber;
	char	*procname;
	va_dcl
# endif
{
	Window	*owind = curwind;
	int	toproc[2];
	pid_t	pid;
	Process	newp;
	Buffer	*newbuf;
	char	*argv[32];
	va_list	ap;

	untieDeadProcess(buf_exists(bufname));
	isprocbuf(bufname);	/* make sure BUFNAME is either nonexistant
				   or is of type B_PROCESS */
	if (access(Portsrv, X_OK) < 0) {
		complain("[Couldn't access %s: %s]", Portsrv, strerror(errno));
		/* NOTREACHED */
	}

	dopipe(toproc);

	if (NumProcs++ == 0)
		kbd_strt();	/* may create kbd process: must be done before fork */
	switch (pid = fork()) {
	case -1:
		pipeclose(toproc);
		if (--NumProcs == 0)
			kbd_stop();
		complain("[Fork failed: %s]", strerror(errno));
		/* NOTREACHED */

	case 0:
		argv[0] = "portsrv";
		va_init(ap, procname);
		make_argv(&argv[1], ap);
		va_end(ap);
		(void) dup2(toproc[0], 0);
		(void) dup2(ProcOutput, 1);
		(void) dup2(ProcOutput, 2);
		pipeclose(toproc);
		jcloseall();
		set_process_env();
		jdbg("ready to exec %s %s\n", Portsrv, argv[0]);
		execv(Portsrv, argv);
		raw_complain("execl failed: %s", strerror(errno));
		_exit(1);
	}

	newp = (Process) emalloc(sizeof *newp);
	newp->p_next = procs;
	newp->p_io_state = IO_NEW;
	newp->p_child_state = C_LIVE;
	newp->p_name = copystr(procname);
	procs = newp;
	newp->p_portpid = pid;
	newp->p_pid = -1;
	newp->p_portlive = YES;

	newbuf = do_select((Window *)NULL, bufname);
	newbuf->b_type = B_PROCESS;
	newp->p_buffer = newbuf;
	newbuf->b_process = newp;	/* sorta circular, eh? */
	pop_wind(bufname, clobber, B_PROCESS);
	ToLast();
	if (!bolp())
		LineInsert(1);
	/* Pop_wind() after everything is set up; important!
	 * Bindings won't work right unless newbuf->b_process is already
	 * set up BEFORE NEWBUF is first SetBuf()'d.
	 */
	newp->p_mark = MakeMark(curline, curchar);

	newp->p_toproc = toproc[1];
	(void) close(toproc[0]);
	SetWind(owind);
}

void
closeiprocs()
{
	Process	p;

	if (ProcOutput != -1)
		close(ProcOutput);
	for (p=procs; p!=NULL; p=p->p_next)
		if (p->p_toproc >= 0)
			close(p->p_toproc);
}

private void
kbd_init()
{
	/* Initiate the keyboard process.
	 * We only get here after a portsrv process has been started
	 * so we know that the portsrv program must exist -- no need to test.
	 */
	int	p[2];

	if (pipe(p) < 0) {
	    complain("Cannot create pipe to kbd process! %s\n", strerror(errno));
	    /* NOTREACHED */
	}
	ProcInput = fd_open("process-input", F_READ|F_LOCKED, p[0],
			    (char *)NULL, 512);
	ProcOutput = p[1];
	switch (kbd_pid = fork()) {
	case -1:
		complain("Cannot fork kbd process! %s\n", strerror(errno));
		/* NOTREACHED */

	case 0:
		(void) setsighandler(SIGINT, SIG_IGN);
		(void) setsighandler(SIGALRM, SIG_IGN);
		close(1);
		if (dup(ProcOutput) < 0) {
		    raw_complain("Cannot dup output fd: %s", strerror(errno));
		    exit(-1);
		}
		jcloseall();
		jdbg("ready to exec %s kbd", Portsrv);
		execl(Portsrv, "kbd", (char *)NULL);
		raw_complain("kbd exec failed: %s", strerror(errno));
		exit(-1);
	}
}

/* kbd_stop() returns true if it changes the state of (i.e. stops)
 * the keyboard process.  This is so kbd stopping and starting in
 * pairs works - see finish() in jove.c.
 */
private bool	kbd_state = NO;

void
kbd_strt()
{
	jdbg("kbd_strt state %d pid %d\n", kbd_state, kbd_pid);
	if (!kbd_state) {
		if (kbd_pid == -1)
			kbd_init();
		else
			kill(kbd_pid, KBDSIG);
		kbd_state = YES;
	}
}

bool
kbd_stop()
{
	jdbg("kbd_stop state %d pid %d\n", kbd_state, kbd_pid);
	if (kbd_state) {
		kbd_state = NO;
		kill(kbd_pid, KBDSIG);
		return YES;
	}
	return NO;
}

void
kbd_kill()
{
	jdbg("kbd_kill state %d pid %d\n", kbd_state, kbd_pid);
	if (kbd_pid != -1) {
		kill(kbd_pid, SIGKILL);
		kbd_pid = -1;
	}
}

#else /* !PIPEPROCS */

#include <sys/time.h>
#include "select.h"

# ifdef USE_OPENPTY	/* modern BSDs have openpty(3) */
#  ifdef HAVE_LIBUTIL_H	/* but disagree about header! */
#   include <libutil.h>
#  else
#   ifdef HAVE_PTY_H
#    include <pty.h>
#   else
#    include <util.h>
#   endif
#  endif
# endif

# ifdef SVR4_PTYS
#  include <stdlib.h>	/* for grantpt and unlockpt, at least in Solaris 2.3 */
#  if _XOPEN_SOURCE >= 500
    /* Linux/glibc no longer pretends to support STREAMS (XSR) (2008) */
#   if defined(_XOPEN_STREAMS) && _XOPEN_STREAMS != -1
#    include <stropts.h>
#   endif
#  else
#   include <sys/stropts.h>
#  endif
  extern char	*ptsname proto((int /*filedes*/));	/* get name of slave */
# endif

# ifdef IRIX_PTYS
#  include <sys/types.h>
#  include <sys/stat.h>
# endif

void
read_pty_proc(fd)
register int	fd;
{
	register Process	p;
	int	n;
	char	ibuf[1024+1];	/* NOTE: room for added NUL */

	for (p = procs; ; p = p->p_next) {
		if (p == NULL) {
			writef("\riproc: unknown fd %d", fd);
			return;
		}
		if (p->p_fd == fd)
			break;
	}

	n = read(fd, (UnivPtr) ibuf, sizeof(ibuf) - 1);
	jdbg("pty read %d returned %d errno %d from %s\n", fd, n, errno, 
	     p->p_name);
	if (n <= 0) {
		if (n < 0) {
			switch (errno) {
			case EIO:
			case EWOULDBLOCK:
#if EWOULDBLOCK != EAGAIN
			case EAGAIN:
#endif
				/* ??? On some systems, pty reads fail initially, for
				 * reasons that are not clear to me (DHR).  This code
				 * forgives these specific kinds of failure until the
				 * process leaves the NEW state.  We hope that this
				 * does not cause a long busy wait.
				 */
				if (p->p_io_state == IO_NEW) {
					jdbg("IO_NEW, got errno %d\n", errno);
					return;
				}

				/* We get here if the i-proc closes stdout
				 * before exiting (eg. Bourne Shell),
				 * so we treat it as a simple EOF.
				 */
				break;
			default:
				/* true I/O error */
				swritef(ibuf, sizeof(ibuf),
					"\n[pty read error: %s]\n", strerror(errno));
				proc_rec(p, ibuf, strlen(ibuf));
				/* now treat as EOF... */
			}
		} /* else, EOF received */
		proc_close(p);
	} else {
		if (p->p_io_state == IO_NEW) {
			p->p_io_state = IO_RUNNING;
			UpdModLine = YES;
		}
		proc_rec(p, ibuf, (size_t)n);
	}
}

void
ProcCont()
{
	Process	p = curbuf->b_process;

	jdbg("ProcCont pid %d %s\n", p->p_pid, p->p_name);
	if (proc_kill(p, SIGCONT) && p->p_child_state == C_STOPPED) {
		p->p_child_state = C_LIVE;
		UpdModLine = YES;
	}
}

/* Sending non-character info through "keyboard"
 *
 * There are two kinds of "out of band" signals we wish to send
 * through the pty: signals (such as SIGINT) and EOF.  On top
 * of that, there is a kind of out of band signal we wish not to
 * send: ERASE and KILL.  Unfortunately, there are a number of ways
 * in which our environment may vary.  Two ioctls are useful:
 * TIOCREMOTE and TIOCSIGNAL/TIOCSIG.
 *
 *
 * TIOCREMOTE:
 *
 * Some systems support the TIOCREMOTE ioctl.  With this ioctl,
 * the "input editing" is suppressed on the characters sent
 * to the slave.  Input editing involves interpreting ERASE,
 * KILL, INTERRUPT, QUIT, EOF, etc. characters.  It is best
 * to run this way since JOVE already does input editing.
 *
 * The scant documentation on TIOCREMOTE in Solaris2.3 suggests
 * that the third argument to the TIOCREMOTE ioctl ought to be
 * an integer (but a pointer to the integer works).  The SunOS4
 * ldterm(4m) manpage says that the third argument is to be a
 * pointer to an integer.  At the moment, we only use the pointer
 * form.
 *
 * Although IRIX defines TIOCREMOTE, it does not seem to work.
 * This may well be true of other systems, so as a safety,
 * we support "NO_TIOCREMOTE" as a feature deselect macro
 * for systems that define TIOCREMOTE but are broken.
 *
 * Under BSDI's BSD/386 v1.[01], the TIOCREMOTE ioctls appear to fail
 * for send_xc (without an error indication), but work otherwise.  This
 * only affects dstop-process, so it is probably best not to define
 * NO_TIOCREMOTE.  Even if we do define NO_TIOCREMOTE, the dstop-process
 * may require the user to do a process-newline (apparently, if the tty
 * input is being canonicalized).  How odd.
 *
 *
 * TIOCSIGNAL/TIOCSIG:
 *
 * Some systems support the TIOCSIGNAL ioctl.  With this ioctl,
 * a signal can be sent through the pty.  Recent (4.3 or later?)
 * BSD systems seem to provide a TIOCSIG ioctl which is similar.
 *
 * The TIOCSIGNAL ioctl is not well documented and seems to be
 * broken on at least IRIX 5.3.  Again, we provide "NO_TIOCSIGNAL"
 * to prevent using it on systems that define it but are broken.
 *
 * Another mystery: systems vary on how the signal number should be
 * passed in the TIOCSIGNAL ioctl.  SunOS4 requires that the third
 * argument be a pointer to an integer, the signal number.  Vanilla
 * SVR4 requires that the third argument be a an integer, the signal
 * number.  Solaris2.3 accepts either.  The SunOS 5.3 STREAMS
 * Programmer's Guide says that the third argument is an int.  We have
 * not figured out, for an arbitrary system with TIOCSIGNAL, how to
 * tell which form the third argument should take.  For now, it is
 * conditionalized on SVR4_PTYS.
 *
 * If TIOCSIGNAL/TIOCSIG isn't supplied (perhaps from termios.h), we
 * cannot send signals to the child when TIOCREMOTE is on, so we just
 * turn it off for a moment.  This is pretty dubious because the user
 * might have done an stty to change or disable the characters.
 *
 * We don't know how to implement dstop using TIOCSIGNAL/TIOCSIG, so
 * we use the dubious trick.
 */

# if !defined(NO_TIOCREMOTE) && !defined(TIOCREMOTE)
#  define NO_TIOCREMOTE	1
# endif

# if !defined(NO_TIOCSIGNAL) && !defined(TIOCSIGNAL) && !defined(TIOCSIG)
#  define NO_TIOCSIGNAL	1
# endif

# ifdef NO_TIOCSIGNAL

#  define kbd_sig(sig, tch, sch)	send_oxc(tch, sch)

# else /* !NO_TIOCSIGNAL */

#  define kbd_sig(sig, tch, sch)	send_sig(sig)

private void
send_sig(sig)
int	sig;
{
	Process	p;

	jdbg("send_sig %d\n", sig);
	if ((p = curbuf->b_process) == NULL || p->p_fd < 0)
		complain("[No process]");
	ToLast();
#  ifdef TIOCSIG
	jdbg("TIOCSIG pid %d %s\n", p->p_pid, p->p_name);
	if (ioctl(p->p_fd, TIOCSIG, sig) < 0)
		complain("TIOCSIG failed: %d %s", errno, strerror(errno));
#  else /* !TIOCSIG */
#   ifdef SVR4_PTYS
	jdbg("SVR4 TIOCSIGNAL pid %d %s\n", p->p_pid, p->p_name);
	if (ioctl(p->p_fd, TIOCSIGNAL, sig) < 0)
		complain("TIOCSIGNAL failed: %d %s", errno, strerror(errno));
#   else /* !SVR4_PTYS */
	jdbg("TIOCSIGNAL pid %d %s\n", p->p_pid, p->p_name);
	if (ioctl(p->p_fd, TIOCSIGNAL, (void *) &sig) < 0)
		complain("TIOCSIGNAL failed: %d %s", errno, strerror(errno));
#   endif /* !SVR4_PTYS */
#  endif /* !TIOCSIG */
}

# endif /* !NO_TIOCSIGNAL */

# if defined(NO_TIOCREMOTE) || defined(NO_TIOCSIGNAL) || (!defined(TERMIO) && !defined(TERMIOS)) || defined(VDSUSP)

#  if defined(TERMIO) || defined(TERMIOS)
#   define send_oxc(tch, sch)	send_xc(sg[NO].c_cc[tch])
#  else
#   define send_oxc(tch, sfld)	send_xc(tc[NO].sfld)
#  endif

private void
send_xc(c)
char	c;
{
	Process	p;

	if ((p = curbuf->b_process) == NULL || p->p_fd < 0)
		complain("[No process]");
	jdbg("send_xc 0x%x pid %d %s \n", c, p->p_pid, p->p_name);
	ToLast();
	{
		char	buf[1+1];	/* NOTE: room for added NUL */

		buf[0] = c;
		proc_rec(p, buf, (size_t)1);

		{
#  ifndef NO_TIOCREMOTE
			int
				off = 0,
				on = 1;

			jdbg("TIOCREMOTE off pid %d %s\n", p->p_pid, p->p_name);
			while (ioctl(p->p_fd, TIOCREMOTE, (UnivPtr) &off) < 0)
				if (errno != EINTR)
					complain("TIOCREMOTE OFF failed: %d %s", errno, strerror(errno));
#  endif /* !NO_TIOCREMOTE */
			for (;;) {
				switch (write(p->p_fd, (UnivPtr) &c, sizeof(c))) {
				case -1: /* error: consider ERRNO */
					if (errno == EINTR)
						continue;	/* interrupted: try again */

					complain("pty write of control failed: %d %s", errno, strerror(errno));
					/* NOTREACHED */
				case 0: /* nothing happened: try again */
					continue;
				case 1: /* done */
					break;
				}
				break;
			}
#  ifndef NO_TIOCREMOTE
			jdbg("TIOCREMOTE on pid %d %s\n", p->p_pid, p->p_name);
			while (ioctl(p->p_fd, TIOCREMOTE, (UnivPtr) &on) < 0)
				if (errno != EINTR)
					complain("TIOCREMOTE ON failed: %d %s", errno, strerror(errno));
#  endif /* !NO_TIOCREMOTE */
		}
	}
}

# endif /* defined(NO_TIOCREMOTE) || defined(NO_TIOCSIGNAL) */

void
ProcEof()
{
# ifdef NO_TIOCREMOTE
	/* we have to write a char */
	send_oxc(VEOF, t_eofc);
# else /* !NO_TIOCREMOTE */
	/* write a zero-length record to signify EOF */
	static char mess[] = "<EOF> ";	/* NOTE: NUL will be overwritten */
	Process	p;

	if ((p = curbuf->b_process) == NULL || p->p_fd < 0)
		complain("[No process]");
	ToLast();
	proc_rec(p, mess, sizeof(mess)-1);
	jdbg("ProcEof pid %d %s\n", p->p_pid, p->p_name);
	while (write(p->p_fd, (UnivPtr) mess, (size_t)0) < 0)
		if (errno != EINTR)
			complain("[error writing EOF to iproc: %d %s]", errno, strerror(errno));
# endif /* !NO_TIOCREMOTE */
}

void
ProcInt()
{
	kbd_sig(SIGINT, VINTR, t_intrc);
}

void
ProcQuit()
{
	kbd_sig(SIGQUIT, VQUIT, t_quitc);
}

void
ProcStop()
{
# if (!defined(TERMIO) && !defined(TERMIOS)) || defined(VSUSP)
	kbd_sig(SIGTSTP, VSUSP, t_suspc);
# else
	complain("[stop-process not supported]");
# endif
}

void
ProcDStop()
{
	/* we don't know how to send a dstop via TIOCSIGNAL/TIOCSIG */
# if (!defined(TERMIO) && !defined(TERMIOS)) || defined(VDSUSP)
	send_oxc(VDSUSP, t_dsuspc);
# else
	complain("[dstop-process not supported]");
# endif
}

private void
proc_close(p)
Process p;
{
	jdbg("proc_close %d %s\n", p->p_pid, p->p_name);
	if (p->p_fd >= 0) {
		(void) close(p->p_fd);
		FD_CLR(p->p_fd, &global_fd);
		p->p_fd = -1;
		p->p_io_state = IO_EOFED;	/* I/O is finished */
		UpdModLine = YES;
	}
}

private void
proc_write(p, buf, nbytes)
Process p;
char	*buf;
size_t	nbytes;
{
	jdbg("proc_write %d bytes to pid %d %s\n", nbytes, p->p_pid, p->p_name);
	if (p->p_fd >= 0) {
		fd_set	mask;

		FD_ZERO(&mask);
		FD_SET(p->p_fd, &mask);
		while (write(p->p_fd, (UnivPtr) buf, nbytes) <  0)
			(void) select(p->p_fd + 1, (fd_set *)NULL, &mask, (fd_set *)NULL,
				(struct timeval *)NULL);
	}
}

# ifdef STDARGS
private void
proc_strt(char *bufname, bool clobber, const char *procname, ...)
# else
private /*VARARGS2*/ void
proc_strt(bufname, clobber, procname, va_alist)
	char	*bufname;
	bool	clobber;
	const char	*procname;
	va_dcl
# endif
{
	va_list	ap;
	char	*argv[32];
	Window *owind = curwind;
	pid_t	pid;
	Process	newp;
	Buffer	*newbuf;
	int	ptyfd = -1;
	int	slvptyfd = -1;

# if !defined(TERMIO) && !defined(TERMIOS)
#  ifdef TIOCSETD
	int	ldisc;	/* tty line discipline */
#  endif
#  ifdef TIOCLSET
	int	lmode;	/* tty local flags */
#  endif
# endif
	char	ttybuf[32];
# ifdef TERMIO
	struct termio sgt;
# endif
# ifdef TERMIOS
	struct termios sgt;
# endif
# ifdef SGTTY
	struct sgttyb sgt;
# endif

# ifdef TIOCGWINSZ
	struct winsize win;
# else
#  ifdef BTL_BLIT
#  include <sys/jioctl.h>
	struct jwinsize jwin;
#  endif
# endif

	strcpy(ttybuf, "?");	 /* to safely print when debugging */
	untieDeadProcess(buf_exists(bufname));
	isprocbuf(bufname);	/* make sure BUFNAME is either nonexistant
				   or is of type B_PROCESS */
	va_init(ap, procname);
	make_argv(argv, ap);
	va_end(ap);
	if (access(argv[0], X_OK) != 0) {
		complain("[Couldn't access shell %s: %s]", argv[0], strerror(errno));
		/* NOTREACHED */
	}

# ifdef IRIX_PTYS
	/* _getpty may fork off a child to execute mkpts to make a slave pty
	 * in /dev (if we need more) and set the correct ownership and
	 * modes on the slave pty.  Since _getpty uses waitpid this works
	 * fine with regard to our other children, but we do have to be
	 * prepared to catch a SIGCHLD for an unknown child and ignore it ...
	 */
	{
		register char	*s = _getpty(&ptyfd, O_RDWR | O_NDELAY, 0600, 0);

		if (s == NULL) {
			message("[No pty from getpty!]");
			goto fail;
		}
		(void)strcpy(ttybuf, s);
		jdbg("IRIX_PTYS _getpty ptyfd %d %s\n", ptyfd, ttybuf);
	}
# endif /* IRIX_PTYS */
# ifdef SVR4_PTYS
	if ((ptyfd = open("/dev/ptmx", O_RDWR | O_BINARY)) < 0) {
		message("[No pty from /dev/ptmx!]");
		goto fail;
	}
	jdbg("SVR4_PTYS ptmx ptyfd %d\n", ptyfd);
#  ifndef GRANTPT_BUG
	/* grantpt() seems to be implemented using a fork/exec.
	 * This is done to allow grantpt do do priviledged things
	 * via a setuid program.  One consequence is that JOVE's
	 * SIGCLD/SIGCHLD handler must not do a wait that would
	 * reap the grantpt process.  So much for library routines
	 * being black boxes.  Worse, this restriction is not documented.
	 */
	if (grantpt(ptyfd) < 0) {
		message("[grantpt failed]");
		goto fail;
	}
	if (unlockpt(ptyfd) < 0) {
		message("[unlockpt failed]");
		goto fail;
	}
	jdbg("SVR4_PTYS grantpt and unlockpt ptyfd %d\n", ptyfd);
#  endif /* !GRANTPT_BUG */
#ifdef TIOCGPTPEER
	/* if we have TIOCGPTPEER (Linux post-2017), much better to use it! */
	slvptyfd = ioctl (ptyfd, TIOCGPTPEER, O_RDWR | O_NOCTTY);
	if (slvptyfd == -1 && errno != EINVAL && errno != ENOTTY) {
		message("[ioctl TIOCGPTPEER failed]");
		goto fail;
	}
	jdbg("SVR4_PTYS TIOCGPTPEER ptyfd %d slv %d\n", ptyfd, slvptyfd);
#endif
	{
		register char	*s = ptsname(ptyfd);

		if (s == NULL) {
			message("[ptsname failed]");
			goto fail;
		}
		strcpy(ttybuf, s);
		jdbg("SVR4_PTYS ptyfd %d %s\n", ptyfd, ttybuf);
	}
#  ifdef TIOCFLUSH
	jdbg("TIOCFLUSH ptyfd %d\n", ptyfd);
	(void) ioctl(ptyfd, TIOCFLUSH, (UnivPtr) NULL);	/* ??? why? */
#  endif
# endif /* SVR4_PTYS */
# ifdef BSD_PTYS
#  ifdef USE_OPENPTY
	if (openpty(&ptyfd, &slvptyfd, ttybuf, NULL, NULL) < 0)
	{
		message("[No pty from openpty!]");
		goto fail;
	}
	jdbg("openpty ptyfd %d slv %d %s\n", ptyfd, slvptyfd, ttybuf);
#  else /* !USE_OPENPTY */
	{
		register const char	*s;

		for (s = "pqrs"; ptyfd<0; s++) {
			register const char	*t;

			if (*s == '\0') {
				message("[Out of ptys in /dev/pty*!]");
				goto fail;
			}
			for (t = "0123456789abcdef"; *t; t++) {
				swritef(ttybuf, sizeof(ttybuf), "/dev/pty%c%c", *s, *t);
				jdbg("trying pty %s\n", ttybuf);
				if ((ptyfd = open(ttybuf, O_RDWR | O_BINARY | O_CLOEXEC)) >= 0) {
					ttybuf[5] = 't';	/* pty => tty */
					/* Make sure other end is available too */
					slvptyfd = open(ttybuf, O_RDWR | O_BINARY | O_CLOEXEC);
					if (slvptyfd > 0)
						break;	/* it worked: use this one */

					/* can't open, so give up on this pty */
					(void) close(ptyfd);
					ptyfd = slvptyfd = -1;
				}
			}
		}
	}
#  endif /* !USE_OPENPTY */
# endif /* BSD_PTYS */
	/* Check that we can write to the pty, else things will fail in the
	 * child, where they're harder to detect.  This will not work with
	 * GRANTPT_BUG because the grantpt and unlockpt have not been done yet.
	 */
# ifndef GRANTPT_BUG
	if (access(ttybuf, R_OK | W_OK) != 0) {
		s_mess("[Couldn't access %s: %s]", ttybuf, strerror(errno));
		goto fail;
	}
	jdbg("can access pty %s\n", ttybuf);
# endif /* !GRANTPT_BUG */

# if !defined(TERMIO) && !defined(TERMIOS)
#  ifdef TIOCGETD
	jdbg("TIOCGETD %s\n", ttybuf);
	(void) ioctl(0, TIOCGETD, (UnivPtr) &ldisc);
#  endif
#  ifdef TIOCLGET
	jdbg("TIOCLGET %s\n", ttybuf);
	(void) ioctl(0, TIOCLGET, (UnivPtr) &lmode);
#  endif
# endif /* !defined(TERMIO) && !defined(TERMIOS) */

# ifdef TIOCGWINSZ
	jdbg("TIOCGWINSZ %s\n", ttybuf);
	(void) ioctl(0, TIOCGWINSZ, (UnivPtr) &win);
# else
#  ifdef BTL_BLIT
	jdbg("JWINSIZE %s\n", ttybuf);
	(void) ioctl(0, JWINSIZE, (UnivPtr) &jwin);
#  endif /* BTL_BLIT */
# endif

	jdbg("before fork, ptyfd %d slv %d buf %s\n", ptyfd, slvptyfd, ttybuf);
	switch (pid = fork()) {
	case -1:
		/* fork failed */
		s_mess("[Fork failed! %s]", strerror(errno));
		goto fail;

	case 0:
		/* child process */

# ifdef GRANTPT_BUG
		/* grantpt() seems to be implemented using a fork/exec.
		 * This is done to allow grantpt do do priviledged things
		 * via a setuid program.  One consequence is that JOVE's
		 * SIGCLD/SIGCHLD handler must not do a wait that would
		 * reap the grantpt process.  So much for library routines
		 * being black boxes.  Worse, this restriction is not documented.
		 *
		 * Worse still, at least Solaris 2.0 and SVR4.0 appear to have
		 * a bug: the wait is not restarted once interrupted.  If the wait
		 * is interrupted, (1) the grantpt will return a failure status
		 * and (2) the process will not be reaped -- it will remain a
		 * zombie until JOVE exits or happens to reap it accidentally.
		 * Any interrupt could cause the premature end of the wait,
		 * but SIGCHLD (actually, SIGCLD) is the most probable.
		 *
		 * To dodge these bullets, we moved the grantpt into the child
		 * where we can turn off signal handling.  Too bad this prevents
		 * us giving good diagnostics for grantpt and unlockpt.
		 * I hope I've found all the signals caught everywhere else.
		 */
		jdbg("child grantpt bug, ptyfd %d slv %d buf %s\n", ptyfd, slvptyfd, ttybuf);
		(void) setsighandler(SIGCHLD, SIG_DFL);
		(void) setsighandler(SIGWINCH, SIG_DFL);
		(void) setsighandler(SIGALRM, SIG_DFL);
		(void) setsighandler(SIGINT, SIG_DFL);
		/* (void) setsighandler(SIGQUIT, SIG_DFL); */	/* dead anyway */
		/* (void) setsighandler(SIGHUP, SIG_DFL); */	/* dead anyway */
		/* (void) setsighandler(SIGTERM, SIG_DFL); */	/* no longer used */
		/* (void) setsighandler(SIGBUS, SIG_DFL); */	/* dead anyway */
		/* (void) setsighandler(SIGSEGV, SIG_DFL); */	/* dead anyway */
		/* (void) setsighandler(SIGPIPE, SIG_DFL); */	/* dead anyway */

		if (grantpt(ptyfd) < 0) {
			_exit(errno + 1);
		}

		if (unlockpt(ptyfd) < 0) {
			_exit(errno + 1);
		}
# endif /* GRANTPT_BUG */
		jcloseall();
		(void) close(0);
		(void) close(1);
		(void) close(2);

# ifdef TERMIOS
		jdbg("setsid %s\n", ttybuf);
		setsid();
# else /* !TERMIOS */
#  ifdef TIOCNOTTY
		/* get rid of controlling tty */
		{
			int	i = open("/dev/tty", O_RDWR | O_BINARY | O_CLOEXEC);

			jdbg("TIOCNOTTY %d %s\n", i, ttybuf);
			if (i >= 0) {
				(void) ioctl(i, TIOCNOTTY, (UnivPtr)NULL);
				(void) close(i);
			}
		}
#  endif /* TIOCNOTTY */
# endif /* !TERMIOS */
		jdbg("child ptyfd %d slv %d buf %s\n", ptyfd, slvptyfd, ttybuf);
		if (slvptyfd >= 0) {
			/* use what we got from openpty */
			(void) dup2(slvptyfd, 0);
		} else {
			if ((slvptyfd = open(ttybuf, O_RDWR | O_BINARY | O_CLOEXEC)) != 0)
				_exit(errno+1);
		}
		(void) dup2(slvptyfd, 1);
		(void) dup2(slvptyfd, 2);

# ifdef BSD_PTYS
		jdbg("closed slv %d %s\n", slvptyfd, ttybuf);
		close(slvptyfd);	/* safe to close now that std* are open */
# endif /* BSD_PTYS */

# ifdef SVR4_PTYS
#  ifdef I_PUSH		/* LINUX/glibc no longer even pretends to support this (2008) */
		jdbg("SVR4_PTYS I_PUSH ptem ldterm ttcompat %s\n", ttybuf);
		(void) ioctl(0, I_PUSH, (UnivPtr) "ptem");
		(void) ioctl(0, I_PUSH, (UnivPtr) "ldterm");
		(void) ioctl(0, I_PUSH, (UnivPtr) "ttcompat");
#  endif
# endif

# ifdef TIOCSCTTY
		/* Make this controling tty.
		 * This is needed by OSF.  It may be needed by BSDPOSIX systems.
		 * It should not hurt on any system that defines TIOCSCTTY.
		 */
		jdbg("TIOCSTTY %s\n", ttybuf);
		if (ioctl(0, TIOCSCTTY) == -1)
			_exit(errno+1);	/* no good way to signal user */

# endif

# ifndef NO_TIOCREMOTE
		/* The TIOCREMOTE ioctl prevents the pty code from doing
		 * input editing on characters from the master.  In particular
		 * we don't want ERASE, KILL, INTERRUPT,  QUIT, etc. characters
		 * interpreted.
		 * On SVR4, this must be done *after* the slave side is set up.
		 * The easiest way to ensure this is to put it here, in the child
		 * after the slave is opened and has the STREAMS modules pushed,
		 * and before the master is closed.
		 */
		{
			int	on = 1;

			jdbg("chuld TIOCREMOTE on %d %s\n", ptyfd, ttybuf);
			if (ioctl(ptyfd, TIOCREMOTE, (UnivPtr) &on) < 0)
				_exit(errno+1);	/* no good way to signal user */
		}
# endif
		jdbg("child closing %d %s\n", ptyfd, ttybuf);
		close(ptyfd);

# if !defined(TERMIO) && !defined(TERMIOS)
#  ifdef TIOCSETD
		jdbg("child TIOCSETD %s\n", ttybuf);
		(void) ioctl(0, TIOCSETD, (UnivPtr) &ldisc);
#  endif
#  ifdef TIOCLSET
		jdbg("child TIOCLSET %s\n", ttybuf);
		(void) ioctl(0, TIOCLSET, (UnivPtr) &lmode);
#  endif
#  ifdef TIOCSETC
		(void) ioctl(0, TIOCSETC, (UnivPtr) &tc[NO]);
#  endif
#  ifdef USE_TIOCSLTC
		jdbg("child TIOCSLTC %s\n", ttybuf);
		(void) ioctl(0, TIOCSLTC, (UnivPtr) &ls[NO]);
#  endif
# endif /* !defined(TERMIO) && !defined(TERMIOS) */

# ifdef TIOCGWINSZ
		jdbg("child TIOCSWINSZ %s\n", ttybuf);
		win.ws_row = curwind->w_height;
		(void) ioctl(0, TIOCSWINSZ, (UnivPtr) &win);
# else /* !TIOCGWINSZ */
#  ifdef BTL_BLIT
		jdbg("child JSWINSIZE %s\n", ttybuf);
		jwin.bytesy = curwind->w_height;
		(void) ioctl(0, JSWINSIZE, (UnivPtr) &jwin);
#  endif
# endif /* !TIOCGWINSZ */

# if defined(TERMIO) || defined(TERMIOS)
		jdbg("child set termio(s)%s\n", ttybuf);
		sgt = sg[NO];
		sgt.c_iflag &= ~(IGNBRK | BRKINT | ISTRIP | INLCR | IGNCR | ICRNL
#  ifdef IXANY	/* not in QNX */
			| IXANY
#  endif
			| IXON | IXOFF);
		sgt.c_lflag &= ~(ECHO);
		sgt.c_oflag &= ~(ONLCR | TABDLY);
#  ifdef TERMIO
		jdbg("child TCSETAW %s\n", ttybuf);
		do {} while (ioctl(0, TCSETAW, (UnivPtr) &sgt) < 0 && errno == EINTR);
#  endif
#  ifdef TERMIOS
		jdbg("child TCSADRAIN %s\n", ttybuf);
		do {} while (tcsetattr(0, TCSADRAIN, &sgt) < 0 && errno == EINTR);
#  endif
# else /* !(defined(TERMIO) || defined(TERMIOS)) */
		jdbg("child set sgt %s\n", ttybuf);
		sgt = sg[NO];
		sgt.sg_flags &= ~(ECHO | CRMOD | ANYP | ALLDELAY | RAW | LCASE | CBREAK | TANDEM);
		(void) stty(0, &sgt);
# endif /* !(defined(TERMIO) || defined(TERMIOS)) */

		jdbg("newpg %s\n", ttybuf);
		NEWPG();
		{
			int	i = getpid();

# ifdef POSIX_PROCS
			jdbg("tcsetpgrp %d %s\n", i, ttybuf);
			tcsetpgrp(0, getpid());
# else /* !POSIX_PROCS */
			jdbg("TIOCSPGRP %d %s\n", i, ttybuf);
			(void) ioctl(0, TIOCSPGRP, (UnivPtr) &i);
# endif /* POSIX_PROCS */
		}

		set_process_env();
		jdbg("ready to exec %s %s\n", argv[0], argv[1]);
		execvp(argv[0], &argv[1]);
		raw_complain("execvp failed! %s", strerror(errno));
		_exit(errno + 1);
	}

# ifdef BSD_PTYS
	jdbg("closing slv %d %s\n", slvptyfd, ttybuf);
	close(slvptyfd);
# endif /* BSD_PTYS */

	newp = (Process) emalloc(sizeof *newp);

# ifdef O_NDELAY
	jdbg("O_NDELAY %d %s\n", ptyfd, ttybuf);
	fcntl(ptyfd, F_SETFL, O_NDELAY);
# endif
# ifdef O_NONBLOCK
	jdbg("O_NONBLOCK %d %s\n", ptyfd, ttybuf);
	fcntl(ptyfd, F_SETFL, O_NONBLOCK);
# endif
	jdbg("started pid %d fd %d %s\n", pid, ptyfd, ttybuf);
	newp->p_fd = ptyfd;
	newp->p_pid = pid;

	newbuf = do_select((Window *)NULL, bufname);
	newbuf->b_type = B_PROCESS;
	newp->p_buffer = newbuf;
	newbuf->b_process = newp;	/* sorta circular, eh? */
	pop_wind(bufname, clobber, B_PROCESS);
	/* Pop_wind() after everything is set up; important!
	 * Bindings won't work right unless newbuf->b_process is already
	 * set up BEFORE NEWBUF is first SetBuf()'d.
	 */
	ToLast();
	if (!bolp())
		LineInsert(1);

	newp->p_name = copystr(procname);
	newp->p_io_state = IO_NEW;
	newp->p_child_state = C_LIVE;
	newp->p_mark = MakeMark(curline, curchar);

	newp->p_next = procs;
	procs = newp;
	FD_SET(newp->p_fd, &global_fd);
	if (global_maxfd <= newp->p_fd)
		global_maxfd = newp->p_fd + 1;
	SetWind(owind);
	return;

fail:
	jdbg("pty open fail %d %d %s\n", ptyfd, slvptyfd, ttybuf);
	if (ptyfd >= 0)
		close(ptyfd);
# ifdef BSD_PTYS
	if (slvptyfd >= 0)
		close(slvptyfd);
# endif /* BSD_PTYS */
}

/* NOTE 1: SIGCHLD is an asynchronous signal.  To safely handle it,
 * the sigchld_handler simply sets a flag requesting later processing.
 * reap_procs is called synchronously to do the actual work.
 *
 * NOTE 2: SIGCHLD is "level triggered" on some systems (HPUX, IRIX, ...).
 * If the signal signal handler is re-established before the child is reaped,
 * another signal will be generated immediately.  For this reason, we
 * defer the re-establishment until reap_procs is done.
 */

volatile bool	procs_to_reap = NO;

/*ARGSUSED*/
SIGRESTYPE
sigchld_handler(junk)
int	UNUSED(junk);	/* needed for signal handler; not used */
{
	procs_to_reap = YES;
	return SIGRESVALUE;
}

void
reap_procs()
{
	wait_status_t	w;
	register pid_t	pid;

	for (;;) {
		pid = wait_opt(&w, (WNOHANG | WUNTRACED));
		if (pid <= 0) {
			if (procs_to_reap) {
				resetsighandler(SIGCHLD, sigchld_handler);
				procs_to_reap = NO;
				/* go around once more to avoid window of vulnerability */
			} else {
				break;
			}
		} else {
			kill_off(pid, w);
		}
	}
}

void
closeiprocs()
{
	Process	p;

	for (p=procs; p!=NULL; p=p->p_next)
		if (p->p_fd >= 0)
			close(p->p_fd);
}

#endif /* !PIPEPROCS */

char	proc_prompt[128] = "% ";	/* VAR: process prompt */

const char *
pstate(p)
Process	p;
{
	static const char	*const ios_name[] = { "New", "Running", "EOFed" };

	if (p == NULL)
		return "No process";

	/* Try *not* to report the whole cross-product of child and IO states. */
	switch (p->p_child_state) {
	case C_LIVE:
		return ios_name[p->p_io_state];
	case C_STOPPED:
		return io_eofed(p)? "Stopped; EOFed" : "Stopped";
	case C_EXITED:
		if (io_eofed(p) && p->p_reason == 0)
			return "Done";
		/* FALLTHROUGH */
	default:
		return sprint("%s %d%s",
			(p->p_child_state == C_EXITED? "Exited" : "Killed"),
			p->p_reason,
			(io_eofed(p)? "" : "; not EOFed"));
	}
}

bool
KillProcs()
{
	register Process	p;
	bool	asked = NO;

	for (p = procs; p != NULL; p = p->p_next) {
		if (!dead(p)) {
			if (!asked) {
				if (!yes_or_no_p("Some interactive processes are still running; leave anyway? "))
					return NO;	/* processes not killed */
				asked = YES;
			}
			if (!child_dead(p))
				(void) proc_kill(p, SIGKILL);
			proc_close(p);
		}
	}
	return YES;	/* processes killed */
}

/* VAR: dbx-mode parse string */
char	dbx_parse_fmt[128] = "line \\([0-9]*\\) in \\{file\\|\\} *\"\\([^\"]*\\)\"";

private void
watch_input(m)
Mark	*m;
{
	Bufpos	save;
	char	fname[FILESIZE],
		lineno[FILESIZE];
	int	lnum;
	Window	*savew = curwind;
	Buffer	*buf;

	DOTsave(&save);
	ToMark(m);
	if (dosearch(dbx_parse_fmt, FORWARD, YES) != NULL) {
		get_FL_info(fname, lineno);
		buf = do_find((Window *)NULL, fname, YES, YES);
		pop_wind(buf->b_name, NO, -1);
		lnum = atoi(lineno);
		SetLine(next_line(buf->b_first, lnum - 1));
		SetWind(savew);
	}
	SetDot(&save);
}

/* Process receive: receives the characters in buf, and appends them to
 * the buffer associated with p.
 */
private void
proc_rec(p, buf, len)
register Process	p;
char	*buf;
size_t	len;
{
	Buffer	*saveb = curbuf;
	register Window	*w;
	register Mark	*savepoint;
	bool	sameplace,
		do_disp;

	if (curwind->w_bufp == p->p_buffer)
		w = curwind;
	else
		w = windbp(p->p_buffer);	/* Is this window visible? */
	do_disp = w != NULL && in_window(w, p->p_mark->m_line) != -1;
	SetBuf(p->p_buffer);
	savepoint = MakeMark(curline, curchar);
	ToMark(p->p_mark);		/* where output last stopped */
	sameplace = savepoint->m_line == curline && savepoint->m_char == curchar;

	/* insert contents of buffer, carefully translating NUL to ^@ */
	buf[len] = '\0';	/* NUL-terminate buffer */
	while (len != 0) {
		if (*buf == '\0') {
			ins_str_wrap("^@", YES, WrapProcessLines ? CO-1 : LBSIZE-1);
			buf++;
			len--;
		} else {
			size_t	sublen = strlen(buf);

			ins_str_wrap(buf, YES, WrapProcessLines ? CO-1 : LBSIZE-1);
			buf += sublen;
			len -= sublen;
		}
	}

	if (do_disp && BufMinorMode(p->p_buffer, DbxMode))
		watch_input(p->p_mark);
	MarkSet(p->p_mark, curline, curchar);
	if (!sameplace)
		ToMark(savepoint);	/* back to where we were */
	DelMark(savepoint);
	/* redisplay now, instead of right after the ins_str, so that
	 * we don't get a bouncing effect if point is not the same as
	 * the process output position
	 */
	if (do_disp) {
		w->w_line = curline;
		w->w_char = curchar;
		redisplay();
	}
	SetBuf(saveb);
}

private bool
proc_kill(p, sig)
register Process	p;
int	sig;
{
	if (p == NULL)
		complain("[no process]");
	if (!child_dead(p)) {
		if (killpg(p->p_pid, sig) != -1)
			return YES;

		s_mess("Cannot kill %s!", proc_bufname(p));
	}
	return NO;
}

/* Free process CHILD.  Do all the necessary cleaning up (closing fd's,
 * etc.).
 */
private void
free_proc(child)
Process	child;
{
	register Process
		p,
		prev = NULL;

	if (!dead(child))
		return;

	untieDeadProcess(child->p_buffer);

	for (p = procs; p != child; prev = p, p = p->p_next)
		;

	if (prev == NULL)
		procs = child->p_next;
	else
		prev->p_next = child->p_next;
	proc_close(child);		/* if not already closed */

	free((UnivPtr) child->p_name);
	free((UnivPtr) child);
}

void
untieDeadProcess(b)
register Buffer	*b;
{
	if (b != NULL) {
		register Process	p = b->b_process;

		if (p != NULL) {
			Buffer	*old = curbuf;

			if (!dead(p))
				complain("Process %s, attached to %b, is %s.",
					 proc_cmd(p), b, pstate(p));
			SetBuf(p->p_buffer);
			DelMark(p->p_mark);
			SetBuf(old);
			p->p_buffer = NULL;
			b->b_process = NULL;
		}
	}
}

void
ProcList()
{
	register Process
		p,
		next;
	const char	*fmt = "%-15s  %-15s  %-8s %s";
	char
		pidstr[16];

	if (procs == NULL) {
		message("[No subprocesses]");
		return;
	}
	TOstart("Process list");

	Typeout(fmt, "Buffer", "Status", "Pid ", "Command");
	Typeout(fmt, "------", "------", "--- ", "-------");
	for (p = procs; p != NULL; p = next) {
		next = p->p_next;
		swritef(pidstr, sizeof(pidstr), "%d", (int) p->p_pid);
		Typeout(fmt, proc_bufname(p), pstate(p), pidstr, p->p_name);
		if (dead(p)) {
			free_proc(p);
			UpdModLine = YES;
		}
	}
	TOstop();
}

private void
do_rtp(mp)
register Mark	*mp;
{
	register Process	p = curbuf->b_process;
	LinePtr	line1 = curline,
		line2 = mp->m_line;
	int	char1 = curchar,
		char2 = mp->m_char;
	char	*gp;
	size_t	nbytes;

	if (dead(p) || p->p_buffer != curbuf)
		return;

	(void) fixorder(&line1, &char1, &line2, &char2);
	while (line1 != line2->l_next) {
		gp = ltobuf(line1, genbuf) + char1;
		if (line1 == line2) {
			nbytes = char2 - char1;
		} else {
			genbuf[Jr_Len] = EOL;	/* replace NUL with EOL */
			nbytes = Jr_Len - char1 + 1;	/* and include it */
		}
		if (nbytes != 0)
			proc_write(p, gp, nbytes);
		line1 = line1->l_next;
		char1 = 0;
	}
}

void
ProcNewline()
{
#ifdef ABBREV
	MaybeAbbrevExpand();
#endif
	SendData(YES);
}

void
ProcSendData()
{
#ifdef ABBREV
	MaybeAbbrevExpand();
#endif
	SendData(NO);
}

private void
SendData(newlinep)
bool	newlinep;
{
	register Process	p = curbuf->b_process;
	register char	*lp,
			*gp;	/* JF fix for better prompt handling */

	if (dead(p))
		return;

	/* If the process mark was involved in a big deletion, because
	 * the user hit ^W or something, then let's do some magic with
	 * the process mark.  Problem is that if the user yanks back the
	 * text he deleted, the mark stays at the beginning of the region,
	 * and so the next time SendData() is called the entire region
	 * will be sent.  That's not good.  So, to deal with that we reset
	 * the mark to the last line, after skipping over the prompt, etc.
	 */
	if (p->p_mark->m_big_delete) {
		Bufpos	bp;

		p->p_mark->m_big_delete = NO;

		DOTsave(&bp);
		ToLast();
		Bol();
		/* While we're looking at a prompt, and while we're
		 * moving forward.  This is for people who accidently
		 * set their process-prompt to ">*" which will always
		 * match!
		 */
		while (LookingAt(proc_prompt, linebuf, curchar)
		&& (REeom > curchar))
			curchar = REeom;
		MarkSet(p->p_mark, curline, curchar);
		SetDot(&bp);
	}

	if (lastp(curline)) {
		Eol();
		if (newlinep)
			LineInsert(1);
		do_rtp(p->p_mark);
		MarkSet(p->p_mark, curline, curchar);
	} else {
		/* Either we're looking at a prompt, or we're not, in
		 * which case we want to strip off the beginning of the
		 * line anything that looks like what the prompt at the
		 * end of the file is.  In other words, if "(dbx) stop in
		 * ProcessNewline" is the line we're on, and the last
		 * line in the buffer is "(dbx) ", then we strip off the
		 * leading "(dbx) " from this line, because we know it's
		 * part of the prompt.  But this only happens if "(dbx) "
		 * isn't one of the process prompts ... follow what I'm
		 * saying?
		 */
		Bol();
		if (LookingAt(proc_prompt, linebuf, curchar)) {
			do {
				curchar = REeom;
			} while (LookingAt(proc_prompt, linebuf, curchar)
			&& (REeom > curchar));
			strcpy(genbuf, linebuf + curchar);
			Eof();
			ins_str(genbuf);
		} else {
			strcpy(genbuf, linebuf + curchar);
			Eof();
			gp = genbuf;
			lp = linebuf;
			while (*lp == *gp && *lp != '\0') {
				lp += 1;
				gp += 1;
			}
			ins_str(gp);
		}
	}
}

void
ShellProc()
{
	char	shbuf[20];
	register Buffer	*b;

	swritef(shbuf, sizeof(shbuf), "*shell-%d*", arg_value());
	b = buf_exists(shbuf);
	if (b == NULL || dead(b->b_process)) {
		char	cbnspace[FILESIZE];

		proc_strt(shbuf, NO, "i-shell", Shell, "-is",
			curbuf->b_fname == NULL? (char *)NULL : strcpy(cbnspace, pr_name(curbuf->b_fname, NO)),
			(char *)NULL);
	}
	pop_wind(shbuf, NO, -1);
}

void
Iprocess()
{
	char	scratch[64],
		*bnm;
	int	cnt = 1;
	Buffer	*bp;
	char	fnspace[FILESIZE];
	char	*fn = curbuf->b_fname == NULL
		? NULL : strcpy(fnspace, pr_name(curbuf->b_fname, NO));

	jamstr(ShcomBuf, ask(ShcomBuf, ProcFmt));
	bnm = MakeName(ShcomBuf);
	truncstr(scratch, bnm);
	while ((bp = buf_exists(scratch)) != NULL && !dead(bp->b_process))
		swritef(scratch, sizeof(scratch), "%s.%d", bnm, cnt++);
	proc_strt(scratch, YES, ShcomBuf, Shell, ShFlags, ShcomBuf, fn, fn, (char *)NULL);
}

pid_t	DeadPid;	/* info about ChildPid, if reaped by kill_off */
wait_status_t	DeadStatus;

#ifdef USE_PROTOTYPES
void
kill_off(pid_t pid, wait_status_t w)
#else
void
kill_off(pid, w)
register pid_t	pid;
wait_status_t	w;
#endif
{
	register Process	child;

	if (pid == ChildPid) {
		/* we are reaping the non-iproc process: record info */
		DeadPid = pid;
		DeadStatus = w;
		return;
	}

	if ((child = proc_pid(pid)) == NULL)
		return;

	UpdModLine = YES;		/* we're changing state ... */
	if (WIFSTOPPED(w))
		child->p_child_state = C_STOPPED;
	else {
#ifdef PIPEPROCS
		/* the true process status comes by pipe, not from wait().
		 * However, we must note the death of portsrv processes.
		 */
		child->p_portlive = NO;
#else /* !PIPEPROCS */
		/* record the details of the death of the process.
		 * Kludge: see if we can get any queued EOF from pty first.
		 * We wish to do this to make our message less confusing.
		 */
		while (!io_eofed(child)) {
			int	nfds;
			fd_set	reads;
			struct timeval	notime;

			FD_ZERO(&reads);
			FD_SET(child->p_fd, &reads);
			notime.tv_sec = 0;
			notime.tv_usec = 0;
			nfds = select(global_maxfd, &reads,
				(fd_set *)NULL, (fd_set *)NULL, &notime);
			if (nfds == 1)
				read_pty_proc(child->p_fd);
			else if (nfds >= 0 || errno != EINTR) {
# ifdef NO_EOF_FROM_PTY
				/* Certain systems never indicate EOF from a slave.
				 * On those systems, we force a close when the direct
				 * child terminates.
				 */
				proc_close(child);
# endif
				break;
			}
		}
		obituary(child, w);
#endif /* !PIPEPROCS */
	}
}

#ifdef USE_PROTOTYPES
private void
obituary(register Process child, wait_status_t w)
#else
private void
obituary(child, w)
register Process	child;
wait_status_t	w;
#endif
{
	UpdModLine = YES;		/* we're changing state ... */
	if (WIFEXITED(w)) {
		child->p_child_state = C_EXITED;
		child->p_reason = WEXITSTATUS(w);
	} else if (WIFSIGNALED(w)) {
		child->p_child_state = C_KILLED;
		child->p_reason = WTERMSIG(w);
	} else {
		/* presume it died peacefully! */
		child->p_child_state = C_EXITED;
		child->p_reason = 0;
	}

	{
		Buffer	*save = curbuf;
		Bufpos	bp;
		char	mesg[128];

		/* insert status message now */
		swritef(mesg, sizeof(mesg), "[Process %s: %s]\n",
			proc_cmd(child), pstate(child));
		SetBuf(child->p_buffer);
		DOTsave(&bp);
		ToLast();
		ins_str(mesg);
		SetDot(&bp);
		SetBuf(save);
		redisplay();
	}
}

#endif /* IPROCS */
