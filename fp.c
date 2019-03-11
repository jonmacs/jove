/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

#include "jove.h"
#include "fp.h"
#include "jctype.h"
#include "disp.h"
#include "fmt.h"

#ifdef MAC
#	include "mac.h"
#else /* !MAC */
#	include <sys/stat.h>
#	ifdef MSFILESYSTEM
#		include <io.h>
#	else /* !MSFILESYSTEM */
#		include <sys/file.h>
#	endif /* !MSFILESYSTEM */
#endif /* !MAC */

#include <errno.h>

#ifdef RAINBOW
private int rbwrite proto((int, char *, int));
#endif

#ifndef L_SET
# define L_SET 0
#endif

#define MAXFILES	20	/* good enough for my purposes */

private File	openfiles[MAXFILES];	/* must be zeroed initially */

File *
fd_open(name, flags, fd, buffer, buf_size)
const char	*name;
char	*buffer;
int	flags,
	fd,
	buf_size;
{
	register File	*fp;
	register int	i;

	for (fp = openfiles, i = 0; i < MAXFILES; i++, fp++)
		if (fp->f_flags == 0)
			break;

	if (i == MAXFILES)
		complain("[Too many open files!]");
	fp->f_bufsize = buf_size;
	fp->f_cnt = 0;
	fp->f_fd = fd;
	fp->f_flags = flags;
	if (buffer == NULL) {
		buffer = emalloc((size_t)buf_size);
		fp->f_flags |= F_MYBUF;
	}
	fp->f_base = fp->f_ptr = buffer;
	fp->f_name = copystr(name);

	return fp;
}

void
gc_openfiles()
{
	register File	*fp;

	for (fp = openfiles; fp < &openfiles[MAXFILES]; fp++)
		if (fp->f_flags != 0 && (fp->f_flags & F_LOCKED) == 0)
			f_close(fp);
}

File *
f_open(name, flags, buffer, buf_size)
const char	*name;
char	*buffer;
int	flags,
	buf_size;
{
	register int	fd;

	switch (F_MODE(flags)) {
	case F_READ:
		fd = open(name, O_RDONLY | O_BINARY);
		break;

	case F_APPEND:
		fd = open(name, O_WRONLY | O_BINARY);
		if (fd != -1) {
			(void) lseek(fd, (off_t)0, 2);
			break;
		}
		/* FALLTHROUGH */
	case F_WRITE:
#ifdef O_CREAT
		fd = open(name, O_CREAT | O_TRUNC | O_BINARY | O_RDWR,
# ifdef UNIX
			(jmode_t)CreatMode
# else
			S_IWRITE | S_IREAD
# endif
			);
#else
		fd = creat(name, (jmode_t)CreatMode);
#endif
		break;

	default:
		fd = -1;	/* avoid uninitialized complaint from gcc -W */
		error("invalid F_MODE");
		/* NOTREACHED */
	}
	if (fd == -1)
		return NULL;

	return fd_open(name, flags, fd, buffer, buf_size);
}

void
f_close(fp)
File	*fp;
{
	const char *what = "close";
	int	err = 0;

	if ((fp->f_flags & (F_WRITE|F_APPEND))
	&& (fp->f_flags & F_ERR) == 0)
	{
		flushout(fp);
#ifdef USE_FSYNC
		if (fsync(fp->f_fd) != 0) {
			what = "fsync";
			err = errno;
		}
#endif
	}
	if (close(fp->f_fd) != 0 && err == 0)
		err = errno;
	if (fp->f_flags & F_MYBUF)
		free((UnivPtr) fp->f_base);
	free((UnivPtr) fp->f_name);
	fp->f_flags = 0;	/* indicates that we're available */
	if (err != 0) {
		/* It would be nice to print fp->f_name, but it's gone.
		 * Perhaps we should allow the memory to leak so that we
		 * could easily print it.
		 */
		error("[%s error: %s]", what, strerror(err));
	}
}

ZXchar
f_filbuf(fp)
File	*fp;
{
	if (fp->f_flags & (F_EOF|F_ERR))
		return EOF;

	fp->f_ptr = fp->f_base;
#ifndef MSDOS
	do {
#endif /* MSDOS */
		fp->f_cnt = read(fp->f_fd, (UnivPtr) fp->f_base, (size_t) fp->f_bufsize);
#ifndef MSDOS
	} while (fp->f_cnt == -1 && errno == EINTR);
#endif /* MSDOS */
	if (fp->f_cnt == -1) {
		/* I/O error -- treat as EOF */
		writef("[Read error: %s]", strerror(errno));
		fp->f_flags |= F_ERR | F_EOF;
		return EOF;
	}
	if (fp->f_cnt == 0) {
		fp->f_flags |= F_EOF;
		return EOF;
	}
	io_chars += fp->f_cnt;
	return f_getc(fp);
}

void
putstr(s)
register const char	*s;
{
	register char	c;

	while ((c = *s++) != '\0')
		scr_putchar(c);
}

void
fputnchar(s, n, fp)
register char	*s;
register int	n;
register File	*fp;
{
	while (--n >= 0)
		f_putc(*s++, fp);
}

#ifndef NO_JSTDOUT
void
flushscreen()
{
	flushout(jstdout);
}
#endif /* !NO_JSTDOUT */

void
f_seek(fp, offset)
register File	*fp;
off_t	offset;
{
	if (fp->f_flags & (F_WRITE|F_APPEND))
		flushout(fp);
	fp->f_cnt = 0;		/* next read will f_filbuf(), next write
				   will flush() with no bad effects */
	lseek(fp->f_fd, offset, L_SET);
}

void
flushout(fp)
register File	*fp;
{
	if (fp->f_flags & (F_READ | F_STRING | F_ERR)) {
		if (fp->f_flags != F_STRING)
			abort();	/* IMPOSSIBLE */
		/* We just banged into the end of a string.
		 * In the interests of continuing, we will cause
		 * the rest of the output to be be heaped in the
		 * last position.  Surely it will end up as a NUL. UGH!
		 */
		fp->f_cnt = 1;
		fp->f_ptr = &fp->f_base[fp->f_bufsize - 1];
	} else {
		char	*p = fp->f_base;

		for (;;) {
			SSIZE_T
				n = fp->f_ptr - p,
				wr;

			if (n <= 0)
				break;

#ifdef RAINBOW
			wr = rbwrite(fp->f_fd, (UnivPtr) p, (size_t)n);
#else
			wr = write(fp->f_fd, (UnivPtr) p, (size_t)n);
#endif
			if (wr >= 0) {
				p += wr;
			} else {
#ifndef MSDOS
				if (errno != EINTR) {
#endif /* MSDOS */
#ifndef NO_JSTDOUT
					if (fp == jstdout)
						break;	/* bail out, silently */
#endif
					fp->f_flags |= F_ERR;
					error("[I/O error(%s); file = %s, fd = %d]",
						strerror(errno), fp->f_name, fp->f_fd);
#ifndef MSDOS
				}
#endif /* MSDOS */
			}
		}

		fp->f_cnt = fp->f_bufsize;
		fp->f_ptr = fp->f_base;
	}
}

bool
f_gets(fp, buf, max)
register File	*fp;
char	*buf;
size_t	max;
{
	register char	*cp = buf;
	register ZXchar	c;
	char	*endp = buf + max - 1;

	if (fp->f_flags & F_EOF)
		return YES;

	while ((c = f_getc(fp)) != EOF && c != EOL) {
		/* We can't store NUL in our buffer, so ignore it.
		 * Similarly, we can only store characters less than NCHARS.
		 * Of course, with a little ingenuity we could store NUL:
		 * NUL could be represented by EOL.
		 */
		if (c == '\0'
#if NCHARS != UCHAR_ROOF
		|| c >= NCHARS
#endif
		)
			continue;

		if (cp >= endp) {
			add_mess(" [Line too long]");
			rbell();
			return YES;
		}
		*cp++ = c;
	}
	*cp = '\0';
	if (c == EOF) {
		if (cp != buf)
			add_mess(" [Incomplete last line]");
		return YES;
	}
#ifdef USE_CRLF
	/* a CR followed by a LF is treated as a NL.
	 * Bug: the line-buffer is effectively shortened by one character.
	 */
	if (cp != buf && cp[-1] == '\r')
		*--cp = '\0';
#endif /* USE_CRLF */
	io_lines += 1;
	return NO;	/* this means okay */
}

/* skip to beginning of next line, i.e., next read returns first
 * character of new line
 */
void
f_toNL(fp)
register File	*fp;
{
	if (fp->f_flags & F_EOF)
		return;

	for (;;) {
		switch (f_getc(fp)) {
		case EOF:
			fp->f_flags |= F_EOF;
			/*FALLTHROUGH*/
		case EOL:
			return;
		}
	}
}

#ifdef PIPEPROCS
size_t
f_readn(fp, addr, n)
register File	*fp;
register char	*addr;
size_t	n;
{
	register size_t	nleft;

	for (nleft = n; nleft > 0; nleft--) {
		ZXchar	c = f_getc(fp);

		if (f_eof(fp))
			break;

		*addr++ = c;
	}
	return n - nleft;
}
#endif /* PIPEPROCS */

/* ScrBufSize is the size of the buffer for jstdout.  It is also the
 * number of characters to be output between checks for input, so
 * it is meaningful even if jstdout isn't used.  Its value is set by
 * settout based on the baud rate of output (on systems with baud rates).
 */
#ifdef NO_JSTDOUT
int	ScrBufSize = 256;
#else
int	ScrBufSize = 1;	/* until settout decides a better value & allocates a buf */
#endif

#ifndef NO_JSTDOUT
private char	one_buf;

private File	stdout_File = {1, 1, 1, F_WRITE, &one_buf, &one_buf, (char *)NULL};
File	*jstdout = &stdout_File;
#endif

#ifdef RAINBOW

/*
 * use the Rainbow's video output function
 */

#include <dos.h>

private int
rbwrite(fd, buf, cnt)
int fd;
char *buf;
int cnt;
{
	union REGS vr;

	if (fd != 1) {
		write(fd, buf, cnt);
	} else {
		while (cnt-- > 0) {
			vr.x.ax = *buf++;
			vr.x.di = 0;
			int86(0x18, &vr, &vr);
		}
	}
}
#endif /* RAINBOW */
