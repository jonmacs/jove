#ifdef UNIX
# ifdef TIOCSLTC
extern struct ltchars	ls1, ls2;
# endif /* TIOCSLTC */

# ifdef TIOCGETC
extern struct tchars	tc1, tc2;
# endif

# ifdef PASS8			/* use pass8 instead of raw for meta-key */
extern int	lmword1, lmword2;	/* local mode word */
# endif

# ifdef BRLUNIX
extern struct sg_brl	sg1, sg2;
# else
#  ifdef SYSV
extern struct termio	sg1, sg2;
#  else /* SYSV */
extern struct sgttyb	sg1, sg2;
#  endif /* SYSV */
# endif /* BRLUNIX */
#endif /* UNIX */
