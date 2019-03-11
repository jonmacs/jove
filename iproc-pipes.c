/***************************************************************************
 * This program is Copyright (C) 1986, 1987, 1988 by Jonathan Payne.  JOVE *
 * is provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is    *
 * included in all the files.                                              *
 ***************************************************************************/

/* NOTE WELL:
 * This file is "included" into iproc.c -- it is not compiled separately!
 */

#include <sgtty.h>

int	NumProcs = 0;

#define isdead(p)	((p) == NULL || proc_state((p)) == DEAD || (p)->p_toproc == -1)

File	*ProcInput;
int	ProcOutput = -1;

pid_t	kbd_pid = -1;

private Process *
proc_pid(pid)
pid_t	pid;
{
	register Process	*p;

	for (p = procs; p != NULL; p = p->p_next)
		if (p->p_portpid == pid)
			break;

	return p;
}

void
read_proc(pid, nbytes)
pid_t	pid;
register int	nbytes;
{
	register Process	*p = proc_pid(pid);

	if (p == NULL) {
		writef("\riproc: unknown pid (%d)", (int)pid);
	} else if (proc_state(p) == NEW) {
		/* first message: pid of real child, not of portsrv */
		pid_t	rpid;

		(void) f_readn(ProcInput, (char *) &rpid, sizeof(pid_t));
		p->p_pid = rpid;
		p->p_state = RUNNING;
	} else if (nbytes == EOF) {
		/* okay to clean up this process */
		wait_status_t	status;
		pid_t	dead_pid;

		(void) f_readn(ProcInput, (char *) &status, sizeof(status));
		do {
			dead_pid = wait(&status);
			if (dead_pid < 0)
				break;
			kill_off(dead_pid, status);
		} while (dead_pid != p->p_portpid);
		proc_close(p);
		makedead(p);
	} else {
		/* regular data */
		while (nbytes > 0) {
			char	ibuf[512];
			size_t n = f_readn(ProcInput, ibuf,
				(size_t)min((int)(sizeof ibuf) - 1, nbytes));

			ibuf[n] = '\0';	/* Null terminate for convenience */
			nbytes -= n;
			proc_rec(p, ibuf);
		}
	}
}

void
ProcKill()
{
	proc_kill(curbuf->b_process, SIGKILL);
}

void
ProcInt()
{
	proc_kill(curbuf->b_process, SIGINT);
}

void
ProcQuit()
{
	proc_kill(curbuf->b_process, SIGQUIT);
}

private void
proc_close(p)
Process	*p;
{
	if (p->p_toproc >= 0) {
		(void) close(p->p_toproc);
		p->p_toproc = -1;	/* writes will fail */
		NumProcs -= 1;
	}
}

private void
proc_write(p, buf, nbytes)
Process	*p;
char	*buf;
size_t	nbytes;
{
	(void) write(p->p_toproc, (UnivConstPtr)buf, nbytes);
}


#ifdef	STDARGS
private void
proc_strt(char *bufname, int clobber, ...)
#else
private /*VARARGS3*/ void
proc_strt(bufname, clobber, va_alist)
	char	*bufname;
	int	clobber;
	va_dcl
#endif
{
	Window	*owind = curwind;
	int	toproc[2];
	pid_t	pid;
	Process	*newp;
	Buffer	*newbuf;
	char	*argv[32],
		*cp,
		cmdbuf[LBSIZE];
	va_list	ap;

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
		va_init(ap, clobber);
		make_argv(&argv[1], ap);
		va_end(ap);
		(void) dup2(toproc[0], 0);
		(void) dup2(ProcOutput, 1);
		(void) dup2(ProcOutput, 2);
		pipeclose(toproc);
		jcloseall();
		jputenv("EMACS=t");
		jputenv("TERM=emacs");
		jputenv("TERMCAP=emacs:co#80:tc=unknown:");
		execv(Portsrv, argv);
		raw_complain("execl failed: %s\n", strerror(errno));
		_exit(1);
	}

	newp = (Process *) emalloc(sizeof *newp);
	newp->p_next = procs;
	newp->p_state = NEW;

	cmdbuf[0] = '\0';
	va_init(ap, clobber);
	while ((cp = va_arg(ap, char *)) != NULL) {
		size_t	pl = strlen(cmdbuf);

		swritef(&cmdbuf[pl], sizeof(cmdbuf)-pl, "%s ", cp);
	}
	va_end(ap);
	va_init(ap, clobber);
	newp->p_name = copystr(cmdbuf);
	procs = newp;
	newp->p_portpid = pid;
	newp->p_pid = -1;

	newbuf = do_select((Window *)NULL, bufname);
	newbuf->b_type = B_PROCESS;
	newp->p_buffer = newbuf;
	newbuf->b_process = newp;	/* sorta circular, eh? */
	pop_wind(bufname, clobber, B_PROCESS);
	ToLast();
	if (!bolp())
		LineInsert(1);
	/* Pop_wind() after everything is set up; important!
	   Bindings won't work right unless newbuf->b_process is already
	   set up BEFORE NEWBUF is first SetBuf()'d. */
	newp->p_mark = MakeMark(curline, curchar, M_FLOATER);
	newp->p_dbx_mode = NO;

	newp->p_toproc = toproc[1];
	newp->p_reason = 0;
	(void) close(toproc[0]);
	SetWind(owind);
}

void
closeiprocs()
{
	Process	*p;

	if (ProcOutput != -1)
		close(ProcOutput);
	for (p=procs; p!=NULL; p=p->p_next)
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

	(void) pipe(p);
	ProcInput = fd_open("process-input", F_READ|F_LOCKED, p[0],
			    (char *)NULL, 512);
	ProcOutput = p[1];
	switch (kbd_pid = fork()) {
	case -1:
		complain("Cannot fork kbd process! %s\n", strerror(errno));
		/* NOTREACHED */

	case 0:
		signal(SIGINT, SIG_IGN);
		signal(SIGALRM, SIG_IGN);
		close(1);
		dup(ProcOutput);
		jcloseall();
		execl(Portsrv, "kbd", (char *)NULL);
		raw_complain("kbd exec failed: %s\n", strerror(errno));
		exit(-1);
	}
}

/* kbd_stop() returns true if it changes the state of (i.e. stops)
   the keyboard process.  This is so kbd stopping and starting in
   pairs works - see finish() in jove.c. */

private int	kbd_state = NO;

void
kbd_strt()
{
	if (kbd_state == NO) {
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
	if (kbd_state == YES) {
		kbd_state = NO;
		kill(kbd_pid, KBDSIG);
		return YES;
	}
	return NO;
}

void
kbd_kill()
{
	if (kbd_pid != -1) {
		kill(kbd_pid, SIGKILL);
		kbd_pid = -1;
	}
}
