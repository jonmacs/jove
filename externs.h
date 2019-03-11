/***************************************************************************
 * This program is Copyright (C) 1986, 1987, 1988 by Jonathan Payne.  JOVE *
 * is provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is    *
 * included in all the files.                                              *
 ***************************************************************************/

#ifdef TXT_TO_C
# define STDIO
#endif

extern int
	abs proto((int));

extern void
	exit proto((int)),
	_exit proto((int)),
	free proto((void *));

extern char
	*getenv proto((char *)),
	*tgoto proto((char *, int, int)),
	*calloc proto((unsigned int, unsigned int)),
	*malloc proto((unsigned int)),
	*realloc proto((void *, unsigned int)),

/* proc.c */
	*MakeName proto((char *command)),

/* ask.c */
	*ask proto((char *, char *, ...)),
	*ask_file proto((char *prmt, char *def, char *buf)),
	*do_ask proto((char *, int (*)(), char *, char *, ...));

extern int
	yes_or_no_p proto((char *, ...));

extern void
	minib_add proto((char *str,int movedown));

extern void
	isprocbuf proto((char *bufname)),
	DoAutoExec proto((char *new,char *old)),
	RegToUnix proto((struct buffer *outbuf,char *cmd));

extern int
/* ctype.h */
	ismword proto((int c)),

/* proc.c */
	UnixToBuf proto((char *,int ,int ,int , ...)),

/* extend.c */
	addgetc proto((void)),
	ask_int proto((char *prompt,int base)),
	aux_complete proto((int c)),
	chr_to_int proto((char *cp,int base,int allints, int *result)),
	complete proto((char * *possible,char *prompt,int flags)),
	joverc proto((char *file)),

/* jove.c */
	charp proto((void)),
	getch proto((void)),
	getchar proto((void)),
	getrawinchar proto((void));

extern void
	error proto((char *, ...)),
	complain proto((char *, ...)),
	confirm proto((char *, ...)),
	finish proto((int code)),
	ttyset proto((int n)),
	tty_reset proto ((void)),
	ToError proto((int forward)),
	Ungetc proto((int c));


#ifdef IBMPC
extern int
	lower proto((char *c)),
	rawkey_ready proto((void));
extern char
	switchar proto((void));
#endif

extern Bufpos
/* insert.c */
	*DoYank proto((struct line *fline,int fchar,struct line *tline,int tchar,struct line *atline,int atchar,struct buffer *whatbuf)),
	*lisp_indent proto((void));

extern void
	Insert proto((int c)),
	LineInsert proto((int num));

/* c.c */
extern Bufpos
	*c_indent proto((int brace)),
	*m_paren proto((int p_type,int dir,int can_mismatch,int can_stop));

extern void
	mp_error proto((void));

extern unsigned char
	chpl proto((void)),
	lpp proto((void));

extern void
	DoJustify proto((struct line *l1,int c1,struct line *l2,int c2,int scrunch,int indent)),

	/* insert.c */
	GCchunks proto((void)),

	/* fmt.c */
	add_mess proto((char *, ...)),
	f_mess proto((char *, ...)),
	format proto((char *buf,int len,char *fmt,char *ap)),
	fwritef proto((struct file *,char *, ...)),
	writef proto((char *, ...)),
	s_mess proto((char *, ...)),
	swritef proto((char *,char *, ...)),

	/* keymaps.c */
	InitKeymaps proto((void)),

	/* paragraph.c */
	do_rfill proto((int ulm)),

#ifndef MAC
	main proto((int argc,char * *argv)),
#endif
	/* misc.c */

	put_bufs proto((int askp)),

	/* term.c */
	putpad proto((char *str,int lines)),
	settout proto((char *ttbuf));

#if defined(IBMPC)
extern void
	write_em proto((char *s)),
	write_emc proto((char *s,int n)),
	write_emif proto(()),
	write_emif proto((char *s));
#endif

#ifdef MAC

extern int
	creat proto((char *,int)),
	open proto((char *,int)),
	close proto((int)),
	read proto((int,char *,unsigned)),
	write proto((int,char *,unsigned)),
	unlink proto((char *)),
	chdir proto((char *)),
	rawchkc proto((void)),
	getArgs proto((char ***));

extern long
	lseek proto((int,long,unsigned)),
 	time(long *);	

#endif /* MAC */

extern char
	*pwd proto((void)),
	*index proto((char *,int)),
	*mktemp proto((char *)),
	*rindex proto((char *,char)),
#ifdef MAC
	*getwd(),
#else
	*getwd proto((char *)),
#endif
	*pfile proto((char *)),
	*gfile proto((char *));

#ifdef MAC
extern void
	MacInit proto((void)),
	InitBinds proto((void)),
	NPlacur proto((int,int)),
	i_lines proto((int,int,int)),
	d_lines proto((int,int,int)),
	clr_page proto((void)),
	clr_eoln proto((void)),
	docontrols proto((void)),
	RemoveScrollBar proto((Window *)),
	InitEvents proto((void)),
	menus_on proto((void)),
	menus_off proto((void));
#endif
