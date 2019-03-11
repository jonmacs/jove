/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

#include "tune.h"
#include "paths.h"
#include <stdio.h>
#include <string.h>

#ifdef REALSTDC
# include <stdlib.h>
# define proto(x)        x
#else
# define proto(x)		()
extern char	*getenv proto((const char *));
#endif

/* A couple of bits stolen from externs.h: */

extern int	access proto((const char */*path*/, int /*mode*/));
#ifndef W_OK
#   define W_OK	2
#   define F_OK	0
#endif

# if !defined(ZTCDOS) && !defined(__BORLANDC__)
/* Zortech incorrectly defines argv as const char **.
 * Borland incorrectly defines argv as char *[] and omits some consts
 * on execl and execlp parameters.
 * On the other hand, each supplies declarations for these functions.
 */
extern int	execlp proto((const char */*file*/, const char */*arg*/, ...));
# endif


static const char	*ShareDir = SHAREDIR;

int
main(argc, argv)
int	argc;
char	*argv[];
{
	char
		cmd[FILESIZE*3],
		fname[FILESIZE],
		*home,
		teachjove[FILESIZE];

	if (argc == 3 && strcmp(argv[1], "-s") == 0) {
		if (strlen(argv[2]) >= FILESIZE*sizeof(char)) {
			printf("teachjove: -s argument too long\n");
			exit(-1);
		}
		ShareDir = argv[2];
	} else if (argc != 1) {
		printf("Usage: teachjove [-s sharedir]\n");
		exit(-1);
	}
	/* ??? "teach-jove" is too long for MSDOS */
	/* ??? should use snprintf, but not available in old C */
	(void) sprintf(teachjove, "%s/teach-jove", ShareDir);
	if ((home = getenv("HOME")) == NULL) {
		printf("teachjove: cannot find your home!\n");
		exit(-1);
	}
	/* ??? "teach-jove" is too long for MSDOS */
	(void) sprintf(fname, "%s/teach-jove", home);
	if (access(fname, F_OK) != 0) {
		(void) sprintf(cmd, "cp %s %s; chmod 644 %s", teachjove, fname, fname);
		system(cmd);
	}
	(void) execlp("jove", "teachjove", fname, (char *) NULL);
	printf("teachjove: cannot execl jove!\n");
	return 1;
}
