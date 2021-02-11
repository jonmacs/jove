/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

/* Recovers JOVE files after a system/editor crash.
 * Usage: recover [-d directory] [-syscrash] [-uid UID][-v] [-D]
 * The -syscrash option is specified in /etc/rc.  It directs recover to
 * move all the jove tmp files from tmp_dir (/tmp) to RECDIR (/usr/preserve).
 * recover -syscrash must be invoked in /etc/rc BEFORE /tmp gets cleared out.
 * (about the same place as expreserve gets invoked to save ed/vi/ex files.
 *
 * The -d option lets you specify the directory to search for tmp files when
 * the default isn't the right one.
 *
 * The -uid option lets you specify the user id if you wish to recover
 * files for another user (presumably as root, since otherwise you should
 * not be able to read the recovery/tmp files)
 *
 * The -v option turns on some verbose chatter to the screen, and -D
 * turns on even more verbose debugging to a file.
 *
 * Change JRECDIR and JTMPDIR in Makefile to change the default directories.
 */

#define USE_STDIO_H 1
#include "jove.h"

#define SMALLSTRSIZE	30	/* used for small buffers */
#define MAXRECDIRSIZE	64	/* enough for RECDIR */
#define MAXFILENAMESIZE	16	/* /jvNNNNNN and trailing NUL */
#define PATHBUFSIZE	(MAXRECDIRSIZE+MAXFILENAMESIZE)

#ifndef RECOVER

int
main(argc, argv)
int	argc;
char	*argv[];
{
	printf("%s: recovery not implemented in this JOVE configuration.\n",
	       argv[0]);
	return 1;
}

#else /* RECOVER */	/* the body is the rest of this file */

#include "sysprocs.h"
#include "rec.h"
#include "paths.h"

#include "recover.h"
#include <sys/stat.h>

#ifndef UNIX
# define signal(x, y)	-1
# define kill(x, y)	-1
# ifdef WIN32
#  define uid_t int
# endif
#else /*UNIX */
# include <signal.h>
# include <sys/file.h>
# include <pwd.h>
# include <time.h>

# ifdef USE_UNAME
#  include <sys/utsname.h>
# endif

/* Strictly speaking, popen is not available in stdio.h in POSIX.1 or ANSI-C.
 * It is part of POSIX.2, and declared incorrectly in OSF/1, so we suppress
 * it for OSF.
 */
#ifndef	_OSF_SOURCE
extern FILE	*popen proto((const char *, const char *));
#endif

# ifndef FULL_UNISTD

/* The parameter of getpwuid is widened uid_t,
 * but there no easy portable way to write this
 */
extern struct passwd *getpwuid(/*widened uid_t*/);

#  ifdef USE_UNAME
extern int	uname proto((struct utsname *));
#  endif

#  ifdef USE_GETHOSTNAME
extern int	gethostname proto((const char *, size_t));
#  endif

extern int	mkdir proto((const char *, jmode_t));
# endif /* !FULL_UNISTD */
#endif /* UNIX */

#ifndef L_SET
# define L_SET	0
# define L_INCR	1
#endif

private const char *progname;
private char	blk_buf[JBUFSIZ];
private long	nleft;
private long	nshort;
private FILE	*ptrs_fp;
private int	data_fd = -1;
private struct rec_head	Header;
private long	Nchars,
	Nlines;
private char	tty[] = "/dev/tty";
private const char	*tmp_dir = TMPDIR;
private uid_t	UserID;
private bool	Verbose = NO;
private bool	Debug = NO;
private FILE	*dfp;
private const char *RecDir = RECDIR;

private struct file_pair {
	char	*file_data,
		*file_rec;
#define INSPECTED	01
	int	file_flags;
	struct file_pair	*file_next;
} *First = NULL;

private long			maxbufs;
private struct rec_entry	**buflist;	/* system initializes to 0 */

#ifndef F_COMPLETION
# define F_COMPLETION	/* since scandir.c is surrounded by an ifdef */
#endif

/* simpler version of one in util.c, needed by scandir.c */
UnivPtr
emalloc(size)
size_t size;
{
	register UnivPtr ptr;

	if ((ptr = malloc(size)) == NULL) {
		fprintf(stderr, "couldn't malloc(%ld)\n", (long)size);
		exit(1);
	}
	return ptr;
}

/* simpler version of one in util.c, needed by scandir.c */
UnivPtr
erealloc(ptr, size)
UnivPtr ptr;
size_t size;
{
	if (ptr == NULL)
		return malloc(size); /* some realloc do not like ptr == NULL */
	if ((ptr = realloc(ptr, size)) == NULL) {
		fprintf(stderr, "couldn't realloc(%ld)\n", (long)size);
		exit(1);
	}
	return ptr;
}

/* copy a string into buffer; truncate silently if too large; NUL-pad.
 * Note: buffer must be 1 larger than n to fit NUL!
 * Duplicated from util.c: needed by scandir.c
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

#include "scandir.c"	/* to get dirent simulation and jscandir */

/* Get a line at `tl' in the tmp file into `buf' which should be LBSIZE
 * long.
 */

private char	*getblock proto((daddr atl));

void
jgetline(tl, buf)
daddr	tl;
char	*buf;
{
	register char	*bp,
			*lp;
	register long	nl;

	lp = buf;
	bp = getblock(tl);
	nl = nleft;

	while ((*lp++ = *bp++) != '\0') {
		if (--nl == 0) {
			/* oops: line didn't end within block: fake it */
			if (Verbose)
				fprintf(dfp, "warning: truncated line %lu\n", (unsigned long)tl);
			*lp++ = '\0';
			break;
		}
	}
}

private jmp_buf	int_env;

private char *
getblock(atl)
daddr	atl;
{
	daddr	bno,
		off;
	static daddr	curblock = MAX_BLOCKS;

	bno = da_to_bno(atl);
	off = da_to_off(atl);
	nleft = JBUFSIZ - off;
	if (Debug)
	    fprintf(dfp, "getblock atl %ld bno %ld off %ld nleft %ld\n",
		    (long)atl, (long)bno, (long)off, (long)nleft);

	if (bno != curblock) {
		JSSIZE_T nb;
		const char *what;
		off_t	    r,
			    boff = bno_to_seek_off(bno);

		if (Debug)
			fprintf(dfp, "lseek %d to bno %lu 0x%lx boff %ld 0x%lx\n", data_fd, (unsigned long)bno, (unsigned long) bno, (long)boff, (long)boff);
		what = "lseek";
		r = lseek(data_fd, boff, L_SET);
		if (r >= 0) {
			what = "read";
			nb = read(data_fd, (UnivPtr)blk_buf, (size_t)JBUFSIZ);
			if (nb >= 0 && nb != JBUFSIZ) {
				if (nshort == 0)
					fprintf(stderr, "short read from JOVE tempfile %ld\n", (long)nb);
				nshort++;
				bno = MAX_BLOCKS;
				if (nb > off) {
					nleft = nb - off;
				} else {
					blk_buf[off] = '\0';
					nleft = 1;
				}
			} else if (nb < 0) {
				r = -1;
			}
		}
		if (r < 0) {
			fprintf(stderr, "%s of JOVE tempfile failed bno %ld: %s\n", what, (long)bno, strerror(errno));
			longjmp(int_env, 1);
			/* NOTREACHED */
		}
		curblock = bno;
	}
	return blk_buf + off;
}

/* This pre-dates strdup, but can we assume it exists now? */
char *
copystr(s)
const char	*s;
{
	char	*str;
	size_t	sz = strlen(s) + 1;

	str = malloc((size_t)sz);
	if (str == NULL) {
		fprintf(stderr, "%s: cannot malloc %lu for copystr.\n", progname,
			(unsigned long)sz);
		exit(-1);
	}
	strcpy(str, s);

	return str;
}

private const char	*CurDir;

/* Scan the DIRNAME directory for jove tmp files, and make a linked list
 * out of them.
 */

private bool	add_name proto((char *));

private void
free_files() {
	while (First != NULL) {
		struct file_pair	*p = First;

		First = p->file_next;
		free((UnivPtr) p->file_data);
		free((UnivPtr) p->file_rec);
		free((UnivPtr) p);
	}
}

private void
get_files(dirname)
const char	*dirname;
{
	char	**nmptr;
	int	nentries;

	/* first, free any previous entries */
	free_files();

	CurDir = dirname;
	nentries = jscandir(dirname, &nmptr, add_name,
		(int (*) ptrproto((UnivConstPtr, UnivConstPtr)))NULL);
	if (nentries != -1)
		freedir(&nmptr, nentries);
}

private bool
add_name(fname)
char *fname;
{
	char	dfile[FILESIZE*2+2], /* CurDir/filename */
		rfile[FILESIZE*2+2];
	struct file_pair	*fp;
	struct rec_head		header;
	int	fd;
	/* jrecstr must match the start of the string in recinit */
#ifdef MAC
	static const char	jrecstr[] = ".jrec";
#else
	static const char	jrecstr[] = "jr";
#endif
	struct stat		stbuf;

	if (strncmp(fname, jrecstr, sizeof(jrecstr)-1) != 0) {
		if (Debug)
			fprintf(dfp, "skipping %s\n", fname);
		return NO;
	}

	/* If we get here, we found a "recover" tmp file, so now
	 * we look for the corresponding "data" tmp file.  First,
	 * though, we check to see whether there is anything in
	 * the "recover" file.  If it's 0 length, there's no point
	 * in saving its name.
	 */
	(void) sprintf(rfile, "%s/%s", CurDir, fname);
	if (stat(rfile, &stbuf) != 0 || (stbuf.st_mode & S_IFMT) != S_IFREG) {
		if (Verbose)
			fprintf(dfp, "skipping non-regular file %s\n", rfile);
		return NO;
	}
	if (Debug)
		fprintf(dfp, "checking %s\n", rfile);
	if ((fd = open(rfile, O_RDONLY | O_BINARY | O_CLOEXEC)) != -1) {
		if (read(fd, (UnivPtr) &header, sizeof header) != sizeof header) {
			close(fd);
			fprintf(stderr, "%s: could not read complete header from %s, skipping\n",
				progname, rfile);
			return NO;
		}
		if (header.RecMagic != RECMAGIC) {
			close(fd);
			fprintf(stderr, "%s: skipping incompatible %s magic 0x%lx != our 0x%lx\n",
				progname, rfile, header.RecMagic, (unsigned long)RECMAGIC);
			return NO;
		}
		close(fd);
	}
	(void) sprintf(dfile, "%s/%s", CurDir, header.TmpFileName);
	if (access(dfile, F_OK) != 0) {
		fprintf(stderr, "%s: can't find the data file `%s' for %s\n",
			progname, header.TmpFileName, rfile);
#ifdef NEVER
		/*
		 * MM: I don't think it's a good idea to delete the files
		 * because access() failed.  We should probably ask the user
		 * if it is ok to delete the file!
		 */
		fprintf(stderr, "so deleting...\n");
		(void) unlink(rfile);
#endif
		return NO;
	}
	/* If we get here, we've found both files, so we put them
	 * in the list.
	 */
	fp = (struct file_pair *) malloc(sizeof *fp);
	if (fp == NULL) {
		fprintf(stderr, "%s: cannot malloc for file_pair.\n", progname);
		exit(-1);
	}
	fp->file_data = copystr(dfile);
	fp->file_rec = copystr(rfile);
	fp->file_flags = 0;
	fp->file_next = First;
	First = fp;

	return YES;
}

private void
options()
{
	printf("Options are:\n");
	printf("	?		list options.\n");
	printf("	get		get a buffer to a file.\n");
	printf("	list		list known buffers.\n");
	printf("	print		print a buffer to terminal.\n");
	printf("	quit		quit and delete jove tmp files.\n");
	printf("	restore		restore all buffers.\n");
}

/* Returns a legitimate buffer # */

private void	tellme proto((const char *, char *, size_t)),
	list proto((void));

private long
getsrc()
{
	char	name[FILESIZE];
	long	number;

	for (;;) {
		tellme("Which buffer ('?' for list)? ", name, sizeof(name));
		if (name[0] == '?')
			list();
		else if (name[0] == '\0')
			return -1;
		else if ((number = atoi(name)) > 0 && number <= Header.Nbuffers)
			return number;
		else {
			long	i;

			for (i = 1; i <= Header.Nbuffers; i++)
				if (strcmp(buflist[i]->r_bname, name) == 0)
					return i;
			printf("%s: unknown buffer.\n", name);
		}
	}
}

/* Get a destination file name. */

private char *
getdest()
{
	static char	filebuf[FILESIZE];

	tellme("Output file: ", filebuf, sizeof(filebuf));
	if (filebuf[0] == '\0')
		return NULL;
	return filebuf;
}

#include "jctype.h"

private char *
readword(buf, buflen)
char	*buf;
size_t	buflen;
{
	int	c;
	char	*bp = buf,
		*ep = buf + buflen - 1;

	do {} while (strchr(" \t\n", c = getchar()) != NULL);

	for (;;) {
		if (c == EOF)
			exit(0);
		if (strchr(" \t\n", c) != NULL)
			break;

		if (bp == ep) {
			*bp = '\0';
			fprintf(stderr, "%lu byte buffer too small for word `%s'",
				(unsigned long) buflen, buf);
			exit(0);
		}
		*bp++ = c;
		c = getchar();
	}
	*bp = '\0';

	return buf;
}

private void
tellme(quest, answer, anslen)
const char	*quest;
char	*answer;
size_t	anslen;
{
	printf("%s", quest);
	fflush(stdout);
	readword(answer, anslen);
}

#ifdef UNIX
private SIGRESTYPE
catch(junk)
int	UNUSED(junk);
{
	longjmp(int_env, 1);
	/*NOTREACHED*/
}
#endif /* UNIX */

private void	get proto((long src, char *dest));

private void
restore()
{
	register long	i;
	char	tofile[FILESIZE+1], /* leading # */
		answer[SMALLSTRSIZE];
	int	nrecovered = 0;

	for (i = 1; i <= Header.Nbuffers; i++) {
		(void) sprintf(tofile, "#%s", buflist[i]->r_bname);
tryagain:
		printf("Restoring %s to %s, okay?", buflist[i]->r_bname,
						     tofile);
		tellme(" ", answer, sizeof(answer));
		switch (answer[0]) {
		case 'y':
			break;

		case 'n':
			continue;

		default:
			tellme("What file should I use instead? ", tofile,
			       sizeof(tofile));
			goto tryagain;
		}
		if (Debug)
			fprintf(dfp, "getting %ld %s to %s\n", i,
				buflist[i]->r_bname, tofile);
		get(i, tofile);
		nrecovered += 1;
	}
	printf("Recovered %d buffers.\n", nrecovered);
}

private void	dump_file proto((long which, FILE *out));

private void
get(src, dest)
long	src;
char	*dest;
{
	FILE	*volatile outfile;	/* "volatile" to preserve outfile across setjmp */

	if (src < 0 || src >= maxbufs || dest == NULL)
		return;

	if (dest == tty) {
		outfile = stdout;
	} else {
		if ((outfile = fopen(dest, "wb")) == NULL) {
			fprintf(stderr, "%s: cannot create %s: %s\n", progname, dest, strerror(errno));
			(void) signal(SIGINT, SIG_DFL);
			return;
		}
		printf("\"%s\"", dest);
	}
	if (setjmp(int_env) == 0) {
		(void) signal(SIGINT, catch);
		dump_file(src, outfile);
	} else {
		printf("\nAborted!\n");
	}
	(void) signal(SIGINT, SIG_DFL);
	if (dest != tty) {
		if (fflush(outfile) == EOF || ferror(outfile) ||
#ifdef USE_FSYNC
		    fsync(fileno(outfile)) < 0 ||
#endif
		    fclose(outfile) == EOF) {
			fprintf(stderr, "Error flushing/closing %s: %s\n", dest, strerror(errno));
		}
		printf(" %ld lines, %ld characters.\n", Nlines, Nchars);
	}
	fflush(stdout);
	if (nshort) {
		fprintf(stderr, "%ld missing lines (short reads of data file)\n", nshort);
		nshort = 0;
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

private void
read_rec(recptr)
struct rec_entry	*recptr;
{
	if (fread((UnivPtr) recptr, sizeof *recptr, (size_t)1, ptrs_fp) != 1)
		fprintf(stderr, "%s: cannot read record. %s\n", progname, strerror(errno));
}

private void
seekto(which)
long	which;
{
	long	offset,
		i;

	offset = sizeof (Header) + (Header.Nbuffers * sizeof (struct rec_entry));
	for (i = 1; i < which; i++)
		offset += buflist[i]->r_nlines * sizeof (daddr);
	fseek(ptrs_fp, offset, L_SET);
}

private void
freeblist()
{
	long	i;
	for (i = 0; i < maxbufs; i++) {
		if (buflist[i]) {
			free((UnivPtr) buflist[i]);
			buflist[i] = NULL;
		}
	}
	free((UnivPtr) buflist);
	buflist = NULL;
	maxbufs = 0;
}

private void
makblist()
{
	long	i;

	fseek(ptrs_fp, (long) sizeof (Header), L_SET);
	if (maxbufs <= Header.Nbuffers) {
		maxbufs = Header.Nbuffers+1;
		buflist = (struct rec_entry **) erealloc(NULL, maxbufs*sizeof(struct rec_entry *));
		for (i = 0; i < maxbufs; i++)
			buflist[i] = NULL;
	}
	for (i = 1; i <= Header.Nbuffers; i++) {
		if (buflist[i] == NULL) {
			buflist[i] = (struct rec_entry *) malloc (sizeof (struct rec_entry));
			if (buflist[i] == NULL) {
				fprintf(stderr, "%s: cannot malloc for makblist.\n", progname);
				exit(-1);
			}
		}
		read_rec(buflist[i]);
	}
	/*
	 * just for safety, unset any remaining buflist
	 * entries. since we should always be using
	 * Header.Nbuffers as a limit, we should never touch these.
	 */
	while (i < maxbufs) {
		if (buflist[i]) {
			buflist[i]->r_bname[0] = buflist[i]->r_fname[0] = '\0';
			buflist[i]->r_nlines = buflist[i]->r_dotline =
				buflist[i]->r_dotchar = 0;
		}
		i++;
	}
}

private daddr
getaddr(fp)
register FILE	*fp;
{
	register int	nchars = sizeof (daddr);
	daddr	addr;
	register char	*cp = (char *) &addr;

	while (--nchars >= 0)
		*cp++ = getc(fp);

	return addr & ~DDIRTY;
}

private void
dump_file(which, out)
long	which;
FILE	*out;
{
	register long	nlines; /* XXX lnum_t */
	register daddr	addr;
	char	buf[JBUFSIZ];

	seekto(which);
	nlines = buflist[which]->r_nlines;
	Nchars = Nlines = 0L;
	while (--nlines >= 0) {
		addr = getaddr(ptrs_fp);
		if (Debug)
			fprintf(dfp, "line %ld addr %lu\n", nlines, (unsigned long)addr);
		jgetline(addr, buf);
		Nlines += 1;
		Nchars += 1 + strlen(buf);
		fputs(buf, out);
		if (nlines > 0) {
#ifdef USE_CRLF
			fputc('\r', out);
#endif
			fputc(EOL, out);
		}
	}
}

/* List all the buffers. */

private void
list()
{
	long	i;

	for (i = 1; i <= Header.Nbuffers; i++)
		printf("%ld) buffer %s  \"%s\" (%ld lines)\n", i,
			buflist[i]->r_bname,
			buflist[i]->r_fname,
			buflist[i]->r_nlines);
}

private void	ask_del proto((const char *prompt, struct file_pair *fp));

private int
doit(fp)
struct file_pair	*fp;
{
	char	answer[SMALLSTRSIZE];
	char	*datafile = fp->file_data,
		*pntrfile = fp->file_rec;
	time_t	tupd;

	ptrs_fp = fopen(pntrfile, "rb");
	if (ptrs_fp == NULL) {
		fprintf(stderr, "%s: cannot read rec file (%s).\n", progname, pntrfile);
		return 0;
	}
	if (Debug)
		fprintf(dfp, "opened %s\n", pntrfile);
	if (fread((UnivPtr) &Header, sizeof Header, (size_t)1, ptrs_fp) != 1) {
		fprintf(stderr, "%s: cannot read header from rec file (%s).\n", progname, pntrfile);
		return 0;
	}
	if (Debug)
		fprintf(dfp, "read header from %s, uid %ld, pid %ld, %ld bufs, %s\n",
			pntrfile, Header.Uid, Header.Pid, Header.Nbuffers, Header.TmpFileName);

	if (Header.Uid != (long)UserID) {
		if (Debug)
		    fprintf(dfp, "different user %ld != %ld\n",
			    (long)Header.Uid, (long)UserID);
		return 0;
	}

	/* Ask about JOVE's that are still running ... */
	if (kill((pid_t)Header.Pid, 0) == 0) {
		if (Debug)
		    fprintf(dfp, "still running pid %ld\n", Header.Pid);
		return 0;
	}

	if (Header.Nbuffers == 0) {
		printf("There are no modified buffers in %s; should I delete the tmp file?", pntrfile);
		ask_del(" ", fp);
		return 1;
	}

	if (Header.Nbuffers < 0) {
		fprintf(stderr, "%s: %s doesn't look like a jove recovery file.\n", progname, pntrfile);
		ask_del("Should I delete it? ", fp);
		return 1;	/* We'll, we sort of found something. */
	}
	tupd = (time_t)Header.UpdTime;
	printf("Found %ld buffer%s last updated: %s",
		Header.Nbuffers, Header.Nbuffers != 1 ? "s" : "", ctime(&tupd));
	data_fd = open(datafile, O_RDONLY | O_BINARY | O_CLOEXEC);
	if (data_fd == -1) {
		fprintf(stderr, "%s: but I can't read the data file (%s).\n", progname, datafile);
		ask_del("Should I delete the tmp files? ", fp);
		return 1;
	}
	if (Debug)
		fprintf(dfp, "opened fd %d data file %s\n", data_fd, datafile);
	makblist();
	list();

	for (;;) {
		long	src;

		tellme("(Type '?' for options): ", answer, sizeof(answer));
		switch (answer[0]) {
		case '\0':
			continue;

		case '?':
			options();
			break;

		case 'l':
			list();
			break;

		case 'p':
			if ((src = getsrc()) < 0)
				break;
			if (Debug)
				fprintf(dfp, "getting %ld %s to %s\n",
					src, buflist[src]->r_bname, tty);
			get(src, tty);
			break;

		case 'q':
			ask_del("Shall I delete the tmp files? ", fp);
			return 1;

		case 'g':
		    {
			/* So it asks for src first. */
			char	*dest;
			if ((src = getsrc()) < 0)
				break;

			dest = getdest();
			if (Debug)
				fprintf(dfp, "getting %ld %s to %s\n",
					src, buflist[src]->r_bname, dest);
			get(src, dest);
			break;
		    }

		case 'r':
			restore();
			break;

		default:
			printf("I don't know how to \"%s\"!\n", answer);
			break;
		}
	}
}

private void	del_files proto((struct file_pair *fp));

private void
ask_del(prompt, fp)
const char	*prompt;
struct file_pair	*fp;
{
	char	yorn[SMALLSTRSIZE];

	tellme(prompt, yorn, sizeof(yorn));
	if (yorn[0] == 'y')
		del_files(fp);
}

private void
del_files(fp)
struct file_pair	*fp;
{
	(void) unlink(fp->file_data);
	(void) unlink(fp->file_rec);
}


#ifdef UNIX
private const char *
hname()
{
	const char *p = "unknown";
#ifdef USE_UNAME
	static struct utsname mach;

	if (uname(&mach) >= 0)
		p = mach.nodename;
#endif
#ifdef USE_GETHOSTNAME
	static char mach[BUFSIZ];

	if (gethostname(mach, sizeof(mach)) >= 0)
		p = mach;
#endif
	return p;
}

private void
MailUser(rec)
struct rec_head *rec;
{
	char mail_cmd[BUFSIZ];
	char *last_update;
	const char *buf_string, *mail_prog;
	FILE *mail_pipe;
	struct passwd *pw;
	int r;
	time_t tupd;

	if ((pw = getpwuid((uid_t)rec->Uid))== NULL)
		return;

	tupd = (time_t)(rec->UpdTime);
	last_update = ctime(&tupd);
	/* Start up mail */
	if ((mail_prog = getenv("JOVEMAILER")) == NULL)
	    mail_prog = "/bin/mail";
	if (strlen(mail_prog) + 1 + strlen(pw->pw_name) + 1 > sizeof(mail_cmd)) {
	    fprintf(stderr, "%s: %u buffer too small for \"%s %s\", skipping\n",
		    progname, (unsigned)sizeof(mail_cmd), mail_prog, pw->pw_name);
	    return;
	}
	sprintf(mail_cmd, "%s %s", mail_prog, pw->pw_name);
	if ((r = setuid(getuid())) < 0) {
	    fprintf(stderr, "WARNING: %s: setuid(getuid()) failed: %s\n",
		    progname, strerror(errno));
	    /*
	     * used to continue without checking return value,
	     * so let that behaviour continue, I guess?
	     */
	}
	if ((mail_pipe = popen(mail_cmd, "w")) == NULL)
		return;

	setbuf(mail_pipe, mail_cmd);
	/* Let's be grammatically correct! */
	buf_string = rec->Nbuffers == 1? "buffer" : "buffers";
	fprintf(mail_pipe, "Subject: Jove saved %ld %s after \"%s\" crashed\n",
		rec->Nbuffers, buf_string, hname());
	fprintf(mail_pipe, " \n");
	fprintf(mail_pipe, "Jove saved %ld %s when the system \"%s\"\n",
		rec->Nbuffers, buf_string, hname());
	fprintf(mail_pipe, "crashed on %s\n\n", last_update);
	fprintf(mail_pipe, "You can retrieve the %s using Jove's -r\n",
		buf_string);
	fprintf(mail_pipe, "(recover option) i.e. give the command.\n");
	fprintf(mail_pipe, "\tjove -r\n");
	fprintf(mail_pipe, "See the Jove manual for more details\n");
	pclose(mail_pipe);
}


private void
savetmps()
{
	struct file_pair	*fp;
	wait_status_t	status;
	pid_t	pid;
	int	fd, rc;
	struct rec_head		header;
	char	buf[PATHBUFSIZE];
	char	*fname;
	struct stat		stbuf;

	if (strcmp(tmp_dir, RecDir) == 0)
		return;		/* Files are moved to the same place. */

	/* sanity check on RecDir */
	if (strlen(RecDir) > MAXRECDIRSIZE) {
		fprintf(stderr, "%s: recovery directory len is %u, must be smaller than %u\n",
			progname, (unsigned)strlen(RecDir), MAXRECDIRSIZE);
		exit(2);
	}
	sprintf(buf, "%s/jv%06u", RecDir, (unsigned)getpid()); /* dummy name */
	stbuf.st_mode = stbuf.st_uid = 0;
	if ((rc = stat(RecDir, &stbuf)) < 0)
		if ((rc = mkdir(RecDir, 0755)) == 0)
			rc = stat(RecDir, &stbuf);
	if (rc < 0 || !S_ISDIR(stbuf.st_mode) || stbuf.st_uid != getuid() /*||
	    access(buf, W_OK) != 0*/) {
		fprintf(stderr, "%s: need writable directory \"%s\" owned by %u: got mode 0%o uid %u rc %d%s\n",
			progname, RecDir, getuid(), (unsigned)stbuf.st_mode,
			(unsigned)stbuf.st_uid, rc, rc < 0 ? strerror(errno) : "");
		exit(2);
	}

	setbuf(stdout, NULL);
	printf("Recovering jove files ... ");
	get_files(tmp_dir);
	for (fp = First; fp != NULL; fp = fp->file_next) {
		if (Debug)
		    fprintf(dfp, "Recovering: %s, %s\n", fp->file_data,
			    fp->file_rec);
		if (stat(fp->file_data, &stbuf) < 0) {
			fprintf(stderr, "%s: stat data %s failed, rec %s: %s\n",
				progname, fp->file_data, fp->file_rec, strerror(errno));
			continue;
		}
		switch (pid = fork()) {
		case -1:
			fprintf(stderr, "%s: can't fork. %s\n!", progname, strerror(errno));
			exit(-1);
			/*NOTREACHED*/

		case 0:
			if ((fd = open(fp->file_rec, O_RDONLY | O_BINARY | O_CLOEXEC)) != -1) {
				if ((read(fd, (UnivPtr) &header, sizeof header) != sizeof header)) {
					close(fd);
					return;
				} else
					close(fd);
			}
			MailUser(&header);
			execl("/bin/mv", "mv", fp->file_data, fp->file_rec,
				  RecDir, (char *)NULL);
			fprintf(stderr, "%s: cannot execl /bin/mv. %s\n", progname, strerror(errno));
			exit(-1);
			/*NOTREACHED*/

		default:
			do {} while (wait(&status) != pid);
			if (WIFSIGNALED(status))
				fprintf(stderr, "%s: copy terminated by signal %d\n.\n", progname, WTERMSIG(status));
			if (WIFEXITED(status))
				fprintf(stderr, "%s: copy exited with %d.\n", progname, WEXITSTATUS(status));
			fname = fp->file_data + strlen(tmp_dir);
			if (strlen(fname)+1 > MAXFILENAMESIZE) {
				fprintf(stderr, "%s: filename \"%s\" len %u, must be smaller than %u\n",
					progname, fname, (unsigned)strlen(fname), MAXFILENAMESIZE-1);
				exit(2);
			}
			strcpy(buf, RecDir);
			strcat(buf, fname);
			if (chown(buf, stbuf.st_uid, stbuf.st_gid) != 0) {
				fprintf(stderr, "%s: chown data %s to %u.%u failed: %s",
					progname, buf, (unsigned)stbuf.st_uid, (unsigned)stbuf.st_gid,
					strerror(errno));
			}
			fname = fp->file_rec + strlen(tmp_dir);
			if (strlen(fname)+1 > MAXFILENAMESIZE) {
				fprintf(stderr, "%s: filename \"%s\" len %u, must be smaller than %u\n",
					progname, fname, (unsigned)strlen(fname), MAXFILENAMESIZE-1);
				exit(2);
			}
			strcpy(buf, RecDir);
			strcat(buf, fname);
			if (chown(buf, stbuf.st_uid, stbuf.st_gid) != 0) {
				fprintf(stderr, "%s: chown rec %s to %u.%u failed: %s",
					progname, buf, (unsigned)stbuf.st_uid, (unsigned)stbuf.st_gid,
					strerror(errno));
			}
			fputc('.', stdout);
		}
	}
	free_files();
	printf("Done.\n");
}
#endif /* UNIX */

private int
lookup(dir)
const char	*dir;
{
	struct file_pair	*fp;
	int	nfound = 0;

	printf("Checking %s ...\n", dir);
	get_files(dir);
	for (fp = First; fp != NULL; fp = fp->file_next) {
		nfound += doit(fp);
		if (ptrs_fp) {
			(void) fclose(ptrs_fp);
			ptrs_fp = NULL;
		}
		if (data_fd > 0) {
			(void) close(data_fd);
			data_fd = -1;
		}
	}
	free_files();
	return nfound;
}

int
main(argc, argv)
int	UNUSED(argc);
char	*argv[];
{
	int	nfound;
	char	**argvp;

	progname = argv[0];
	UserID = getuid();

	/* override tmp_dir with $TMPDIR, RecDir with $JOVERECDIR if any */
	{
		char	*cp = getenv("TMPDIR");

		if (cp != NULL)
			tmp_dir = cp;
		cp = getenv("JOVERECDIR");
		if (cp != NULL)
		    RecDir = cp;
	}

	if (scanvec(argv, "-help")) {
		printf("%s: usage: recover [-d directory] [-syscrash]\n\n", progname);
		printf("Use \"jove -r\" after JOVE has died for some unknown reason.\n\n");
		printf("Use \"%s/recover -syscrash\"\n", LIBDIR);
		printf("\twhen the system is in the process of rebooting.\n");
		printf("\tThis is done automatically at reboot time and\n");
		printf("\tso most of you don't have to worry about that.\n\n");
		printf("Use \"recover -d directory\"\n");
		printf("\twhen the tmp files are stored in 'directory'\n");
		printf("\tinstead of in the default one (%s).\n\n", tmp_dir);
		exit(0);
	}
	if (scanvec(argv, "-v"))
		Verbose = YES;
	if ((argvp = scanvec(argv, "-D")) != NULL) {
		Debug = YES;
		dfp = fopen(argvp[1], "wc");
		fprintf(dfp, "debugging %s\n", ctime(NULL));
	} else {
		dfp = stdout;
	}
	if ((argvp = scanvec(argv, "-d")) != NULL)
		tmp_dir = argvp[1];
#ifdef UNIX
	if (scanvec(argv, "-syscrash")) {
		savetmps();
		exit(0);
	}
#endif
	if ((argvp = scanvec(argv, "-uid")) != NULL)
		UserID = atoi(argvp[1]);
	/* Check default directory */
	nfound = lookup(tmp_dir);
	/* Check whether anything was saved when system died? */
	if (strcmp(tmp_dir, RecDir) != 0)
		nfound += lookup(RecDir);
	if (nfound == 0)
		printf("There's nothing to recover.\n");
	freeblist();
	return 0;
}

#endif /* RECOVER */
