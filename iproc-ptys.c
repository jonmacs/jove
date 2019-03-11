/***************************************************************************
 * This program is Copyright (C) 1986, 1987, 1988 by Jonathan Payne.  JOVE *
 * is provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is    *
 * included in all the files.                                              *
 ***************************************************************************/

/* NOTE WELL:
 * This file is "included" into iproc.c -- it is not compiled separately!
 */

#include <sys/time.h>
#include <fcntl.h>
#include "ttystate.h"
#include "select.h"

#ifdef SVR4_PTYS
# include <sys/stropts.h>
  extern char	*ptsname proto((int /*filedes*/));	/* get name of slave */
#endif

#ifdef IRIX_PTYS
# include <sys/sysmacros.h>
# include <sys/stat.h>
#endif

#define isdead(p)	((p) == NULL || proc_state((p)) == DEAD || (p)->p_fd == -1)

private Process *
proc_pid(pid)
pid_t	pid;
{
	register Process	*p;

	for (p = procs; p != NULL; p = p->p_next)
		if (p->p_pid == pid)
			break;

	return p;
}

void
read_proc(fd)
register int	fd;
{
	register Process	*p;
	int	n;
	char	ibuf[1024];

	for (p = procs; p != NULL; p = p->p_next)
		if (p->p_fd == fd)
			break;

	if (p == NULL) {
		writef("\riproc: unknown fd %d", fd);
		return;
	}

	n = read(fd, (UnivPtr) ibuf, sizeof(ibuf) - 1);
	if (n == -1 && (errno == EIO || errno == EWOULDBLOCK)) {
		if (proc_state(p) == NEW)
			return;
		proc_close(p);
		makedead(p);
		return;
	} else {
		if (proc_state(p) != RUNNING) {
			proc_state(p) = RUNNING;
			UpdModLine = YES;
		}
	}
	if (n <= 0) {
		if (n == 0)
			strcpy(ibuf, "[Process EOF]");
		else
			swritef(ibuf, sizeof(ibuf),
				"\n[pty read error: %s]\n", strerror(errno));
		proc_close(p);
	} else
		ibuf[n] = '\0';
	proc_rec(p, ibuf);
}

void
ProcKill()
{
	register Buffer	*b;
	char	*bname;

	bname = ask_buf(curbuf);

	if ((b = buf_exists(bname)) == NULL)
		complain("[No such buffer]");
	if (b->b_process == NULL)
		complain("%s not tied to a process.", bname);
	proc_kill(b->b_process, SIGKILL);
}

void
ProcCont()
{
	Process	*p;

	if ((p = curbuf->b_process) == NULL)
		complain("[No process]");
	if (p->p_state != DEAD) {
		proc_kill(p, SIGCONT);
		UpdModLine = YES;
		p->p_state = RUNNING;
	}
}

private void
send_p(c)
char	c;
{
	Process	*p;
	char	buf[2];

	if ((p = curbuf->b_process) == NULL)
		complain("[No process]");
	ToLast();
	buf[0] = c;
	buf[1] = '\0';
	proc_rec(p, buf);
	(void) write(p->p_fd, (UnivPtr) &c, sizeof(c));
}

void
ProcEof()
{
#if defined(TERMIO) || defined(TERMIOS)
	send_p(sg[NO].c_cc[VEOF]);
#else
	send_p(tc[NO].t_eofc);
#endif
}

void
ProcInt()
{
#if defined(TERMIO) || defined(TERMIOS)
	send_p(sg[NO].c_cc[VINTR]);
#else
	send_p(tc[NO].t_intrc);
#endif
}

void
ProcQuit()
{
#if defined(TERMIO) || defined(TERMIOS)
	send_p(sg[NO].c_cc[VQUIT]);
#else
	send_p(tc[NO].t_quitc);
#endif
}

void
ProcStop()
{
#if defined(TERMIO) || defined(TERMIOS)
	send_p(sg[NO].c_cc[VSUSP]);
#else
	send_p(ls[NO].t_suspc);
#endif
}

void
ProcDStop()
{
#if defined(TERMIO) || defined(TERMIOS)
	send_p(sg[NO].c_cc[VDSUSP]);
#else
	send_p(ls[NO].t_dsuspc);
#endif
}

private void
proc_close(p)
Process *p;
{
	SigHold(SIGCHLD);	/* be mutually exclusive */

	if (p->p_fd >= 0) {
		(void) close(p->p_fd);
		FD_CLR(p->p_fd, &global_fd);
		p->p_fd = -1;
	}

	SigRelse(SIGCHLD);
}

private void
proc_write(p, buf, nbytes)
Process *p;
char	*buf;
size_t	nbytes;
{
	fd_set	mask;

	FD_ZERO(&mask);
	FD_SET(p->p_fd, &mask);
	while (write(p->p_fd, (UnivPtr) buf, nbytes) <  0)
		(void) select(p->p_fd + 1, (fd_set *)NULL, &mask, (fd_set *)NULL,
			(struct timeval *)NULL);
}

#ifdef	STDARGS
private void
proc_strt(char *bufname, int clobber, ...)
#else
private /*VARARGS2*/ void
proc_strt(bufname, clobber, va_alist)
	char	*bufname;
	int	clobber;
	va_dcl
#endif
{
	va_list	ap;
	char	*argv[32],
		*cp;
	Window *owind = curwind;
	pid_t	pid;
	Process	*newp;
	Buffer 	*newbuf;
	int	i,
		ptyfd = -1;

#if !defined(TERMIO) && !defined(TERMIOS)
# ifdef	TIOCSETD
	int	ldisc;	/* tty line discipline */
# endif
# ifdef	TIOCLSET
	int	lmode;	/* tty local flags */
# endif
#endif
	register char	*s,
			*t;
	char	ttybuf[32];
	char	cmdbuf[LBSIZE];
#ifdef TERMIO
	struct termio sgt;
#endif
#ifdef TERMIOS
	struct termios sgt;
#endif
#ifdef SGTTY
	struct sgttyb sgt;
#endif

#ifdef	TIOCGWINSZ
	struct winsize win;
#else
#  ifdef	BTL_BLIT
#  include <sys/jioctl.h>
	struct jwinsize jwin;
#  endif
#endif

	isprocbuf(bufname);	/* make sure BUFNAME is either nonexistant
				   or is of type B_PROCESS */
	va_init(ap, clobber);
	make_argv(argv, ap);
	va_end(ap);
	if (access(argv[0], X_OK) != 0) {
		complain("[Couldn't access %s: %s]", argv[0], strerror(errno));
		/* NOTREACHED */
	}

#ifdef IRIX_PTYS
	if ((ptyfd = open("/dev/ptc", O_RDWR)) < 0) {
		message("[No ptys!]");
		goto fail;
	} else {
		struct stat sb;
		
		if (fstat(ptyfd, &sb) < 0) {
			message("slave pty failed");
			close(ptyfd);
			goto fail;
		}
		(void) sprintf(ttybuf, "/dev/ttyq%d", minor(sb.st_rdev));
	}
#endif /* IRIX_PTYS */
#ifdef SVR4_PTYS
	if ((ptyfd = open("/dev/ptmx", O_RDWR)) < 0) {
		message("[No ptys!]");
		goto fail;
	}

	if ((s = ptsname(ptyfd)) == NULL) {
		close(ptyfd);
		goto fail;
	}
	strcpy(ttybuf, s);
	(void) ioctl(ptyfd, TIOCFLUSH, (UnivPtr) NULL);	/* ??? why? */
#endif /* SVR4PTYS */
#ifdef BSD_PTYS
	for (s = "pqrs"; ptyfd<0; s++) {
		if (*s == '\0')
			complain("[Out of ptys!]");
		for (t = "0123456789abcdef"; *t; t++) {
			swritef(ttybuf, sizeof(ttybuf), "/dev/pty%c%c", *s, *t);
			if ((ptyfd = open(ttybuf, 2)) >= 0) {
				ttybuf[5] = 't';    /* pty => tty */
				/* make sure both ends are available */
				if ((i = open(ttybuf, 2)) < 0) {
					(void) close(ptyfd);
					ptyfd = -1;
				} else {
					(void) close(i);
					break;
				}
			}
		}
	}
#endif /* BSD_PTYS */
	/*
	 * Check that we can write to the pty, else things will fail in the
	 * child, where they're harder to detect.  This may not work
	 * with SVR4PTYS before the grantpt and unlockpt.
	 */
#ifndef	SVR4PTYS
	if (access(ttybuf, W_OK) != 0) {
		complain("[Couldn't access %s: %s]", ttybuf, strerror(errno));
		/* NOTREACHED */
	}
#endif	/* !SVR4PTYS */
	
#if !defined(TERMIO) && !defined(TERMIOS)
# ifdef	TIOCGETD
	(void) ioctl(0, TIOCGETD, (UnivPtr) &ldisc);
# endif
# ifdef	TIOCLGET
	(void) ioctl(0, TIOCLGET, (UnivPtr) &lmode);
# endif
#endif /* !defined(TERMIO) && !defined(TERMIOS) */

#ifdef	TIOCGWINSZ
	(void) ioctl(0, TIOCGWINSZ, (UnivPtr) &win);
#else
#  ifdef	BTL_BLIT
	(void) ioctl(0, JWINSIZE, (UnivPtr) &jwin);
#  endif	/* BTL_BLIT */
#endif

	SigHold(SIGCHLD);
#ifdef	SIGWINCH
	SigHold(SIGWINCH);
#endif
	switch (pid = fork()) {
	case -1:
		/* fork failed */

		{
		int	ugh = errno;	/* hold across library calls */

		(void) close(ptyfd);
		message("[Fork failed! ");
		message(strerror(ugh));
		message("]");
		goto fail;
		}

	case 0:
		/* child process */

		SigRelse(SIGCHLD);

#ifdef SVR4_PTYS
		/* grantpt() seems to be implemented via system().
		 * This means that SIGCLD/SIGCHLD must not be caught.
		 * For this reason, we perform the grantpt and unlockpt
		 * in the child where we can ignore SIGCHLD.
		 */
		(void) signal(SIGCHLD, SIG_DFL);	/* we don't have children now */

		if (grantpt(ptyfd) < 0) {
			_exit(errno + 1);
		}

		if (unlockpt(ptyfd) < 0) {
			_exit(errno + 1);
		}
#endif
#ifdef	SIGWINCH
		SigRelse(SIGWINCH);
#endif
		jcloseall();
		(void) close(0);
		(void) close(1);
		(void) close(2);

#ifdef	TIOCNOTTY
		if ((i = open("/dev/tty", 2)) >= 0) {
			(void) ioctl(i, TIOCNOTTY, (UnivPtr)NULL);
			(void) close(i);
		}
#endif
#ifdef TERMIOS
		setsid();
#endif
		if (open(ttybuf, 2) != 0)
			exit(-1);
		(void) dup2(0, 1);
		(void) dup2(0, 2);

#ifdef SVR4_PTYS
		(void) ioctl(0, I_PUSH, (UnivPtr) "ptem");
		(void) ioctl(0, I_PUSH, (UnivPtr) "ldterm");
		(void) ioctl(0, I_PUSH, (UnivPtr) "ttcompat");
#endif

#if !defined(TERMIO) && !defined(TERMIOS)
# ifdef	TIOCSETD
		(void) ioctl(0, TIOCSETD, (UnivPtr) &ldisc);
# endif
# ifdef	TIOCLSET
		(void) ioctl(0, TIOCLSET, (UnivPtr) &lmode);
# endif
# ifdef	TIOCSETC
		(void) ioctl(0, TIOCSETC, (UnivPtr) &tc[NO]);
# endif
# ifdef	SGTTY
#  ifdef	TIOCSLTC
		(void) ioctl(0, TIOCSLTC, (UnivPtr) &ls[NO]);
#  endif
# endif
#endif /* !defined(TERMIO) && !defined(TERMIOS) */

#ifdef	TIOCGWINSZ
#    ifdef	SIGWINCH
		(void) signal(SIGWINCH, SIG_IGN);
#    endif
		win.ws_row = curwind->w_height;
		(void) ioctl(0, TIOCSWINSZ, (UnivPtr) &win);
#else	/* !TIOCGWINSZ */
#  ifdef	BTL_BLIT
		jwin.bytesy = curwind->w_height;
		(void) ioctl(0, JSWINSIZE, (UnivPtr) &jwin);
#  endif
#endif	/* !TIOCGWINSZ */

#if defined(TERMIO) || defined(TERMIOS)
		sgt = sg[NO];
		sgt.c_iflag &= ~(INLCR | ICRNL | IGNCR);
		sgt.c_iflag &= ~(BRKINT | IGNBRK | IGNPAR | ISTRIP | IXON | IXANY | IXOFF);
		sgt.c_lflag &= ~(ECHO);
		sgt.c_oflag &= ~(ONLCR);
# ifdef TERMIO
		(void) ioctl(0, TCSETAW, (UnivPtr) &sgt);
# endif
# ifdef TERMIOS
		(void) tcsetattr(0, TCSADRAIN, &sgt);
# endif
#else /* !(defined(TERMIO) || defined(TERMIOS)) */
		sgt = sg[NO];
		sgt.sg_flags &= ~(ECHO | CRMOD | ANYP | ALLDELAY | RAW | LCASE | CBREAK | TANDEM);
		(void) stty(0, &sgt);
#endif /* !(defined(TERMIO) || defined(TERMIOS)) */

#ifdef TIOCREMOTE
		{
			int	on = 1;

			(void) ioctl(0, TIOCREMOTE, (UnivPtr) &on);
		}
#endif

		i = getpid();
#ifdef POSIX_PROCS
		(void) setpgid(0, i);
		tcsetpgrp(0, i);
#else /* !POSIX_PROCS */
		(void) ioctl(0, TIOCSPGRP, (UnivPtr) &i);
		SETPGRP(0, i);
#endif /* POSIX_PROCS */

		jputenv("EMACS=t");
		jputenv("TERM=emacs");
		jputenv("TERMCAP=emacs:co#80:tc=unknown:");
		execvp(argv[0], &argv[1]);
		raw_complain("execvp failed! %s\n", strerror(errno));
		_exit(errno + 1);
	}

	newp = (Process *) emalloc(sizeof *newp);

#ifdef	O_NDELAY
	fcntl(ptyfd, F_SETFL, O_NDELAY);
#endif
#ifdef	O_NONBLOCK
	fcntl(ptyfd, F_SETFL, O_NONBLOCK);
#endif
	newp->p_fd = ptyfd;
	newp->p_pid = pid;

	newbuf = do_select((Window *)NULL, bufname);
	newbuf->b_type = B_PROCESS;
	newp->p_buffer = newbuf;
	newbuf->b_process = newp;	/* sorta circular, eh? */
	pop_wind(bufname, clobber, B_PROCESS);
	/* Pop_wind() after everything is set up; important!
	   Bindings won't work right unless newbuf->b_process is already
	   set up BEFORE NEWBUF is first SetBuf()'d. */
	ToLast();
	if (!bolp())
		LineInsert(1);

	cmdbuf[0] = '\0';
	va_init(ap, clobber);
	while ((cp = va_arg(ap, char *)) != NULL) {
		size_t	pl = strlen(cmdbuf);

		swritef(&cmdbuf[pl], sizeof(cmdbuf)-pl, "%s ", cp);
	}
	va_end(ap);

	newp->p_name = copystr(cmdbuf);
	newp->p_state = NEW;
	newp->p_reason = 0;
	newp->p_mark = MakeMark(curline, curchar, M_FLOATER);
	newp->p_dbx_mode = NO;

	newp->p_next = procs;
	procs = newp;
	FD_SET(newp->p_fd, &global_fd);
	if (global_maxfd <= newp->p_fd)
		global_maxfd = newp->p_fd + 1;
	SetWind(owind);

fail:
	SigRelse(SIGCHLD);
#ifdef	SIGWINCH
	SigRelse(SIGWINCH);
#endif
}

/*ARGSUSED*/
SIGRESTYPE
proc_child(junk)
int	junk;	/* needed for signal handler; not used */
{
	int save_errno = errno;	/* Subtle, but necessary! */
	wait_status_t	w;
	register pid_t	pid;

	for (;;) {
		pid = wait_opt(&w, (WNOHANG | WUNTRACED));
		if (pid <= 0)
			break;
		kill_off(pid, w);
	}
	errno = save_errno;
	return SIGRESVALUE;
}

void
closeiprocs()
{
	Process	*p;

	for (p=procs; p!=NULL; p=p->p_next)
		close(p->p_fd);
}
