/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

/* generic singly linked list package */

typedef struct cons	List;

struct cons {
	void *	car;	/* pointer to element */
	List	*cdr;
};

#define list_next(lp)	((lp)->cdr)
#define list_data(lp)	((lp)->car)

extern void
	*list_push(List **, void *),
	*list_pop(List **);
