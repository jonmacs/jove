/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

/* values for the `flags' argument to complete (may be combined) */
#define CASEIND		001	/* map all to lower case */
#define ALLOW_OLD	002	/* accept member */
#define ALLOW_INDEX	004	/* accept index number as answer (on CR) */
#define ALLOW_NEW	010	/* accept non-member (on CR) */
#define ALLOW_EMPTY	020	/* accept empty answer (on CR) */

extern int	InJoverc;	/* depth in sourcing */

extern bool	joverc proto((char *file));

#ifdef USE_PROTOTYPES
struct variable;	/* forward declaration preventing prototype scoping */
#endif /* USE_PROTOTYPES */

extern void
	DoAutoExec proto((char *new, char *old)),
	vpr_aux proto((const struct variable *, char *, size_t)),
	vset_aux proto((const struct variable *, char *));

extern ZXchar
	addgetc proto((void));

extern long
	ask_long proto((const char *def, const char *prompt, int base));

extern int
	ask_int proto((const char *def, const char *prompt, int base)),
	complete proto((const char *const *possible, const char *def, const char *prompt, int flags));

extern bool
	chr_to_int proto((const char *cp, int base, bool allints, int *result)),
	chr_to_long proto((const char *cp, int base, bool allints, long *result));

/* Commands: */
extern void
	BufPos proto((void)),
	CAutoExec proto((void)),
	Extend proto((void)),
	MAutoExec proto((void)),
	PrVar proto((void)),
	InsVar proto((void)),
	SetVar proto((void)),
	Source proto((void));
