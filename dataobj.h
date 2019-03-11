/***************************************************************************
 * This program is Copyright (C) 1986, 1987, 1988 by Jonathan Payne.  JOVE *
 * is provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is    *
 * included in all the files.                                              *
 ***************************************************************************/

#define FUNCTION	1
#define VARIABLE	2
#define MACRO		3
#define KEYMAP		4
#ifdef	MAC
# define BUFFER		6	/* menus can point to buffers, too */
# define STRING		7	/* a menu string or divider */
#endif

#define TYPEMASK	07
#define obj_type(o)	((o)->Type & TYPEMASK)
#define MAJOR_MODE	010
#define MINOR_MODE	020
#define MODIFIER	040
#define MODFUNC		(FUNCTION|MODIFIER)
#define DefMajor(x)	(FUNCTION|MAJOR_MODE|((x) << 8))
#define DefMinor(x)	(FUNCTION|MINOR_MODE|((x) << 8))

typedef struct data_obj {
	int	Type;
	char	*Name;
} data_obj;	/* prefix of cmd, macro, keymap and variable structs */

extern data_obj	*LastCmd;	/* last command invoked */

extern char	*ProcFmt;	/* ": %f " -- name of LastCmd */

extern data_obj
	*findcom proto((const char *prompt)),
	*findmac proto((const char *prompt)),
	*findvar proto((const char *prompt));
