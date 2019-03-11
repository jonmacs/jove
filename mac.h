/************************************************************************
 * This program is Copyright (C) 1986-1994 by Jonathan Payne.  JOVE is  *
 * provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is *
 * included in all the files.                                           *
 ************************************************************************/

/* Macintosh related things. K. Mitchum 2/88 */

#ifdef MAC	/* the body is the rest of this file */

#define NMENUS 6
#define NMENUITEMS 40	/* This has GOT to be enough! */

typedef data_obj *menumap[NMENUITEMS];
struct menu {
	char *Name;
	int menu_id;
	MenuHandle Mn;
	menumap m;
};

struct stat {
	int st_dev;		/* volume number */
	long st_ino;		/* file number on volume */
	dev_t st_rdev;
	off_t st_size;		/* logical end of file */
	int st_mode;
	time_t st_mtime;	/* last modified */
};

#define S_IFDIR 2

typedef char *va_list;
#define va_dcl va_list va_alist;
#define va_start(l) { (l) = (va_list)&va_alist; }
#define va_arg(l,m) (((m*)((l) += sizeof(m)))[-1])
#define va_end(l) { (l) = NULL; }

extern void	MarkVar((struct variable *vp, int mnu, int itm));

extern jmp_buf auxjmp;

extern char
	*pfile proto((char *)),
	*gfile proto((char *));

extern int	getArgs proto((char ***));

extern bool	rawchkc proto((void));

extern void
	MacInit proto((void)),
	NPlacur proto((int, int)),
	i_lines proto((int, int, int)),
	d_lines proto((int, int, int)),
	clr_page proto((void)),
	clr_eoln proto((void)),
	docontrols proto((void)),
	RemoveScrollBar proto((Window *)),
	InitEvents proto((void)),
	menus_on proto((void));

extern bool
	Keyonly,
	Bufchange,
	Modechange,
	EventCmd,
	Windchange;

/* Variables: */

extern bool
	Macmode;	/* VAR: use Mac file selector */

#endif /* MAC */
