/***************************************************************************
 * This program is Copyright (C) 1986, 1987, 1988 by Jonathan Payne.  JOVE *
 * is provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is    *
 * included in all the files.                                              *
 ***************************************************************************/

/* tables defined in keys.txt/keys.c */

extern data_obj
	*MainKeys[NCHARS],
	*EscKeys[NCHARS],
	*CtlxKeys[NCHARS];

#ifdef IBMPC
extern data_obj	*NonASCIIKeys[NCHARS];
#endif

#ifdef	MAC					/* used in About Jove... */
# define F_MAINMAP '\001'
# define F_PREF1MAP '\002'
# define F_PREF2MAP '\003'
#endif

#define OTHER_CMD	0
#define ARG_CMD		1
#define LINECMD		2
#define KILLCMD		3	/* so we can merge kills */
#define YANKCMD		4	/* so we can do ESC Y (yank-pop) */

extern int
	this_cmd,	/* ... */
	last_cmd;	/* last command ... to implement appending to kill buffer */

extern void
	InitKeymaps proto((void)),
	dispatch proto((int c));

extern bool
	PrefChar proto((int c));	/* Is `c' a prefix character? */

/* Commands: */
extern void
	Apropos proto((void)),
	BindAKey proto((void)),
	BindMac proto((void)),
	DescBindings proto((void)),
	DescCom proto((void)),
	DescVar proto((void)),
	KeyDesc proto((void)),
	KmBind proto((void)),
	MakeKMap proto((void)),
#ifdef	IPROCS
	ProcBind proto((void)),
	ProcKmBind proto((void)),
#endif
	UnbindC proto((void));
