/***************************************************************************
 * This program is Copyright (C) 1986, 1987, 1988 by Jonathan Payne.  JOVE *
 * is provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is    *
 * included in all the files.                                              *
 ***************************************************************************/

struct rec_head {
#ifdef UNIX
	int		Uid;		/* uid of owner */
	pid_t		Pid;		/* pid of jove process */
#endif
	time_t		UpdTime;	/* last time this was updated */
	int		Nbuffers;	/* number of buffers */
	daddr		FreePtr;	/* position of DFree */
};

struct rec_entry {
	char	r_bname[128],
		r_fname[128];
	int	r_nlines,
		r_dotline,	/* so we can really save the context */
		r_dotchar;
};

extern void
	SyncRec proto((void)),
	recclose proto((void)),
	recremove proto((void)),
	FullRecover proto((void));

/* Variables: */

extern int	SyncFreq;		/* how often to sync the file pointers */
