/************************************************************************
 * This program is Copyright (C) 1986 by Jonathan Payne.  JOVE is       *
 * provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is *
 * included in all the files.                                           *
 ************************************************************************/

#include "jove.h"
#include <varargs.h>

#ifdef IPROCS

int	proc_child();

#ifdef PIPEPROCS
#   include "iproc-pipes.c"
#else
#   include "iproc-ptys.c"
#endif

char	proc_prompt[80] = "% ";

KillProcs()
{
	register Process	*p;
	register int	killem = -1;		/* -1 means undetermined */
	register char	*yorn;

	for (p = procs; p != 0; p = p->p_next)
		if (!isdead(p)) {
			if (killem == -1) {
				yorn = ask("y", "Should I kill your i-processes? ");
				killem = (Upper(*yorn) == 'Y');
			}
			if (killem)
				proc_kill(p, SIGKILL);
		}
}

pbuftiedp(b)
register Buffer	*b;
{
	register Process	*p = b->b_process;

	if (!isdead(p))
		complain("Process %s, attached to %b, is %s.",
			 proc_cmd(p), b, pstate(p));
}

/* Process receive: receives the characters in buf, and appends them to
   the buffer associated with p. */

static
proc_rec(p, buf)
register Process	*p;
char	*buf;
{
	Buffer	*saveb = curbuf;
	register Window	*w;
	register Mark	*savepoint;
	int	sameplace = 0,
		do_disp = 0;

	if (curwind->w_bufp == p->p_buffer)
		w = curwind;
	else
		w = windbp(p->p_buffer);	/* Is this window visible? */
	if (w != 0)
		do_disp = (in_window(w, p->p_mark->m_line) != -1);
	SetBuf(p->p_buffer);
	savepoint = MakeMark(curline, curchar, FLOATER);
	ToMark(p->p_mark);	/* Where output last stopped. */
	if (savepoint->m_line == curline && savepoint->m_char == curchar)
		sameplace++;

	ins_str(buf, YES);
	if (do_disp) {
		w->w_line = curline;
		w->w_char = curchar;
		redisplay();
	}
	MarkSet(p->p_mark, curline, curchar);
	if (!sameplace)
		ToMark(savepoint);	/* Back to where we were. */
	DelMark(savepoint);
	SetBuf(saveb);
}

proc_kill(p, sig)
register Process	*p;
{
	if (isdead(p))
		return;
	if (killpg(p->p_pid, sig) == -1)
		s_mess("Cannot kill %s!", proc_buf(p));
}

/* Deal with a process' death.  proc_rec turns on the FREEUP bit when it
   it gets the "EOF" from portsrv.  FREEUP'd processes get unlinked from
   the list, and the proc stucture and proc_buf(p) get free'd up, here. */

private
DealWDeath()
{
	register Process	*p,
				*next,
				*prev = 0;
	
	for (p = procs; p != 0; p = next) {
		next = p->p_next;
		if (!p->p_eof) {
			prev = p;
			continue;
		}
		proc_close(p);
		PopPBs();			/* not a process anymore */
		p->p_buffer->b_process = 0;	/* we're killing ourself */
		free((char *) p->p_name);
		free((char *) p);
		if (prev)
			prev->p_next = next;
		else
			procs = next;
	}
}

ProcList()
{
	register Process	*p;
	char	*fmt = "%-15s  %-15s  %-8s %s",
		pidstr[10];

	if (procs == 0) {
		message("[No subprocesses]");
		return;
	}
	TOstart("Process list", TRUE);

	Typeout(fmt, "Buffer", "Status", "Pid ", "Command");
	Typeout(fmt, "------", "------", "--- ", "-------");
	for (p = procs; p != 0; p = p->p_next) {
		sprintf(pidstr, "%d", p->p_pid);
		Typeout(fmt, proc_buf(p), pstate(p), pidstr, p->p_name);
	}
	DealWDeath();
	TOstop();
}

ProcNewline()
{
	SendData(YES);
}

ProcSendData()
{
	SendData(NO);
}

private
SendData(newlinep)
{
	register Process	*p = curbuf->b_process;

	if (isdead(p))
		return;
	if (lastp(curline)) {
		Eol();
		if (newlinep)
			LineInsert(1);
		do_rtp(p->p_mark);
		MarkSet(p->p_mark, curline, curchar);
	} else {
		Bol();
		while (LookingAt(proc_prompt, linebuf, curchar))
			SetDot(dosearch(proc_prompt, 1, 1));
		strcpy(genbuf, linebuf + curchar);
		ToLast();
		ins_str(genbuf, NO);
	}
}

ShellProc()
{
	char	*shbuf = "*shell*";
	register Buffer	*b;

	b = buf_exists(shbuf);
	if (b == 0 || isdead(b->b_process))
		proc_strt(shbuf, NO, Shell, "-i", (char *) 0);
	pop_wind(shbuf, NO, -1);
}

Iprocess()
{
	extern char	ShcomBuf[100],
			*MakeName();
	register char	*command;

	command = ask(ShcomBuf, ProcFmt);
	null_ncpy(ShcomBuf, command, (sizeof ShcomBuf) - 1);
	proc_strt(MakeName(command), YES, Shell, ShFlags, command, (char *) 0);
}

proc_child()
{
	union wait	w;
	register int	pid;

	for (;;) {
#ifndef VMUNIX
		pid = wait2(&w.w_status, (WNOHANG | WUNTRACED));
#else
		pid = wait3(&w, (WNOHANG | WUNTRACED), (struct rusage *) 0);
#endif
		if (pid <= 0)
			break;
		kill_off(pid, w);
	}
}

kill_off(pid, w)
register int	pid;
union wait	w;
{
	char	str[128];
	register Process	*child;

	if ((child = proc_pid(pid)) == 0)
		return;

	if (WIFSTOPPED(w))
		child->p_state = STOPPED;
	else {
		child->p_state = DEAD;
		if (WIFEXITED(w))
			child->p_howdied = EXITED;
		else if (WIFSIGNALED(w)) {
			child->p_reason = w.w_termsig;
			child->p_howdied = KILLED;
		}
		proc_close(child);
	}
	sprintf(str, "[Process %s: %s]\n",
		proc_cmd(child),
		pstate(child));
	proc_rec(child, str);
}

/* Push/pod process bindings.  I openly acknowledge that this is a
   kludge, but I can't be bothered making it right. */

struct proc_bind {
	int		pb_key;
	data_obj	**pb_map;
	data_obj	*pb_push;
	data_obj	*pb_cmd;
	struct proc_bind *pb_next;
};

struct proc_bind *PBinds = 0;

PopPBs()
{
	register struct proc_bind *p;

	for (p = PBinds; p != 0; p = p->pb_next)
		p->pb_map[p->pb_key] = p->pb_push;
}

PushPBs()
{
	register struct proc_bind *p;

	for (p = PBinds; p != 0; p = p->pb_next) {
		p->pb_push = p->pb_map[p->pb_key];
		p->pb_map[p->pb_key] = p->pb_cmd;
	}
}
/* VARARGS0 */

ProcBind()
{
	register data_obj	*d;

	if ((d = findcom(ProcFmt)) == 0)
		return;
	s_mess(": %f %s ", d->Name);
	ProcB2(mainmap, EOF, d);
}

ProcB2(map, lastkey, cmd)
data_obj	**map,
		*cmd;
{
	register struct proc_bind *p;
	register data_obj	**nextmap;
	int	c;

	c = addgetc();
	if (c == EOF) {
		if (lastkey == EOF)
			complain("[Empty key sequence]");
		complain("[Unexpected end-of-line]");
	} else {
		if (nextmap = IsPrefix(map[c]))
			ProcB2(nextmap, c, cmd);
		else {
			if (curbuf->b_process)
				PopPBs();

			for (p = PBinds; p != 0; p = p->pb_next)
				if (p->pb_key == c && p->pb_map == map)
					break;
			if (p == 0) {
				p = (struct proc_bind *) emalloc(sizeof *p);
				p->pb_next = PBinds;
				PBinds = p;
			}
			p->pb_map = map;
			p->pb_key = c;
			p->pb_cmd = cmd;

			if (curbuf->b_process)
				PushPBs();
		}
	}
}

#endif IPROCS
