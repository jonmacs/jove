/***************************************************************************
 * This program is Copyright (C) 1986, 1987, 1988 by Jonathan Payne.  JOVE *
 * is provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is    *
 * included in all the files.                                              *
 ***************************************************************************/

extern void
	del_char proto((int dir,int num,int OK_kill)),
	reg_kill proto((struct line *line2, int char2, bool dot_moved));

extern  struct line
	*reg_delete proto((struct line *line1,int char1,struct line *line2,int char2));

/* kill buffer */
#define NUMKILLS	10	/* number of kills saved in the kill ring */
extern Line	*killbuf[NUMKILLS];

extern int	killptr;	/* index into killbuf */

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
