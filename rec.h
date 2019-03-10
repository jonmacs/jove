/************************************************************************
 * This program is Copyright (C) 1986 by Jonathan Payne.  JOVE is       *
 * provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is *
 * included in all the files.                                           *
 ************************************************************************/

struct rec_head {
	int	Uid,		/* Uid of owner. */
		Pid;		/* Pid of jove process. */
	time_t	UpdTime;	/* Last time this was updated. */
	int	Nbuffers;	/* Number of buffers. */
};

struct rec_entry {
	char	r_bname[128],
		r_fname[128];
	int	r_nlines;
};

