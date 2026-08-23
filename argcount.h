/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

/* macros for getting at and setting the current argument count */

#define arg_value()		arg_count
#define arg_or_default(x)	(is_an_arg()? arg_count : (x))
#define set_arg_value(n)	{ arg_state = AS_NUMERIC; arg_count = (n); }
#define clr_arg_value()		{ arg_state = AS_NONE; arg_count = 1; }
#define is_an_arg()		(arg_state != AS_NONE)
#define is_non_minus_arg()		(arg_state != AS_NONE && arg_state != AS_NEGSIGN)

#define	save_arg(as,ac)	{ (ac) = arg_count; (as) = arg_state; }
#define	restore_arg(as,ac)	{ arg_count = (ac); arg_state = (as); }

extern void	negate_arg(void);

/* Commands: */

extern void
	Digit(void),
	DigitMinus(void),
	Digit0(void),
	Digit1(void),
	Digit2(void),
	Digit3(void),
	Digit4(void),
	Digit5(void),
	Digit6(void),
	Digit7(void),
	Digit8(void),
	Digit9(void),
	TimesFour(void);

/* private to macros */

extern int
	arg_state;	/* NO, YES, or YES_NODIGIT */
extern long
	arg_count;

#define	AS_NONE	0	/* no arg */
#define	AS_NUMERIC	1	/* numeric arg supplied */
#define	AS_NEGSIGN	2	/* only minus sign supplied */
#define	AS_TIMES	3	/* multiplicative request */
