/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

#include "jove.h"
#include <errno.h>
#include "jctype.h"
#include "disp.h"
#include "fp.h"
#include "ask.h"
#include "chars.h"
#include "fmt.h"
#include "insert.h"
#include "macros.h"
#include "marks.h"
#include "move.h"
#include "rec.h"

#ifdef MAC
# include "mac.h"
#endif

bool
blnkp(buf)
register char	*buf;
{
	register char	c;

	do {
		c = *buf++;
	} while (jiswhite(c));
	return c == '\0';	/* It's NUL if we got to the end of the Line */
}

bool
within_indent()
{
	register int	i;

	i = curchar;
	for (;;) {
		if (--i < 0)
			return YES;

		if (!jiswhite(linebuf[i]))
			return NO;
	}
}

void
DotTo(line, col)
LinePtr	line;
int	col;
{
	Bufpos	bp;

	bp.p_line = line;
	bp.p_char = col;
	SetDot(&bp);
}

/* If bp->p_line is != current line, then save current line.  Then set dot
 * to bp->p_line, and if they weren't equal get that line into linebuf.
 */
void
SetDot(bp)
register Bufpos	*bp;
{
	register bool	notequal;

	if (bp == NULL)
		return;

	notequal = bp->p_line != curline;
	if (notequal)
		lsave();
	if (bp->p_line != NULL)
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

int	MarkThresh = 22;	/* VAR: moves greater than MarkThresh will SetMark (avg screen size) */

private int	line_diff;

int
inorder(nextp, char1, endp, char2)
register LinePtr	nextp,
		endp;
int	char1,
	char2;
{
	register LinePtr	prevp = nextp;

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
	while (nextp!=NULL) {
		nextp = nextp->l_next;
		line_diff += 1;
		if (nextp == endp)
			return YES;
	}
	while (prevp!=NULL) {
		prevp = prevp->l_prev;
		line_diff += 1;
		if (prevp == endp)
			return NO;
	}
	/* the lines are not ordered */
	return -1;
}

/* Number of lines (forward OR back) from nextp to endp.
 * Note: if they are not related, returns 0.
 */
int
LineDist(nextp, endp)
LinePtr	nextp,
		endp;
{
	return inorder(nextp, 0, endp, 0) == -1? 0 : line_diff;
}

/* Number of lines forward from "from" to "to"; -1 if not found.
 * Note: if "to" is (LinePtr)NULL, returns number of lines to end + 1
 */
int
LinesTo(from, to)
register LinePtr
	from,
	to;
{
	int	n = 0;

	for (;;) {
		if (from == to)
			return n;

		if (from == NULL)
			return -1;

		n += 1;
		from = from->l_next;
	}
}

void
PushPntp(line)
register LinePtr	line;
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
LinePtr	line;
{
	return strlen(lcontents(line));
}

void
to_word(dir)
register int	dir;
{
	if (dir == FORWARD) {
		for(;;) {
			register ZXchar	c = ZXC(linebuf[curchar]);

			if (c == '\0') {
				if (curline->l_next == NULL)
					break;	/* failure: out of buffer */

				SetLine(curline->l_next);
			} else if (jisword(c)) {
				break;	/* success */
			} else {
				curchar += 1;
			}
		}
	} else {
		for (;;) {
			if (bolp()) {
				if (curline->l_prev == NULL)
					break;	/* failure: out of buffer */

				SetLine(curline->l_prev);
				Eol();
			} else if (jisword(linebuf[curchar - 1])) {
				break;	/* success */
			} else {
				curchar -= 1;
			}
		}
	}
}

/* Are there any modified buffers?  Allp means include B_PROCESS
 * buffers in the check.
 */
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

const char *
filename(b)
register const Buffer	*b;
{
	return b->b_fname ? pr_name(b->b_fname, YES) : "[No file]";
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
register LinePtr	line;
{
	return line == curline? linebuf : lbptr(line);
}

char *
ltobuf(line, buf)
LinePtr	line;
char	*buf;
{
	if (line == curline) {
		if (buf != linebuf)
			strcpy(buf, linebuf);
		Jr_Len = strlen(linebuf);
	} else {
		jgetline(line->l_dline, buf);
	}
	return buf;
}

void
DOTsave(buf)
Bufpos *buf;
{
	buf->p_line = curline;
	buf->p_char = curchar;
}

/* Return YES iff we had to rearrange the order. */

bool
fixorder(line1, char1, line2, char2)
register LinePtr	*line1,
		*line2;
register int	*char1,
		*char2;
{
	LinePtr	tline;
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
LinePtr	first,
		what;
{
	return LinesTo(first, what) != -1;
}

/* Make curbuf (un)modified and tell the redisplay code to update the modeline
 * if it will need to be changed.
 */
void
modify()
{
	if (!curbuf->b_modified) {
		UpdModLine = YES;
		curbuf->b_modified = YES;
	}
	DOLsave = YES;
#ifdef RECOVER
	if (curbuf->b_type != B_SCRATCH)
		ModCount += 1;
#endif
}

void
unmodify()
{
	if (curbuf->b_modified) {
		UpdModLine = YES;
		curbuf->b_modified = NO;
	}
}

/* Set or clear the divergence flag for `buf'.
 * A buffer that contains a file is considered to have diverged
 * if the file in the filesystem appears to have changed since the
 * last time the buffer was loaded from or saved to that file.
 * If the flag has changed, tell the redisplay code to update the
 * modeline.
 */
void
diverge(buf, d)
Buffer	*buf;
bool	d;
{
	if (buf->b_diverged != d) {
		UpdModLine = YES;
		buf->b_diverged = d;
	}
}

int
numcomp(s1, s2)
register const char	*s1,
		*s2;
{
	register int	count = 0;

	while (*s1 != '\0' && *s1++ == *s2++)
		count += 1;
	return count;
}

#ifdef FILENAME_CASEINSENSITIVE
int
numcompcase(s1, s2)
register const char	*s1,
		*s2;
{
	register int	count = 0;

	while (*s1 != '\0' && CharDowncase(*s1++) == CharDowncase(*s2++))
		count += 1;
	return count;
}
#endif

char *
copystr(str)
const char	*str;
{
	return str == NULL? NULL :
		strcpy(emalloc(strlen(str) + 1), str);
}

#ifndef byte_copy
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
	static const char	mesg[] = "[line too long]";

	if (flag == JMP_COMPLAIN)
		complain(mesg);
	else
		error(mesg);
}

/* Insert num copies of character c at offset atchar in buffer buf of size max */

void
ins_c(c, buf, atchar, num, max)
char	c;
char	*buf;
int	atchar,
	num,
	max;
{
	/* hint to reader: all copying and filling is done right to left */
	register char	*from, *to;
	int taillen;

	if (num <= 0)
		return;

	from = &buf[atchar];
	taillen = *from == '\0'?  1 : strlen(from) + 1;	/* include NUL */
	if (atchar + taillen + num > max)
		len_error(JMP_COMPLAIN);
	from += taillen;
	to = from + num;
	do {
		*--to = *--from;
	} while (--taillen != 0);
	while (to != from)
		*--to = c;
}

bool
TwoBlank()
{
	register LinePtr	next = curline->l_next;

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
			len_error(JMP_ERROR);
	} while ((*onto++ = *from++) != '\0');
}

char *
IOerr(err, file)
const char	*err, *file;
{
	return sprint("Couldn't %s \"%s\".", err, file);
}

#ifdef UNIX
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
#endif /* UNIX */

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
			error("[Out of memory]");
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
	if (ptr == NULL) {
		ptr = emalloc(size);
	} else if ((ptr = realloc(ptr, size)) == NULL) {
		/* no second chance for realloc! */
		error("[out of memory]");
		/* NOTREACHED */
	}
	return ptr;
}

/* Return the basename of pathname F.
 *
 * - System V release 4 includes a function named "basename" in libgen.
 *   It is incompatible with ours:
 *   + it strips trailing "/" characters (does this matter?)
 *   + although not clearly documented, this stripping modifies the argument!
 *   + it handles the NULL pointer and the null string as "."
 *
 * - LINUX also provides a basename
 *   + at least one version of Slackware puts the prototype in <unistd.h>
 *     so it cannot be ignored.
 *   + This LINUX prototype declares the parameter to be of const char *
 *     type.  This is incompatible with ours and with SVR4's.
 *   + The fact that the argument is a pointer to const implies that
 *     the source string cannot be modified.  Therefore trailing "/"
 *     characters are not stripped from the source.
 *   + Either stripping isn't done to the result OR the result must be
 *     placed in a distinct chunk of memory.  How is this memory managed?
 *
 * To avoid conflict, we have renamed ours to "jbasename".
 */

const char *
jbasename(f)
register const char	*f;
{
	register char	*cp;

#ifdef MSFILESYSTEM
	if (f[0] != '\0'  && f[1] == ':')
		f += 2;
	if ((cp = strrchr(f, '\\')) != NULL)
		f = cp + 1;
#endif /* MSFILESYSTEM */
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
	if (to == -1)
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

/* Are s1 and s2 equal, at least for the first n chars, ignoring case? */

bool
caseeqn(s1, s2, n)
register const char	*s1,
		*s2;
register size_t	n;
{
	if (s1==NULL || s2==NULL)
		return NO;

	for (;;) {
		if (n == 0)
			return YES;
		n--;
		if (!cind_eq(*s1, *s2++))
			return NO;

		if (*s1++ == '\0')
			return YES;
	}
}

/* copy a string into buffer; truncate silently if too large; NUL-pad.
 * Note: buffer must be 1 larger than n to fit NUL!
 * Duplicated in recover.c: needed by scandir.c
 */
void
null_ncpy(to, from, n)
char	*to;
const char	*from;
size_t	n;
{
	(void) strncpy(to, from, n);
	to[n] = '\0';
}

/* Copy a string into a buffer; truncate silently if string is too large */
void
truncstrsub(buf, str, bufsz)
char *buf;
size_t bufsz;
const char *str;
{
	if (strlen(str) < bufsz)
		strcpy(buf, str);
	else if (bufsz == 0)
		complain("internal error");	/* cannot even fit NUL */
	else {
		strncpy(buf, str, bufsz - 1);
		buf[bufsz-1] = '\0';
	}
}

/* Copy a string into a buffer; complain if string is too large */
void
jamstrsub(buf, str, bufsz)
char *buf;
const char *str;
size_t bufsz;
{
	if (strlen(str) < bufsz)
		strcpy(buf, str);
	else
		complain("string too long");
}

bool
sindex(pattern, string)
register const char	*pattern,
		*string;
{
	register size_t	len = strlen(pattern);

	if (len == 0)
		return YES;

	while (*string != '\0') {
		if (*pattern == *string && strncmp(pattern, string, len) == 0)
			return YES;

		string += 1;
	}
	return NO;
}

/* Free, then allocate a block.
 * Like erealloc, except that the previous contents of the block are lost.
 */

UnivPtr
freealloc(obj, size)
register UnivPtr	obj;
size_t	size;
{
	if (obj != NULL)
		obj = realloc(obj, size);
	if (obj == NULL)
		obj = emalloc(size);
	return obj;
}

/* order file names (parameter for qsort) */
int
fnamecomp(a, b)
UnivConstPtr	a,
	b;
{
	return strcmp(*(const char **)a, *(const char **)b);
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
	return 0 < errnum && errnum < sys_nerr
		? sys_errlist[errnum] : sprint("Error number %d", errnum);
}
#endif /* NO_STRERROR */

/* decode a pair of characters representing \x or ^x */

ZXchar
DecodePair(first, second)
ZXchar	first, second;
{
	if (second == EOF || second == '\n')
		complain("unexpected end of file after %p", first);
	if (first == '^') {
		if (second == '?') {
			second = DEL;
		} else {
			ZXchar	us = CharUpcase(second);

			if (us < '@' || '_' < us)
				complain("unknown control character %p", second);
			second = CTL(us);
		}
	}
	return second;
}

#if defined(IPROCS)
const char **
jenvinit(envp)
Env *envp;
{
	if (envp->e_data == NULL) {
	    envp->e_data = (const char **) environ; /* avoid gcc warning */
	    envp->e_malloced = NO;
	    envp->e_headroom = 0;
	}
	return envp->e_data;
}
    
/* Put a definition into the environment.
 * Same as putenv(3) in SVID 3, POSIX, and BSD 4.3.
 */
void
jputenv(envp, def)
Env *envp;
const char *def;
{
	static bool env_malloced = NO;	/* should we free it when replacing? */
	const char **p, **e;
	const char *eq;

	if ((eq = strchr(def, '=')) == NULL)
		return;
	for (p = e = jenvinit(envp); ; p++) {
		if (*p == NULL) {
			if (envp->e_headroom == 0) {
#				define JENV_INCR	5
				size_t	sz = ((p-e) + 1) * sizeof(char *);
				const char	**ne = (const char **)
					malloc(sz + JENV_INCR*sizeof(char *));

				if (ne == NULL)
					break;	/* malloc failed: give up -- doesn't matter much */

				byte_copy(envp->e_data, ne, sz);
				p = ne + (p-e);
				if (envp->e_malloced)
					free((UnivPtr)envp->e_data);
				envp->e_data = e = ne;
				envp->e_malloced = YES;
				envp->e_headroom = JENV_INCR;
#				undef JENV_INCR
			}
			envp->e_headroom -= 1;
			*p++ = copystr(def);
			*p = NULL;
			break;
		}
		if (strncmp(*p, def, (size_t) (eq - def + 1)) == 0) {
			*p = copystr(def);
			break;
		}
	}
}

/* Remove any definitions of name from the environment.
 * Same as 4.3BSD's unsetenv(3).
 */
void
junsetenv(envp, name)
Env *envp;
const char *name;
{
	const char **p, **q;
	size_t l = strlen(name);

	for (p = q = jenvinit(envp);;) {
		const char *e = *p++;

		*q = e;

		if (e == NULL)
			break;

		if (strncmp(e, name, l) == 0 && e[l] == '=') {
			/* unset this one by not advancing q */
			envp->e_headroom += 1;
		} else {
			q += 1;
		}
	}
}
#endif /* defined(IPROCS) */
