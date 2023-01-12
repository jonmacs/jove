/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

typedef void(*cmdproc_t)  ptrproto((void));

struct cmd {
	/* Type and Name must match data_obj */
	int	Type;
	const char	*Name;
	const cmdproc_t 	c_proc;
#ifdef MAC
	char c_map;			/* prefix map for About Jove... */
	char c_key;			/* key binding for About Jove... */
#endif
};

extern const struct cmd	commands[];
extern const struct cmd	*cmdidx[IDXSZ];

extern const struct cmd
	*FindCmd proto((cmdproc_t));

extern void
	ExecCmd proto((const data_obj *cp));
