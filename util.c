/***************************************************************************
 * This program is Copyright (C) 1986, 1987, 1988 by Jonathan Payne.  JOVE *
 * is provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is    *
 * included in all the files.                                              *
 ***************************************************************************/

#include "jove.h"
#include "ctype.h"
#include "termcap.h"
#include "disp.h"
#include "fp.h"
#include <signal.h>
#include <errno.h>
#include "ask.h"
#include "fmt.h"
#include "insert.h"
#include "macros.h"
#include "marks.h"
#include "move.h"

#ifdef	MAC
# include "mac.h"
#endif

char *
StrIndex(dir, buf, charpos, what)
int	dir;	/* FORWARD or BACKWARD */
register char	*buf;
int	charpos;
register int	what;
{
	register char	*cp = &buf[charpos];
	register int	c;

	if (dir > 0) {
		while ((c = *cp++) != '\0')
			if ((c == what) != '\0')
				return (cp - 1);
	} else {
		while (cp >= buf && (c = *cp--)!='\0')
			if (c == what)
				return (cp + 1);
	}
	return NULL;
}

bool
blnkp(buf)
register char	*buf;
{
	register char	c;

	do ; while ((c = *buf++)!='\0' && (c == ' ' || c == '\t'));
	return c == 0;	/* It's zero if we got to the end of the Line */
}

bool
within_indent()
{
	register char	c;
	register int	i;

	i = curchar;
	do ; while (--i >= 0 && ((c = linebuf[i]) == ' ' || c == '\t'));
	return i < 0;		/* it's < 0 if we got to the beginning */
}

void
DotTo(line, col)
Line	*line;
int	col;
{
	Bufpos	bp;

	bp.p_line = line;
	bp.p_char = col;
	SetDot(&bp);
}

/* If bp->p_line is != current line, then save current line.  Then set dot
   to bp->p_line, and if they weren't equal get that line into linebuf.  */

void
SetDot(bp)
register Bufpos	*bp;
{
	register int	notequal;

	if (bp == NULL)
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

void
ToLast()
{
	SetLine(curbuf->b_last);
	Eol();
}

int	MarkThresh = 22;	/* average screen size ... */
static int	line_diff;

int
LineDist(nextp, endp)
register Line	*nextp,
		*endp;
{
	(void) inorder(nextp, 0, endp, 0);
	return line_diff;
}

int
inorder(nextp, char1, endp, char2)
register Line	*nextp,
		*endp;
int	char1,
	char2;
{
	register Line	*prevp = nextp;

	line_diff = 0;
	if (nextp == endp)
		return char1 < char2;

	while (nextp && prevp) {
		nextp = nextp->l_next;
		prevp = prevp->l_prev;
		line_diff += 1;
		if (nextp == endp)
			return YES;
		if (prevp == endp)
			return NO;
	}
	while (nextp!=NULL && nextp!=endp) {
		nextp = nextp->l_next;
		line_diff += 1;
	}
	while (prevp!=NULL && prevp!=endp) {
		prevp = prevp->l_prev;
		line_diff += 1;
	}
	/* nextp == prevp implies both are NULL: the lines are not ordered */
	return nextp==prevp? -1 : nextp==endp;
}

void
PushPntp(line)
register Line	*line;
{
	if (LineDist(curline, line) >= MarkThresh)
		set_mark();
}

void
ToFirst()
{
	SetLine(curbuf->b_first);
}

int
length(line)
Line	*line;
{
	return strlen(lcontents(line));
}

void
to_word(dir)
register int	dir;
{
	if (dir == FORWARD) {
		for(;;) {
			register char	c;

			while ((c = linebuf[curchar]) != '\0' && !jisword(c))
				curchar += 1;
			if (c != '\0')
				break;	/* success */
			if (curline->l_next == NULL)
				break;	/* failure: out of buffer */
			SetLine(curline->l_next);
		}
	} else {
		for (;;) {
			while (!bolp() && !jisword(linebuf[curchar - 1]))
				curchar -= 1;
			if (!bolp())
				break;	/* success */
			if (curline->l_prev == NULL)
				break;	/* failure: out of buffer */
			SetLine(curline->l_prev);
			Eol();
		}
	}
}

/* Are there any modified buffers?  Allp means include B_PROCESS
   buffers in the check. */

bool
ModBufs(allp)
bool	allp;
{
	register Buffer	*b;

	for (b = world; b != NULL; b = b->b_next)
		if (b->b_type != B_SCRATCH
		&& (b->b_type == B_FILE || allp)
		&& IsModified(b))
			return YES;
	return NO;
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

	swritef(line, sizeof(line), "%d", num);
	return line;
}

int
min(a, b)
register int	a,
		b;
{
	return (a < b) ? a : b;
}

int
max(a, b)
register int	a,
		b;
{
	return (a > b) ? a : b;
}

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
		jgetline(line->l_dline, buf);
	return buf;
}

void
DOTsave(buf)
Bufpos *buf;
{
	buf->p_line = curline;
	buf->p_char = curchar;
}

/* Return none-zero if we had to rearrange the order. */

bool
fixorder(line1, char1, line2, char2)
register Line	**line1,
		**line2;
register int	*char1,
		*char2;
{
	Line	*tline;
	int	tchar;

	if (inorder(*line1, *char1, *line2, *char2))
		return NO;

	tline = *line1;
	tchar = *char1;
	*line1 = *line2;
	*char1 = *char2;
	*line2 = tline;
	*char2 = tchar;

	return YES;
}

bool
inlist(first, what)
register Line	*first,
		*what;
{
	while (first) {
		if (first == what)
			return YES;
		first = first->l_next;
	}
	return NO;
}

/* Make `buf' (un)modified and tell the redisplay code to update the modeline
   if it will need to be changed. */

int	ModCount = 0;

void
modify()
{
	if (!curbuf->b_modified) {
		UpdModLine = YES;
		curbuf->b_modified = YES;
	}
	DOLsave = YES;
	if (!Asking)
		ModCount += 1;
}

void
unmodify()
{
	if (curbuf->b_modified) {
		UpdModLine = YES;
		curbuf->b_modified = NO;
	}
}

int
numcomp(s1, s2)
register char	*s1,
		*s2;
{
	register int	count = 0;

	while (*s1 != '\0' && *s1++ == *s2++)
		count += 1;
	return count;
}

char *
copystr(str)
char	*str;
{
	char	*val;

	if (str == NULL)
		return NULL;
	val = emalloc((size_t) (strlen(str) + 1));

	strcpy(val, str);
	return val;
}

#ifndef	byte_copy
void
byte_copy(from, to, count)
UnivConstPtr	*from;
UnivPtr		*to;
register size_t	count;
{
	register const char	*p = from;
	register char		*q = to;

	if (count != 0) {
	    do *q++ = *p++; while (--count != 0);
	}
}
#endif

void
len_error(flag)
int	flag;
{
	char	*mesg = "[line too long]";

	if (flag == COMPLAIN)
		complain(mesg);
	else
		error(mesg);
}

/* Insert num number of c's at offset atchar in a linebuf of LBSIZE */

void
ins_c(c, buf, atchar, num, max)
int	c;
char	*buf;
int	atchar,
	num,
	max;
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

int
TwoBlank()
{
	register Line	*next = curline->l_next;

	return (next != NULL
		&& *(lcontents(next)) == '\0'
		&& next->l_next != NULL
		&& *(lcontents(next->l_next)) == '\0');
}

void
linecopy(onto, atchar, from)
register char	*onto,
		*from;
int	atchar;
{
	register char	*endp = &onto[LBSIZE];

	onto += atchar;

	do {
		if (onto >= endp)
			len_error(ERROR);
	} while ((*onto++ = *from++) != '\0');
}

char *
IOerr(err, file)
char	*err, *file;
{
	return sprint("Couldn't %s \"%s\".", err, file);
}

#ifdef	UNIX
void
dopipe(p)
int	*p;
{
	if (pipe(p) == -1)
		complain("[Pipe failed: %s]", strerror(errno));
}

void
pipeclose(p)
int	*p;
{
	(void) close(p[0]);
	(void) close(p[1]);
}
#endif	/* UNIX */

/* NOSTRICT */

UnivPtr
emalloc(size)
size_t	size;
{
	register UnivPtr	ptr;

	if ((ptr = malloc(size)) == NULL) {
		/* Try garbage collecting lines */
		GCchunks();
		if ((ptr = malloc(size)) == NULL) {
			/* Uh ... Oh screw it! */
			error("[Out of memory] ");
			/* NOTREACHED */
		}
	}
	return ptr;
}

UnivPtr
erealloc(ptr, size)
UnivPtr	ptr;
size_t	size;
{
	if ((ptr = realloc(ptr, size)) == NULL) {
		/* no second chance for realloc! */
		error("[out of memory]");
		/* NOTREACHED */
	}
	return ptr;
}

/* Return the basename of file F. */

char *
basename(f)
register char	*f;
{
	register char	*cp;

#ifdef	MSDOS
	if (f[0] != '\0'  && f[1] == ':')
		f += 2;
	if ((cp = strrchr(f, '\\')) != NULL)
		f = cp + 1;
#endif	/* MSDOS */
	if ((cp = strrchr(f, '/')) != NULL)
		f = cp + 1;
	return f;
}

void
push_env(savejmp)
jmp_buf	savejmp;
{
	byte_copy((UnivPtr) mainjmp, (UnivPtr) savejmp, sizeof (jmp_buf));
}

void
pop_env(savejmp)
jmp_buf	savejmp;
{
	byte_copy((UnivPtr) savejmp, (UnivPtr) mainjmp, sizeof (jmp_buf));
}

/* get the time buf, designated by *timep, from FROM to TO. */
char *
get_time(timep, buf, from, to)
time_t	*timep;
char	*buf;
int	from,
	to;
{
	time_t	now;
	char	*cp;

	if (timep != NULL)
		now = *timep;
	else
		(void) time(&now);
	cp = ctime(&now) + from;
#ifndef	MSDOS
	if (to == -1)
#else	/* MSDOS */
	if ((to == -1) && (cp[strlen(cp)-1] == '\n'))
#endif	/* MSDOS */
		cp[strlen(cp) - 1] = '\0';		/* Get rid of \n */
	else
		cp[to - from] = '\0';
	if (buf) {
		strcpy(buf, cp);
		return buf;
	} else {
		return cp;
	}
}

int
casecmp(s1, s2)
register const char	*s1,
		*s2;
{
	if (s1==NULL || s2==NULL)
		return 1;	/* which is not zero ... */
	while (CharUpcase(*s1) == CharUpcase(*s2++))
		if (*s1++ == '\0')
			return 0;
	return (*s1 - *--s2);
}

int
casencmp(s1, s2, n)
register const char	*s1,
		*s2;
register size_t	n;
{
	if (s1==NULL || s2==NULL)
		return 1;	/* which is not zero ... */
	for (;;) {
		if (n == 0)
			return 0;
		n--;
		if (CharUpcase(*s1) != CharUpcase(*s2++))
			return *s1 - *--s2;
		if (*s1++ == '\0')
			return 0;
	}
}

void
null_ncpy(to, from, n)
char	*to;
const char	*from;
size_t	n;
{
	(void) strncpy(to, from, n);
	to[n] = '\0';
}

bool
sindex(pattern, string)
register char	*pattern,
		*string;
{
	register size_t	len = strlen(pattern);

	while (*string != '\0') {
		if (*pattern == *string && strncmp(pattern, string, len) == 0)
			return YES;
		string += 1;
	}
	return NO;
}

#ifdef	ZORTECH
/* ZORTECH only accepts va_list in a prototype */
void
make_argv(register char *argv[], va_list ap)
#else
void
make_argv(argv, ap)
register char	*argv[];
va_list	ap;
#endif
{
	register int	i = 0;

	argv[i++] = va_arg(ap, char *);
	argv[i++] = basename(argv[0]);
	do ; while ((argv[i++] = va_arg(ap, char *)) != NULL);
}

int
pnt_line()
{
	register Line	*lp = curbuf->b_first;
	register int	i;

	for (i = 0; lp != NULL; i++, lp = lp->l_next)
		if (lp == curline)
			break;
	return i + 1;
}

/* Free, then allocate a block.
 * Like erealloc, except that the previous contents of the block are lost.
 */

UnivPtr
freealloc(obj, size)
register UnivPtr	obj;
size_t	size;
{
	register UnivPtr	new = NULL;

	if (obj)
		new = realloc(obj, size);
	if (new == NULL)
		new = emalloc(size);
	return new;
}

#ifdef NO_STRERROR
extern int sys_nerr;
extern char *sys_errlist[];

/*
 * Unix version of strerror - map error number to descriptive string.
 * ANSI systems should have this.
 */
char *
strerror(errnum)
int errnum;
{
	if (errnum > 0 && errnum < sys_nerr)
		return(sys_errlist[errnum]);
	return sprint("Error number %d", errnum);
}
#endif
