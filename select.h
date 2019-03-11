/***************************************************************************
 * This program is Copyright (C) 1986, 1987, 1988 by Jonathan Payne.  JOVE *
 * is provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is    *
 * included in all the files.                                              *
 ***************************************************************************/

#ifdef HAVE_SELECT_H
# include <sys/select.h>
#endif

#ifndef FD_SET		/* usually set in sys/types.h (AIX: sys/select.h) */

  typedef long fd_set;

# ifndef FD_SETSIZE
#  define FD_SETSIZE      32
# endif

# define FD_SET(fd, fdset)	(*(fdset) |= (1L << (fd)))
# define FD_CLR(fd, fdset)	(*(fdset) &= ~(1L << (fd))
# define FD_ISSET(fd, fdset)	(*(fdset) & (1L << (fd)))
# define FD_ZERO(fdset)		(*(fdset) = 0)

#endif /* FD_SET */
