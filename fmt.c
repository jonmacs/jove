/************************************************************************
 * This program is Copyright (C) 1986 by Jonathan Payne.  JOVE is       *
 * provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is *
 * included in all the files.                                           *
 ************************************************************************/

#include "jove.h"
#include "io.h"
#include "termcap.h"

#include <varargs.h>

char	mesgbuf[MESG_SIZE];

/* VARARGS2 */

format(buf, len, fmt, ap)
char	*buf,
	*fmt;
va_list	ap;
{
	File	strbuf,
		*sp = &strbuf;

 	sp->f_ptr = sp->f_base = buf;
	sp->f_fd = -1;		/* Not legit for files */
	sp->f_cnt = len;
	sp->f_flags = F_STRING;
	sp->f_bufsize = len;

	doformat(sp, fmt, ap);
	putc('\0', sp);
}

private
PPchar(c, str)
int	c;
char	*str;
{
	char	*cp = str;

	if (c == '\033')
		strcpy(cp, "ESC");
	else if (c < ' ')
		sprintf(cp, "C-%c", c + '@');
	else if (c == '\177')
		strcpy(cp, "^?");
	else
		sprintf(cp, "%c", c);
}

private struct fmt_state {
	int	precision,
		width,
		leftadj;
	char	padc;
	File	*iop;
} current_fmt;

private
putld(d, base)
long	d;
{
	int	length = 1;
	long	tmpd = d;

	if (current_fmt.width == 0 && current_fmt.precision) {
		current_fmt.width = current_fmt.precision;
		current_fmt.padc = '0';
	}
	while (tmpd = (tmpd / base))
		length++;
	if (d < 0)
		length++;
	if (!current_fmt.leftadj)
		pad(current_fmt.padc, current_fmt.width - length);
	if (d < 0) {
		putc('-', current_fmt.iop);
		d = -d;
	}
	outld(d, base);
	if (current_fmt.leftadj)
		pad(current_fmt.padc, current_fmt.width - length);
}

private
outld(d, base)
long	d;
{
	register long	n;

	if (n = (d / base))
		outld(n, base);
	putc((int) ('0' + (int) (d % base)), current_fmt.iop);
}

private
puts(str)
char	*str;
{
	int	length;
	register char	*cp;

	if (str == 0)
#if pyr
		str = "";
#else
		str = "(null)";
#endif
	length = strlen(str);
	if (current_fmt.precision == 0 || length < current_fmt.precision)
		current_fmt.precision = length;
	else
		length = current_fmt.precision;
	cp = str;
	if (!current_fmt.leftadj)
		pad(' ', current_fmt.width - length);
	while (--current_fmt.precision >= 0)
		putc(*cp++, current_fmt.iop);
	if (current_fmt.leftadj)
		pad(' ', current_fmt.width - length);
}

private
pad(c, amount)
register int	c,
		amount;
{
	while (--amount >= 0)
		putc(c, current_fmt.iop);
}

private
doformat(sp, fmt, ap)
register File	*sp;
register char	*fmt;
va_list	ap;
{
	register char	c;
	struct fmt_state	prev_fmt;

	prev_fmt = current_fmt;
	current_fmt.iop = sp;

	while (c = *fmt++) {
		if (c != '%') {
			putc(c, current_fmt.iop);
			continue;
		}

		current_fmt.padc = ' ';
		current_fmt.precision = current_fmt.leftadj = current_fmt.width = 0;
		c = *fmt++;
		if (c == '-') {
			current_fmt.leftadj = YES;
			c = *fmt++;
		}
		if (c == '0') {
			current_fmt.padc = '0';
			c = *fmt++;
		}
		while (c >= '0' && c <= '9') {
			current_fmt.width = current_fmt.width * 10 + (c - '0');
			c = *fmt++;
		}
		if (c == '*') {
			current_fmt.width = va_arg(ap, int);
			c = *fmt++;
		}
		if (c == '.') {
			c = *fmt++;
			while (c >= '0' && c <= '9') {
				current_fmt.precision = current_fmt.precision * 10 + (c - '0');
				c = *fmt++;
			}
			if (c == '*') {
				current_fmt.precision = va_arg(ap, int);
				c = *fmt++;
			}
		}
	reswitch:
		/* At this point, fmt points at one past the format letter. */
		switch (c) {
		case '%':
			putc('%', current_fmt.iop);
			break;
	
		case 'D':
			putld(va_arg(ap, long), 10);
			break;
	
		case 'b':
		    {
			Buffer	*b = va_arg(ap, Buffer *);

			puts(b->b_name);
			break;
		    }

		case 'c':
			putc(va_arg(ap, int), current_fmt.iop);
			break;
	
		case 'd':
			putld((long) va_arg(ap, int), 10);
			break;
	
		case 'f':	/* current command name gets inserted here! */
			puts(LastCmd->Name);
			break;

		case 'l':
			c = CharUpcase(*++fmt);
			goto reswitch;
	
		case 'n':
			if (va_arg(ap, int) != 1)
				puts("s");
			break;

		case 'o':
			putld((long) va_arg(ap, int), 8);
			break;
	
		case 'p':
		    {
		    	char	cbuf[20];

		    	PPchar(va_arg(ap, int), cbuf);
		    	puts(cbuf);
		    	break;
		    }

		case 's':
			puts(va_arg(ap, char *));
			break;
		
		default:
			complain("Unknown format directive: \"%%%c\"", c);
		}
	}
	current_fmt = prev_fmt;
}

/* VARARGS1 */

char *
sprint(fmt, va_alist)
char	*fmt;
va_dcl
{
	va_list	ap;
	static char	line[100];

	va_start(ap);
	format(line, sizeof line, fmt, ap);
	va_end(ap);
	return line;
}

/* VARARGS1 */

printf(fmt, va_alist)
char	*fmt;
va_dcl
{
	va_list	ap;

	va_start(ap);
	doformat(stdout, fmt, ap);
	va_end(ap);
}

/* VARARGS1 */

fprintf(fp, fmt, va_alist)
File	*fp;
char	*fmt;
va_dcl
{
	va_list	ap;

	va_start(ap);
	doformat(fp, fmt, ap);
	va_end(ap);
}

/* VARARGS2 */

sprintf(str, fmt, va_alist)
char	*str,
	*fmt;
va_dcl
{
	va_list	ap;

	va_start(ap);
	format(str, 130, fmt, ap);
	va_end(ap);
}

/* VARARGS1 */

s_mess(fmt, va_alist)
char	*fmt;
va_dcl
{
	va_list	ap;

	if (InJoverc)
		return;
	va_start(ap);
	format(mesgbuf, sizeof mesgbuf, fmt, ap);
	va_end(ap);
	message(mesgbuf);
}

/* VARARGS1 */

f_mess(fmt, va_alist)
char	*fmt;
va_dcl
{
	va_list	ap;

	va_start(ap);
	format(mesgbuf, sizeof mesgbuf, fmt, ap);
	va_end(ap);
	DrawMesg(NO);
	UpdMesg++;	/* Still needs updating (for convenience) */
}

/* VARARGS1 */

add_mess(fmt, va_alist)
char	*fmt;
va_dcl
{
	int	mesg_len = strlen(mesgbuf);
	va_list	ap;

	if (InJoverc)
		return;
	va_start(ap);
	format(&mesgbuf[mesg_len], (sizeof mesgbuf) - mesg_len, fmt, ap);
	va_end(ap);
	message(mesgbuf);
}
