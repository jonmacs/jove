/***************************************************************************
 * This program is Copyright (C) 1986, 1987, 1988 by Jonathan Payne.  JOVE *
 * is provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is    *
 * included in all the files.                                              *
 ***************************************************************************/

struct variable {
	/* Type and Name must match data_obj */
	int	Type;		/* in this case a variable */
	char	*Name;		/* name is always second */
	UnivPtr	v_value;
	size_t	v_size;
	int	v_flags;
};

/* variable types/flags */
#define V_BASE10	01	/* is integer in base 10 */
#define V_BASE8		02	/* is integer in base 8 */
#define V_BOOL		04	/* is a boolean */
#define V_STRING	010	/* is a string */
#define V_CHAR		020	/* is a character */
#define V_FILENAME	040	/* a file name (implies V_STRING) */
#define V_TYPEMASK	077	/* mask off the extra bits */
#define V_MODELINE	0100	/* update modeline */
#define V_CLRSCREEN	0200	/* clear and redraw screen */
#define V_TTY_RESET	0400	/* redo the tty modes */

extern const struct variable	variables[];
