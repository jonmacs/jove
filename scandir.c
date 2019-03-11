/***************************************************************************
 * This program is Copyright (C) 1986, 1987, 1988 by Jonathan Payne.  JOVE *
 * is provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is    *
 * included in all the files.                                              *
 ***************************************************************************/

/*
 * This file is used as a compiled module by Jove and also included as
 * source in recover.c
 */
#ifndef TUNED
# include "jove.h"
#endif
#include "scandir.h"

#ifdef	F_COMPLETION

#ifdef	UNIX

# include <sys/stat.h>

# ifdef	M_XENIX
#  include <sys/ndir.h>
#  ifndef dirent
#   define dirent direct
#  endif
# endif

# ifdef BSD_DIR
#  include <sys/dir.h>
#  ifndef dirent
#   define dirent direct
#  endif
# endif

/* default to dirent.h */
# if !defined(dirent) && !defined(DIRENT_EMULATE)
#  include <dirent.h>
# endif

# ifdef DIRENT_EMULATE

typedef struct {
	int	d_fd;		/* File descriptor for this directory */
} DIR;

private int
closedir(dp)
DIR	*dp;
{
	(void) close(dp->d_fd);
	free((UnivPtr) dp);
	return 0;   /* don't know how to fail */
}

private DIR *
opendir(dir)
char	*dir;
{
	DIR	*dp = (DIR *) emalloc(sizeof *dp);
	struct stat	stbuf;

	if ((dp->d_fd = open(dir, 0)) == -1) {
		/* this doesn't exist */
	} else if ((fstat(dp->d_fd, &stbuf) == -1) || !(stbuf.st_mode & S_IFDIR)) {
		/* this isn't a directory! */
		(void) close(dp->d_fd);
	} else {
		return dp;
	}
	free((UnivPtr) dp);
	return NULL;
}

private dirent *
readdir(dp)
DIR	*dp;
{
	static dirent	dir;

	do {
		if (read(dp->d_fd, (UnivPtr) &dir, sizeof dir) != sizeof dir)
			return NULL;
	} while (dir.d_ino == 0);

	return &dir;
}

#endif	/* DIRENT_EMULATE */

/* jscandir returns the number of entries or -1 if the directory cannot
   be opened or malloc fails. */

int
jscandir(dir, nmptr, qualify, sorter)
char	*dir;
char	***nmptr;
bool	(*qualify) proto((char *));
int	(*sorter) proto((UnivConstPtr, UnivConstPtr));
{
	DIR	*dirp;
	struct  dirent	*entry;
	char	**ourarray;
	unsigned int	nalloc = 10,
			nentries = 0;

	if ((dirp = opendir(dir)) == NULL)
		return -1;
	ourarray = (char **) emalloc(nalloc * sizeof (char *));
	while ((entry = readdir(dirp)) != NULL) {
		if (qualify != NULL && !(*qualify)(entry->d_name))
			continue;
		/* note: test ensures one space left in ourarray for NULL */
		if (nentries+1 == nalloc)
			ourarray = (char **) erealloc((UnivPtr) ourarray, (nalloc += 10) * sizeof (char *));
		ourarray[nentries++] = copystr(entry->d_name);
	}
	closedir(dirp);
	ourarray[nentries] = NULL;

	if (sorter != NULL)
		qsort((UnivPtr) ourarray, nentries, sizeof (char **), sorter);
	*nmptr = ourarray;

	return nentries;
}

#endif	/* UNIX */

#ifdef	MSDOS
# include <dos.h>

# ifndef ZORTECH
#  include <search.h>
# endif

/* Scandir returns the number of entries or -1 if the directory cannot
   be opened or malloc fails. */

bool	MatchDir = NO;

int
jscandir(dir, nmptr, qualify, sorter)
char	*dir;
char	***nmptr;
bool	(*qualify) proto((char *));
int	(*sorter) proto((UnivConstPtr, UnivConstPtr));
{
	struct find_t entry;
	char	**ourarray;
	unsigned int	nalloc = 10,
			nentries = 0;

	{
		char dirname[FILESIZE];
		char *ptr;

		strcpy(dirname, dir);
		ptr = &dirname[strlen(dirname)-1];
		if (!((dirname[1] == ':' && dirname[2] == '\0') || *ptr == '/' || *ptr == '\\'))
			*++ptr = '/';
		strcpy(ptr+1, "*.*");

		if (_dos_findfirst(dirname, MatchDir? _A_SUBDIR : _A_NORMAL|_A_RDONLY|_A_HIDDEN|_A_SUBDIR, &entry))
		   return -1;
	}
	ourarray = (char **) emalloc(nalloc * sizeof (char *));
	do  {
		char filename[FILESIZE];

		if (MatchDir && (entry.attrib&_A_SUBDIR) == 0)
			continue;
		strlwr(entry.name);
		if (qualify != NULL && !(*qualify)(entry.name))
			continue;
		/* note: test ensures one space left in ourarray for NULL */
		if (nentries+1 == nalloc)
			ourarray = (char **) erealloc((char *) ourarray, (nalloc += 10) * sizeof (char *));
		strcpy(filename, entry.name);
		if ((entry.attrib&_A_SUBDIR) != 0)
			strcat(filename, "/");
		ourarray[nentries++] = copystr(filename);
	} while (_dos_findnext(&entry) == 0);
	ourarray[nentries] = NULL;

	if (sorter != (int (*)())NULL)
		qsort((char *) ourarray, nentries, sizeof (char **), sorter);
	*nmptr = ourarray;

	return nentries;
}

#endif	/* MSDOS */

void
freedir(nmptr, nentries)
char	***nmptr;
int	nentries;
{
	char	**ourarray = *nmptr;

	while (--nentries >= 0)
		free((UnivPtr) *ourarray++);
	free((UnivPtr) *nmptr);
	*nmptr = NULL;
}

int
alphacomp(a, b)
UnivConstPtr	a,
	b;
{
	return strcmp(*(const char **)a, *(const char **)b);
}
#endif /* F_COMPLETION */
