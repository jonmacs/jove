/***************************************************************************
 * This program is Copyright (C) 1986, 1987, 1988 by Jonathan Payne.  JOVE *
 * is provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is    *
 * included in all the files.                                              *
 ***************************************************************************/

#define	C_UPPER	0001	/* UPPER case */
#define	C_LOWER	0002	/* LOWER case */
#define	C_DIGIT	0004	/* DIGIT */
#define	C_PUNCT	0010	/* PUNCTuation */
#define	C_CTRL	0020	/* ConTRoL */
#define	C_WORD	0040	/* WORD */
#define	C_BRA	0100	/* open BRAket */
#define	C_KET	0200	/* close braKET */

#define	has_syntax(c,s)	((CharTable[FUNDAMENTAL][c]&(s)) != 0)
#define	jisword(c)	has_syntax(c, C_WORD)
#define	jisalpha(c)	has_syntax(c, C_UPPER|C_LOWER)
#define	jisupper(c)	has_syntax(c, C_UPPER)
#define	jislower(c)	has_syntax(c, C_LOWER)
#define	jisdigit(c)	has_syntax(c, C_DIGIT)
#define	jisopenp(c)	has_syntax(c, C_BRA)
#define	jisclosep(c)	has_syntax(c, C_KET)

#define	jiswhite(c)	((c) == ' ' || (c) == '\t')	/* NOT isspace! */

extern bool	jisident proto((int));

/* #define	jtoascii(c)	((c)&CHARMASK) */
#define	jiscntrl(c)	(((CharTable[FUNDAMENTAL][(c)&CHARMASK]) & C_CTRL) != 0)

#ifdef	ASCII7
# define	jtolower(c)	((c)|040)
#else	/* !ASCII7 */
  extern int jtolower proto((int));
#endif	/* !ASCII7 */

extern const unsigned char	CharTable[NMAJORS][NCHARS];
extern const char	RaiseTable[NCHARS];
#define	CharUpcase(c)	(RaiseTable[c])
