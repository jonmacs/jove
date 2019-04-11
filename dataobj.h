/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

#define COMMAND	1
#define VARIABLE	2
#define MACRO		3
#define FULL_KEYMAP	4
#define SPARSE_KEYMAP	5
#ifdef MAC
# define BUFFER		6	/* menus can point to buffers, too */
# define STRING		7	/* a menu string or divider */
#endif

#define TYPEMASK	07
#define obj_type(o)	((o)->Type & TYPEMASK)
#define MAJOR_MODE	010
#define MINOR_MODE	020
#define MODIFIER	040
#define MODCMD		(COMMAND|MODIFIER)
#define MAJOR_SHIFT	8
#define DefMajor(x)	(COMMAND|MAJOR_MODE|((x) << MAJOR_SHIFT))
#define DefMinor(x)	(COMMAND|MINOR_MODE|((x) << MAJOR_SHIFT))

/* prefix of cmd, macro, keymap, variable, and sometimes buffer structs */
typedef struct {
	int	Type;
	const char	*Name;
} data_obj;

extern const data_obj	*LastCmd;	/* last command invoked */

extern const char	*ProcFmt;	/* ": %f " -- name of LastCmd */

extern const data_obj
	*findcom proto((const char *prompt)),
	*findmac proto((const char *prompt)),
	*findvar proto((const char *prompt));
