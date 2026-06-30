/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

extern void
	del_char(int dir, int num, jbool OK_kill),
	reg_kill(LinePtr line2, int char2, jbool dot_moved);

extern LinePtr
	reg_delete(LinePtr line1,int char1,LinePtr line2,int char2),
	new_kill(void);

/* kill buffer */

#define NUMKILLS	16	/* number of kills saved in the kill ring */
extern LinePtr	killbuf[NUMKILLS];
extern int	killptr;	/* index of newest entry (if any) */

extern void DelKillRing(void);	/* delete newest entry */

/* Commands: */

extern void
	CopyRegion(void),
	DelBlnkLines(void),
	DelNChar(void),
	DelNWord(void),
	DelPChar(void),
	DelPWord(void),
	DelReg(void),
	DelWtSpace(void);
