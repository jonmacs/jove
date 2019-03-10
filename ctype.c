/************************************************************************
 * This program is Copyright (C) 1986 by Jonathan Payne.  JOVE is       *
 * provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is *
 * included in all the files.                                           *
 ************************************************************************/

#include "jove.h"
#include "ctype.h"

int	SyntaxTable = FUNDAMENTAL;	/* Current table to use. */

char CharTable[NMAJORS][128] = {
{	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C,
	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C,
	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C,
	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C,
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P,
	_Op|_P,	_Cl|_P,	_P,	_P,	_P,	_P,	_P,	_P,
	_W|_N,	_W|_N,	_W|_N,	_W|_N,	_W|_N,	_W|_N,	_W|_N,	_W|_N,
	_W|_N,	_W|_N,	_P,	_P,	_P,	_P,	_P,	_P,
	_P,	_W|_U,	_W|_U,	_W|_U,	_W|_U,	_W|_U,	_W|_U,	_W|_U,
	_W|_U,	_W|_U,	_W|_U,	_W|_U,	_W|_U,	_W|_U,	_W|_U,	_W|_U,
	_W|_U,	_W|_U,	_W|_U,	_W|_U,	_W|_U,	_W|_U,	_W|_U,	_W|_U,
	_W|_U,	_W|_U,	_W|_U,	_Op|_P,	_P,	_Cl|_P,	_P,	_P,
	_P,	_W|_L,	_W|_L,	_W|_L,	_W|_L,	_W|_L,	_W|_L,	_W|_L,
	_W|_L,	_W|_L,	_W|_L,	_W|_L,	_W|_L,	_W|_L,	_W|_L,	_W|_L,
	_W|_L,	_W|_L,	_W|_L,	_W|_L,	_W|_L,	_W|_L,	_W|_L,	_W|_L,
	_W|_L,	_W|_L,	_W|_L,	_Op|_P,	_P,	_Cl|_P,	_P,	_C	},

{	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C,
	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C,
	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C,
	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C,
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P|_W,
	_Op|_P,	_Cl|_P,	_P,	_P,	_P,	_P,	_P,	_P,
	_W|_N,	_W|_N,	_W|_N,	_W|_N,	_W|_N,	_W|_N,	_W|_N,	_W|_N,
	_W|_N,	_W|_N,	_P,	_P,	_P,	_P,	_P,	_P,
	_P,	_W|_U,	_W|_U,	_W|_U,	_W|_U,	_W|_U,	_W|_U,	_W|_U,
	_W|_U,	_W|_U,	_W|_U,	_W|_U,	_W|_U,	_W|_U,	_W|_U,	_W|_U,
	_W|_U,	_W|_U,	_W|_U,	_W|_U,	_W|_U,	_W|_U,	_W|_U,	_W|_U,
	_W|_U,	_W|_U,	_W|_U,	_Op|_P,	_P,	_Cl|_P,	_P,	_P,
	_P,	_W|_L,	_W|_L,	_W|_L,	_W|_L,	_W|_L,	_W|_L,	_W|_L,
	_W|_L,	_W|_L,	_W|_L,	_W|_L,	_W|_L,	_W|_L,	_W|_L,	_W|_L,
	_W|_L,	_W|_L,	_W|_L,	_W|_L,	_W|_L,	_W|_L,	_W|_L,	_W|_L,
	_W|_L,	_W|_L,	_W|_L,	_Op|_P,	_P,	_Cl|_P,	_P,	_C	},

{	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C,
	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C,
	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C,
	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C,
	_P,	_P,	_P,	_P,	_P|_W,	_P,	_P,	_P,
	_Op|_P,	_Cl|_P,	_P,	_P,	_P,	_P,	_P,	_P,
	_W|_N,	_W|_N,	_W|_N,	_W|_N,	_W|_N,	_W|_N,	_W|_N,	_W|_N,
	_W|_N,	_W|_N,	_P,	_P,	_P,	_P,	_P,	_P,
	_P,	_W|_U,	_W|_U,	_W|_U,	_W|_U,	_W|_U,	_W|_U,	_W|_U,
	_W|_U,	_W|_U,	_W|_U,	_W|_U,	_W|_U,	_W|_U,	_W|_U,	_W|_U,
	_W|_U,	_W|_U,	_W|_U,	_W|_U,	_W|_U,	_W|_U,	_W|_U,	_W|_U,
	_W|_U,	_W|_U,	_W|_U,	_Op|_P,	_P,	_Cl|_P,	_P,	_P|_W,
	_P,	_W|_L,	_W|_L,	_W|_L,	_W|_L,	_W|_L,	_W|_L,	_W|_L,
	_W|_L,	_W|_L,	_W|_L,	_W|_L,	_W|_L,	_W|_L,	_W|_L,	_W|_L,
	_W|_L,	_W|_L,	_W|_L,	_W|_L,	_W|_L,	_W|_L,	_W|_L,	_W|_L,
	_W|_L,	_W|_L,	_W|_L,	_Op|_P,	_P,	_Cl|_P,	_P,	_C	
#ifndef LISP
}
#else
},

{	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C,
	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C,
	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C,
	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C,
	_P,	_W|_P,	_P,	_P,	_W|_P,	_W|_P,	_W|_P,	_P,
	_Op|_P,	_Cl|_P,	_W|_P,	_W|_P,	_P,	_W|_P,	_P,	_P,
	_W|_N,	_W|_N,	_W|_N,	_W|_N,	_W|_N,	_W|_N,	_W|_N,	_W|_N,
	_W|_N,	_W|_N,	_W|_P,	_P,	_W|_P,	_W|_P,	_W|_P,	_W|_P,
	_W|_P,	_W|_U,	_W|_U,	_W|_U,	_W|_U,	_W|_U,	_W|_U,	_W|_U,
	_W|_U,	_W|_U,	_W|_U,	_W|_U,	_W|_U,	_W|_U,	_W|_U,	_W|_U,
	_W|_U,	_W|_U,	_W|_U,	_W|_U,	_W|_U,	_W|_U,	_W|_U,	_W|_U,
	_W|_U,	_W|_U,	_W|_U,	_Op|_P,	_P,	_Cl|_P,	_W|_P,	_W|_P,
	_P,	_W|_L,	_W|_L,	_W|_L,	_W|_L,	_W|_L,	_W|_L,	_W|_L,
	_W|_L,	_W|_L,	_W|_L,	_W|_L,	_W|_L,	_W|_L,	_W|_L,	_W|_L,
	_W|_L,	_W|_L,	_W|_L,	_W|_L,	_W|_L,	_W|_L,	_W|_L,	_W|_L,
	_W|_L,	_W|_L,	_W|_L,	_Op|_W|_P,	_W|_P,	_Cl|_W|_P,	_W|_P,	_W|_C	},
#endif
};

ismword(c)
{
	return ((CharTable[curbuf->b_major])[c]&(_W));
}
