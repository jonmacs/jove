/* proto: macro to allow us to prototype any function declaration
 * without upsetting old compilers.
 */

#ifdef	__STDC__
# define	REALSTDC	1
#else
#define	const	/*const*/
#endif

#ifdef	REALSTDC
# define    USE_PROTOTYPES  1
#endif

#ifdef	USE_PROTOTYPES
# define proto(x)        x
# ifdef	NO_PTRPROTO
   /* on these systems, a prototype cannot be used for a pointer to function */
#  define ptrproto(x)		()
# else
#  define ptrproto(x)		x
# endif
#else
# define proto(x)		()
# define ptrproto(x)		()
#endif

#define private static

extern	Tty	ttysw;

extern	Menu	main_menu;

#ifndef	SUNVIEW
extern	Xv_Font font;	/* ??? not in sunview */
#endif

extern void	menu_init proto((void));
