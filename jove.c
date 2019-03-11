/***************************************************************************
 * This program is Copyright (C) 1986, 1987, 1988 by Jonathan Payne.  JOVE *
 * is provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is    *
 * included in all the files.                                              *
 ***************************************************************************/

/* Contains the main loop initializations, and some system dependent
   type things, e.g. putting terminal in CBREAK mode, etc. */

#include "jove.h"
#include "fp.h"
#include "termcap.h"
#include "ctype.h"
#include "chars.h"
#include "disp.h"
#include "reapp.h"	/* for find_tag() */
#include "sysprocs.h"
#include "rec.h"
#include "ask.h"
#include "extend.h"
#include "fmt.h"
#include "getch.h"
#include "loadavg.h"
#include "mac.h"
#include "macros.h"
#include "marks.h"
#include "proc.h"
#include "screen.h"
#include "term.h"
#include "version.h"
#include "wind.h"
#ifdef	IPROCS
# include "iproc.h"
# ifdef	PTYPROCS
#  include "select.h"
# endif
#endif

#ifdef SCO	/* ??? what is this for? */
# include <sys/stream.h>
# include <sys/ptem.h>
#endif

#include <signal.h>
#include <errno.h>

#ifdef	MAC

# include "mac.h"
# define	WINRESIZE	1

#else	/* !MAC */

# include <sys/stat.h>

#endif	/* !MAC */

#ifdef	MSDOS
# include <process.h>
# define SIGHUP	99
private	void
	break_off proto((void)),
	break_rst proto((void));
private char	switchar proto((void));

# ifdef	ZORTECH
unsigned int	_stack = 0x2000;	/* Zortech's way of specifying stack size */
# endif
#endif

private void
	ttyset proto((bool n)),
	UnsetTerm proto((char *)),
	do_sgtty proto((void)),
	DoKeys proto((bool firsttime));

#ifdef NONBLOCKINGREAD
private void	setblock proto((bool on));
#endif

/* Various tty state structures.
 * Each is an array, subscripted by one of "NO" or "YES".
 */

#ifdef	UNIX

#include "ttystate.h"

#ifdef SCO	/* ??? Surely this is needed for ODT? */
# undef TIOCGWINSZ
#endif

#ifdef	TIOCGWINSZ
#  ifdef	SIGWINCH
#    define	WINRESIZE	1
#  endif
#endif

# ifdef	BIFF
private struct stat	tt_stat;	/* for biff */
#  if	!defined(USE_FSTAT) || !defined(USE_FCHMOD)
private char	*tt_name = NULL;		/* name of the control tty */
extern char	*ttyname proto((int));		/* for systems w/o fstat ... */
#  endif
private bool	dw_biff = NO;		/* whether or not to fotz at all */
# endif	/* BIFF */

#endif	/* UNIX */

bool	errormsg;

char	NullStr[] = "";
jmp_buf	mainjmp;

#ifdef USE_SELECT
fd_set	global_fd;	/* set of file descriptors of interest (for select) */
int	global_maxfd;
#endif

SIGRESTYPE
finish(code)
int	code;
{
	int save_errno = errno;	/* Subtle, but necessary! */
	static int	Crashing = 0;	/* we are in the middle of crashing */
	bool	CoreDump = (code != 0 && code != SIGHUP),
		DelTmps = YES;		/* Usually we delete them. */

	if (code == SIGINT) {
		char	c;
#if	defined(IPROCS) && defined(PIPEPROCS)
		bool	started;
#endif

		(void) signal(code, finish);
		f_mess("Abort (Type 'n' if you're not sure)? ");
#ifdef UNIX
#if	defined(IPROCS) && defined(PIPEPROCS)
		started = kbd_stop();
# endif
		/*
		 * Yuk!  This doesn't deal with all cases, we really need a
		 * standard jove input routine that's lower than jgetchar so
		 * that this can use it.  The code that this replaces was even
		 * more ugly.  What about nonblocking reads? -- MM.
		 */
#ifdef	NONBLOCKINGREAD
		setblock(YES);	/* turn blocking on (in case it was off) */
#endif
		for (;;) {
			c = 'n';
			if (read(0, (UnivPtr) &c, sizeof(c)) < 0) {
				switch (errno) {
				case EINTR:
#if	EWOULDBLOCK != EAGAIN	/* aliases in SvR4 */
				case EWOULDBLOCK:
#endif
				case EAGAIN:
					continue;
				}
			}
			break;
		}
#if	defined(IPROCS) && defined(PIPEPROCS)
		if (started)
			kbd_strt();
# endif /* defined(IPROCS) && defined(PIPEPROCS) */
#endif /* UNIX */
#ifdef MSDOS
		c = getrawinchar();
#endif	/* MSDOS */
		message(NullStr);
		if ((c & 0377) != 'y') {
			redisplay();
			errno = save_errno;
			return SIGRESVALUE;
		}
	}
	DisabledRedisplay = YES;
#ifndef	MAC
	UnsetTerm(NullStr);
#endif
#if	defined(IPROCS) && defined(PIPEPROCS)
	kbd_kill();		/* kill the keyboard process */
#endif
#ifndef	MSDOS
	if (code != 0) {
		if (!Crashing) {
			Crashing = YES;
			lsave();
			SyncRec();
			writef("JOVE CRASH!! (code %d; last errno %d)\n",
				code, save_errno);
			if (ModBufs(YES)) {
				writef("Your buffers have been saved.\n");
				writef("Use \"jove -r\" to have a look at them.\n");
				DelTmps = NO;	/* Don't delete anymore. */
			} else {
				writef("You didn't lose any work.\n");
			}
		} else {
			writef("\r\nYou may have lost your work!\n");
		}
	}
#endif	/* MSDOS */
	flushscreen();
	if (DelTmps) {
#if	defined(IPROCS) && defined(PTYPROCS)
		(void) signal(SIGCHLD, SIG_IGN);
#endif
		tmpremove();
#ifndef	MSDOS
		recremove();
#endif	/* MSDOS */
	}
#ifdef	UNIX
	if (CoreDump)
		abort();
#ifdef	USE_EXIT
	exit(0);
#else
	_exit(0);
#endif
#else	/* !UNIX */
#ifdef	MSDOS
	break_rst();	/* restore previous ctrl-c handling */
#endif
	exit(0);
#endif	/* !UNIX */
	/*NOTREACHED*/
}

private char	smbuf[20],
		*bp = smbuf;
private int	nchars = 0;

private char	peekbuf[10],
		*peekp = peekbuf;

#ifdef NONBLOCKINGREAD

#include <fcntl.h>

private void
setblock(on)	/* turn blocking on or off */
bool	on;
{
	static int blockf, nonblockf;
	static bool	first = YES;

	if (first) {
		int flags;

		first = NO;
		if ((flags = fcntl(0, F_GETFL, 0)) == -1)
			finish(SIGHUP);
		blockf = flags & ~O_NDELAY;	/* make sure O_NDELAY is off */
		nonblockf = flags | O_NDELAY;	/* make sure O_NDELAY is on */
	}
	if (fcntl(0, F_SETFL, on ? blockf : nonblockf) == -1)
		finish(SIGHUP);
}

#endif	/* NONBLOCKINGREAD */

private int
Peekc()
{
	return peekp == peekbuf? EOF : *--peekp & 0377;
}

void
Ungetc(c)
int	c;
{
	if (peekp != &peekbuf[(sizeof peekbuf) - 1])
		*peekp++ = c;
}

bool	InputPending = NO;

char	*Inputp = NULL;

int
jgetchar()
{
	register int	c;

	while (nchars <= 0) {
		bp = smbuf;
#ifdef	MSDOS
		*bp = getrawinchar();
		nchars = 1;
#else	/* !MSDOS */
# if	defined(IPROCS) && defined(PTYPROCS)
		/* Get a character from the keyboard, first checking for
		   any input from a process.  Handle that first, and then
		   deal with the terminal input. */
		{
			fd_set	reads;
			int
				nfds,
				fd;

			bp = smbuf;
			do {
				reads = global_fd;
				nfds = select(global_maxfd,
					&reads, (fd_set *)NULL, (fd_set *)NULL,
					(struct timeval *)NULL);
			} while (nfds < 0 && errno == EINTR);

			if (nfds == -1) {
				complain("\rerror in select: %s", strerror(errno));
				/* NOTREACHED */
			}
			if (FD_ISSET(0, &reads)) {
				nchars = read(0, (UnivPtr) smbuf, sizeof(smbuf));
				if (nchars < 0 && errno != EINTR)
					finish(SIGHUP);
				nfds -= 1;
			}
			for (fd=1; nfds != 0; fd += 1) {
				if (FD_ISSET(fd, &reads)) {
					nfds -= 1;
					read_proc(fd);
				}
			}
		}
# else	/* !(defined(IPROCS) && defined(PTYPROCS)) */
#  if	defined(IPROCS) && defined(PIPEPROCS)
		if (NumProcs > 0) {
			/* Handle process input until kbd input arrives */
			struct header	header;
			size_t	n = f_readn(ProcInput, (char *) &header, sizeof(header));

			if (n != sizeof(header)) {
				raw_complain("\r\nError reading kbd process, expected %d, got %d bytes\r\n", sizeof header, n);
				finish(SIGHUP);
			}
			if (header.pid == kbd_pid) {
				/* data is from the keyboard process */
				nchars = f_readn(ProcInput, smbuf, (size_t)header.nbytes);
				if (nchars != header.nbytes) {
					raw_complain("\r\nError reading kbd process, expected %d, got %d bytes.\r\n", header.nbytes, nchars);
					finish(SIGHUP);
				}
			} else {
				/* data is from an interactive process */
				read_proc(header.pid, header.nbytes);
				if (NumProcs == 0)
					(void) kbd_stop();
			}
		} else /*...*/
#  endif	/* defined(IPROCS) && defined(PIPEPROCS) */
		/*...*/ {
			nchars = read(0, (UnivPtr) smbuf, sizeof smbuf);
			if (nchars < 0 && errno != EINTR)
				finish(SIGHUP);
		}
# endif	/* !(defined(IPROCS) && defined(PTYPROCS)) */
#endif	/* !MSDOS */
	}
	c = *bp++;
	if ((c & 0200) && MetaKey) {
		*--bp = c & CHARMASK;
		nchars += 1;
		c = ESC;
	}
	InputPending = --nchars > 0;
	return c & CHARMASK;
}

/* Returns non-zero if a character waiting */

bool
charp()
{
	if (InJoverc != 0 || nchars > 0 || Inputp != NULL)
		return YES;
#ifdef	FIONREAD
	{
		/*
		 * Some manual pages, notably SunOS4.1.3 say 'c' should be
		 * 'long', but that's a lie -- it's an 'int' according to all
		 * kernels I've seen (including SunOS4.1.3) and most other
		 * manual pages.  At any rate, 'int' works correctly on 32 and
		 * 62 bit architectures, whereas long breaks on the 64
		 * bitters. -- MM.
		 */
		int c;

		if (ioctl(0, FIONREAD, (UnivPtr) &c) == -1)
			c = 0;
		return c > 0;
	}
#else
#ifdef	NONBLOCKINGREAD
	setblock(NO);		/* turn blocking off */
	nchars = read(0, (UnivPtr) smbuf, sizeof smbuf);	/* Is anything there? */
	setblock(YES);		/* turn blocking on */
	bp = smbuf;			/* make sure bp points to it */
	return nchars > 0;	/* just say we found something */
#else
#ifdef	MSDOS
	return rawkey_ready();
#else
#ifdef	MAC
	return rawchkc();
#else
	return NO;	/* who knows? */
#endif
#endif
#endif
#endif
}

/*
 * Tries to pause for delay/10 seconds OR until a character is typed at the
 * keyboard.  This works well on systems with select() and not so well on the
 * rest.
 */

#ifdef	USE_SELECT
# include <sys/time.h>
#endif

#ifdef	MSDOS
# include <bios.h>
# include <dos.h>
#endif

void
SitFor(delay)
int	delay;
{
#ifdef	MAC
	long
		start,
		end;

#define Ticks ((long *) 0x16A)	/* 1/60 sec */
	Keyonly = YES;
	redisplay();
	start = *Ticks;

	end = start + delay * 6;
	do ; while ((InputPending = charp()) == NO && *Ticks < end);
#undef	Ticks

#else	/* !MAC */

#ifndef	MSDOS
	if (!charp()) {
#ifdef USE_SELECT
		struct timeval	timer;
		fd_set	readfds;

		/* So messages that aren't error messages don't
		 * hang around forever.
		 * Gross that I had to snarf this from getch()
		 */
		if (!UpdMesg && !Asking && mesgbuf[0] && !errormsg)
			message(NullStr);
		redisplay();

		timer.tv_sec = (delay / 10);
		timer.tv_usec = (delay % 10) * 100000;
		FD_ZERO(&readfds);
		FD_SET(0, &readfds);
		(void) select(1,
			&readfds, (fd_set *)NULL, (fd_set *)NULL,
			&timer);
#else	/* ! USE_SELECT */
		/* Pause by spitting NULs at the terminal.  Ugh! */
		static const int cps[] = {
			0,
			5,
			7,
			11,
			13,
			15,
			20,
			30,
			60,
			120,
			180,
			240,
			480,
			960,
			1920,
			1920,
		};
		register int	nchars,
				check_cnt;

		nchars = (delay * cps[ospeed]) / 10;
		check_cnt = BufSize;
		redisplay();
		if (!NP) {
			while ((--nchars > 0) && !InputPending) {
				jputchar(PC);
				if (--check_cnt == 0) {
					check_cnt = BufSize;
					InputPending = charp();
				}
			}
		}
#endif	/* USE_SLECT */
	}
#else	/* MSDOS */

	long	start,
		end;
#ifndef	IBMPC
	struct dostime_t tc;
#endif

	redisplay();
#ifdef	IBMPC
	_bios_timeofday(_TIME_GETCLOCK, &start);
#else
	_dos_gettime(&tc);
	start = (long)(tc.hour*60L*60L*10L)+(long)(tc.minute*60L*10L)+
	    (long)(tc.second*10)+(long)(tc.hsecond/10);
#endif
	end = (start + delay);
	do  {
		if ((InputPending = charp()) != NO)
			break;
#ifdef	IBMPC
		if (_bios_timeofday(_TIME_GETCLOCK, &start))
			break;	/* after midnight */
#else
		start = (long)(tc.hour*60L*60L*10L)+(long)(tc.minute*60L*10L)+
			(long)(tc.second*10)+(long)(tc.hsecond/10);
#endif
	} while (start < end);
#endif	/* MSDOS */
#endif	/* !MAC */
}

#ifdef	MSDOS
#include <time.h>
#endif

char
	key_strokes[100],
	*keys_p = key_strokes;

void
pp_key_strokes(buffer, size)
char	*buffer;
size_t	size;
{
	char	*buf_end = buffer + size - 1,
		*kp = key_strokes,
		c;

	*buffer = '\0';
	while ((c = *kp++) != '\0') {
		swritef(buffer, (size_t) (buf_end-buffer), "%p ", c);
		buffer += strlen(buffer);
		if (buffer >= buf_end)
			break;
	}
}

private int	*slowp = NULL;	/* for waitchar() */

/*ARGSUSED*/
private SIGRESTYPE
slowpoke(junk)
int	junk;
{
	int save_errno = errno;	/* Subtle, but necessary! */
	char	buffer[100];

	if (slowp)
		*slowp = YES;
	pp_key_strokes(buffer, sizeof (buffer));
	f_mess(buffer);
	errno = save_errno;
	return SIGRESVALUE;
}

#define N_SEC	1	/* will be precisely 1 second on 4.2 */

int
waitchar(slow)
int	*slow;
{
	int	c;
#ifdef	UNIX
	unsigned int	old_time;
	SIGRESTYPE	(*oldproc) proto((int));
#else	/* !UNIX */
	long sw, time();
#endif	/* !UNIX */

	slowp = slow;

	if (in_macro())		/* make macros faster ... */
		return getch();

	/* If slow is a valid pointer and it's value is yes, then
	   we know we have already been slow during this sequence,
	   so we just wait for the character and then echo it. */
	if (slow != NULL && *slow == YES) {
		c = getch();
		slowpoke(0);
		return c;
	}
#ifdef	UNIX
	oldproc = signal(SIGALRM, slowpoke);

	if ((old_time = alarm((unsigned) N_SEC)) == 0)
		old_time = UpdFreq;
	c = getch();
	(void) alarm(old_time);
	(void) signal(SIGALRM, oldproc);

	if (slow != NULL && *slow == YES)
		slowpoke(0);
	return c;

#else	/* !UNIX */
#ifdef	MAC
	Keyonly = YES;
	if (charp() || in_macro()) {
		c = getch();	/* to avoid flicker */
		if (slow != NULL && *slow == YES)
			slowpoke(0);
		return c;
	}
#endif
	time(&sw);
	sw += N_SEC;
	while (time(NULL) <= sw)
		if (charp() || in_macro())
			return getch();
#ifdef	MAC
	menus_off();
#endif
	slowpoke(0);
	c = getch();
	slowpoke(0);

	return c;
#endif	/* !UNIX */
}

#ifdef	BIFF
private void	biff_init proto((void));
#endif

private void
ResetTerm()
{
#ifdef	IBMPC
	pcResetTerm();
#endif
	do_sgtty();		/* this is so if you change baudrate or stuff
				   like that, JOVE will notice. */
	ttyset(YES);
#ifdef	TERMCAP
	putpad(TI, 1);
	putpad(VS, 1);
	putpad(KS, 1);
#endif
#ifdef	UNIX
	(void) chkmail(YES);	/* force it to check to we can be accurate */
#endif
#ifdef	BIFF
	if (BiffChk != dw_biff)
		biff_init();
	/* just in case we changed our minds about whether to deal with
	   biff */
#endif
}

private void
UnsetTerm(mesg)
char	*mesg;
{
	ttyset(NO);
#ifdef	TERMCAP
#ifdef	ID_CHAR
	INSmode(NO);
#endif /* ID_CHAR */
	putpad(KE, 1);
	putpad(VE, 1);
	Placur(ILI, 0);
	putpad(CE, 1);
	if (TE)
		putpad(TE, 1);
#else /* !TERMCAP */
	Placur(ILI, 0);
	clr_eoln();
#endif /* TERMCAP */
	if (mesg[0] != '\0')
		writef("%s\n", mesg);
	flushscreen();
#ifdef	IBMPC
	pcUnsetTerm();
#endif /* IBMPC */
}

void
PauseJove()
{
#ifdef	JOB_CONTROL
	UnsetTerm(ModBufs(NO) ? "[There are modified buffers]" : NullStr);
	(void) kill(0, SIGTSTP);
	ResetTerm();
	ClAndRedraw();
#else /* ! JOB_CONTROL */
	message("[Job Control not supported]");
#endif /* JOB_CONTROL */
}

void
jcloseall()
{
	tmpclose();
	recclose();
#ifdef	LOAD_AV
	closekmem();
#endif	/* LOAD_AV */
#ifdef	IPROCS
	closeiprocs();
#endif
}

#ifdef	SUBSHELL
void
Push()
{
#ifndef	MSDOS
	pid_t	pid;
	SIGRESTYPE	(*old_quit) ptrproto((int)) = signal(SIGQUIT, SIG_IGN);
#endif	/* !MSDOS */
	SIGRESTYPE	(*old_int) ptrproto((int)) = signal(SIGINT, SIG_IGN);
# if	defined(IPROCS) && defined(PIPEPROCS)
	bool	started;
# endif

#ifndef	MSDOS
#ifdef	IPROCS
	SigHold(SIGCHLD);
#endif
#ifdef	WINRESIZE
	SigHold(SIGWINCH);
#endif
#ifdef UNIX
	alarm((unsigned)0);
#endif
# if	defined(IPROCS) && defined(PIPEPROCS)
	started = kbd_stop();
# endif
	switch (pid = fork()) {
	case -1:
# if	defined(IPROCS) && defined(PIPEPROCS)
		if (started)
			kbd_strt();
# endif
		complain("[Fork failed: %s]", strerror(errno));
		/*NOTREACHED*/

	case 0:
		UnsetTerm(ModBufs(NO) ? "[There are modified buffers]" : NullStr);
#ifdef	WINRESIZE
		SigRelse(SIGWINCH);
#endif
#ifdef	IPROCS
		SigRelse(SIGCHLD);
#endif
		(void) signal(SIGTERM, SIG_DFL);
#else	/* MSDOS */
		UnsetTerm(ModBufs(NO) ? "[There are modified buffers]" : NullStr);
#endif	/* MSDOS */
		(void) signal(SIGINT, SIG_DFL);
#ifdef	UNIX
		(void) signal(SIGQUIT, SIG_DFL);
		jcloseall();
		/* note that curbuf->bfname may be NULL */
		execl(Shell, basename(Shell), "-is", pr_name(curbuf->b_fname, NO),
			(char *)NULL);
		raw_complain("[Execl failed: %s]", strerror(errno));
		_exit(1);
	}
	dowait(pid, (wait_status_t *) NULL);
#ifdef	IPROCS
	SigRelse(SIGCHLD);
#endif
#endif	/* UNIX */
#ifdef	MSDOS
	break_rst();
	if (spawnl(0, Shell, basename(Shell), (char *)NULL) == -1)
		message("[Spawn failed]");
#endif	/* MSDOS */
	ResetTerm();
#ifdef	WINRESIZE
	SigRelse(SIGWINCH);
#endif
	ClAndRedraw();
#ifndef	MSDOS
	(void) signal(SIGQUIT, old_quit);
#else	/* MSDOS */
	break_off();
	getCWD();
#endif	/* MSDOS */
	(void) signal(SIGINT, old_int);
#ifdef UNIX
	if (UpdFreq != 0)
		(void) alarm((unsigned) (UpdFreq - (time((time_t *)NULL) % UpdFreq)));
#endif
# if	defined(IPROCS) && defined(PIPEPROCS)
	if (started)
		kbd_strt();
# endif
}

#endif	/* SUBSHELL */

bool	OKXonXoff = NO;	/* ^S and ^Q initially DON'T work */
int	IntChar = CTL(']');

#ifdef	BTL_BLIT
#	include <sys/jioctl.h>
#endif

private void
ttsize()
{
#ifdef	UNIX
#   ifdef	TIOCGWINSZ
	struct winsize win;

	if (ioctl(0, TIOCGWINSZ, (UnivPtr) &win) == 0) {
		/* Note: Solaris 2 / OpenWindows 3 generates SIGWINCH
		 * several signals while creating a window.  For the
		 * first few of them TIOCGWINSZ returns nonsensical sizes.
		 * To protect JOVE, we ignore outliers.
		 */
		if (0<win.ws_col && win.ws_col<JBUFSIZ-1)
			CO = win.ws_col;
		if (0<win.ws_row && win.ws_row<200)
			LI = win.ws_row;
	}
#   else	/* !TIOCGWINSZ */
#	ifdef	BTL_BLIT
	struct jwinsize jwin;

	if (ioctl(0, JWINSIZE, (UnivPtr) &jwin) == 0) {
		if (jwin.bytesx)
			CO = jwin.bytesx;
		if (jwin.bytesy)
			LI = jwin.bytesy;
	}
#	endif	/* BTL_BLIT */
#   endif	/* !TIOCGWINSZ */
#endif	/* UNIX */
#ifdef	MAC
	CO = getCO();	/* see mac.c */
	LI = getLI();
	Windchange = YES;
	clr_page();
#endif
	ILI = LI - 1;
}

#ifdef	BIFF
private void
biff_init()
{
	dw_biff = BiffChk
#   ifdef USE_FSTAT
		&& fstat(0, &tt_stat) != -1
#   else
		&& ((tt_name != NULL) || (tt_name = ttyname(0)) != NULL)
		&& stat(tt_name, &tt_stat) != -1
#   endif
		&& (tt_stat.st_mode & S_IEXEC);	/* he's using biff */

}

private void
biff(on)
int	on;
{
	if (dw_biff) {
#ifdef	USE_FCHMOD
		(void) fchmod(0,
			on? tt_stat.st_mode : (tt_stat.st_mode & ~S_IEXEC));
#else
		if (tt_name != NULL || (tt_name = ttyname(0)) != NULL)
			(void) chmod(tt_name,
				on? tt_stat.st_mode : (tt_stat.st_mode & ~S_IEXEC));
#endif
	}
}

#endif	/* BIFF */

private void
ttinit()
{
#ifdef	BIFF
	biff_init();
#endif
#ifdef SGTTY
# ifdef	TIOCSLTC
	(void) ioctl(0, TIOCGLTC, (UnivPtr) &ls[NO]);
	ls[YES] = ls[NO];
	ls[YES].t_suspc = (char) -1;
	ls[YES].t_dsuspc = (char) -1;
	ls[YES].t_flushc = (char) -1;
	ls[YES].t_lnextc = (char) -1;
# endif

# ifdef	TIOCGETC
	/* Change interupt and quit. */
	(void) ioctl(0, TIOCGETC, (UnivPtr) &tc[NO]);
	tc[YES] = tc[NO];
	tc[YES].t_intrc = IntChar;
	tc[YES].t_quitc = (char) -1;
	if (OKXonXoff) {
		tc[YES].t_stopc = (char) -1;
		tc[YES].t_startc = (char) -1;
	}
# endif	/* TIOCGETC */
#endif /* SGTTY */
	do_sgtty();
}

private int	done_ttinit = NO;

private void
do_sgtty()
{
#ifdef	UNIX
# ifdef	TERMIO
	(void) ioctl(0, TCGETA, (UnivPtr) &sg[NO]);
# endif
# ifdef TERMIOS
	(void) tcgetattr(0, &sg[NO]);
# endif

# ifdef SGTTY
	(void) gtty(0, &sg[NO]);

#  ifdef LPASS8
	(void) ioctl(0, TIOCLGET, (UnivPtr) &lmword[NO]);
	lmword[YES] = lmword[NO];
	if (MetaKey)
		lmword[YES] |= LPASS8;
#  endif /* LPASS8 */

#  ifdef LTILDE
	if (Hazeltine)
		lmword[YES] &= ~LTILDE;
#  endif /* LTILDE */
# endif	/* SGTTY */
	sg[YES] = sg[NO];

# if defined(TERMIO) || defined(TERMIOS)
#  ifdef TAB3
	TABS = (sg[NO].c_oflag & TAB3) != TAB3;
#  endif
#  ifdef TERMIOS
	ospeed = cfgetospeed(&sg[NO]);
#  else /* ! TERMIOS */
#   ifdef CBAUD
	ospeed = sg[NO].c_cflag & CBAUD;
#   else /* ! CBAUD */
	ospeed = B9600;	/* XXX */
#   endif /* CBAUD */
#  endif /* TERMIOS */

	if (OKXonXoff)
		sg[YES].c_iflag &= ~(IXON | IXOFF);
	sg[YES].c_iflag &= ~(INLCR|ICRNL|IGNCR);
	sg[YES].c_lflag &= ~(ICANON|ECHO);
	sg[YES].c_oflag &= ~(OPOST);
	sg[YES].c_cc[VINTR] = IntChar;

#  ifndef VDISABLE
#   ifdef _POSIX_VDISABLE
#    define VDISABLE	_POSIX_VDISABLE
#   else /* !_POSIX_VDISABLE */
#    define VDISABLE	0
#   endif /* !_POSIX_VDISABLE */
#  endif /* VDISABLE */

#  ifdef VQUIT	
	sg[YES].c_cc[VQUIT] = VDISABLE;
#  endif
	/* VERASE, VKILL, VEOL2 irrelevant */
	/* Beware aliasing! VMIN is VEOF and VTIME is VEOL */
#  ifdef VSWTCH
	sg[YES].c_cc[VSWTCH] = VDISABLE;
#  endif

	/* Under SunOS 4.0, <termios.h> does not match POSIX,
	 * so we cannot use it.  Instead, we use <termio.h>.
	 * Unfortunately, <termio.h> includes <termios.h>
	 * so the extra V symbols get defined, but they must
	 * not be used in struct termio since the c_cc array
	 * does not have entries for all of them!  This is why
	 * the following goo is doubly ifdefed.
	 */

#  ifdef TERMIOS
#   ifdef VSUSP
	sg[YES].c_cc[VSUSP] = VDISABLE;
#   endif
#   ifdef VDSUSP
	sg[YES].c_cc[VDSUSP] = VDISABLE;
#   endif
#   ifdef VDISCARD
	/* ??? Under Solaris 2.1 needs VDISCARD disabled, or it will
	 * be processed by the tty driver, but not under SVR4!
	 */
	sg[YES].c_cc[VDISCARD] = VDISABLE;	/* flush output */
#   endif
#   ifdef VLNEXT
	sg[YES].c_cc[VLNEXT] = VDISABLE;	/* literal next char */
#   endif
#  endif /* TERMIOS */

	sg[YES].c_cc[VMIN] = 1;
	sg[YES].c_cc[VTIME] = 1;
# endif /* defined(TERMIO) || defined(TERMIOS) */

# ifdef SGTTY
	TABS = !(sg[NO].sg_flags & XTABS);
	sg[YES].sg_flags &= ~(XTABS|ECHO|CRMOD);
	ospeed = sg[NO].sg_ospeed;

#  ifdef	LPASS8
	sg[YES].sg_flags |= CBREAK;
#  else
	sg[YES].sg_flags |= (MetaKey ? RAW : CBREAK);
#  endif
# endif	/* SGTTY */
#endif	/* UNIX */

#ifdef	MSDOS
# ifndef	IBMPC
	setmode(1, 0x8000);
# endif	/* IBMPC */
	TABS = NO;
#endif	/* MSDOS */
}

void
tty_reset()
{
	if (done_ttinit) {
		ttyset(NO);	/* go back to original modes */
		ttinit();
		ttyset(YES);
	}
}

/* If n is NO reset to original modes */

private void
ttyset(n)
bool	n;
{
	if (!done_ttinit && !n)	/* Try to reset before we've set! */
		return;
#ifdef	UNIX
# ifdef	TERMIO
	(void) ioctl(0, TCSETAW, (UnivPtr) &sg[n]);
# endif	/* TERMIO */

# ifdef	TERMIOS
	(void) tcsetattr(0, TCSADRAIN, &sg[n]);
# endif /* TERMIOS */

# ifdef	SGTTY
#  ifdef TIOCSETN
	(void) ioctl(0, TIOCSETN, (UnivPtr) &sg[n]);
#  else
	(void) stty(0, &sg[n]);
#  endif

#  ifdef TIOCGETC
	(void) ioctl(0, TIOCSETC, (UnivPtr) &tc[n]);
#  endif /* TIOCSETC */
#  ifdef TIOCSLTC
	(void) ioctl(0, TIOCSLTC, (UnivPtr) &ls[n]);
#  endif /* TIOCSLTC */
#  ifdef LPASS8
	(void) ioctl(0, TIOCLSET, (UnivPtr) &lmword[n]);
#  endif /* LPASS8 */
# endif /* SGTTY */
#endif	/* UNIX */

#ifdef	MSDOS
# ifndef	IBMPC
	setmode(1, n? 0x8000 : 0x4000);
# endif
#endif	/* MSDOS */
	done_ttinit = YES;
#ifdef	BIFF
	biff(!n);
#endif
}

bool	Interactive = NO;	/* True when we invoke with the command handler? */

int
	LastKeyStruck,
	MetaKey = NO;

int
getch()
{
	register int	c,
			peekc;

	if (Inputp) {
		if ((c = *Inputp++) != '\0')
			return LastKeyStruck = c;
		Inputp = NULL;
	}

	if (InJoverc)
		return EOF;	/* somethings wrong if Inputp runs out while
				   we're reading a .joverc file. */

#ifndef	MSDOS
	if (ModCount >= SyncFreq) {
		ModCount = 0;
		SyncRec();
	}
#endif	/* MSDOS */

	/* If there are no ungetc'd characters,
	   AND we're interactive or we're not executing a macro,
	   we read from the terminal (i.e., jgetchar()).
	   Note: characters only get put in macros from inside this if. */
	if ((peekc = c = Peekc()) == EOF
	&& (Interactive || (c = mac_getc()) == EOF))
	{
		/* So messages that aren't error messages don't
		 * hang around forever.
		 * Note: this code is duplicated in SitFor()!
		 */
		if (!UpdMesg && !Asking && mesgbuf[0] != '\0' && !errormsg)
			message(NullStr);
		redisplay();
#ifdef	UNIX
		inIOread = YES;
#endif
		if ((c = jgetchar()) == EOF)
			finish(SIGHUP);
#ifdef	UNIX
		inIOread = NO;
#endif

		if (!Interactive && InMacDefine)
			mac_putc(c);
	}
	if (peekc == EOF)	/* don't add_stroke peekc's */
		add_stroke(c);
	return LastKeyStruck = c;
}

#ifdef	UNIX
private void
dorecover()
{
	/* Since recover is a normal cooked mode program, reset the terminal */
	UnsetTerm(NullStr);
#if	defined(IPROCS) && defined(PIPEPROCS)
	kbd_kill();		/* kill the keyboard process */
#endif
	execl(Recover, "recover", "-d", TmpFilePath, (char *) NULL);
	writef("%s: execl failed! %s\n", Recover, strerror(errno));
	flushscreen();
	_exit(-1);
	/* NOTREACHED */
}
#endif	/* UNIX */

void
ShowVersion()
{
	s_mess("Jonathan's Own Version of Emacs (%s)", version);
}

private void
UNIX_cmdline(argc, argv)
int	argc;
char	*argv[];
{
	int	lineno = 0,
		nwinds = 1;
	Buffer	*b;

	ShowVersion();
	while (argc > 1) {
		if (argv[1][0] != '-' && argv[1][0] != '+') {
			bool	force = (nwinds > 0 || lineno != 0);

#ifdef	MSDOS
			strlwr(argv[1]);
#endif
			minib_add(argv[1], force);
			b = do_find(nwinds > 0 ? curwind : (Window *) NULL,
				    argv[1], force);
			if (force) {
				SetABuf(curbuf);
				SetBuf(b);
				if (lineno >= 0)
					SetLine(next_line(curbuf->b_first, lineno));
				else
					SetLine(curbuf->b_last);
				if (nwinds > 1)
					NextWindow();
				if (nwinds)
					nwinds -= 1;
			}
			lineno = 0;
		} else	switch (argv[1][1]) {
			case 'd':
				argv += 1;
				argc -= 1;
				break;

			case 'j':	/* Ignore .joverc in HOME */
				break;
#ifdef	SUBSHELL
			case 'p':
				argv += 1;
				argc -= 1;
				if (argv[1] != NULL) {
					SetBuf(do_find(curwind, argv[1], NO));
					ErrParse();
					nwinds = 0;
				}
				break;
#endif
			case 't':
				/* check if syntax is -tTag or -t Tag */
				if (argv[1][2] != '\0') {
					find_tag(&(argv[1][2]), YES);
				} else {
					argv += 1;
					argc -= 1;
					if (argv[1] != NULL)
						find_tag(argv[1], YES);
				}
				break;

			case 'w':
				if (argv[1][2] == '\0')
					nwinds += 1;
				else {
					int	n;

					(void) chr_to_int(&argv[1][2], 10, NO, &n);
					nwinds += -1 + n;
				}
				(void) div_wind(curwind, nwinds - 1);
				break;

			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				(void) chr_to_int(&argv[1][1], 10, NO, &lineno);
				lineno -= 1;
				break;
			case '\0':
				lineno = -1;	/* goto end of file ... */
				break;		/* just like some people's */
		}				/* favourite editor */
		argv += 1;
		argc -= 1;
	}
}

#ifdef	STDARGS
void
error(const char *fmt, ...)
#else
/*VARARGS1*/ void
error(fmt, va_alist)
	const char	*fmt;
	va_dcl
#endif
{
	va_list	ap;

	if (fmt) {
		va_init(ap, fmt);
		format(mesgbuf, sizeof mesgbuf, fmt, ap);
		va_end(ap);
		UpdMesg = YES;
	}
	rbell();
	longjmp(mainjmp, ERROR);
}

#ifdef	STDARGS
void
complain(const char *fmt, ...)
#else
/*VARARGS1*/ void
complain(fmt, va_alist)
	const char	*fmt;
	va_dcl
#endif
{
	va_list	ap;

	if (fmt) {
		va_init(ap, fmt);
		format(mesgbuf, sizeof mesgbuf, fmt, ap);
		va_end(ap);
		UpdMesg = YES;
	}
	rbell();
	longjmp(mainjmp, COMPLAIN);
}

#ifdef	STDARGS
void
raw_complain(const char *fmt, ...)
#else
/*VARARGS1*/ void
raw_complain(fmt, va_alist)
	const char	*fmt;
	va_dcl
#endif
{
	va_list	ap;

	if (fmt) {
		va_init(ap, fmt);
		format(mesgbuf, sizeof mesgbuf, fmt, ap);
		va_end(ap);
		write(2, (UnivConstPtr)mesgbuf, strlen(mesgbuf));
	}
}

#ifdef	STDARGS
void
confirm(const char *fmt, ...)
#else
/*VARARGS1*/ void
confirm(fmt, va_alist)
	const char	*fmt;
	va_dcl
#endif
{
	va_list	ap;

	va_init(ap, fmt);
	format(mesgbuf, sizeof mesgbuf, fmt, ap);
	va_end(ap);
	if (!yes_or_no_p("%s", mesgbuf))
		longjmp(mainjmp, COMPLAIN);
}

/* Recursive edit.
 * Guarantee: current buffer will still be current and
 * it will be in the current window.  If not, complain!
 */

int	RecDepth = 0;

void
Recur()
{
	Buffer	*b = curbuf;
	Mark	*m;

	m = MakeMark(curline, curchar, M_FLOATER);

	RecDepth += 1;
	UpdModLine = YES;
	DoKeys(NO);	/* NO means not first time */
	UpdModLine = YES;
	RecDepth -= 1;
	if (!valid_bp(b))
		complain("Buffer gone!");
	tiewind(curwind, b);
	SetBuf(b);
	if (!is_an_arg())
		ToMark(m);
	DelMark(m);
}

#ifdef	MAC
jmp_buf auxjmp;
#endif

private int	iniargc;	/* main sets these for DoKeys() */
private char	**iniargv;

private void
DoKeys(firsttime)
bool	firsttime;
{
	int	c;
	jmp_buf	savejmp;

	push_env(savejmp);

	switch (setjmp(mainjmp)) {
	case 0:
		if (firsttime)
			UNIX_cmdline(iniargc, iniargv);
		break;

	case QUIT:
		if (RecDepth == 0) {
			if (ModMacs()) {
				rbell();
				if (!yes_or_no_p("Some MACROS haven't been saved; leave anyway? "))
					break;
			}
			if (ModBufs(NO)) {
				rbell();
				if (!yes_or_no_p("Some buffers haven't been saved; leave anyway? "))
					break;
			}
#ifdef	IPROCS
			KillProcs();
#endif
		}
		pop_env(savejmp);
		return;

	case ERROR:
		getDOT();	/* God knows what state linebuf was in */
		/*FALLTHROUGH*/
	case COMPLAIN:
		{
		gc_openfiles();		/* close any files we left open */
		errormsg = YES;
		unwind_macro_stack();
		Asking = NO;
		curwind->w_bufp = curbuf;
		DisabledRedisplay = NO;
		redisplay();
		break;
		}
	}

	this_cmd = last_cmd = OTHER_CMD;

	for (;;) {
#ifdef	MAC
		setjmp(auxjmp);
#endif
		if (this_cmd != ARG_CMD) {
			clr_arg_value();
			last_cmd = this_cmd;
			init_strokes();
		}
#ifdef	MAC
		HiliteMenu(0);
		EventCmd = NO;
		menus_on();
#endif
		c = getch();
		if (c != EOF)
			dispatch(c);
	}
}

private char **
scanvec(args, str)
register char	**args,
		*str;
{
	while (*args) {
		if (strcmp(*args, str) == 0)
			return args;
		args += 1;
	}
	return NULL;
}

#ifdef	UNIX
int	UpdFreq = 30,
	inIOread = NO;

/*ARGSUSED*/
private SIGRESTYPE
updmode(junk)
int	junk;	/* passed in on signal; of no interest */
{
	int save_errno = errno;	/* Subtle, but necessary! */

	UpdModLine = YES;
	if (inIOread)
		redisplay();
#ifndef	BSD_SIGS
	(void) signal(SIGALRM, updmode);
#endif
	if (UpdFreq != 0)
		(void) alarm((unsigned) (UpdFreq - (time((time_t *)NULL) % UpdFreq)));
	errno = save_errno;
	return SIGRESVALUE;
}
#endif	/* UNIX */

#ifdef	WINRESIZE
SIGRESTYPE
win_reshape(junk)
int	junk;	/* passed in when invoked by a signal; of no interest */
{
	int save_errno = errno;	/* Subtle, but necessary! */
	register int	oldLI;
	register int newsize, total;
	register Window *wp;

#ifdef	UNIX
	(void) SigHold(SIGWINCH);
#endif
	/*
	 * Save old number of lines.
	 */
	oldLI = LI;

	/*
	 * Get new line/col info.
	 */
	ttsize();

	/*
	 * LI has changed, and now holds the
	 * new value.
	 */
	/*
	 *  Go through the window list, changing each window size in
	 *  proportion to the resize. If a window becomes too small,
	 *  delete it. We keep track of all the excess lines (caused by
	 *  roundoff!), and give them to the current window, as a sop -
	 *  can't be more than one or two lines anyway. This seems fairer
	 *  than just resizing the current window.
	 */
	wp = fwind;
	total = 0;
	do {
		newsize = LI * wp->w_height / oldLI;

		if (newsize < 2) {
			total += wp->w_height;
			wp = wp->w_next;
			del_wind(wp->w_prev);
		} else {
			wp->w_height = newsize;
			total += newsize;
			wp = wp->w_next;
		}
	} while (wp != fwind);

	curwind->w_height += LI - total - 1;

	/* Make a new screen structure */
	make_scr();
	/* Do a 'hard' update on the screen - clear and redraw */
	cl_scr(YES);
	flushscreen();
	redisplay();

#ifdef	UNIX
	(void) SigRelse(SIGWINCH);
	(void) signal(SIGWINCH, win_reshape);
#endif
	errno = save_errno;
	return SIGRESVALUE;
}
#endif

int
#ifdef	MAC	/* will get args from user, if option key held during launch */
main()
{
	int argc;
	char **argv;
#else	/* !MAC */
main(argc, argv)
int	argc;
char	*argv[];
{
#endif	/* !MAC */
	char	*cp;
	char	ttbuf[MAXTTYBUF];
#ifdef	pdp11
	/* On the PDP-11, UNIX allocates at least 8K.
	 * In order not to waste this space, we allocate
	 * a bunch of buffers as autos.
	 */

	char	s_iobuff[LBSIZE],
		s_genbuf[LBSIZE],
		s_linebuf[LBSIZE];

	iobuff = s_iobuff;
	genbuf = s_genbuf;
	linebuf = s_linebuf;
#endif

#ifdef	MAC
	MacInit();		/* initializes all */
	argc = getArgs(&argv);
#endif	/* MAC */

	iniargc = argc;
	iniargv = argv;

	if (setjmp(mainjmp)) {
		writef("\rAck! I can't deal with error \"%s\" now.\n\r", mesgbuf);
		finish(SIGTERM);	/* some bad signal (not SIGHUP) */
	}

#ifdef	MSDOS
	/* import the temporary file path from the environment and
	   fix the string, so that we can append a slash safely	*/

	if (((cp = getenv("TMPDIR"))!=NULL || (cp = getenv("TEMP"))!=NULL)
	&& (*cp != '\0'))
	{
		strcpy(TmpFilePath, cp);
		cp = &TmpFilePath[strlen(TmpFilePath)-1];
		if ((*cp == '/') || (*cp == '\\'))
			*cp = '\0';
	}
	ShFlags[0] = switchar();
#endif	/* MSDOS */

	getTERM();	/* Get terminal. */
	if (getenv("METAKEY"))
		MetaKey = YES;
	ttsize();
#ifdef	MAC
	InitEvents();
#else
	InitCM();
#endif

	d_cache_init();		/* initialize the disk buffer cache */
#ifdef	SUBSHELL
# ifdef	MSDOS
	if ((cp = getenv("COMSPEC"))!=NULL && (*cp != '\0')) {
		strcpy(Shell, cp);
	}
# else	/* !MSDOS */
	if ((cp = getenv("SHELL"))!=NULL && (*cp != '\0')) {
		strcpy(Shell, cp);
	}
# endif	/* !MSDOS */
#endif /* SUBSHELL */
#ifdef MSDOS
	if ((cp = getenv("DESCRIBE"))!=NULL && (*cp != '\0'))
	   strcpy(CmdDb, cp);
#endif

	make_scr();
	mac_init();	/* Initialize Macros */
	winit();	/* Initialize Window */
#if	defined(IPROCS) && defined(PTYPROCS)
# ifdef SIGCHLD
	(void) signal(SIGCHLD, proc_child);
# endif
#endif
#ifdef	USE_SELECT
	FD_ZERO(&global_fd);
	FD_SET(0, &global_fd);
	global_maxfd = 1;
#endif
	buf_init();

	{
		char	**argp;

		if ((argp = scanvec(argv, "-d"))!=NULL
#ifdef	UNIX
		&& chkCWD(argp[1])
#endif
		)
			setCWD(argp[1]);
		else
			getCWD();	/* After we setup curbuf in case we have to getwd() */
	}

	HomeDir = getenv("HOME");
	if (HomeDir == NULL)
		HomeDir = "/";
	HomeLen = strlen(HomeDir);

#ifdef	UNIX
	if ((cp = getenv("MAIL")) != NULL && strlen(cp) < (size_t)FILESIZE) {
		strcpy(Mailbox, cp);
	}
#endif

	InitKeymaps();

	ttinit();	/* initialize terminal (before ~/.joverc) */
	settout(ttbuf);	/* not until we know baudrate */
	ResetTerm();

	(void) joverc(Joverc);			/* system wide .joverc */
	cp = NULL;
#ifndef	MAC
	/* If a JOVERC environment variable is set, then use that instead */
	if ((cp = getenv("JOVERC"))!=NULL && (*cp != '\0'))
	   (void) joverc(cp);
#endif	/* !MAC */
	if (!scanvec(argv, "-j") && (!cp || *cp == '\0')) {
		char	tmpbuf[FILESIZE];

		swritef(tmpbuf, sizeof(tmpbuf), "%s/.joverc", HomeDir);
		(void) joverc(tmpbuf);		/* .joverc in home directory */
	}

#ifndef	MSDOS
	if (scanvec(argv, "-r"))
		dorecover();
	if (scanvec(argv, "-rc"))
		FullRecover();
#endif	/* MSDOS */

#ifdef	MSDOS
	(void) signal(SIGINT, SIG_IGN);
	break_off();	/* disable ctrl-c checking */
#endif	/* MSDOS */
#ifdef	UNIX
	(void) signal(SIGHUP, finish);
	(void) signal(SIGINT, finish);
	(void) signal(SIGBUS, finish);
	(void) signal(SIGSEGV, finish);
	(void) signal(SIGPIPE, finish);
	/* ??? Why should we ignore SIGTERM? */
	/* (void) signal(SIGTERM, SIG_IGN); */
# ifdef	WINRESIZE
	(void) signal(SIGWINCH, win_reshape);
# endif
	/* set things up to update the modeline every UpdFreq seconds */
	(void) signal(SIGALRM, updmode);
	if (UpdFreq != 0)
		(void) alarm((unsigned) (UpdFreq - (time((time_t *)NULL) % UpdFreq)));
#endif	/* UNIX */
	cl_scr(YES);
	flushscreen();
	RedrawDisplay();	/* start the redisplay process. */
	DoKeys(YES);
	finish(0);
	/* NOTREACHED*/
}

#ifdef	MSDOS

/* ??? how many of these includes are redundant? */

#include <dos.h>
#include <fcntl.h>
#include <sys/stat.h>

/* ??? Set the DOS path separator character to '/' from '\\' */
private char
switchar()
{
  union REGS regs;

  regs.h.ah = 0x37;
  regs.h.al = 0;
  intdos(&regs, &regs);
  return regs.h.dl;
}

private	char break_state;

/* set the break state to off */
private void
break_off()
{
	union REGS regs;

	regs.h.ah = 0x33;		/* break status */
	regs.h.al = 0x00;		/* request current state */
	intdos(&regs, &regs);
	break_state = regs.h.dl;
	bdos(0x33, 0, 1);	/* turn off break */
}

/* reset the break state */
private void
break_rst()
{
	bdos(0x33, break_state, 1);
}
#endif
