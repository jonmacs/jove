/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

/* tables defined in keys.txt/keys.c */

extern data_obj	*MainKeys[NCHARS];
extern data_obj	*EscKeys[NCHARS];
extern data_obj	*CtlxKeys[NCHARS];

#ifdef PCNONASCII
extern data_obj	*NonASCIIKeys[NCHARS];
#endif

#ifdef MAC			/* used in About Jove... */
#define F_MAINMAP '\001'
#define F_PREF1MAP '\002'
#define F_PREF2MAP '\003'
#endif

#define OTHER_CMD	0
#define ARG_CMD		1
#define LINECMD		2	/* so we can preserve screen col in moves */
#define KILLCMD		3	/* so we can merge kills */
#define YANKCMD		4	/* so we can do yank-pop (ESC Y) */
#define UNDOABLECMD	5	/* so we can do yank-pop to undo */
#define MOUSE_CMD	6	/* to detect other cmds when button is down */

extern int this_cmd;		/* ... */
extern int last_cmd;		/* last command ... to implement appending to kill buffer */

extern void InitKeymaps(void);
extern void dispatch(ZXchar c);
extern jbool IsPrefixChar(ZXchar c);

extern void	DelObjRef(data_obj *);

/* Commands: */
extern void Apropos(void);
extern void LBindAKey(void);
extern void LBindMac(void);
extern void LBindMap(void);
extern void BindAKey(void);
extern void BindMac(void);
extern void BindMap(void);
extern void DescBindings(void);
extern void DescCom(void);
extern void DescVar(void);
extern void KeyDesc(void);
extern void Unbound(void);

#ifdef IPROCS
extern void PBindAKey(void);
extern void PBindMac(void);
extern void PBindMap(void);
#endif
