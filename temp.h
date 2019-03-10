/*************************************************************************
 * This program is copyright (C) 1985, 1986 by Jonathan Payne.  It is    *
 * provided to you without charge for use only on a licensed Unix        *
 * system.  You may copy JOVE provided that this notice is included with *
 * the copy.  You may not sell copies of this program or versions        *
 * modified for use on microcomputer systems, unless the copies are      *
 * included with a Unix system distribution and the source is provided.  *
 *************************************************************************/

#ifndef VMUNIX

#if BUFSIZ == 512
#	define	BLKMSK	01777
#	define	BNDRY	16
#	define	INCRMT	0100
#	define	LBTMSK	0760
#	define	NMBLKS	1018
#	define	OFFBTS	6
#	define	OFFMSK	077
#	define	SHFT	3
#else
#	define	BLKMSK	0777
#	define	BNDRY	16
#	define	INCRMT	0200
#	define	LBTMSK	01760
#	define	NMBLKS	506
#	define	OFFBTS	7
#	define	OFFMSK	0177
#	define	SHFT	3
#endif

#else

#define	BLKMSK	077777
#define	BNDRY	2
#define	INCRMT	02000
#define	LBTMSK	01776
#define	NMBLKS	077770
#define	OFFBTS	10
#define	OFFMSK	01777
#define	SHFT	0

#endif VMUNIX

extern int	nleft,		/* Number of good characters left in current block */
		tmpfd;
extern disk_line
		tline;	/* Pointer to end of tmp file */

extern char	*tfname;
