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

extern jbool	joverc(char *file);

struct variable;	/* forward declaration preventing prototype scoping */

extern void
	DoAutoExec(char *new, char *old),
	vpr_aux(const struct variable *, char *, size_t),
	vset_aux(const struct variable *, char *);

extern ZXchar
	addgetc(void);

extern long
	ask_long(const char *def, const char *prompt, int base);

extern int
	ask_int(const char *def, const char *prompt, int base),
	complete(const char *const *possible, const char *def, const char *prompt, int flags);

extern jbool
	chr_to_int(const char *cp, int base, jbool allints, int *result),
	chr_to_long(const char *cp, int base, jbool allints, long *result);

/* Commands: */
extern void
	BufPos(void),
	CAutoExec(void),
	Extend(void),
	MAutoExec(void),
	PrVar(void),
	InsVar(void),
	SetVar(void),
	Source(void);
