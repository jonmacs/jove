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

char CaseEquiv[] = {
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
	'X',	'Y',	'Z',	'{',	'|',	'}',	'~',	'\177'
};

