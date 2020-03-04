/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

#ifdef RECOVER	/* the body is the rest of this file */
#include "temp.h"

#ifndef UNIX
# define getuid() 0
#endif

/* Format of records within the jrec file: information to allow
 * recovery from a crash.
 *
 * Note: the recovery info is
 * (1) architecture dependent -- it contains longs, among other things
 * (2) version dependent -- format 1 started in 4.15.16, version 2 in 4.17.2.8
 * (3) configuration dependent -- affected by any change to FILESIZE
 * RECMAGIC tries to encode all those things so we can at least detect
 * mismatches (and perhaps, down the road, at least recover different
 * configurations.
 * Even though this uses long, which might be 64bits, we keep RECMAGIC
 * to fit in 32bits, so that it can fit on most machines. Similarly,
 * we only use long as a field width, to make this more uniform;
 * wastes bits on 64bit machines (which can afford it) but is nice
 * for 32bit machines (or even 16bit, since their long is usually 32bits)
 * The bulk of any jrec file is the daddr pointers anyway.
 */

#define LG_JBUFSIZMIN	7
#if LG_JBUFSIZ < LG_JBUFSIZMIN || (LG_JBUFSIZ-LG_JBUFSIZMIN) > 15
Error will not be able to encode recovery header LG_JBUFSIZ safely
#endif

#define LG_FILESIZEMIN	6
#if LG_FILESIZE < LG_FILESIZEMIN || (LG_FILESIZE-LG_FILESIZEMIN) > 15
Error will not be able to encode recovery header LG_FILESIZE safely
#endif

#if LG_CHNK_CHARS > 15
Error will not be able to encode recovery header LG_CHNK_CHARS safely
#endif

# define RECVER		2	    /* JOVE recovery file, version 2 */
# define RECMAGIC	(((((((((((((long)'J' << 4) | \
                                  RECVER) << 4) | \
                                (LG_JBUFSIZ-LG_JBUFSIZMIN)) << 4) | \
                               sizeof(long)) << 4) | \
                             sizeof(daddr)) << 4) | \
                           (LG_FILESIZE-LG_FILESIZEMIN)) << 4) | \
                         LG_CHNK_CHARS)

struct rec_head {
	long		RecMagic;	/* (partial) compatibility check */
	long		Uid;		/* uid of owner */
	long		Pid;		/* pid of jove process */
	long		UpdTime;	/* last time this was updated */
	long		Nbuffers;	/* number of buffers */
	daddr		FreePtr;	/* position of DFree */
	char		TmpFileName[FILESIZE];	/* name of corresponding tempfile */
};

struct rec_entry {
	char	r_bname[FILESIZE],
		r_fname[FILESIZE];
	long	r_nlines,
		r_dotline,	/* so we can really save the context */
		r_dotchar;
};

#endif /* RECOVER */
