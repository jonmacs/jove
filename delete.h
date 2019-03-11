/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

extern void
	del_char proto((int dir,int num,bool OK_kill)),
	reg_kill proto((LinePtr line2, int char2, bool dot_moved));

extern LinePtr
	reg_delete proto((LinePtr line1,int char1,LinePtr line2,int char2)),
	new_kill proto((void));

/* kill buffer */

#define NUMKILLS	16	/* number of kills saved in the kill ring */
extern LinePtr	killbuf[NUMKILLS];
extern int	killptr;	/* index of newest entry (if any) */

extern void DelKillRing proto((void));	/* delete newest entry */

/* Commands: */

extern void
	CopyRegion proto((void)),
	DelBlnkLines proto((void)),
	DelNChar proto((void)),
	DelNWord proto((void)),
	DelPChar proto((void)),
	DelPWord proto((void)),
	DelReg proto((void)),
	DelWtSpace proto((void));
