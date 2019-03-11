/************************************************************************
 * This program is Copyright (C) 1986 by Jonathan Payne.  JOVE is       *
 * provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is *
 * included in all the files.                                           *
 ************************************************************************/

#include "jove.h"
#include "ctype.h"
#include "termcap.h"
#include <signal.h>
#include <varargs.h>

struct cmd *
FindCmd(proc)
register int 	(*proc)();
{
	register struct cmd	*cp;

  	for (cp = commands; cp->Name; cp++)
		if (cp->c_proc == proc)
			return cp;
	return 0;
}

int	Interactive;	/* True when we invoke with the command handler? */
data_obj	*LastCmd;
char	*ProcFmt = ": %f ";

ExecCmd(cp)
data_obj	*cp;
{
	LastCmd = cp;
	if (cp->Type & MAJOR_MODE)
		SetMajor((cp->Type >> 8));
	else if (cp->Type & MINOR_MODE)
		TogMinor((cp->Type >> 8));
	else	switch (cp->Type&TYPEMASK) {
		case MACRO:
			do_macro((struct macro *) cp);
			break;

		case FUNCTION:
		    {
		    	struct cmd	*cmd = (struct cmd *) cp;

			if (cmd->c_proc)
				(*cmd->c_proc)();
		    }
	}
}

Line *
lastline(lp)
register Line	*lp;
{
	register Line	*next;

	while (next = lp->l_next)
		lp = next;
	return lp;
}

private int	*slowp = 0;

char	key_strokes[100];
private char	*key_p = key_strokes;

init_strokes()
{
	key_strokes[0] = 0;
	key_p = key_strokes;
}

add_stroke(c)
{
	if (key_p + 5 > &key_strokes[(sizeof key_strokes) - 1])
		key_p = key_strokes;
	sprintf(key_p, "%p ", c);
	key_p += strlen(key_p);
}

slowpoke()
{
	if (slowp)
		*slowp = YES;
	f_mess(key_strokes);
}

#ifndef MSDOS
#ifdef BSD4_2
#	define N_SEC	1	/* will be precisely 1 second on 4.2 */
#else
#	define N_SEC	2	/* but from 1 to 2 seconds otherwise */
#endif
#else /* MSDOS */
#define N_SEC	1
int in_macro();
#endif /* MSDOS */

waitchar(slow)
int	*slow;
{
#ifndef MSDOS
#ifdef EUNICE
	return getch();
#endif
	unsigned int	old_time;
	int	c;
	int	(*oldproc)();
#else /* MSDOS */
	long sw, time();
#endif /* MSDOS */

	slowp = slow;
	if (slow)
		*slow = NO;
#ifndef MSDOS
	oldproc = signal(SIGALRM, slowpoke);

	if ((old_time = alarm((unsigned) N_SEC)) == 0)
		old_time = UpdFreq;
	c = getch();
	(void) alarm(old_time);
	(void) signal(SIGALRM, oldproc);

	return c;
#else /* MSDOS */
	time(&sw);
	sw += N_SEC;
	while(time(0) <= sw)
		if (charp() || in_macro())
			return getch();
	slowpoke();
	return getch();
#endif /* MSDOS */
}

/* dir > 0 means forward; else means backward. */

char *
StrIndex(dir, buf, charpos, what)
char	*buf,
	what;
{
	char	*cp = &buf[charpos],
		c;

	if (dir > 0) {
		while (c = *cp++)
			if (c == what)
				return (cp - 1);
	} else {
		while (cp >= buf && (c = *cp--))
			if (c == what)
				return (cp + 1);
	}
	return 0;
}

blnkp(buf)
register char	*buf;
{
	register char	c;

	while ((c = *buf++) && (c == ' ' || c == '\t'))
		;
	return c == 0;	/* It's zero if we got to the end of the Line */
}

Line *
next_line(line, num)
register Line	*line;
register int	num;
{
	if (num < 0)
		return prev_line(line, -num);
	if (line)
		while (--num >= 0 && line->l_next != 0)
			line = line->l_next;
	return line;
}

Line *
prev_line(line, num)
register Line	*line;
register int	num;
{
	if (num < 0)
		return next_line(line, -num);
	if (line)
		while (--num >= 0 && line->l_prev != 0)
			line = line->l_prev;
	return line;
}

DotTo(line, col)
Line	*line;
{
	Bufpos	bp;

	bp.p_line = line;
	bp.p_char = col;
	SetDot(&bp);
}

/* If bp->p_line is != current line, then save current line.  Then set dot
   to bp->p_line, and if they weren't equal get that line into linebuf.  */

SetDot(bp)
register Bufpos	*bp;
{
	register int	notequal;

	if (bp == 0)
		return;

	notequal = bp->p_line != curline;
	if (notequal)
		lsave();
	if (bp->p_line)
		curline = bp->p_line;
	if (notequal)
		getDOT();
	curchar = bp->p_char;
	if (curchar > length(curline))
		curchar = length(curline);
}

ToLast()
{
	SetLine(curbuf->b_last);
	Eol();
}

int	MarkThresh = 22;	/* average screen size ... */
static int	line_diff;

LineDist(nextp, endp)
register Line	*nextp,
		*endp;
{
	(void) inorder(nextp, 0, endp, 0);
	return line_diff;
}

inorder(nextp, char1, endp, char2)
register Line	*nextp,
		*endp;
{
	int	count = 0;
	register Line	*prevp = nextp;

	line_diff = 0;
	if (nextp == endp)
		return char1 < char2;

	while (nextp || prevp) {
		if (nextp == endp || prevp == endp)
			break;
		if (nextp)
			nextp = nextp->l_next;
		if (prevp)
			prevp = prevp->l_prev;
		count += 1;
	}
	if (nextp == 0 && prevp == 0)
		return -1;
	line_diff = count;

	return nextp == endp;
}

PushPntp(line)
register Line	*line;
{
	if (LineDist(curline, line) >= MarkThresh)
		set_mark();
}

ToFirst()
{
	SetLine(curbuf->b_first);
}

length(line)
Line	*line;
{
	return strlen(lcontents(line));
};

to_word(dir)
register int	dir;
{
	register char	c;

	if (dir == FORWARD) {
		while ((c = linebuf[curchar]) != 0 && !isword(c))
			curchar += 1;
		if (eolp()) {
			if (curline->l_next == 0)
				return;
			SetLine(curline->l_next);
			to_word(dir);
			return;
		}
	} else {
		while (!bolp() && (c = linebuf[curchar - 1], !isword(c)))
			curchar -= 1;
		if (bolp()) {
			if (curline->l_prev == 0)
				return;
			SetLine(curline->l_prev);
			Eol();
			to_word(dir);
		}
	}
}

/* Are there any modified buffers?  Allp means include B_PROCESS
   buffers in the check. */

ModBufs(allp)
{
	register Buffer	*b;

	for (b = world; b != 0; b = b->b_next) {
		if (b->b_type == B_SCRATCH)
			continue;
		if ((b->b_type == B_FILE || allp) && IsModified(b))
			return 1;
	}
	return 0;
}

char *
filename(b)
register Buffer	*b;
{
	return b->b_fname ? pr_name(b->b_fname, YES) : "[No file]";
}

char *
itoa(num)
register int	num;
{
	static char	line[15];

	sprintf(line, "%d", num);
	return line;
}

min(a, b)
register int	a,
		b;
{
	return (a < b) ? a : b;
}

max(a, b)
register int	a,
		b;
{
	return (a > b) ? a : b;
}

tiewind(w, bp)
register Window	*w;
register Buffer	*bp;
{
	int	not_tied = (w->w_bufp != bp);

	UpdModLine = YES;	/* kludge ... but speeds things up considerably */
	w->w_line = bp->b_dot;
	w->w_char = bp->b_char;
	w->w_bufp = bp;
	if (not_tied)
		CalcWind(w);	/* ah, this has been missing since the
				   beginning of time! */
}

extern int	Jr_Len;

char *
lcontents(line)
register Line	*line;
{
	if (line == curline)
		return linebuf;
	else
		return lbptr(line);
}

char *
ltobuf(line, buf)
Line	*line;
char	*buf;
{
	if (line == curline) {
		if (buf != linebuf)
			strcpy(buf, linebuf);
		Jr_Len = strlen(linebuf);
	} else
		getline(line->l_dline, buf);
	return buf;
}

DOTsave(buf)
Bufpos *buf;
{
	buf->p_line = curline;
	buf->p_char = curchar;
}

/* Return none-zero if we had to rearrange the order. */

fixorder(line1, char1, line2, char2)
register Line	**line1,
		**line2;
register int	*char1,
		*char2;
{
	Line	*tline;
	int	tchar;

	if (inorder(*line1, *char1, *line2, *char2))
		return 0;

	tline = *line1;
	tchar = *char1;
	*line1 = *line2;
	*char1 = *char2;
	*line2 = tline;
	*char2 = tchar;

	return 1;
}

inlist(first, what)
register Line	*first,
		*what;
{
	while (first) {
		if (first == what)
			return 1;
		first = first->l_next;
	}
	return 0;
}

/* Make `buf' (un)modified and tell the redisplay code to update the modeline
   if it will need to be changed. */

int	ModCount = 0;

modify()
{
	extern int	DOLsave;

	if (!curbuf->b_modified) {
		UpdModLine = YES;
		curbuf->b_modified = YES;
	}
	DOLsave = YES;
	if (!Asking)
		ModCount += 1;
}

unmodify()
{
	if (curbuf->b_modified) {
		UpdModLine = YES;
		curbuf->b_modified = NO;
	}
}

numcomp(s1, s2)
register char	*s1,
		*s2;
{
	register int	count = 0;

	while (*s1 != 0 && *s1++ == *s2++)
		count += 1;
	return count;
}

char *
copystr(str)
char	*str;
{
	char	*val;

	if (str == 0)
		return 0;
	val = emalloc(strlen(str) + 1);

	strcpy(val, str);
	return val;
}

#ifndef byte_copy
byte_copy(from, to, count)
register char	*from,
		*to;
register int	count;
{
	while (--count >= 0)
		*to++ = *from++;
}
#endif

len_error(flag)
{
	char	*mesg = "[line too long]";

	(flag == COMPLAIN) ? complain(mesg) : error(mesg);
}

/* Insert num number of c's at offset atchar in a linebuf of LBSIZE */

ins_c(c, buf, atchar, num, max)
char	c, *buf;
{
	register char	*pp, *pp1;
	register int	len;
	int	numchars;	/* number of characters to copy forward */

	if (num <= 0)
		return;
	len = atchar + strlen(&buf[atchar]);
	if (len + num >= max)
		len_error(COMPLAIN);
	pp = &buf[len + 1];		/* + 1 so we can --pp (not pp--) */
	pp1 = &buf[len + num + 1];
	numchars = len - atchar;
	while (numchars-- >= 0)
		*--pp1 = *--pp;
	pp = &buf[atchar];
	while (--num >= 0)
		*pp++ = c;
}

TwoBlank()
{
	register Line	*next = curline->l_next;

	return ((next != 0) &&
		(*(lcontents(next)) == '\0') &&
		(next->l_next != 0) &&
		(*(lcontents(next->l_next)) == '\0'));
}

linecopy(onto, atchar, from)
register char	*onto,
		*from;
{
	register char	*endp = &onto[LBSIZE - 2];

	onto += atchar;

	while (*onto = *from++)
		if (onto++ >= endp)
			len_error(ERROR);
}

char *
IOerr(err, file)
char	*err, *file;
{
	return sprint("Couldn't %s \"%s\".", err, file);
}

#ifndef MSDOS
pclose(p)
int	*p;
{
	(void) close(p[0]);
	(void) close(p[1]);
}

dopipe(p)
int	p[];
{
	if (pipe(p) == -1)
		complain("[Pipe failed]");
}

#endif /* MSDOS */
/* NOSTRICT */

char *
emalloc(size)
{
	register char	*ptr;

	if (ptr = malloc((unsigned) size))
		return ptr;
	/* Try garbage collecting lines */
	GCchunks();
	if (ptr = malloc((unsigned) size))
		return ptr;
	/* Uh ... Oh screw it! */
	error("[Out of memory] ");
	/* NOTREACHED */
}

/* Return the basename of file F. */

char *
basename(f)
register char	*f;
{
	register char	*cp;

	if (cp = rindex(f, '/'))
		return cp + 1;
	else
#ifdef MSDOS
		if (cp = rindex(f, '\\'))
			return cp + 1;
#endif /* MSDOS */
		return f;
}

push_env(savejmp)
jmp_buf	savejmp;
{
	byte_copy((char *) mainjmp, (char *) savejmp, sizeof (jmp_buf));
}

pop_env(savejmp)
jmp_buf	savejmp;
{
	byte_copy((char *) savejmp, (char *) mainjmp, sizeof (jmp_buf));
}

#ifdef LOAD_AV
# if defined(BSD4_2) && !defined(BSD2_10)
#   if defined(PURDUE_EE) && (defined(vax) || defined(gould))

get_la(dp)
double *dp;
{
	*dp = (double) loadav(0) / 100.0;
}

#   else !PURDUE_EE || (!vax && !gould)

#ifdef sun
#   include <sys/param.h>
#endif
#include <nlist.h>

static struct	nlist nl[] = {
	{ "_avenrun" },
#define	X_AVENRUN	0
	{ "" }
};

get_la(dp)
double	*dp;
{
#ifdef sun
	long	avenrun[3];
#else
	double	avenrun[3];
#endif
	static int	kmem = 0;

	if (kmem == -1) {
		*dp = 4.0;	/* So shell commands will say "Chugging" */
		return;
	} else if (kmem == 0) {
		if ((kmem = open("/dev/kmem", 0)) == -1) {
			f_mess("Can't open kmem for load average.");
			*dp = 4.0;
			return;
		}
		nlist("/vmunix", nl);
	}
	lseek(kmem, (long) nl[X_AVENRUN].n_value, 0);
	read(kmem, (char *) avenrun, sizeof(avenrun));
#ifdef sun
	*dp = (double) avenrun[0] / FSCALE;
#else
	*dp = avenrun[0];
#endif
}

#    endif
#  else !BSD4_2 || BSD2_10

get_la(dp)
double	*dp;
{
	short	avg[3];

	gldav(avg);
	*dp = (double) avg[0] / 256;
}

#  endif
#endif /* LOAD_AV */

/* get the time buf, designated by *timep, from FROM to TO. */
char *
get_time(timep, buf, from, to)
char	*buf;
time_t	*timep;
{
	time_t	now;
	char	*cp;
	extern char	*ctime();

	if (timep != 0)
		now = *timep;
	else
		(void) time(&now);
	cp = ctime(&now) + from;
#ifndef MSDOS
	if (to == -1)
#else /* MSDOS */
	if ((to == -1) && (cp[strlen(cp)-1] == '\n'))
#endif /* MSDOS */
		cp[strlen(cp) - 1] = '\0';		/* Get rid of \n */
	else
		cp[to - from] = '\0';
	if (buf) {
		strcpy(buf, cp);
		return buf;
	} else
		return cp;
}

#ifndef MSDOS
strlen(s)
register char	*s;
{
	register char	*base = s;

	while (*s++)
		;
	return (s - base) - 1;
}

char *
index(s, c)
register char	*s;
register int	c;
{
	register int	c1;

	if (c != 0)
		while (c1 = *s++)
			if (c == c1)
				return s - 1;
	return 0;
}

strcmp(s1, s2)
register char	*s1,
		*s2;
{
	if (!s1 || !s2)
		return 1;	/* which is not zero ... */
	while (*s1 == *s2++)
		if (*s1++ == '\0')
			return 0;
	return (*s1 - *--s2);
}

#endif /* MSDOS */

casecmp(s1, s2)
register char	*s1,
		*s2;
{
	if (!s1 || !s2)
		return 1;	/* which is not zero ... */
	while (CharUpcase(*s1) == CharUpcase(*s2++))
		if (*s1++ == '\0')
			return 0;
	return (*s1 - *--s2);
}

casencmp(s1, s2, n)
register char	*s1,
		*s2;
register int	n;
{
	if (!s1 || !s2)
		return 1;	/* which is not zero ... */
	while (--n >= 0  && (CharUpcase(*s1) == CharUpcase(*s2++)))
		if (*s1++ == '\0')
			return 0;
	return ((n < 0) ? 0 : *s1 - *--s2);
}

null_ncpy(to, from, n)
char	*to,
	*from;
{
	(void) strncpy(to, from, n);
	to[n] = '\0';
}

#ifndef MSDOS
strcpy(t, f)
register char	*t,
		*f;
{
	while (*t++ = *f++)
		;
}

#endif /* MSDOS */

/* Tries to pause for delay/10 seconds OR until a character is typed
   at the keyboard.  This works well on BSD4_2 and not so well on the
   rest.  Returns 1 if it returned because of keyboard input, or 0
   otherwise. */

SitFor(delay)
unsigned int	delay;
{
#ifndef MSDOS
#if defined(BSD4_2) && !defined(BSD2_10)
#include <sys/time.h>

	struct timeval	timer;
	long	readfds = 1;

	timer.tv_sec = (delay / 10);
	timer.tv_usec = (delay % 10) * 100000;

	if (charp())
		return;
	/* gross that I had to snarf this from getch() */
	if (!UpdMesg && !Asking) {	/* Don't erase if we are asking */
		if (mesgbuf[0] && !errormsg)
			message(NullStr);
	}
	redisplay();
	select(1, &readfds, (long *)0, (long *)0, &timer);
#else
	static int cps[] = {
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
	register int	nchars;

	if (charp())
		return;
	nchars = (delay * cps[ospeed]) / 10;
	redisplay();
	while ((--nchars > 0) && !InputPending) {
		putchar(0);
		if (OkayAbort) {
			OkayAbort = 0;
			InputPending = charp();
		}
	}
#endif
#else /* MSDOS */
	long	start,
		end,
		curtenths();

	redisplay();
	start = curtenths();
	end = (start + (long) delay) % 864000L;	/* it might wrap past midnight */
	do
		if (InputPending = charp())
			break;
	while (curtenths() < end);
#endif /* MSDOS */
}

#ifdef MSDOS

#include <dos.h>
/*
 * Return the number of 10ths of seconds since the beginning of the day.
 */
private long
curtenths()
{
	union REGS	regs;

	regs.h.ah = 0x2C;
	int86(0x21, &regs, &regs);
	return (long) ((long) regs.h.ch * 36000L) + ((long) regs.h.cl * 600L) +
					((long) regs.h.dh * 10L) +
					((long) regs.h.dl / 10L);
}
#endif /* MSDOS */

sindex(pattern, string)
register char	*pattern,
		*string;
{
	register int	len = strlen(pattern);

	while (*string != '\0') {
		if (*pattern == *string && strncmp(pattern, string, len) == 0)
			return TRUE;
		string += 1;
	}
	return FALSE;
}

make_argv(argv, ap)
register char	*argv[];
va_list	ap;
{
	register char	*cp;
	register int	i = 0;

	argv[i++] = va_arg(ap, char *);
	argv[i++] = basename(argv[0]);
	while (cp = va_arg(ap, char *))
		argv[i++] = cp;
	argv[i] = 0;
}

pnt_line()
{
	register Line	*lp = curbuf->b_first;
	register int	i;

	for (i = 0; lp != 0; i++, lp = lp->l_next)
		if (lp == curline)
			break;
	return i + 1;
}
