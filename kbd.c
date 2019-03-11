#include "tune.h"
#include <signal.h>

#ifdef BSD4_2
# define pause()	sigpause(0)
#endif

struct header {
	int	pid,
		nbytes;
	char	buf[10];
};

#define HEADER_SIZE	(2 * sizeof (int))

/* JOVE sends SIGQUIT whenever it wants the kbd process (this program)
   to stop competing for input from the keyboard.  JOVE does this when
   JOVE realizes that there are no more interactive processes running.
   The reason we go through all this trouble is that JOVE slows down
   a lot when it's getting its keyboard input via a pipe. */

static int strt_read();

static int
hold_read()
{
	signal(SIGQUIT, strt_read);
	pause();
	return 0;	/* gotta return some int */
}

static int
strt_read()
{
	signal(SIGQUIT, hold_read);
	return 0;	/* gotta return some int */
}

int
main(argc, argv)
int	argc;
char	**argv;
{
	struct header	header;
	int	pid,
		n;

	signal(SIGINT, SIG_IGN);
	pid = getpid();
	header.pid = pid;

	hold_read();
	while ((n = read(0, header.buf, sizeof (header.buf))) > 0) {
		header.nbytes = n;
		write(1, &header, HEADER_SIZE + n);
	}
	return 0;
}
