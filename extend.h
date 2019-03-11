/***************************************************************************
 * This program is Copyright (C) 1986, 1987, 1988 by Jonathan Payne.  JOVE *
 * is provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is    *
 * included in all the files.                                              *
 ***************************************************************************/

/* return codes for command completion (all < 0 because >= 0 are
   legitimate offsets into array of strings */

#define AMBIGUOUS	(-2)	/* matches more than one at this point */
#define UNIQUE		(-3)	/* matches only one string */
#define ORIGINAL	(-4)	/* matches no strings at all! */
#define NULLSTRING	(-5)	/* just hit return without typing anything */

/* values for the `flags' argument to complete */
#define NOTHING		0	/* opposite of RET_STATE */
#define RET_STATE	1	/* return state when we hit return */
#define RCOMMAND	2	/* we are reading a joverc file */
#define CASEIND		4	/* map all to lower case */

extern int	InJoverc;	/* depth in sourcing */

extern bool	joverc proto((char *file));

#ifdef	USE_PROTOTYPES
struct variable;	/* forward declaration preventing prototype scoping */
#endif	/* USE_PROTOTYPES */

extern void
	DoAutoExec proto((char *new, char *old)),
	vpr_aux proto((const struct variable *, char *, size_t));

extern int
	addgetc proto((void)),
	ask_int proto((char *prompt, int base)),
	complete proto((char **possible, const char *prompt, int flags));

extern int	chr_to_int proto((char *cp, int base, int allints, int *result));
#define INT_OKAY	0
#define INT_BAD		(-1)

/* Commands: */
extern void
	BufPos proto((void)),
	CAutoExec proto((void)),
	Extend proto((void)),
	MAutoExec proto((void)),
	PrVar proto((void)),
	SetVar proto((void)),
	Source proto((void));
