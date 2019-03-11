/************************************************************************
 * This program is Copyright (C) 1986 by Jonathan Payne.  JOVE is       *
 * provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is *
 * included in all the files.                                           *
 ************************************************************************/

/* Contains the main loop initializations, and some system dependent
   type things, e.g. putting terminal in CBREAK mode, etc. */

#include "jove.h"
#include "io.h"
#include "termcap.h"

#include <varargs.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#ifndef MSDOS
#   ifndef SYSV
#	include <sgtty.h>
#   else
#	include <termio.h>
#   endif /* SYSV */
#endif /* MSDOS */
#include <fcntl.h>

#ifndef MSDOS
#ifdef TIOCSLTC
struct ltchars	ls1,
		ls2;
#endif /* TIOCSLTC */

#ifdef TIOCGETC
struct tchars	tc1,
		tc2;
#endif

#ifdef BRLUNIX
struct sg_brl	sg1, sg2;
#else
#ifdef SYSV
struct termio	sg1, sg2;
#else /* SYSV */
struct sgttyb	sg1, sg2;
#endif /* SYSV */
#endif /* BRLUNIX */

#ifdef BIFF
private struct stat	tt_stat;	/* for biff */
#ifndef BSD4_2
private char	*tt_name = 0;		/* name of the control tty */
extern char	*ttyname();		/* for systems w/o fchmod ... */
#endif
private int	dw_biff = NO;		/* whether or not to fotz at all */
#endif
#endif /* MSDOS */

time_t	time0;			/* when jove started up */
int	errormsg;
extern char	*tfname;
char	NullStr[] = "";

#ifdef MSDOS
#define SIGHUP	99
#endif /* MSDOS */

finish(code)
{
	int	CoreDump = (code != 0 && code != SIGHUP),
		DelTmps = 1;		/* Usually we delete them. */

	if (code == SIGINT) {
		char	c;

#ifndef MENLO_JCL
		(void) signal(code, finish);
#endif
		f_mess("Abort (Type 'n' if you're not sure)? ");
#ifndef MSDOS
		(void) read(0, &c, 1);
#else /* MSDOS */
		c = getrawinchar();
#endif /* MSDOS */
		message(NullStr);
		if ((c & 0377) != 'y') {
			redisplay();
			return;
		}
	}
	ttyset(OFF);
#ifndef IBMPC
	UnsetTerm(NullStr);
#ifndef MSDOS
	if (code != 0) {
		if (!Crashing) {
			Crashing = YES;
			lsave();
			SyncRec();
			printf("JOVE CRASH!! (code %d)\n", code);
			if (ModBufs(1)) {
				printf("Your buffers have been saved.\n");
				printf("Use \"jove -r\" to have a look at them.\n");
				DelTmps = 0;	/* Don't delete anymore. */
			} else
				printf("You didn't lose any work.\n");
		} else
			printf("\r\nYou may have lost your work!\n");
	}
#endif /* MSDOS */
#else /* IBMPC */
	UnsetTerm();
#endif /* MSDOS */
	flusho();
	if (DelTmps) {
		tmpclose();
#ifndef MSDOS
		recclose();
#endif /* MSDOS */
	}
#ifndef MSDOS
	if (CoreDump)
		abort();
#ifdef PROFILING
	exit(0);
#else
	_exit(0);
#endif
#else /* MSDOS */
	break_rst();	/* restore previous ctrl-c handling */
	exit(0);
#endif /* MSDOS */
}

private char	smbuf[20],
		*bp = smbuf;
private int	nchars = 0;

private char	peekbuf[10],
		*peekp = peekbuf;

#ifdef SYSV
void
setblock(fd, on)	/* turn blocking on or off */
register int	fd, on;
{
    static int blockf, nonblockf;
    static int first = 1;
    int flags;

    if (first) {
	first = 0;
	if ((flags = fcntl(fd, F_GETFL, 0)) == -1)
	    finish(SIGHUP);
	blockf = flags & ~O_NDELAY;	/* make sure O_NDELAY is off */
	nonblockf = flags | O_NDELAY;	/* make sure O_NDELAY is on */
    }
    if (fcntl(fd, F_SETFL, on ? blockf : nonblockf) == -1)
	finish(SIGHUP);
}
#endif /* SYSV */

Peekc()
{
	int	c;

	if (peekp == peekbuf)
		c = -1;
	else
		c = *--peekp & 0377;
	return c;
}

Ungetc(c)
{
	if (peekp == &peekbuf[(sizeof peekbuf) - 1])
		return;		/* Sorry, can't oblige you ... */
	*peekp++ = c;
}

char	*Inputp = 0;

#ifdef IPROCS
#ifdef PIPEPROCS
getchar()
{
	extern int	errno;
	register int	c;

	if (nchars <= 0) {
		do
			nchars = read(0, smbuf, sizeof smbuf);
#ifdef SYSV
		while (nchars == 0 || (nchars < 0 && errno == EINTR));
		if (nchars < 0)
#else
		while (nchars < 0 && errno == EINTR);
		if (nchars <= 0)
#endif /* SYSV */
			finish(SIGHUP);
		bp = smbuf;
		InputPending = nchars > 1;
	}
	if (((c = *bp) & 0200) && MetaKey != 0) {
		*bp = (c & 0177);
		return '\033';
	}
	nchars -= 1;
	return (*bp++ & 0177);
}
#else /* PIPEPROCS */
getchar()
{
	extern long	global_fd;
	long		reads;
	extern int	NumProcs,
			errno;
	register int	tmp,
			nfds;
	int		c;

	if (nchars <= 0) {
		/* Get a character from the keyboard, first checking for
		   any input from a process.  Handle that first, and then
		   deal with the terminal input. */
		if (NumProcs > 0) {
			do {
				do {
					reads = global_fd;
					nfds = select(32, &reads, (long *)0, (long *)0, (struct timeval *)0);
				} while (nfds < 0 && errno == EINTR);

				switch (nfds) {
				case -1:
					printf("\rerror %d in select %ld", errno, global_fd);
					global_fd = 1;
					break;
				default:
					if (reads & 01) {
						nchars = read(0, smbuf, sizeof(smbuf));
						reads &= ~01;
						nfds -= 1;
					}

					while (nfds--) {
						tmp = ffs(reads) - 1;
						read_proc(tmp);
						reads &= ~(1 << tmp);
					}

					break;
				}
			} while (nchars <= 0);
		} else {
			do
				nchars = read(0, smbuf, sizeof(smbuf));
			while (nchars < 0 && errno == EINTR);
		}

		if (nchars <= 0)
			finish(SIGHUP);

		bp = smbuf;
		InputPending = (nchars > 1);
	}

	if (((c = *bp) & 0200) && MetaKey != 0) {
		*bp = (c & 0177);
		return '\033';
	}
	nchars -= 1;
	return *bp++ & 0377;
}
#endif /* PIPEPROCS */
#else /* IPROCS */
getchar()
{
	extern int	errno;
	register int	c;

	if (nchars <= 0) {
		do {
#ifdef MSDOS
			*smbuf = getrawinchar();
			nchars = 1;
#else
			nchars = read(0, smbuf, sizeof smbuf);
#endif
		}
		while (nchars < 0
#ifndef MSDOS
							&& errno == EINTR
#endif
												);

		if (nchars <= 0)
			finish(SIGHUP);
		bp = smbuf;
		InputPending = nchars > 1;
	}
	if (((c = *bp) & 0200) && MetaKey != 0) {
		*bp = (c & 0177);
		return '\033';
	}
	nchars -= 1;
	return *bp++ & 0377;
}

#endif /* IPROCS */

int	InputPending = 0;

/* Returns non-zero if a character waiting */

charp()
{
#ifndef MSDOS
	int	some = 0;
#endif /* MSDOS */

	if (InJoverc != 0 || nchars > 0 || Inputp != 0)
		return 1;
#ifdef BRLUNIX
	{
		static struct sg_brl gttyBuf;

		gtty(0, (char *) &gttyBuf);
		if (gttyBuf.sg_xflags & INWAIT)
			some += 1;
	}
#endif
#ifdef FIONREAD
	{
		long c;

		if (ioctl(0, FIONREAD, (struct sgttyb *) &c) == -1)
			c = 0;
		some = (c > 0);
	}
#endif /* FIONREAD */
#ifdef SYSV
	setblock(0, 0);		/* turn blocking off */
	nchars = read(0, smbuf, sizeof smbuf);	/* Is anything there? */
	setblock(0, 1);		/* turn blocking on */
	if (nchars > 0)		/* something was there */
	    bp = smbuf;		/* make sure bp points to it */
	some = (nchars > 0);	/* just say we found something */
#endif /* SYSV */
#ifdef c70
	some = !empty(0);
#endif
#ifdef MSDOS
    return rawkey_ready();
#else
	return some;
#endif /* MSDOS */
}

#ifndef IBMPC
ResetTerm()
{
	putpad(TI, 1);
	putpad(VS, 1);
	putpad(KS, 1);
#ifdef BIFF
	if (BiffChk != dw_biff)
		biff_init();
	/* just in case we changed our minds about whether to deal with
	   biff */
#endif
#ifndef MSDOS
	(void) chkmail(YES);	/* force it to check to we can be accurate */
#endif
	do_sgtty();		/* this is so if you change baudrate or stuff
				   like that, JOVE will notice. */
	ttyset(ON);
}

UnsetTerm(mesg)
char	*mesg;
{
	ttyset(OFF);
	putpad(KE, 1);
	putpad(VE, 1);
	putpad(TE, 1);
#ifdef ID_CHAR
	INSmode(0);
#endif
	Placur(ILI, 0);
	printf("%s", mesg);
	putpad(CE, 1);
	flusho();
}

#ifdef JOB_CONTROL
PauseJove()
{
	UnsetTerm(ModBufs(0) ? "[There are modified buffers]" : NullStr);
	(void) kill(0, SIGTSTP);
	ResetTerm();
	ClAndRedraw();
}
#endif
#endif /* IBMPC */

Push()
{
	int	pid,

#ifdef MSDOS
   		(*old_int)() = signal(SIGINT, SIG_IGN);
#else /* MSDOS */
   		(*old_int)() = signal(SIGINT, SIG_IGN),
		(*old_quit)() = signal(SIGQUIT, SIG_IGN);
#endif /* MSDOS */

#ifndef MSDOS
#ifdef IPROCS
	sighold(SIGCHLD);
#endif
	switch (pid = fork()) {
	case -1:
		complain("[Fork failed]");

	case 0:
		UnsetTerm(NullStr);
#ifdef IPROCS
		sigrelse(SIGCHLD);
#endif
		(void) signal(SIGTERM, SIG_DFL);
#else /* MSDOS */
	UnsetTerm();
#endif /* MSDOS */
		(void) signal(SIGINT, SIG_DFL);
#ifndef MSDOS
		(void) signal(SIGQUIT, SIG_DFL);
		execl(Shell, basename(Shell), (char *)0);
		message("[Execl failed]");
		_exit(1);
	}
    	dowait(pid, (int *) 0);
#ifdef IPROCS
	sigrelse(SIGCHLD);
#endif
#else /* MSDOS */
	break_rst();
	if (spawnl(0, Shell, basename(Shell), (char *)0) == -1)
		message("[Spawn failed]"); 
#endif /* MSDOS */
    	ResetTerm();
    	ClAndRedraw();
#ifndef MSDOS
	(void) signal(SIGQUIT, old_quit);
#else /* MSDOS */
	break_off();
#endif /* MSDOS */
    	(void) signal(SIGINT, old_int);
}

int	OKXonXoff = 0,		/* ^S and ^Q initially DON'T work */
	IntChar = CTL(']');

ttsize()
{
#ifndef MSDOS
#ifdef TIOCGWINSZ
	struct winsize win;

	if (ioctl (0, TIOCGWINSZ, &win) == 0) {
		if (win.ws_col)
			CO = win.ws_col;
		if (win.ws_row)
			LI = win.ws_row;
	}
#else /* TIOCGWINSZ */
#ifdef BTL_BLIT
#include <sys/jioctl.h>
	struct jwinsize jwin;

	if (ioctl(0, JWINSIZE, &jwin) == 0) {
		if (jwin.bytesx)
			CO = jwin.bytesx;
		if (jwin.bytesy)
			LI = jwin.bytesy;
	}
#endif /* BTL_BLIT */
#endif /* TIOCGWINSZ */
#endif /* MSDOS */
	ILI = LI - 1;
}

#ifdef BIFF
biff_init()
{
	dw_biff = ((BiffChk) &&
#ifndef BSD4_2
		   ((tt_name != 0) || (tt_name = ttyname(0))) &&
		   (stat(tt_name, &tt_stat) != -1) &&
#else
		   (fstat(0, &tt_stat) != -1) &&
#endif
		   (tt_stat.st_mode & S_IEXEC));	/* he's using biff */

}

biff(on)
{
	if (dw_biff == NO)
		return;
#ifndef BSD4_2
	(void) chmod(tt_name, on ? tt_stat.st_mode :
				   (tt_stat.st_mode & ~S_IEXEC));
#else
	(void) fchmod(0, on ? tt_stat.st_mode :
			      (tt_stat.st_mode & ~S_IEXEC));
#endif
}

#endif

ttinit()
{
#ifdef BIFF
	biff_init();
#endif
#ifdef TIOCSLTC
	(void) ioctl(0, TIOCGLTC, (struct sgttyb *) &ls1);
	ls2 = ls1;
	ls2.t_suspc = (char) -1;
	ls2.t_dsuspc = (char) -1;
	ls2.t_flushc = (char) -1;
	ls2.t_lnextc = (char) -1;
#endif

#ifdef TIOCGETC
	/* Change interupt and quit. */
	(void) ioctl(0, TIOCGETC, (struct sgttyb *) &tc1);
	tc2 = tc1;
	tc2.t_intrc = IntChar;
	tc2.t_quitc = (char) -1;
	if (OKXonXoff) {
		tc2.t_stopc = (char) -1;
		tc2.t_startc = (char) -1;
	}
#endif /* TIOCGETC */
	do_sgtty();
}

private int	done_ttinit = 0;

do_sgtty()
{
#ifndef MSDOS
#ifdef SYSV
	(void) ioctl(0, TCGETA, (char *) &sg1);
#else
	(void) gtty(0, &sg1);
#endif /* SYSV */
	sg2 = sg1;

#ifdef SYSV
	TABS = !((sg1.c_oflag & TAB3) == TAB3);
	ospeed = sg1.c_cflag & CBAUD;

	if (OKXonXoff)
		sg2.c_iflag &= ~(IXON | IXOFF);
	sg2.c_iflag &= ~(INLCR|ICRNL|IGNCR);
	sg2.c_lflag &= ~(ISIG|ICANON|ECHO);
	sg2.c_oflag &= ~(OCRNL|ONLCR);
	sg2.c_cc[VMIN] = sizeof smbuf;
	sg2.c_cc[VTIME] = 1;
#else
	TABS = !(sg1.sg_flags & XTABS);
	ospeed = sg1.sg_ospeed;
#ifdef BRLUNIX
	sg2.sg_flags &= ~(ECHO | CRMOD);
	sg2.sg_flags |= CBREAK;

	/* VT100 Kludge: leave STALL on for flow control if DC3DC1 (Yuck.) */
	sg2.sg_xflags &= ~((sg2.sg_xflags&DC3DC1 ? 0 : STALL) | PAGE);
#else
	sg2.sg_flags &= ~(ECHO | CRMOD);
#endif /* BRLUNIX */

#ifdef EUNICE
	sg2.sg_flags |= RAW;	/* Eunice needs RAW mode last I heard. */
#else
#ifdef PURDUE_EE
#   ifdef pdp11
	sg2.sg_flags |= RAW;
#   else
	sg2.sg_flags |= (MetaKey ? RAW : CBREAK);
#   endif
#else
	sg2.sg_flags |= (MetaKey ? RAW : CBREAK);
#endif /* PURDUE_EE */
#endif /* EUNICE */
#endif /* SYSV */
#else /* MSDOS */
	TABS = 0;
#endif /* MSDOS */
}

tty_reset()
{
	if (!done_ttinit)
		return;
	ttyset(OFF);	/* go back to original modes */
	ttinit();
	ttyset(ON);
}

/* If n is OFF reset to original modes */

ttyset(n)
{
	if (!done_ttinit && n == 0)	/* Try to reset before we've set! */
		return;
#ifndef MSDOS
#ifdef SYSV
	(void) ioctl(0, TCSETAW, n == 0 ? (struct sgttyb *) &sg1 : (struct sgttyb *) &sg2);
#else
#ifdef BRLUNIX
	(void) stty(0, n == 0 ? (struct sgttyb *) &sg1 : (struct sgttyb *) &sg2);
#else
	(void) ioctl(0, TIOCSETN, n == 0 ? (struct sgttyb *) &sg1 : (struct sgttyb *) &sg2);
#endif /* BRLUNIX */
#endif /* SYSV */

#ifdef TIOCSETC
	(void) ioctl(0, TIOCSETC, n == 0 ? (struct sgttyb *) &tc1 : (struct sgttyb *) &tc2);
#endif /* TIOCSETC */
#ifdef TIOCSLTC
	(void) ioctl(0, TIOCSLTC, n == 0 ? (struct sgttyb *) &ls1 : (struct sgttyb *) &ls2);
#endif /* TIOCSLTC */
#endif /* MSDOS */
	done_ttinit = 1;
#ifdef BIFF
	biff(!n);
#endif
}

int	this_cmd,
	last_cmd;

dispatch(c)
register int	c;
{
	data_obj	*cp;

	this_cmd = 0;
#ifndef IBMPC
	cp = mainmap[c & 0177];
#else /* IBMPC */
	cp = mainmap[c];
#endif /* IBMPC */

	if (cp == 0) {
		rbell();
		clr_arg_value();
		errormsg = NO;
		message(NullStr);
	} else
		ExecCmd(cp);
}

int	LastKeyStruck,
	MetaKey = 0;

getch()
{
	register int	c,
			peekc;
#ifdef IPROCS
	extern int	NumProcs;
#endif
	extern int	ModCount,
			Interactive;

	if (Inputp) {
		if ((c = *Inputp++) != 0)
			return LastKeyStruck = c;
		Inputp = 0;
	}

	if (InJoverc)
		return EOF;	/* somethings wrong if Inputp runs out while
				   we're reading a .joverc file. */

#ifndef MSDOS
	if (ModCount >= SyncFreq) {
		ModCount = 0;
		SyncRec();
	}

#endif /* MSDOS */
	/* If we're not interactive and we're not executing a macro,
	   AND there are no ungetc'd characters, we read from the
	   terminal (i.e., getch()).  And characters only get put
	   in macros from inside this if. */
	if (((peekc = c = Peekc()) == -1) && (Interactive || ((c = mac_getc()) == -1))) {
		/* So messages that aren't error messages don't
		   hang around forever. */
		if (!UpdMesg && !Asking) {	/* Don't erase if we are asking */
			if (mesgbuf[0] && !errormsg)
				message(NullStr);
		}
		redisplay();
#ifdef IPROCS
#  ifdef PIPEPROCS
		if (NumProcs > 0) {
			sigrelse(INPUT_SIG);
			sigrelse(SIGCHLD);
		}
#  endif
#endif
		inIOread = 1;
		if ((c = getchar()) == EOF)
			finish(SIGHUP);
		inIOread = 0;

#ifdef IPROCS
#  ifdef PIPEPROCS
		if (NumProcs > 0) {
			sighold(INPUT_SIG);
			sighold(SIGCHLD);
		}
#  endif
#endif
		if (!Interactive && InMacDefine)
			mac_putc(c);
	}
	if (peekc == -1)	/* don't add_stroke peekc's */
		add_stroke(c);
	return LastKeyStruck = c;
}

#ifndef MSDOS
dorecover()
{
	execl(Recover, "recover", "-d", TmpFilePath, (char *) 0);
	printf("%s: execl failed!\n", Recover);
	flusho();
	_exit(-1);
}
#endif /* MSDOS */
		

ShowVersion()
{
	extern char	*version;

	s_mess("Jonathan's Own Version of Emacs (%s)", version);
}

UNIX_cmdline(argc, argv)
char	*argv[];
{
	int	lineno = 0,
		nwinds = 1;
	Buffer	*b;

	ShowVersion();
	while (argc > 1) {
		if (argv[1][0] != '-' && argv[1][0] != '+') {
			int	force = (nwinds > 0 || lineno != 0);

#ifdef MSDOS
			strlwr(argv[1]);
#endif /* MSDOS */
			minib_add(argv[1], force ? YES : NO);
			b = do_find(nwinds > 0 ? curwind : (Window *) 0,
				    argv[1], force);
			if (force) {
				SetABuf(curbuf);
				SetBuf(b);
				SetLine(next_line(curbuf->b_first, lineno));
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

			case 'p':
				argv += 1;
				argc -= 1;
				SetBuf(do_find(curwind, argv[1], 0));
				ParseAll();
				nwinds = 0;
				break;

			case 't':
				argv += 1;
				argc -= 1;
				find_tag(argv[1], YES);
				break;

			case 'w':
				if (argv[1][2] == '\0')
					nwinds += 1;
				else
					nwinds += -1 + chr_to_int(&argv[1][2], 10, NIL);
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
				lineno = chr_to_int(&argv[1][1], 10, 0) - 1;
				break;
		}
		argv += 1;
		argc -= 1;
	}
}

/* VARARGS1 */

error(fmt, va_alist)
char	*fmt;
va_dcl
{
	va_list	ap;

	if (fmt) {
		va_start(ap);
		format(mesgbuf, sizeof mesgbuf, fmt, ap);
		va_end(ap);
		UpdMesg = YES;
	}
	rbell();
	(void) longjmp(mainjmp, ERROR);
}

/* VARARGS1 */

complain(fmt, va_alist)
char	*fmt;
va_dcl
{
	va_list	ap;

	if (fmt) {
		va_start(ap);
		format(mesgbuf, sizeof mesgbuf, fmt, ap);
		va_end(ap);
		UpdMesg = YES;
	}
	rbell();
	(void) longjmp(mainjmp, COMPLAIN);
}

/* VARARGS1 */

confirm(fmt, va_alist)
char	*fmt;
va_dcl
{
	char	*yorn;
	va_list	ap;

	va_start(ap);
	format(mesgbuf, sizeof mesgbuf, fmt, ap);
	va_end(ap);
	yorn = ask((char *) 0, mesgbuf);
	if (*yorn != 'Y' && *yorn != 'y')
		(void) longjmp(mainjmp, COMPLAIN);
}

int	RecDepth = 0;

Recur()
{
	char	bname[128];
	Mark	*m;

	sprintf(bname, "%s", curbuf->b_name);
	m = MakeMark(curline, curchar, M_FLOATER);

	RecDepth += 1;
	UpdModLine = YES;
	DoKeys(1);	/* 1 means not first time */
	UpdModLine = YES;
	RecDepth -= 1;
	SetBuf(do_select(curwind, bname));
	if (!is_an_arg())
		ToMark(m);
	DelMark(m);
}

jmp_buf	mainjmp;
int	iniargc;	/* main sets these for DoKeys() */
char	**iniargv;

DoKeys(nocmdline)
{
	int	c;
	jmp_buf	savejmp;

	push_env(savejmp);

	switch (setjmp(mainjmp)) {
	case 0:
		if (!nocmdline)
			UNIX_cmdline(iniargc, iniargv);
		break;

	case QUIT:
		if (RecDepth == 0) {
			if (ModMacs()) {
				rbell();
				if (CharUpcase(*ask("No",
"Some MACROS haven't been saved; leave anyway? ")) != 'Y')
					break;
			}
			if (ModBufs(0)) {
				rbell();
				if (CharUpcase(*ask("No",
"Some buffers haven't been saved; leave anyway? ")) != 'Y')
					break;
			}
#ifdef IPROCS
			KillProcs();
#endif
		}
		pop_env(savejmp);
		return;

	case ERROR:
		getDOT();	/* God knows what state linebuf was in */

	case COMPLAIN:
		gc_openfiles();	/* close any files we left open */
		errormsg = YES;
		unwind_macro_stack();
		Asking = 0;
		curwind->w_bufp = curbuf;
		redisplay();
		break;
	}

	this_cmd = last_cmd = 0;

	for (;;) {
		if (this_cmd != ARG_CMD) {
			clr_arg_value();
			last_cmd = this_cmd;
			init_strokes();
		}
		c = getch();
		if (c == -1)
			continue;
	 	dispatch(c);
	}
}

int	Crashing = 0;

char **
scanvec(args, str)
register char	**args,
		*str;
{
	while (*args) {
		if (strcmp(*args, str) == 0)
			return args;
		args += 1;
	}
	return 0;
}

#ifndef MSDOS
int	UpdFreq = 30,
	inIOread = 0;
#else /* MSDOS */
int	inIOread = 0;
#endif /* MSDOS */

updmode()
{
	UpdModLine = YES;
	if (inIOread)
		redisplay();
#ifndef MSDOS
#ifndef JOB_CONTROL
	(void) signal(SIGALRM, updmode);
#endif
	(void) alarm((unsigned) UpdFreq);
#endif /* MSDOS */
}

#ifndef MSDOS
#ifdef TIOCGWINSZ
#ifdef SIGWINCH
extern win_reshape();
#endif
#endif
#else /* MSDOS */
char	ttbuf[TTBUFSIZ];
#endif /* MSDOS */

#ifdef TIOCGWINSZ
#ifdef SIGWINCH
win_reshape()
{
	register int diff;

	(void) signal(SIGWINCH, SIG_IGN);

	/*
	 * Save old number of lines.
	 */
	diff = LI;

	/*
	 * Get new line/col info.
	 */
	ttsize();

	/*
	 * LI has changed, and now holds the
	 * new value.  See how much the size
	 * changed.
	 */
	diff = LI - diff;

	/*
	 * Change the size of the current window
	 * only.  If they shrank by more than
	 * the window size, tough.
	 */
	if ((curwind->w_height + diff) < 2)
		curwind->w_height = 2;
	else
		curwind->w_height += diff;

	make_scr();
	redisplay();

	(void) signal(SIGWINCH, win_reshape);
}
#endif
#endif

main(argc, argv)
char	*argv[];
{
#ifndef MSDOS
	char	ttbuf[MAXTTYBUF],
#ifndef VMUNIX
		s_iobuff[LBSIZE],
		s_genbuf[LBSIZE],
		s_linebuf[LBSIZE],
#endif
		*cp;


#ifndef VMUNIX
	/* The way I look at it, there ain't no way I is gonna run
	   out of stack space UNLESS I have some kind of infinite
	   recursive bug.  So why use up some valuable memory, when
	   there is plenty of space on the stack?  (This only matters
	   on wimpy pdp11's, of course.) */

	iobuff = s_iobuff;
	genbuf = s_genbuf;
	linebuf = s_linebuf;
#endif

#else /* MSDOS */
   char	*cp,
		*x,
		*getenv();
#endif /* MSDOS */
	errormsg = 0;

	iniargc = argc;
	iniargv = argv;

	if (setjmp(mainjmp)) {
		printf("\rAck! I can't deal with error \"%s\" now.\n\r", mesgbuf);
		finish(0);
	}

#ifdef MSDOS
	/* import the temporary file path from the environment and
	   fix the string, so that we can append a slash safely	*/

	if ((x = getenv("TMP")) && *x) {
	   cp = TmpFilePath;
	   while (*x) {
			 *cp++ = *x++;
	   }
	   if ((cp[-1] == '/') || (cp[-1] == '\\'))
		  cp--;
	   *cp = 0;
	}
	ShFlags[0] = switchar();
			
#endif /* MSDOS */
	getTERM();	/* Get terminal. */
	if (getenv("METAKEY"))
		MetaKey = 1;
	ttsize();
	InitCM();

	d_cache_init();		/* initialize the disk buffer cache */

#ifndef MSDOS
	if (cp = getenv("SHELL"))
#else /* MSDOS */
	if ((cp = getenv("COMSPEC")) && *cp) {
#endif /* MSDOS */
		strcpy(Shell, cp);
#ifdef MSDOS
	}
	if ((x = getenv("DESCRIBE")) && *x)
	   strcpy(CmdDb, x);
#endif /* MSDOS */

	make_scr();
	mac_init();	/* Initialize Macros */
	winit();	/* Initialize Window */
#ifdef IPROCS
	pinit();	/* Pipes/process initialization */
#endif
	SetBuf(do_select(curwind, Mainbuf));

#ifdef CHDIR
	{
		char	**argp;

		if ((argp = scanvec(argv, "-d")) && (argp[1][0] == '/'))
			setCWD(argp[1]);
		else
			getCWD();	/* After we setup curbuf in case we have to getwd() */
	}
#endif
	HomeDir = getenv("HOME");
	if (HomeDir == 0)
		HomeDir = "/";
	HomeLen = strlen(HomeDir);
#ifndef MSDOS
#ifdef SYSV
	sprintf(Mailbox, "/usr/mail/%s", getenv("LOGNAME"));
#else
	sprintf(Mailbox, "/usr/spool/mail/%s", getenv("USER"));
#endif /* SYSV */
	(void) joverc(Joverc);
	if (!scanvec(argv, "-j")) {
		char	tmpbuf[100];
#endif /* MSDOS */

#ifndef MSDOS
		sprintf(tmpbuf, "%s/.joverc", HomeDir);
		(void) joverc(tmpbuf);
	}
	if (scanvec(argv, "-r"))
		dorecover();
	if (scanvec(argv, "-rc"))
		FullRecover();
#else /* MSDOS */
	if (x = getenv("JOVERC"))
	   (void) joverc(x);
	(void) joverc(Joverc);	/* same in current directory	*/
#endif /* MSDOS */

#ifdef MSDOS
	(void) time(&time0);
#endif /* MSDOS */
	ttinit();	/* initialize terminal (after ~/.joverc) */
	settout(ttbuf);	/* not until we know baudrate */
#ifdef MSDOS
	ResetTerm();
#endif /* MSDOS */

#ifndef MSDOS
	(void) signal(SIGHUP, finish);
#else /* MSDOS */
	break_off();	/* disable ctrl-c checking */
#endif /* MSDOS */
	(void) signal(SIGINT, finish);
#ifndef MSDOS
	(void) signal(SIGBUS, finish);
	(void) signal(SIGSEGV, finish);
	(void) signal(SIGPIPE, finish);
	(void) signal(SIGTERM, SIG_IGN);
#ifdef TIOCGWINSZ
#ifdef SIGWINCH
	(void) signal(SIGWINCH, win_reshape);
#endif
#endif
#endif /* MSDOS */

	/* set things up to update the modeline every UpdFreq seconds */
#ifndef MSDOS
	(void) signal(SIGALRM, updmode);
	(void) time(&time0);
	(void) alarm((unsigned) (60 - (time0 % 60)));
#endif /* MSDOS */

#ifndef MSDOS
	ResetTerm();
#endif /* MSDOS */
	cl_scr(1);
	flusho();
	RedrawDisplay();	/* start the redisplay process. */
	DoKeys(0);
	finish(0);
}
