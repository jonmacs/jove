/************************************************************************
 * This program is Copyright (C) 1986-1994 by Jonathan Payne.  JOVE is  *
 * provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is *
 * included in all the files.                                           *
 ************************************************************************/

#include "jove.h"
#include "jctype.h"

#define	cU	C_UPPER	/* Upper case */
#define	cL	C_LOWER	/* Lower case */
#define	cN	C_DIGIT	/* Numeric */
#define	cP	C_PUNCT	/* Punctuation */
#define	cV	C_PRINT	/* printable (Visible) */
#define	cW	C_WORD	/* Word */
#define	cOp	C_BRA	/* Open Parenthesis */
#define	cCl	C_KET	/* Close Parenthesis */

const unsigned char CharTable[NMAJORS][NCHARS] = {
	/* FUNDAMENTAL mode */
    {
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	cP|cV,	cP|cV,	cP|cV,	cP|cV,	cP|cV,	cP|cV,	cP|cV,	cP|cV,
	cOp|cP|cV,	cCl|cP|cV,	cP|cV,	cP|cV,	cP|cV,	cP|cV,	cP|cV,	cP|cV,
	cW|cN|cV,	cW|cN|cV,	cW|cN|cV,	cW|cN|cV,	cW|cN|cV,	cW|cN|cV,	cW|cN|cV,	cW|cN|cV,
	cW|cN|cV,	cW|cN|cV,	cP|cV,	cP|cV,	cP|cV,	cP|cV,	cP|cV,	cP|cV,
	cP|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,
	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,
	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,
	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cOp|cP|cV,	cP|cV,	cCl|cP|cV,	cP|cV,	cP|cV,
	cP|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,
	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,
	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,
	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cOp|cP|cV,	cP|cV,	cCl|cP|cV,	cP|cV,	0,
#if NCHARS != 128
# ifdef IBMPC	/* code page 437 English */
	cW|cU|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV,
	cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cU|cV, cW|cU|cV,
	cW|cU|cV, cW|cL|cV, cW|cU|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV,
	cW|cL|cV, cW|cU|cV, cW|cU|cV, cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,
	cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cU|cV, cP|cV,    cP|cV,
	cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,
	cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,
	cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,
	cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,
	cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,
	cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,
	cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,
	cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,
	cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,
	cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,
	cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,
# else /* !IBMPC */
#  ifdef MAC	/* See Inside Macintosh Vol One p. 247 */
	cW|cU|cV, cW|cU|cV, cW|cU|cV, cW|cU|cV, cW|cU|cV, cW|cU|cV, cW|cU|cV, cW|cL|cV,
	cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV,
	cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV,
	cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV,
	cP|cV, cP|cV, cP|cV, cP|cV, cP|cV, cP|cV, cP|cV, cP|cV,
	cP|cV, cP|cV, cP|cV, cP|cV, cP|cV, cP|cV, cW|cU|cV, cW|cU|cV,
	cP|cV, cP|cV, cP|cV, cP|cV, cP|cV, cW|cU|cV, cW|cL|cV, cW|cU|cV,
	cW|cU|cV, cW|cL|cV, cP|cV, cP|cV, cP|cV, cW|cU|cV, cW|cL|cV, cW|cL|cV,
	cP|cV, cP|cV, cP|cV, cP|cV, cP|cV, cP|cV, cW|cU|cV, cP|cV,
	cP|cV, cP|cV, cP|cV, cW|cU|cV, cW|cU|cV, cW|cU|cV, cW|cU|cV, cW|cU|cV,
	cP|cV, cP|cV, cP|cV, cP|cV, cP|cV, cP|cV, cP|cV, cP|cV,
	cW|cU|cV, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
#  else /* !MAC */
	/* control, by default */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
#  endif /* !MAC */
# endif /* !IBMPC */
#endif /* NCHARS != 128 */
    },

	/* TEXT mode */
    {
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	cP|cV,	cP|cV,	cP|cV,	cP|cV,	cP|cV,	cP|cV,	cP|cV,	cP|cW|cV,
	cOp|cP|cV,	cCl|cP|cV,	cP|cV,	cP|cV,	cP|cV,	cP|cV,	cP|cV,	cP|cV,
	cW|cN|cV,	cW|cN|cV,	cW|cN|cV,	cW|cN|cV,	cW|cN|cV,	cW|cN|cV,	cW|cN|cV,	cW|cN|cV,
	cW|cN|cV,	cW|cN|cV,	cP|cV,	cP|cV,	cP|cV,	cP|cV,	cP|cV,	cP|cV,
	cP|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,
	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,
	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,
	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cOp|cP|cV,	cP|cV,	cCl|cP|cV,	cP|cV,	cP|cV,
	cP|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,
	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,
	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,
	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cOp|cP|cV,	cP|cV,	cCl|cP|cV,	cP|cV,	0,
#if NCHARS != 128
# ifdef IBMPC	/* code page 437 English */
	cW|cU|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV,
	cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cU|cV, cW|cU|cV,
	cW|cU|cV, cW|cL|cV, cW|cU|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV,
	cW|cL|cV, cW|cU|cV, cW|cU|cV, cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,
	cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cU|cV, cP|cV,    cP|cV,
	cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,
	cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,
	cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,
	cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,
	cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,
	cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,
	cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,
	cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,
	cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,
	cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,
	cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,    cP|cV,
# else /* !IBMPC */
#  ifdef MAC	/* See Inside Macintosh Vol One p. 247 */
	cW|cU|cV, cW|cU|cV, cW|cU|cV, cW|cU|cV, cW|cU|cV, cW|cU|cV, cW|cU|cV, cW|cL|cV,
	cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV,
	cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV,
	cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV, cW|cL|cV,
	cP|cV, cP|cV, cP|cV, cP|cV, cP|cV, cP|cV, cP|cV, cP|cV,
	cP|cV, cP|cV, cP|cV, cP|cV, cP|cV, cP|cV, cW|cU|cV, cW|cU|cV,
	cP|cV, cP|cV, cP|cV, cP|cV, cP|cV, cW|cU|cV, cW|cL|cV, cW|cU|cV,
	cW|cU|cV, cW|cL|cV, cP|cV, cP|cV, cP|cV, cW|cU|cV, cW|cL|cV, cW|cL|cV,
	cP|cV, cP|cV, cP|cV, cP|cV, cP|cV, cP|cV, cW|cU|cV, cP|cV,
	cP|cV, cP|cV, cP|cV, cW|cU|cV, cW|cU|cV, cW|cU|cV, cW|cU|cV, cW|cU|cV,
	cP|cV, cP|cV, cP|cV, cP|cV, cP|cV, cP|cV, cP|cV, cP|cV,
	cW|cU|cV, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
#  else /* !MAC */
	/* control, by default */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
#  endif /* !MAC */
# endif /* !IBMPC */
#endif /* NCHARS != 128 */
    },

	/* CMODE */
    {
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	cP|cV,	cP|cV,	cP|cV,	cP|cV,	cP|cW|cV,	cP|cV,	cP|cV,	cP|cV,
	cOp|cP|cV,	cCl|cP|cV,	cP|cV,	cP|cV,	cP|cV,	cP|cV,	cP|cV,	cP|cV,
	cW|cN|cV,	cW|cN|cV,	cW|cN|cV,	cW|cN|cV,	cW|cN|cV,	cW|cN|cV,	cW|cN|cV,	cW|cN|cV,
	cW|cN|cV,	cW|cN|cV,	cP|cV,	cP|cV,	cP|cV,	cP|cV,	cP|cV,	cP|cV,
	cP|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,
	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,
	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,
	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cOp|cP|cV,	cP|cV,	cCl|cP|cV,	cP|cV,	cP|cW|cV,
	cP|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,
	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,
	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,
	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cOp|cP|cV,	cP|cV,	cCl|cP|cV,	cP|cV,	0,
#if NCHARS != 128
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
#endif /* NCHARS != 128 */
    },

	/* LISP mode */
#ifdef LISP
    {
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	cP|cV,	cW|cP|cV,	cP|cV,	cP|cV,	cW|cP|cV,	cW|cP|cV,	cW|cP|cV,	cP|cV,
	cOp|cP|cV,	cCl|cP|cV,	cW|cP|cV,	cW|cP|cV,	cP|cV,	cW|cP|cV,	cP|cV,	cW|cV,
	cW|cN|cV,	cW|cN|cV,	cW|cN|cV,	cW|cN|cV,	cW|cN|cV,	cW|cN|cV,	cW|cN|cV,	cW|cN|cV,
	cW|cN|cV,	cW|cN|cV,	cW|cP|cV,	cP|cV,	cW|cP|cV,	cW|cP|cV,	cW|cP|cV,	cW|cP|cV,
	cW|cP|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,
	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,
	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,
	cW|cU|cV,	cW|cU|cV,	cW|cU|cV,	cOp|cP|cV,	cP|cV,	cCl|cP|cV,	cW|cP|cV,	cW|cP|cV,
	cP|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,
	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,
	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,
	cW|cL|cV,	cW|cL|cV,	cW|cL|cV,	cOp|cW|cP|cV,	cW|cP|cV,	cCl|cW|cP|cV,	cW|cP|cV,	cW|cV,
# if NCHARS != 128
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
# endif /* NCHARS != 128 */
    },
#endif /* LISP */
};

#undef	cU
#undef	cL
#undef	cN
#undef	cP
#undef	cV
#undef	cW
#undef	cOp
#undef	cCl

bool
jisident(c)
char	c;
{
#ifdef USE_CTYPE
	return (CharTable[curbuf->b_major][ZXC(c)] & C_WORD) != 0 || jisword(c);
#else
	return (CharTable[curbuf->b_major][ZXC(c)] & C_WORD) != 0;
#endif
}

#ifdef USE_CTYPE

# ifndef NO_SETLOCALE

#  include <locale.h>

char	LcCtype[32] = "";	/* VAR: lc-ctype, for use in setlocale */

/* adjust the locale to reflect possible change to LcCtype */

void
locale_adjust()
{
	char	*res = setlocale(LC_CTYPE, LcCtype);

	if (res != NULL) {
		/* success: if it fits, record result */
		if (strlen(res) < sizeof(LcCtype))
			strcpy(LcCtype, res);
	} else if (LcCtype[0] != '\0') {
		/* Failure, but not for "".  Complain, after recovering.
		 * Note: We don't try to print a message if "" fails because it
		 * might be the initializing call, too early for "complain",
		 * or it might be the recursive call.
		 */
		char	temp[sizeof(LcCtype)];

		strcpy(temp, LcCtype);
		res = setlocale(LC_CTYPE, (char *)NULL);
		if (res != NULL && strlen(res) < sizeof(LcCtype)) {
			strcpy(LcCtype, res);
		} else {
			LcCtype[0] = '\0';	/* default */
			locale_adjust();	/* note: this will recurse only one level */
		}
		complain("Unrecognized lc-ctype: %s", temp);
	}
}

# endif /* !NO_SETLOCALE */

#else /* !USE_CTYPE */

/* Map lower case characters to upper case and the rest to themselves. */

const char	RaiseTable[NCHARS] = {
	'\000',	'\001',	'\002',	'\003',	'\004',	'\005',	'\006',	'\007',
	'\010',	'\011',	'\012',	'\013',	'\014',	'\015',	'\016',	'\017',
	'\020',	'\021',	'\022',	'\023',	'\024',	'\025',	'\026',	'\027',
	'\030',	'\031',	'\032',	'\033',	'\034',	'\035',	'\036',	'\037',
	'\040',	'!',	'"',	'#',	'$',	'%',	'&',	'\'',
	'(',	')',	'*',	'+',	',',	'-',	'.',	'/',
	'0',	'1',	'2',	'3',	'4',	'5',	'6',	'7',
	'8',	'9',	':',	';',	'<',	'=',	'>',	'?',
	'@',	'A',	'B',	'C',	'D',	'E',	'F',	'G',
	'H',	'I',	'J',	'K',	'L',	'M',	'N',	'O',
	'P',	'Q',	'R',	'S',	'T',	'U',	'V',	'W',
	'X',	'Y',	'Z',	'[',	'\\',	']',	'^',	'_',
	'`',	'A',	'B',	'C',	'D',	'E',	'F',	'G',
	'H',	'I',	'J',	'K',	'L',	'M',	'N',	'O',
	'P',	'Q',	'R',	'S',	'T',	'U',	'V',	'W',
	'X',	'Y',	'Z',	'{',	'|',	'}',	'~',	'\177',
# if NCHARS != 128
#  ifdef IBMPC
	/* Only codes changed are lowercase Umlauted letters (indented):
	 *	Ae '\216'; ae '\204'
	 *	Oe '\231'; oe '\224'
	 *	Ue '\232'; ue '\201'
	 */
	'\200',	 '\232',	'\202',	'\203',	 '\216',	'\205',	'\206',	'\207',
	'\210',	'\211',	'\212',	'\213',	'\214',	'\215',	'\216',	'\217',
	'\220',	'\221',	'\222',	'\223',	 '\231',	'\225',	'\226',	'\227',
	'\230',	'\231',	'\232',	'\233',	'\234',	'\235',	'\236',	'\237',
	'\240',	'\241',	'\242',	'\243',	'\244',	'\245',	'\246',	'\247',
	'\250',	'\251',	'\252',	'\253',	'\254',	'\255',	'\256',	'\257',
	'\260',	'\261',	'\262',	'\263',	'\264',	'\265',	'\266',	'\267',
	'\270',	'\271',	'\272',	'\273',	'\274',	'\275',	'\276',	'\277',
	'\300',	'\301',	'\302',	'\303',	'\304',	'\305',	'\306',	'\307',
	'\310',	'\311',	'\312',	'\313',	'\314',	'\315',	'\316',	'\317',
	'\320',	'\321',	'\322',	'\323',	'\324',	'\325',	'\326',	'\327',
	'\330',	'\331',	'\332',	'\333',	'\334',	'\335',	'\336',	'\337',
	'\340',	'\341',	'\342',	'\343',	'\344',	'\345',	'\346',	'\347',
	'\350',	'\351',	'\352',	'\353',	'\354',	'\355',	'\356',	'\357',
	'\360',	'\361',	'\362',	'\363',	'\364',	'\365',	'\366',	'\367',
	'\370',	'\371',	'\372',	'\373',	'\374',	'\375',	'\376',	'\377',
#  else /*!IBMPC*/
#   ifdef MAC
	/* '\230' -> '\313'
	 * '\212' -> '\200'
	 * '\213' -> '\314'
	 * '\214' -> '\201'
	 * '\215' -> '\202'
	 * '\216' -> '\203'
	 * '\226' -> '\204'
	 * '\232' -> '\205'
	 * '\233' -> '\315'
	 * '\237' -> '\206'
	 * '\271' -> '\270'
	 * '\317' -> '\316'
	 */
	'\200',	'\201',	'\202',	'\203',	'\204',	'\205',	'\206',	'\207',
	 '\313',	'\211',	 '\200',	 '\314',	 '\201',	 '\202',	 '\203',	'\217',
	'\220',	'\221',	'\222',	'\223',	'\224',	'\225',	 '\204',	'\227',
	'\230',	'\231',	 '\205',	 '\315',	'\234',	'\235',	'\236',	 '\206',
	'\240',	'\241',	'\242',	'\243',	'\244',	'\245',	'\246',	'\247',
	'\250',	'\251',	'\252',	'\253',	'\254',	'\255',	'\256',	'\257',
	'\260',	'\261',	'\262',	'\263',	'\264',	'\265',	'\306',	'\267',
	'\270',	 '\270',	'\272',	'\273',	'\274',	'\275',	'\256',	'\257',
	'\300',	'\301',	'\302',	'\303',	'\304',	'\305',	'\306',	'\307',
	'\310',	'\311',	'\312',	'\313',	'\314',	'\315',	'\316',	 '\316',
	'\320',	'\321',	'\322',	'\323',	'\324',	'\325',	'\326',	'\327',
	'\330',	'\331',	'\332',	'\333',	'\334',	'\335',	'\336',	'\337',
	'\340',	'\341',	'\342',	'\343',	'\344',	'\345',	'\346',	'\347',
	'\350',	'\351',	'\352',	'\353',	'\354',	'\355',	'\356',	'\357',
	'\360',	'\361',	'\362',	'\363',	'\364',	'\365',	'\366',	'\367',
	'\370',	'\371',	'\372',	'\373',	'\374',	'\375',	'\376',	'\377',
#   else /*!MAC*/
	/* identity, by default */
	'\200',	'\201',	'\202',	'\203',	'\204',	'\205',	'\206',	'\207',
	'\210',	'\211',	'\212',	'\213',	'\214',	'\215',	'\216',	'\217',
	'\220',	'\221',	'\222',	'\223',	'\224',	'\225',	'\226',	'\227',
	'\230',	'\231',	'\232',	'\233',	'\234',	'\235',	'\236',	'\237',
	'\240',	'\241',	'\242',	'\243',	'\244',	'\245',	'\246',	'\247',
	'\250',	'\251',	'\252',	'\253',	'\254',	'\255',	'\256',	'\257',
	'\260',	'\261',	'\262',	'\263',	'\264',	'\265',	'\266',	'\267',
	'\270',	'\271',	'\272',	'\273',	'\274',	'\275',	'\276',	'\277',
	'\300',	'\301',	'\302',	'\303',	'\304',	'\305',	'\306',	'\307',
	'\310',	'\311',	'\312',	'\313',	'\314',	'\315',	'\316',	'\317',
	'\320',	'\321',	'\322',	'\323',	'\324',	'\325',	'\326',	'\327',
	'\330',	'\331',	'\332',	'\333',	'\334',	'\335',	'\336',	'\337',
	'\340',	'\341',	'\342',	'\343',	'\344',	'\345',	'\346',	'\347',
	'\350',	'\351',	'\352',	'\353',	'\354',	'\355',	'\356',	'\357',
	'\360',	'\361',	'\362',	'\363',	'\364',	'\365',	'\366',	'\367',
	'\370',	'\371',	'\372',	'\373',	'\374',	'\375',	'\376',	'\377',
#   endif /*!MAC*/
#  endif /* !IBMPC*/
# endif /* NCHARS != 128*/
};

/* Map upper case characters to lower case and the rest to themselves. */

const char	LowerTable[NCHARS] = {
	'\000',	'\001',	'\002',	'\003',	'\004',	'\005',	'\006',	'\007',
	'\010',	'\011',	'\012',	'\013',	'\014',	'\015',	'\016',	'\017',
	'\020',	'\021',	'\022',	'\023',	'\024',	'\025',	'\026',	'\027',
	'\030',	'\031',	'\032',	'\033',	'\034',	'\035',	'\036',	'\037',
	'\040',	'!',	'"',	'#',	'$',	'%',	'&',	'\'',
	'(',	')',	'*',	'+',	',',	'-',	'.',	'/',
	'0',	'1',	'2',	'3',	'4',	'5',	'6',	'7',
	'8',	'9',	':',	';',	'<',	'=',	'>',	'?',
	'@',	'a',	'b',	'c',	'd',	'e',	'f',	'g',
	'h',	'i',	'j',	'k',	'l',	'm',	'n',	'o',
	'p',	'q',	'r',	's',	't',	'u',	'v',	'w',
	'x',	'y',	'z',	'[',	'\\',	']',	'^',	'_',
	'`',	'a',	'b',	'c',	'd',	'e',	'f',	'g',
	'h',	'i',	'j',	'k',	'l',	'm',	'n',	'o',
	'p',	'q',	'r',	's',	't',	'u',	'v',	'w',
	'x',	'y',	'z',	'{',	'|',	'}',	'~',	'\177',
# if NCHARS != 128
#  ifdef IBMPC
	/* Only codes changed are uppercase Umlauted letters (indented):
	 *	Ae '\216'; ae '\204'
	 *	Oe '\231'; oe '\224'
	 *	Ue '\232'; ue '\201'
	 */
	'\200',	'\201',	'\202',	'\203',	'\204',	'\205',	'\206',	'\207',
	'\210',	'\211',	'\212',	'\213',	'\214',	'\215',	 '\204',	'\217',
	'\220',	'\221',	'\222',	'\223',	'\224',	'\225',	'\226',	'\227',
	'\230',	 '\224',	 '\201',	'\233',	'\234',	'\235',	'\236',	'\237',
	'\240',	'\241',	'\242',	'\243',	'\244',	'\245',	'\246',	'\247',
	'\250',	'\251',	'\252',	'\253',	'\254',	'\255',	'\256',	'\257',
	'\260',	'\261',	'\262',	'\263',	'\264',	'\265',	'\266',	'\267',
	'\270',	'\271',	'\272',	'\273',	'\274',	'\275',	'\276',	'\277',
	'\300',	'\301',	'\302',	'\303',	'\304',	'\305',	'\306',	'\307',
	'\310',	'\311',	'\312',	'\313',	'\314',	'\315',	'\316',	'\317',
	'\320',	'\321',	'\322',	'\323',	'\324',	'\325',	'\326',	'\327',
	'\330',	'\331',	'\332',	'\333',	'\334',	'\335',	'\336',	'\337',
	'\340',	'\341',	'\342',	'\343',	'\344',	'\345',	'\346',	'\347',
	'\350',	'\351',	'\352',	'\353',	'\354',	'\355',	'\356',	'\357',
	'\360',	'\361',	'\362',	'\363',	'\364',	'\365',	'\366',	'\367',
	'\370',	'\371',	'\372',	'\373',	'\374',	'\375',	'\376',	'\377',
#  else /*!IBMPC*/
#   ifdef MAC
	/* '\200' -> '\212'
	 * '\201' -> '\214'
	 * '\202' -> '\215'
	 * '\203' -> '\216'
	 * '\204' -> '\226'
	 * '\205' -> '\232'
	 * '\206' -> '\237'
	 * '\270' -> '\271'
	 * '\313' -> '\230'
	 * '\314' -> '\213'
	 * '\315' -> '\233'
	 * '\316' -> '\317'
	 */
	 '\212',	 '\214',	 '\215',	 '\216',	 '\226',	 '\232',	 '\237',	'\207',
	'\210',	'\211',	'\212',	'\213',	'\214',	'\215',	'\216',	'\217',
	'\220',	'\221',	'\222',	'\223',	'\224',	'\225',	'\226',	'\227',
	'\230',	'\231',	'\232',	'\233',	'\234',	'\235',	'\236',	'\237',
	'\240',	'\241',	'\242',	'\243',	'\244',	'\245',	'\246',	'\247',
	'\250',	'\251',	'\252',	'\253',	'\254',	'\255',	'\256',	'\257',
	'\260',	'\261',	'\262',	'\263',	'\264',	'\265',	'\266',	'\267',
	 '\271',	'\271',	'\272',	'\273',	'\274',	'\275',	'\276',	'\277',
	'\300',	'\301',	'\302',	'\303',	'\304',	'\305',	'\306',	'\307',
	'\310',	'\311',	'\312',	 '\230',	 '\213',	 '\233',	 '\317',	'\317',
	'\320',	'\321',	'\322',	'\323',	'\324',	'\325',	'\326',	'\327',
	'\330',	'\331',	'\332',	'\333',	'\334',	'\335',	'\336',	'\337',
	'\340',	'\341',	'\342',	'\343',	'\344',	'\345',	'\346',	'\347',
	'\350',	'\351',	'\352',	'\353',	'\354',	'\355',	'\356',	'\357',
	'\360',	'\361',	'\362',	'\363',	'\364',	'\365',	'\366',	'\367',
	'\370',	'\371',	'\372',	'\373',	'\374',	'\375',	'\376',	'\377',
#   else /*!MAC*/
	/* identity, by default */
	'\200',	'\201',	'\202',	'\203',	'\204',	'\205',	'\206',	'\207',
	'\210',	'\211',	'\212',	'\213',	'\214',	'\215',	'\216',	'\217',
	'\220',	'\221',	'\222',	'\223',	'\224',	'\225',	'\226',	'\227',
	'\230',	'\231',	'\232',	'\233',	'\234',	'\235',	'\236',	'\237',
	'\240',	'\241',	'\242',	'\243',	'\244',	'\245',	'\246',	'\247',
	'\250',	'\251',	'\252',	'\253',	'\254',	'\255',	'\256',	'\257',
	'\260',	'\261',	'\262',	'\263',	'\264',	'\265',	'\266',	'\267',
	'\270',	'\271',	'\272',	'\273',	'\274',	'\275',	'\276',	'\277',
	'\300',	'\301',	'\302',	'\303',	'\304',	'\305',	'\306',	'\307',
	'\310',	'\311',	'\312',	'\313',	'\314',	'\315',	'\316',	'\317',
	'\320',	'\321',	'\322',	'\323',	'\324',	'\325',	'\326',	'\327',
	'\330',	'\331',	'\332',	'\333',	'\334',	'\335',	'\336',	'\337',
	'\340',	'\341',	'\342',	'\343',	'\344',	'\345',	'\346',	'\347',
	'\350',	'\351',	'\352',	'\353',	'\354',	'\355',	'\356',	'\357',
	'\360',	'\361',	'\362',	'\363',	'\364',	'\365',	'\366',	'\367',
	'\370',	'\371',	'\372',	'\373',	'\374',	'\375',	'\376',	'\377',
#   endif /*!MAC*/
#  endif /* !IBMPC*/
# endif /* NCHARS != 128*/
};

#endif /* !USE_CTYPE */
