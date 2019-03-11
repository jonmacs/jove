/***************************************************************************
 * This program is Copyright (C) 1986, 1987, 1988 by Jonathan Payne.  JOVE *
 * is provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is    *
 * included in all the files.                                              *
 ***************************************************************************/

#ifdef	IPROCS

/* typedef struct process Process in jove.h */

struct process {
	Process	*p_next;
#ifdef	PIPEPROCS
	int	p_toproc;	/* write end of pipe to process */
	pid_t	p_portpid,	/* pid of child (the portsrv) */
		p_pid;		/* pid of real child i.e. not portsrv */
#else
	int	p_fd;		/* file descriptor of pty? opened r/w */
	pid_t	p_pid;		/* pid of child (the shell) */
#endif
	Buffer	*p_buffer;	/* add output to end of this buffer */
	char	*p_name;	/* ... */
	char	p_state,	/* State */
		p_howdied,	/* Killed? or Exited? */
		p_reason;	/* If signaled, p_reason is the signal; else
				   it is the the exit code */
	Mark	*p_mark;	/* where output left us */
	char	p_dbx_mode;	/* whether to parse output for file/lineno
				   pairs */
};

#ifdef	PIPEPROCS

extern int  NumProcs;

#define KBDSIG		SIGUSR1	/* used for shoulder tapping */

/* Messages from kbd and portsrv to jove.
 * We depend on writes to a pipe being atomic.
 * This seems to be the case if the write is short enough (<4k?)
 * but not documented in the UNIX manuals.
 */

struct header {
	pid_t	pid;	/* sender */
	int	nbytes;	/* data length */
};

struct lump {
	struct header	header;
	char	data[128];	/* data being sent */
};

extern File	*ProcInput;
extern pid_t	kbd_pid;

extern void	kbd_strt proto((void));
extern bool	kbd_stop proto((void));
extern void	read_proc proto((pid_t, int));
extern void	kbd_kill proto((void));

#else

extern void	read_proc proto((int));
extern SIGRESTYPE	proc_child proto((int));

#endif

extern void
	KillProcs proto((void)),
	closeiprocs proto((void)),
	pbuftiedp proto((Buffer *));

extern char *
	pstate proto((Process *));

extern void
	kill_off proto((pid_t, wait_status_t));

/* Commands: */
extern void
	ProcInt proto((void)),
	DBXpoutput proto((void)),
	Iprocess proto((void)),
	ShellProc proto((void)),
	ProcQuit proto((void)),
	ProcKill proto((void)),
#  ifdef	PTYPROCS
	ProcCont proto((void)),
	ProcDStop proto((void)),
	ProcEof proto((void)),
	ProcStop proto((void)),
#  endif
	ProcSendData proto((void)),
	ProcNewline proto((void)),
	ProcList proto((void));

/* Variables: */

extern char
	proc_prompt[128],	/* process prompt */
	dbx_parse_fmt[128];	/* dbx-mode parse string */

#endif
