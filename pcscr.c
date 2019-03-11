/***************************************************************************
 * This program is Copyright (C) 1986, 1987, 1988 by Jonathan Payne.  JOVE *
 * is provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is    *
 * included in all the files.                                              *
 ***************************************************************************/

#include "jove.h"

#ifdef	IBMPC

#include "pcscr.h"
#include "chars.h"
#include "screen.h"

/* here come the actual emulation routines	*/

#include <dos.h>
#include <conio.h>

private BYTE near get_mode proto((void));

private WORD
	near cur_page proto((void)),
	near get_cur proto((void));

private void
	near ch_out proto((BYTE, BYTE)),
	near clr_eop proto((void)),
	near cur_advance proto((void)),
	near cur_down proto((void)),
	near cur_left proto((void)),
	near cur_right proto((void)),
	near cur_up proto((void)),
	near line_feed proto((void)),
	near set_cur proto((WORD)),
	near set_mode proto((BYTE)),
	near wherexy proto((BYTE *, BYTE *));

#define VIDEO   0x10

#define intr(n, r)	int86((n), (r), (r));

BYTE CHPL=80,
     LPP=25,
     CUR_PAGE=0,
     C_ATTR = 0x07,
     C_X=0,
     C_Y=0;

int Fgcolor = 7,
    Bgcolor = 0,
	Mdcolor = 0;

void setcolor(fg, bg)
BYTE fg, bg;
{
   C_ATTR = ((bg&0xf)<<4)|(fg&0xf);
}

private
WORD near cur_page()
{
   union REGS vr;

   vr.h.ah = 0x0f;
   intr(VIDEO, &vr);
   return vr.h.bh;
}

private
void near set_cur(xy)
WORD xy;
{
   union REGS vr;

   vr.h.bh = CUR_PAGE;
   vr.h.ah = 0x02;
   vr.x.dx = xy;
   intr(VIDEO, &vr);
}

private
WORD near get_cur()
{
   union REGS vr;

   vr.h.bh = CUR_PAGE;
   vr.h.ah = 0x03;
   intr(VIDEO, &vr);
   return (vr.x.dx);
}

private BYTE near
get_mode()
{
  union REGS vr;

  vr.h.ah = 0x0f;
  intr(VIDEO, &vr);
  return vr.h.al;
}

BYTE
lpp()
{
   int far *regen = (int far *) 0x44C;
   BYTE chpl();

   return (*regen&0xff00)/2/chpl();
}

#ifdef	USE_PROTOTYPES
/* BYTE is subject to default argument promotions */
private void near
set_mode(BYTE n)
#else
private void near
set_mode(n)
BYTE n;
#endif
{
  union REGS vr;

  vr.h.ah = 0x00;
  vr.h.al = n;
  intr(VIDEO, &vr);
}

#define gotoxy(x,y)	set_cur((x)<<8|((y)&0xff))
#define cur_mov(x,y)	set_cur((C_X=(x))<<8|((C_Y=(y))&0xff))

private
void near
wherexy( x, y)
BYTE *x, *y;
{
  register WORD xy;

  xy = get_cur();
  *x = xy>>8;
  *y = xy&0xff;
}

#define wherex()	C_X
#define wherey()	C_Y

#ifdef	USE_PROTOTYPES
/* BYTE is subject to default argument promotions */
void /*near*/
scr_win(int no, BYTE ulr, BYTE ulc, BYTE lrr, BYTE lrc)
#else
void /*near*/
scr_win(no, ulr, ulc, lrr, lrc)
int no;
BYTE ulr, ulc, lrr, lrc;
#endif
{
  union REGS vr;

  if (no >= 0)
     vr.h.ah = 0x06;
  else {
     vr.h.ah = 0x07;
     no = - no;
  }
  vr.h.al = no;
  vr.x.cx = ulr<<8 | ulc;
  vr.x.dx = lrr<<8 | lrc;
  vr.h.bh = C_ATTR;
  intr(VIDEO, &vr);
}

BYTE
chpl()
{
  union REGS vr;

  vr.h.ah = 0x0f;
  intr(VIDEO, &vr);
  return vr.h.ah;
}

void /*near*/
clr_page()
{
	scr_win(0, 0, 0, LPP-1, CHPL-1);
	gotoxy(C_X = 0, C_Y = 0);
}

private void near
cur_right()
{
   if (C_Y < CHPL-1)
      C_Y++;
   gotoxy(C_X, C_Y);
}

private void near
cur_up()
{
   if (C_X)
      C_X--;
   gotoxy(C_X, C_Y);
}

private void near
cur_left()
{
   if (C_Y)
      C_Y--;
   gotoxy(C_X, C_Y);
}

private void near
cur_down()
{
   if (C_X < LPP-1)
      C_X++;
   gotoxy(C_X, C_Y);
}

#ifdef	USE_PROTOTYPES
/* BYTE is subject to default argument promotions */
private void near
ch_out(BYTE c, BYTE n)
#else
private void near
ch_out(c, n)
BYTE c, n;
#endif
{
  union REGS vr;

  vr.h.ah = 0x09;
  vr.h.al = c;
  vr.h.bl = C_ATTR;
  vr.h.bh = CUR_PAGE;
  vr.x.cx = n;
  intr(VIDEO, &vr);
}

#define wrch(c)		ch_out((c), 1), cur_advance()

#define home_cur()	gotoxy(C_X = 0, C_Y = 0)

void /*near*/
clr_eoln()
{
	ch_out(' ', CHPL-wherey());
}

private void near
clr_eop()
{
  clr_eoln();
  scr_win(LPP-1-wherex(), wherex()+1, 0, LPP-1, CHPL-1);
}

void
init_43()
{
   BYTE far *info = (BYTE far *) 0x487;
   WORD far *CRTC = (WORD far *) 0x463;
   union REGS vr;
   WORD cur;

   CUR_PAGE = cur_page();
   CHPL = chpl();
   LPP = lpp();

   if (get_mode()!=3)
      set_mode(3);
   cur = get_cur();

   vr.x.ax = 0x1112;
   vr.h.bl = 0;
   intr(VIDEO, &vr);

   *info |= 1;
   vr.x.ax = 0x0100;
   vr.h.bh = 0;
   vr.x.cx = 0x0600;
   intr(VIDEO, &vr);

   outp(*CRTC, 0x14);
   outp(*CRTC+1, 0x07);

   vr.x.ax = 0x1200;
   vr.h.bl = 0x20;
   intr(VIDEO, &vr);

   LPP = lpp();

   set_cur(cur);
   wherexy(&C_X, &C_Y);
}

void
reset_43()
{
   BYTE far *info = (BYTE far *) 0x487;
   WORD far *CRTC = (WORD far *) 0x463;
   union REGS vr;

   set_mode(3);

   *info &= 128;
   vr.x.ax = 0x0100;
   vr.h.bh = 0x0607;
   vr.x.cx = 0x0607;
   intr(VIDEO, &vr);

   outp(*CRTC, 0x14);
   outp(*CRTC+1, 13);

}

#define scr_up()		scr_win(1, 0, 0, LPP-1, CHPL-1)
#define back_space()	cur_left()

private void near
line_feed()
{
   if (++C_X > LPP-1) {
      C_X = LPP-1;
      scr_up();
   }
   gotoxy(C_X, C_Y);
}

#define BELL_P 0x61			/* speaker */
#define BELL_D 0x2dc			/* 550 hz  */
#define TIME_P 0x40			/* timer   */
#define TINI   182			/* 10110110b timer initialization */

void
dobell(x)
{
   unsigned int n = 0x8888;
   int orgval;

   outp(TIME_P+3, TINI);
   outp(TIME_P+2, BELL_D&0xff);
   outp(TIME_P+2, BELL_D>>8);
   orgval = inp(BELL_P);
   outp(BELL_P, orgval|3);		/* turn speaker on  */
   do ; while (--n > 0);
   outp(BELL_P, orgval);
}

#define carriage_return()	gotoxy(wherex(), C_Y = 0)

private void near
cur_advance()
{
   if (++C_Y > CHPL-1) {
      C_Y = 0;
      if (++C_X > LPP-1) {
	 scr_up();
	 C_X = LPP-1;
      }
   }
   gotoxy(C_X, C_Y);
}

void
init_term()
{
   if (lpp() == 43)
      reset_43();
   CUR_PAGE = cur_page();
   CHPL = chpl();
   LPP = lpp();
   wherexy(&C_X, &C_Y);
}

void
write_em(s)
char *s;
{
  while (*s)
	normfun(*s++);
}

void
write_emif(s)
char *s;
{
  if (s)
	 write_em(s);
}

void
write_emc(s, n)
char *s;
int n;
{
   while (n--)
	 normfun(*s++);
}

#ifdef	USE_PROTOTYPES
/* char is subject to default argument promotions */
void /*near*/
normfun(char c)
#else
void near
normfun(c)
char c;
#endif
{
	switch (c) {
	case LF: line_feed(); break;
	case CR: carriage_return(); break;
	case BS: back_space(); break;
	case CTL('G'): dobell(0); break;
	case '\0': break;
	default: wrch(c);
	}
}


/* No cursor optimization on an IBMPC, this simplifies things a lot.
   Think about it: it would be silly!
 */

int	phystab = 8;

void
Placur(line, col)
{
	cur_mov(line, col);
	CapCol = col;
	CapLine = line;
}

void
SO_on()
{
	if (Mdcolor)
		setcolor(Mdcolor&0xf, Mdcolor>>4);
	else
		setcolor(Bgcolor, Fgcolor);
}

void
SO_off()
{
   setcolor(Fgcolor, Bgcolor);
}

extern bool EGA;

void
pcUnsetTerm()
{
  if (EGA)
	 reset_43();
}


void
pcResetTerm()
{
	if (EGA)
	   init_43();
	else
	   init_term();
}

#endif	IBMPC
