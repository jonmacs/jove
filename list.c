/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

#include "jove.h"
#include "list.h"

private List *
list_new(void)
{
	List	*new;

	new = (List *) emalloc(sizeof (List));
	new->car = NULL;
	return new;
}

/* push an object to the beginning of list */

void *
list_push(List **list, void *element)
{
	List	*new;

	new = list_new();
	new->cdr = *list;
	new->car = element;
	*list = new;
	return element;
}

void *
list_pop(List **list)
{
	List	*cell = *list;
	void	*element;

	if (cell == NULL)
		return NULL;

	element = cell->car;
	*list = cell->cdr;
	free(cell);
	return element;
}
