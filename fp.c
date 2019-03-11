/************************************************************************
 * This program is Copyright (C) 1986 by Jonathan Payne.  JOVE is       *
 * provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is *
 * included in all the files.                                           *
 ************************************************************************/

#include "jove.h"
#include "io.h"
#include "ctype.h"
#include "termcap.h"
#include <sys/stat.h>
#ifndef MSDOS
#include <sys/file.h>
#else /* MSDOS */
#include <fcntl.h>
#endif /* MSDOS */
#include <errno.h>

#ifndef L_SET
#	define L_SET 0
#endif

#define MAXFILES	20	/* good enough for my purposes */

private File	_openfiles[MAXFILES] = {0};

private File *
f_alloc(name, flags, fd, buffer, buf_size)
char	*name,
	*buffer;
{
	register File	*fp;
	register int	i;

	for (fp = _openfiles, i = 0; i < MAXFILES; i++, fp++)
		if (fp->f_flags == 0)
			break;
	if (i == MAXFILES)
		complain("[Too many open files!]");
	fp->f_bufsize = buf_size;
	fp->f_cnt = 0;
	fp->f_fd = fd;
	fp->f_flags = flags;
	if (buffer == 0) {
		buffer = emalloc(buf_size);
		fp->f_flags |= F_MYBUF;
	}
	fp->f_base = fp->f_ptr = buffer;
	fp->f_name = copystr(name);

	return fp;
}

gc_openfiles()
{
	register File	*fp;

	for (fp = _openfiles; fp < &_openfiles[MAXFILES]; fp++)
		if (fp->f_flags != 0 && (fp->f_flags & F_LOCKED) == 0)
			f_close(fp);
}

File *
fd_open(name, flags, fd, buffer, bsize)
char	*name,
	*buffer;
{
	return f_alloc(name, flags, fd, buffer, bsize);
}

File *
f_open(name, flags, buffer, buf_size)
char	*name,
	*buffer;
{
	register int	fd;
	int	mode = F_MODE(flags);

	if (mode == F_READ)
		fd = open(name, 0);
	if (mode == F_APPEND) {
		fd = open(name, 1);
		if (fd == -1)
			mode = F_WRITE;
		else
			(void) lseek(fd, 0L, 2);
	}
	if (mode == F_WRITE)
		fd = creat(name, CreatMode);
	if (fd == -1)
		return NIL;
#ifdef MSDOS
	else
		setmode(fd, 0x8000);
#endif /* MSDOS */
	return f_alloc(name, flags, fd, buffer, buf_size);
}

f_close(fp)
File	*fp;
{
	flush(fp);
#ifdef BSD4_2 
	if (fp->f_flags & (F_WRITE|F_APPEND))
		(void) fsync(fp->f_fd);
#endif 
	(void) close(fp->f_fd);
	if (fp->f_flags & F_MYBUF)
		free(fp->f_base);
	free(fp->f_name);
	fp->f_flags = 0;	/* indicates that we're available */
}

filbuf(fp)
File	*fp;
{
	if (fp->f_flags & (F_EOF|F_ERR))
		return EOF;
	fp->f_ptr = fp->f_base;
#ifndef MSDOS
	do
#endif /* MSDOS */
		fp->f_cnt = read(fp->f_fd, fp->f_base, fp->f_bufsize);
#ifndef MSDOS
	while (fp->f_cnt == -1 && errno == EINTR);
#endif /* MSDOS */
	if (fp->f_cnt == -1) {
		printf("[Read error %d]", errno);
		fp->f_flags |= F_ERR;
	}
	if (fp->f_cnt == 0) {
		fp->f_flags |= F_EOF;
		return EOF;
	}
	io_chars += fp->f_cnt;
	return getc(fp);
}

putstr(s)
register char	*s;
{
#ifndef IBMPC
	register int	c;

	while (c = *s++)
		putchar(c);
#else /* IBMPC */
	write_emif(s);
#endif /* IBMPC */
}

fputnchar(s, n, fp)
register char	*s;
register int	n;
register File	*fp;
{
	while (--n >= 0)
		putc(*s++, fp);
}

flusho()
{
#ifndef IBMPC
	_flush(EOF, stdout);
#endif /* IBMPC */
}

flush(fp)
File	*fp;
{
	_flush(EOF, fp);
}

f_seek(fp, offset)
register File	*fp;
off_t	offset;
{
	if (fp->f_flags & F_WRITE)
		flush(fp);
	fp->f_cnt = 0;		/* next read will filbuf(), next write
				   will flush() with no bad effects */
	lseek(fp->f_fd, (long) offset, L_SET);
}

_flush(c, fp)
register File	*fp;
{
	register int	n;

	if (fp->f_flags & (F_READ | F_STRING | F_ERR))
		return;
	if (((n = (fp->f_ptr - fp->f_base)) > 0) &&
#ifndef IBMPC
	    (write(fp->f_fd, fp->f_base, n) != n) &&
	    (fp != stdout)) {
#else /* IBMPC */
	    (write(fp->f_fd, fp->f_base, n) != n)) {
#endif /* IBMPC */
	    	fp->f_flags |= F_ERR;
		error("[I/O error(%d); file = %s, fd = %d]",
			errno, fp->f_name, fp->f_fd);
	}

#ifndef IBMPC
	if (fp == stdout)
		OkayAbort = YES;
#endif /* IBMPC */
	fp->f_cnt = fp->f_bufsize;
	fp->f_ptr = fp->f_base;
	if (c != EOF)
		putc(c, fp);
}

f_gets(fp, buf, max)
register File	*fp;
char	*buf;
{
	register char	*cp = buf;
	register int	c;
	char	*endp = buf + max - 1;

	if (fp->f_flags & F_EOF)
		return EOF;
	while (((c = getc(fp)) != EOF) && (c != '\n')) {
		if (c == NULL)
			break;		/* sorry we don't read nulls */
#ifdef MSDOS
		if (c == '\r') {
			if ((c = getc(fp)) == '\n')
			   break;
			else
			   *cp++ = '\r';
		}
#endif /* MSDOS */
		if (cp >= endp) {
			add_mess(" [Line too long]");
			rbell();
			return EOF;
		}
		*cp++ = c;
	}
	*cp = '\0';
	if (c == EOF) {
		if (cp != buf)
			add_mess(" [Incomplete last line]");
		fp->f_flags |= F_EOF;
		return EOF;
	}
	io_lines += 1;
	return NIL;	/* this means okay */
}

/* skip to beginning of next line, i.e., next read returns first
   character of new line */
f_toNL(fp)
register File	*fp;
{
	register int	c;

	if (fp->f_flags & F_EOF)
		return;
	while (((c = getc(fp)) != EOF) && (c != '\n'))
		;
	if (c == EOF)
		fp->f_flags |= F_EOF;
}

f_readn(fp, addr, n)
register File	*fp;
register char	*addr;
register int	n;
{
	while (--n >= 0)
		*addr++ = getc(fp);
}

f_getint(fp)
File	*fp;
{
	int	n = 0,
		c;

	while (isdigit(c = getc(fp)))
		n = (n * 10) + c;
	return n;
}

/* Deals with output to the terminal, setting up the amount of characters
   to be buffered depending on the output baud rate.  Why it's in a 
   separate file I don't know ... */

private char	one_buf;

int	BufSize = 1;

private File	_stdout = {1, 1, 1, F_WRITE, &one_buf, &one_buf};
File	*stdout = &_stdout;

/* put a string with padding */

#ifndef IBMPC
tputc(c)
{
	putchar(c);
}

#undef putchar		/* for files which forget to include io.h,
			   here's a real putchar procedure. */
putchar(c)
{
	putc(c, stdout);
}

#endif /* IBMPC */
putpad(str, lines)
char	*str;
{
#ifndef IBMPC
	if (str)
		tputs(str, lines, tputc);
#else /* IBMPC */
	write_emif(str);
#endif /* IBMPC */
}

/* Determine the number of characters to buffer at each baud rate.  The
   lower the number, the quicker the response when new input arrives.  Of
   course the lower the number, the more prone the program is to stop in
   output.  Decide what matters most to you. This sets BufSize to the right
   number or chars, and initiaizes `stdout'.  */

settout(ttbuf)
char	*ttbuf;
{
#ifndef MSDOS
	static int speeds[] = {
		1,	/* 0	*/
		1,	/* 50	*/
		1,	/* 75	*/
		1,	/* 110	*/
		1,	/* 134	*/
		1,	/* 150	*/
		1,	/* 200	*/
		2,	/* 300	*/
		4,	/* 600	*/
		8,	/* 1200 */
		16,	/* 1800	*/
		32,	/* 2400	*/
		128,	/* 4800	*/
		256,	/* 9600	*/
		512,	/* EXTA	*/
		1024	/* EXT	*/
	};
	BufSize = min(MAXTTYBUF, speeds[ospeed] * max(LI / 24, 1));
	stdout = fd_open("/dev/tty", F_WRITE|F_LOCKED, 1, ttbuf, BufSize);
#else /* MSDOS */
	BufSize = TTBUFSIZ; 
	stdout = fd_open("con", F_WRITE|F_LOCKED, 1, ttbuf, BufSize);
#endif /* MSDOS */
}

