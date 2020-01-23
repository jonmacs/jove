#ifdef JTC
/*
 * Minimal standalone hardwired terminfo/termcap replacement
 * that only supports the subset of vt[123]xx/ANSI
 * X3.64/ECMA-48/ISO6429 needed by Jove.  Since everyone in
 * the year 2020CE probably uses a terminal emulator that
 * supports far more than the set needed by Jove, the approach
 * here is to define a few levels of capabilities, use JOVEVT
 * or TERM to select a level and fake out the tgetent/tputs
 * routines.  This does not provide tparm/tgoto to interpret
 * the termcap/terminfo % operators, instead providing just
 * the one and two-arg functions (jtcarg[12]) that Jove needs,
 * and assuming the two-arg increments row/col as %i does.  If
 * someone finds themselves on a terminal that does not
 * support the necessary minimal subset of ANSI X.3, then you
 * need to compile jove with one of
 * termcap/terminfo/curses/ncurses/ncursesw! Please tell
 * jovedev about that!
 */
/* Mark Moraes, 20200109 */

#include "jove.h"
#include "jctype.h"
#include "fmt.h"	/* for swritef */
#include "fp.h"		/* for flushscreen */

#ifdef USE_SELECT
#  include <sys/time.h>
#  include "select.h"
#endif

#ifdef TERMIOS
# include <termios.h>
# include <sys/ioctl.h>
# ifdef TIOCGWINSZ
private struct winsize jtwin;
#  define JVTCOLS jtwin.ws_col
#  define JVTROWS jtwin.ws_row
# endif /* TIOCGWINSZ */
#endif /* TERMIOS */
#ifndef JVTCOLS
# define JVTCOLS 80
# define JVTROWS 24
#endif

#ifdef TEST_STANDALONE
extern int strncasecmp proto((const char *, const char *, size_t));
extern int snprintf proto((char *, size_t, const char *, ...));
extern int printf(const char *, ...);
extern int putchar(int);
extern void fflush(void *); /* XXX */
extern void *stdout; /* XXX */
#define flushscreen() fflush(stdout)
#define caseeqn(s1, s2, n) (strncasecmp(s1, s2, n) == 0)
#define swritef snprintf
#undef jisdigit
#define jisdigit(c) (c >= '0' && c <= '9')
#endif

/* symbolic values for capability level */
#define ANSI 99	  /* cursor motions, standout/underline */
#define VT100 100 /* scrolling region, flash/visual bell, cursor 'application keypad' mode */
#define VT102 102 /* generally what most VT100 emulators handle */
#define VT125 125 /* anything above this has a meta- or alt- key */
#define VT320 320 /* SR/SF aka SU/SD to pan up or down by multiple lines */
#define VTALT 999 /* alternate screen, like xterm, rxvt, and friends */

typedef struct jtermcap {
	const char *capname;
	const char *capval;
	unsigned caplevel;
} JTermcap;

#define JTMAX 128
private char jtarg[JTMAX];
private int jtlev = -1;

private JTermcap jtc[] = {
	{"vs", "\033[?12;25h", VTALT}, /* ATT610 blinking cursor + DECTCEM show cursor*/
	{"ve", "\033[?12l\033[?25h", VTALT}, /* turn off ATT610 but leave cursor? */
	{"al", "\033[L", ANSI},
	{"dl", "\033[M", ANSI},
	{"cs", "\033[%d;%dr", VT100}, /* DECSTBM, so I assume not ANSI? targ2 so will do %i-like incr both args */
	{"so", "\033[7m", ANSI},
	{"se", "\033[27m", ANSI},
	{"us", "\033[4m", ANSI},
	{"ue", "\033[24m", ANSI},
	{"cm", "\033[%d;%dH", ANSI}, /* targ2, so will do %i-like incr both args */
	{"cl", "\033[H\033[J", ANSI}, /* more portable than \033[2J if we need the \033[H to position cursor anyway? */
	{"ce", "\033[K", ANSI},
	{"ho", "\033[H", ANSI},
	{"up", "\033[A", ANSI},
	{"do", "\n", 0}, /* shorter than \033[B */
	{"sf", "\n", 0}, /* shorter than \033D */
	{"sr", "\033M", ANSI},
	{"vb", "\033[?5h$\033[?5l", VT100},	/* $ means 100ms delay, i.e. $<100/>  */
	{"ks", "\033[?1h\033=", VT100},
	{"ke", "\033[?1l\033>", VT100},
	{"ti", "\0337\033[?47h\033[3;8g", VTALT}, /* or \033[?1049h\033[3;8g (clears all tabs and sets tabs every 8 spaces) */
	{"te", "\033[2J\033[?47l\0338", VTALT}, /* or \033[?1049l */
	{"bl", "\007", 0},
	{"AL", "\033[%dL", VT102},
	{"DL", "\033[%dM", VT102},
	{"SF", "\033[%dS", VT320}, /* in ANSI, but not much support, it seems. vttest says it appeared in VT320 */
	{"SR", "\033[%dT", VT320}, /* in ANSI, but not much support, it seems. vttest says it appeared in VT320 */
	{"im", "\033[4h", VT102},
	{"dc", "\033[P", VT102},
	{"ei", "\033[4l", VT102},
	{"IC", "\033[%d@", VT102},
	{"DC", "\033[%dP", VT102},
};

typedef struct jtlevel {
	const char *tprefix;
	unsigned tlevel;
} JTLevel;

private JTLevel jtc_emu[] = {
	{"ansi", ANSI},
	{"xterm", VTALT},
	{"rxvt", VTALT},
	{"xnuppc", VTALT},
	{"putty", VTALT},
	{"linux", VT125},
	{"screen", VTALT},
	{"gnome", VTALT},
	{"konsole", VTALT},
	{"eterm", VTALT},
	{"nsterm", VTALT},
	{"kterm", VTALT},
	{"vte", VTALT},
	{"st", VTALT},
};

char *
jtcarg1(fmt, p)
const char *fmt;
int p;
{
	if (fmt)
		swritef(jtarg, sizeof jtarg, fmt, p);
	else
		jtarg[0] = '\0';
	return jtarg;
}

/*
 * XXX always increment both args because the capability
 * strings for cm and cs on ANSI/ECMA/VTxxx terminals need
 * that, and that is all JOVE uses targ2 for. i.e.
 * hardwires the %i behaviour from terminfo/termcap
 */
char *
jtcarg2(fmt, destcol, destline)
const char *fmt;
int destcol, destline;
{
	if (fmt)
		swritef(jtarg, sizeof jtarg, fmt, destline+1, destcol+1);
	else
		jtarg[0] = '\0';
	return jtarg;
}

/*
 * same as jove.c SitFor but without charp/redisplay to avoid
 * risk of recursion.  If you do not have select, use regular
 * termcap.  A jdelay function ought to be factored out of jove.c
 * and moved to unix.c, ibmpcdos.c, win32.c, mac.c.
 */
void
jdelay(delay)
int delay;
{
	struct timeval	timer;
	fd_set	readfds;
	
	timer.tv_sec = (delay / 10);
	timer.tv_usec = (delay % 10) * 100000;
	FD_ZERO(&readfds);
	FD_SET(0, &readfds);
	(void) select(1,
		      &readfds, (fd_set *)NULL, (fd_set *)NULL,
		      &timer);
}

void
tputs(str, lines, putfunc)
const char *str;
int lines;
void (*putfunc) proto((int));
{
	const char *cp = str;
	ZXchar c;
	bool needflush = NO;
	while (cp && (c = ZXC(*cp++)) != '\0') {
		if (c == '$') {
			flushscreen();
			needflush = YES;
			jdelay(1);  /* tenth of a second */
		} else {
			(*putfunc)(c);
		}
	}
	if (needflush) flushscreen();
}

int
tgetent(buf, tenv)
char *buf;
const char *tenv;
{
	const char *jtcenv = getenv("JOVEVT");
	if (jtcenv != NULL) {
		if (caseeqn(jtcenv, "alt", 3) || caseeqn(jtcenv, "vtalt", 5)) {
			jtlev = VTALT;
		} else if (caseeqn(jtcenv, "vt", 2)) {
			jtlev = atoi(jtcenv+2);
		} else {
			jtlev = atoi(jtcenv);
		}
	} else {
		JTLevel *j = jtc_emu;
		if (caseeqn(tenv, "vt", 2) && jisdigit(tenv[2])) {
			jtlev = atoi(&tenv[2]);
		}
		for (j = jtc_emu;
		     j < jtc_emu + (sizeof(jtc_emu)/sizeof(jtc_emu[0])); j++) {
			size_t tlen = strlen(j->tprefix);
			if (caseeqn(tenv, j->tprefix, tlen) &&
			    (tenv[tlen] == '\0' || tenv[tlen] == '-' ||
			     tenv[tlen] == '.' || jisdigit(tenv[tlen]))) {
				jtlev = j->tlevel;
				break;
			}
		}
	}
#ifdef TIOCGWINSZ
	if (ioctl(0, TIOCGWINSZ, (UnivPtr) &jtwin) < 0)
		return -1;
#endif
	return jtlev >= 0;
}

#define capeq(cap, val) (cap[0] == val[0] && cap[1] == val[1] && \
			 cap[2] == val[2] && cap[2] == '\0')

int
tgetflag(capname)
const char *capname;
{
	if (capeq(capname, "mi") && jtlev >= VT125)
		return YES; /* terminfo for vt102 does not have move-in-insert */
	else if (capeq(capname, "km") && jtlev >= VTALT)
		return YES; /* metakey only on modern terminal emulators */
	/*
	 * NP is not checked on modern machines with select(),
	 * which are the only ones with xterm etc where NP
	 * might be true.
	 */
	return NO;
}


int
tgetnum(capname)
const char *capname;
{
	if (capeq(capname, "co")) {
		return JVTCOLS;
	} else if (capeq(capname, "li")) {
		return JVTROWS;
	} else if (capeq(capname, "it")) {
		return 8;
	} else if (capeq(capname, "ug") || capeq(capname, "sg")) {
		return 0;
	}
	return -1;
}

const char *
tgetstr(capname, area)
const char *capname;
char **area;
{
	JTermcap *j;
	for (j = jtc; j < jtc + (sizeof(jtc)/sizeof(jtc[0])); j++) {
		if (capeq(j->capname, capname) && j->caplevel <= jtlev) {
			if (area && *area) {
				size_t caplen = strlen(j->capval) + 1;
				memcpy(*area, j->capval, caplen);
				*area += caplen;
			}
			return j->capval;
		}
	}
	return 0;
}

#ifdef TEST_STANDALONE
/* To test, build with
 * LANG=C gcc -o testjtc jtc.c -g -O -Wall -DTEST_STANDALONE -DJTC -DTERMIOS
 */

/* make terminfo string printable */
void
puts_encoded(s)
const char *s;
{
	unsigned c;
	if (s) while ((c = (*s++)&0xff) != '\0') {
		/*printf(" 0x%x\n", c);*/
		if (c == 0x1b) {
			/* convention for ESC per terminfo(5) */
			printf("\\E");
		} else if (c == 0x7f) {
			/* convention for DEL e.g. terminfo(5) */
			printf("^?");
		} else if (c < ' ') {
			/* ^ followed up uppercase letter */
			printf("^%c", c | 0x40);
		} else if (c < 0x7f) {
			printf("%c", c);
		} else {
			printf("\\%03o", c);
		}
	}
	printf("\n");
}

void
xtputs(delay, s, li)
int delay, li;
const char *s;
{
	tputs(s, li, putchar);
	fflush(stdout);
	jdelay(delay);
}
    
int main(argc, argv)
int argc;
char **argv;
{
	int i, j;
	static const char *capn[] = {"co", "li", "it", "ug", "sg", "xx"};
	static const char *capf[] = {"NP", "mi", "km", "ul", "xs", "hz"};
	const char *cp;
	for (i = 1; i < argc; i++) {
		int r = tgetent(NULL, argv[i]);
		printf("tgetent(%s) -> %d jtlev=%d\n", argv[i], r, jtlev);
		for (j = 0; j < sizeof(capn)/sizeof(capn[0]); j++) {
			int v = tgetnum(capn[j]);
			printf("  %s=%d\n", capn[j], v);
		}
		for (j = 0; j < sizeof(capf)/sizeof(capf[0]); j++) {
			int v = tgetflag(capf[j]);
			printf("  %s=%s\n", capf[j], v == YES ? "YES" : "NO");
		}
		for (j = 0; j < sizeof(jtc)/sizeof(jtc[0]); j++) {
			const char *s = tgetstr(jtc[j].capname, NULL);
			printf("  %s=", jtc[j].capname);
			puts_encoded(s);
		}
	}
	if (argc > 1) exit(0);
	printf("about to enter screen mode:\n");
	fflush(stdout);
	jdelay(10);
	cp = getenv("TERM");
	tgetent(NULL, cp);
	xtputs(1, tgetstr("ti", NULL), 0);
	xtputs(1, jtcarg1(tgetstr("ks", NULL),0), 0);
	xtputs(10, tgetstr("cl", NULL), 24);
	xtputs(10, "hello\nworld", 2);
	xtputs(1, jtcarg2(tgetstr("cm", NULL), 4, 0), 1);
	xtputs(10, jtcarg1(tgetstr("dc", NULL), 1), 1);
	xtputs(1, jtcarg2(tgetstr("cm", NULL), 2, 1), 1);
	xtputs(10, "wsa", 1);
	xtputs(1, jtcarg2(tgetstr("cs", NULL), 5, 0), 1);
	xtputs(1, tgetstr("ho", NULL), 1);
	xtputs(10, tgetstr("sr", NULL), 1);
	xtputs(10, "111111", 1);
	xtputs(1, tgetstr("ho", NULL), 1);
	xtputs(1, tgetstr("do", NULL), 1);
	xtputs(1, tgetstr("do", NULL), 1);
	xtputs(10, tgetstr("sf", NULL), 1);
	xtputs(10, "222222", 1);
	xtputs(5, jtcarg1(tgetstr("ho", NULL), 1), 1);
	cp = tgetstr("SR", NULL);
	if (cp)
		xtputs(10, jtcarg1(cp, 2), 2);
	else {
		xtputs(5, jtcarg1(tgetstr("sr", NULL), 1), 1);
		xtputs(5, jtcarg1(tgetstr("sr", NULL), 1), 1);
	}
	xtputs(5, jtcarg2(tgetstr("cm", NULL),0,5), 1);
	cp = tgetstr("SF", NULL);
	if (cp) 
		xtputs(10, jtcarg1(cp, 2), 2);
	else {
		xtputs(5, jtcarg1(tgetstr("sf", NULL),1), 1);
		xtputs(5, jtcarg1(tgetstr("sf"),1), 1);
	}
	j = tgetnum("li");
	xtputs(5, tgetstr("dl", NULL), 1);
	xtputs(5, tgetstr("so", NULL), 1);
	printf("%d",j);
	fflush(stdout);
	xtputs(5, tgetstr("se", NULL), 1);
	xtputs(5, tgetstr("vb", NULL), 1);
	xtputs(5, jtcarg2(tgetstr("cs", NULL),j-1,0), 0);
	xtputs(5, jtcarg2(tgetstr("cm", NULL),0,j-1), 0);
	xtputs(5, tgetstr("ce", NULL), 0);
	xtputs(0, tgetstr("ke", NULL), 0);
	xtputs(0, tgetstr("te", NULL), 0);
	printf("done.\n");
	return 0;
}
#endif /* TEST_STANDALONE */
#endif /* JTC */
