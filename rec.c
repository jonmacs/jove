/************************************************************************
 * This program is Copyright (C) 1986 by Jonathan Payne.  JOVE is       *
 * provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is *
 * included in all the files.                                           *
 ************************************************************************/

#include "jove.h"
#include "io.h"
#include "rec.h"
#include <sys/file.h>

private int	rec_fd = 0;
private char	*recfname;
private File	*rec_out;

#ifndef L_SET
#	define L_SET 0
#endif

private struct rec_head	Header;

recinit()
{
	char	buf[128];

	sprintf(buf, "%s/%s", TmpFilePath, p_tempfile);
	recfname = copystr(buf);
	recfname = mktemp(recfname);
	rec_fd = creat(recfname, 0644);
	if (rec_fd == -1) {
		complain("Cannot create \"%s\"; recovery disabled.", recfname);
		return;
	}
	/* Initialize the record IO. */
	rec_out = fd_open(recfname, F_WRITE|F_LOCKED, rec_fd, iobuff, LBSIZE);

	/* Initialize the record header. */
	Header.Uid = getuid();
	Header.Pid = getpid();
	Header.UpdTime = 0L;
	Header.Nbuffers = 0;
	(void) write(rec_fd, (char *) &Header, sizeof Header);
}

recclose()
{
	if (rec_fd == -1)
		return;
	(void) close(rec_fd);
	(void) unlink(recfname);
}

static
putaddr(addr, p)
disk_line	addr;
register File	*p;
{
	register char	*cp = (char *) &addr;
	register int	nchars = sizeof (disk_line);

	while (--nchars >= 0)
		putc(*cp++ & 0377, p);
}

static
putn(cp, nbytes)
register char	*cp;
register int	nbytes;
{
	while (--nbytes >= 0)
		putc(*cp++ & 0377, rec_out);
}

/* Write out the line pointers for buffer B. */

static
dmppntrs(b)
register Buffer	*b;
{
	register Line	*lp;

	for (lp = b->b_first; lp != 0; lp = lp->l_next)
		putaddr(lp->l_dline, rec_out);
}

/* dump the buffer info and then the actual line pointers. */

static
dmp_buf(b)
register Buffer	*b;
{
	static struct rec_entry	record;
	register Line	*lp;
	register int	nlines = 0;

	for (lp = b->b_first; lp != 0; lp = lp->l_next, nlines++)
		;
	strcpy(record.r_fname, b->b_fname ? b->b_fname : NullStr);
	strcpy(record.r_bname, b->b_name);
	record.r_nlines = nlines;
	putn((char *) &record, sizeof record);
	dmppntrs(b);
}

/* Goes through all the buffers and syncs them to the disk. */

int	SyncFreq = 50;

SyncRec()
{
	register Buffer	*b;

	if (rec_fd == 0)
		recinit();	/* Init recover file. */
	if (rec_fd == -1)
		return;
	lseek(rec_fd, 0L, L_SET);
	(void) time(&Header.UpdTime);
	Header.Nbuffers = 0;
	for (b = world; b != 0; b = b->b_next)
		if (b->b_type == B_SCRATCH || !IsModified(b))
			continue;
		else
			Header.Nbuffers++;
	putn((char *) &Header, sizeof Header);
	if (Header.Nbuffers != 0) {
		SyncTmp();
		for (b = world; b != 0; b = b->b_next)
			if (b->b_type == B_SCRATCH || !IsModified(b))
				continue;
			else
				dmp_buf(b);
	}
	flush(rec_out);
}
