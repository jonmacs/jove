/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

struct macro {
	/* Type and Name must match data_obj */
	int	Type;		/* in this case a macro */
	const char	*Name;		/* name is always second ... */
	int	m_len;		/* length of macro so we can use ^@ */
	char	*m_body;	/* actual body of the macro */
	struct macro	*m_nextm;
};

extern jbool
	InMacDefine;	/* are we defining a macro right now? */

extern struct macro	*macros;

extern jbool
	in_macro(void),
	ModMacs(void);

extern ZXchar
	mac_getc(void);

extern void
	mac_init(void),
	do_macro(struct macro *mac),
	unwind_macro_stack(void),
	mac_putc(DAPchar c),
	note_dispatch(void);

/* Commands: */
extern void
	DefKBDMac(void),
	ExecMacro(void),
	Forget(void),
	MacInter(void),
	NameMac(void),
	Remember(void),
	RunMacro(void),
	WriteMacs(void);

/* dataobj.h:
 *	findmac
 */
