/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

#ifdef IPROCS	/* the body is the rest of this file */

typedef struct process	*Process;	/* process reference (opaque type) */

# ifdef PIPEPROCS

extern char	Portsrv[FILESIZE];	/* path to portsrv program (in LibDir) */

extern int  NumProcs;

#  define KBDSIG		SIGUSR1	/* used for shoulder tapping */

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

extern void	kbd_strt(void);
extern jbool	kbd_stop(void);
extern void	read_pipe_proc(pid_t, int);
extern void	kbd_kill(void);

# else /* !PIPEPROCS */

extern void	read_pty_proc(int);

extern SIGRESTYPE
		sigchld_handler(int);
extern volatile jbool	
		procs_to_reap;
extern void	reap_procs(void);

# endif /* !PIPEPROCS */

extern void
	closeiprocs(void),
	untieDeadProcess(Buffer *);

extern jbool
	KillProcs(void);

extern const char
	*pstate(Process);

extern pid_t	DeadPid;	/* info about ChildPid, if reaped by kill_off */
extern wait_status_t	DeadStatus;

extern void
	kill_off(pid_t, wait_status_t);

/* Commands: */
extern void
	ProcInt(void),
	Iprocess(void),
	ShellProc(void),
	ProcQuit(void),
	ProcKill(void),
# ifdef PTYPROCS
	ProcCont(void),
	ProcDStop(void),
	ProcEof(void),
	ProcStop(void),
# endif
	ProcSendData(void),
	ProcNewline(void),
	ProcList(void),
	IprocEnvExport(void),
	IprocEnvShow(void),
	IprocEnvUnset(void);

/* Variables: */

extern char
	proc_prompt[128],	/* VAR: process prompt */
	dbx_parse_fmt[128];	/* VAR: dbx-mode parse string */

#endif /* IPROCS */
