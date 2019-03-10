/*************************************************************************
 * This program is copyright (C) 1985, 1986 by Jonathan Payne.  It is    *
 * provided to you without charge for use only on a licensed Unix        *
 * system.  You may copy JOVE provided that this notice is included with *
 * the copy.  You may not sell copies of this program or versions        *
 * modified for use on microcomputer systems, unless the copies are      *
 * included with a Unix system distribution and the source is provided.  *
 *************************************************************************/

#include "jove.h"
#include <sys/ioctl.h>

#ifndef TXT_TO_C
extern int
	EscPrefix(),
	CtlxPrefix(),
	MiscPrefix(),
	UnbindC(),
	ShowVersion(),
	WVisSpace(),
#ifdef ANSICODES
	AnsiCodes(),
#endif
	AppReg(),
	Apropos(),
	BackChar(),
	Bparen(),
	BackWord(),
	Bof(),
	Bol(),
	Bos(),
	Bow(),
	BindAKey(),
	BindMac(),
	BufPos(),
	CasRegLower(),
	CasRegUpper(),
	CapChar(),
	CapWord(),
	LowWord(),
	UppWord(),
#ifdef CHDIR
	Chdir(),
	prCWD(),
	prDIRS(),
	Pushd(),
	Popd(),
#endif
	prCTIME(),
	ChrToOct(),
	ClAndRedraw(),
	MakeErrors(),
	CopyRegion(),
	BufSelect(),
	DelBlnkLines(),
	DelNChar(),
	DelNWord(),
	OneWindow(),
	DelPChar(),
	DelPWord(),
	DelReg(),
	KillSome(),
	DelWtSpace(),
	DelCurWindow(),
	KeyDesc(),
	Digit(),
	Digit0(),
	Digit1(),
	Digit2(),
	Digit3(),
	Digit4(),
	Digit5(),
	Digit6(),
	Digit7(),
	Digit8(),
	Digit9(),
	DescBindings(),
	DescCom(),
	Eof(),
	Eol(),
	Eos(),
	Eow(),
	ForPara(),
	BackPara(),
	BufErase(),
	PtToMark(),
	Extend(),
	ExecMacro(),
	RunMacro(),
	Leave(),
	FindFile(),
	WindFind(),
	FindTag(),
	FDotTag(),
	ToIndent(),
	ForChar(),
	Fparen(),
	ForWord(),
	FourTime(),
	GoLine(),
	GrowWindow(),
	IncFSearch(),
	IncRSearch(),
	InsFile(),
	Justify(),
	RegJustify(),
	SetLMargin(),
	SetRMargin(),
	BufKill(),
	KillBos(),
	KillEos(),
	KillEOL(),
	KillExpr(),
	BufList(),
	NotModified(),
	NameMac(),
	DelMacro(),
	Newline(),
	OpenLine(),
	LineAI(),
	ShowErr(),
	NextError(),
	PrevError(),
	NextLine(),
	NextPage(),
	NextWindow(),
	Recur(),
	PopMark(),
	PageNWind(),
	Tab(),
	DoParen(),
	ParseAll(),
	XParse(),
#ifdef SPELL
	SpelWords(),
#endif
#ifdef JOB_CONTROL
	PauseJove(),
#endif
	PrevLine(),
	PrevPage(),
	PrevWindow(),
	Push(),
	RegReplace(),
	QRepSearch(),
	QuotChar(),
	ReadFile(),
	ReadMacs(),
	RedrawDisplay(),
	ReNamBuf(),
	RepSearch(),
	DownScroll(),
	UpScroll(),
	ForSearch(),
	RevSearch(),
	SelfInsert(),
	SetVar(),
 	SetMark(),
	ShellCom(),
	ShToBuf(),
	ShrWindow(),
	Source(),
#ifdef SPELL
	SpelBuffer(),
#endif
	SplitWind(),
	Remember(),
	Forget(),
	StrLength(),
	TransChar(),
	TransLines(),
	SaveFile(),
	WtModBuf(),
	WriteFile(),
	WriteMacs(),
	WrtReg(),
	Yank(),
	YankPop(),
	PrVar(),
	FilterRegion(),
	WNumLines(),

#ifdef IPROCS
	IShell(),
	ProcInt(),
	ProcQuit(),
	ProcKill(),
#  ifndef PIPEPROCS
#    ifdef TIOCSLTC
	ProcEof(),
	ProcStop(),
	ProcCont(),
	ProcDStop(),
#    endif
#  endif
	ProcNewline(),
	ProcList(),
	ProcBind(),
	Iprocess(),
#endif

#ifdef LISP
	GSexpr(),	/* Grind S Expression. */
#endif
	FAutoExec(),
	MAutoExec(),

	DefMAbbrev(),
	DefGAbbrev(),
	SaveAbbrevs(),
	RestAbbrevs(),
	EditAbbrevs(),
	BindMtoW(),

#ifdef CMT_FMT
	Comment(),
#endif

	MacInter();		/* This is the last one. */


#	define WIRED_FUNC(c)	c

#else TXT_TO_C

#	define WIRED_FUNC(c)	0

#endif TXT_TO_C

struct funct	commands[] = {
	FUNCTION, "Prefix-1", WIRED_FUNC(EscPrefix),
	FUNCTION, "Prefix-2", WIRED_FUNC(CtlxPrefix),
	FUNCTION, "Prefix-3", WIRED_FUNC(MiscPrefix),
#ifdef ANSICODES
	FUNCTION, "ansi-codes", WIRED_FUNC(AnsiCodes),
#endif
	FUNCTION, "append-region", WIRED_FUNC(AppReg),
	FUNCTION, "apropos", WIRED_FUNC(Apropos),
	FUNCTION, "auto-execute-command", WIRED_FUNC(FAutoExec),
	FUNCTION, "auto-execute-macro", WIRED_FUNC(MAutoExec),
	DefMinor(Fill), "auto-fill-mode", 0,
	DefMinor(Indent), "auto-indent-mode", 0,
	FUNCTION, "backward-character", WIRED_FUNC(BackChar),
	FUNCTION, "backward-paragraph", WIRED_FUNC(BackPara),
	FUNCTION, "backward-s-expression", WIRED_FUNC(Bparen),
	FUNCTION, "backward-sentence", WIRED_FUNC(Bos),
	FUNCTION, "backward-word", WIRED_FUNC(BackWord),
	FUNCTION, "beginning-of-file", WIRED_FUNC(Bof),
	FUNCTION, "beginning-of-line", WIRED_FUNC(Bol),
	FUNCTION, "beginning-of-window", WIRED_FUNC(Bow),
	FUNCTION, "bind-to-key", WIRED_FUNC(BindAKey),
	FUNCTION, "bind-macro-to-key", WIRED_FUNC(BindMac),
#ifdef ABBREV
	FUNCTION, "bind-macro-to-word-abbrev", WIRED_FUNC(BindMtoW),
#endif
	FUNCTION, "buffer-position", WIRED_FUNC(BufPos),
	DefMajor(CMODE), "c-mode", 0,
	FUNCTION, "case-character-capitalize", WIRED_FUNC(CapChar),
	FUNCTION, "case-region-lower", WIRED_FUNC(CasRegLower),
	FUNCTION, "case-region-upper", WIRED_FUNC(CasRegUpper),
	FUNCTION, "case-word-capitalize", WIRED_FUNC(CapWord),
	FUNCTION, "case-word-lower", WIRED_FUNC(LowWord),
	FUNCTION, "case-word-upper", WIRED_FUNC(UppWord),
	FUNCTION, "character-to-octal-insert", WIRED_FUNC(ChrToOct),
#ifdef CHDIR
	FUNCTION, "cd", WIRED_FUNC(Chdir),
#endif
	FUNCTION, "clear-and-redraw", WIRED_FUNC(ClAndRedraw),
	FUNCTION, "compile-it", WIRED_FUNC(MakeErrors),
#ifdef IPROCS
#  ifndef PIPEPROCS
#    ifdef TIOCSLTC
	FUNCTION, "continue-process", WIRED_FUNC(ProcCont),
#    endif
#  endif
#endif
	FUNCTION, "copy-region", WIRED_FUNC(CopyRegion),
	FUNCTION, "current-error", WIRED_FUNC(ShowErr),
	FUNCTION, "date", WIRED_FUNC(prCTIME),
#ifdef ABBREV
	FUNCTION, "define-mode-word-abbrev", WIRED_FUNC(DefMAbbrev),
	FUNCTION, "define-global-word-abbrev", WIRED_FUNC(DefGAbbrev),
#endif
	FUNCTION, "delete-blank-lines", WIRED_FUNC(DelBlnkLines),
	FUNCTION, "delete-buffer", WIRED_FUNC(BufKill),
	FUNCTION, "delete-macro", WIRED_FUNC(DelMacro),
	FUNCTION, "delete-next-character", WIRED_FUNC(DelNChar),
	FUNCTION, "delete-other-windows", WIRED_FUNC(OneWindow),
	FUNCTION, "delete-previous-character", WIRED_FUNC(DelPChar),
	FUNCTION, "delete-white-space", WIRED_FUNC(DelWtSpace),
	FUNCTION, "delete-current-window", WIRED_FUNC(DelCurWindow),
	FUNCTION, "describe-bindings", WIRED_FUNC(DescBindings),
	FUNCTION, "describe-command", WIRED_FUNC(DescCom),
	FUNCTION, "describe-key", WIRED_FUNC(KeyDesc),
	FUNCTION, "describe-variable", WIRED_FUNC(DescCom),
	FUNCTION, "digit", WIRED_FUNC(Digit),
	FUNCTION, "digit-1", WIRED_FUNC(Digit1),
	FUNCTION, "digit-2", WIRED_FUNC(Digit2),
	FUNCTION, "digit-3", WIRED_FUNC(Digit3),
	FUNCTION, "digit-4", WIRED_FUNC(Digit4),
	FUNCTION, "digit-5", WIRED_FUNC(Digit5),
	FUNCTION, "digit-6", WIRED_FUNC(Digit6),
	FUNCTION, "digit-7", WIRED_FUNC(Digit7),
	FUNCTION, "digit-8", WIRED_FUNC(Digit8),
	FUNCTION, "digit-9", WIRED_FUNC(Digit9),
	FUNCTION, "digit-0", WIRED_FUNC(Digit0),
#ifdef CHDIR
	FUNCTION, "dirs", WIRED_FUNC(prDIRS),
#endif
#ifdef IPROCS
#  ifndef PIPEPROCS
#    ifdef TIOCSLTC
	FUNCTION, "dstop-process", WIRED_FUNC(ProcDStop),
#    endif
#  endif
#endif
#ifdef ABBREV
	FUNCTION, "edit-word-abbrevs", WIRED_FUNC(EditAbbrevs),
#endif
	FUNCTION, "end-of-file", WIRED_FUNC(Eof),
	FUNCTION, "end-of-line", WIRED_FUNC(Eol),
	FUNCTION, "end-of-window", WIRED_FUNC(Eow),
#ifdef IPROCS
#  ifndef PIPEPROCS
#    ifdef TIOCSLTC
	FUNCTION, "eof-process", WIRED_FUNC(ProcEof),
#    endif
#  endif
#endif
	FUNCTION, "erase-buffer", WIRED_FUNC(BufErase),
	FUNCTION, "exchange-point-and-mark", WIRED_FUNC(PtToMark),
	FUNCTION, "execute-named-command", WIRED_FUNC(Extend),
	FUNCTION, "execute-keyboard-macro", WIRED_FUNC(ExecMacro),
	FUNCTION, "execute-macro", WIRED_FUNC(RunMacro),
	FUNCTION, "exit-jove", WIRED_FUNC(Leave),
#ifdef CMT_FMT
 	FUNCTION, "fill-comment", WIRED_FUNC(Comment),
#endif CMT_FMT
	FUNCTION, "fill-paragraph", WIRED_FUNC(Justify),
	FUNCTION, "fill-region", WIRED_FUNC(RegJustify),
	FUNCTION, "filter-region", WIRED_FUNC(FilterRegion),
	FUNCTION, "find-file", WIRED_FUNC(FindFile),
	FUNCTION, "find-tag", WIRED_FUNC(FindTag),
	FUNCTION, "find-tag-at-point", WIRED_FUNC(FDotTag),
	FUNCTION, "first-non-blank", WIRED_FUNC(ToIndent),
	FUNCTION, "forward-character", WIRED_FUNC(ForChar),
	FUNCTION, "forward-paragraph", WIRED_FUNC(ForPara),
	FUNCTION, "forward-s-expression", WIRED_FUNC(Fparen),
	FUNCTION, "forward-sentence", WIRED_FUNC(Eos),
	FUNCTION, "forward-word", WIRED_FUNC(ForWord),
	DefMajor(FUNDAMENTAL), "fundamental-mode", 0,
#ifdef LISP
	FUNCTION, "grind-s-expr", WIRED_FUNC(GSexpr),
#endif
	FUNCTION, "goto-line", WIRED_FUNC(GoLine),
	FUNCTION, "grow-window", WIRED_FUNC(GrowWindow),
	FUNCTION, "handle-tab", WIRED_FUNC(Tab),
	FUNCTION, "i-search-forward", WIRED_FUNC(IncFSearch),
	FUNCTION, "i-search-reverse", WIRED_FUNC(IncRSearch),
	FUNCTION, "insert-file", WIRED_FUNC(InsFile),
#ifdef IPROCS
	FUNCTION, "interrupt-process", WIRED_FUNC(ProcInt),
	FUNCTION, "i-shell", WIRED_FUNC(IShell),
	FUNCTION, "i-shell-command", WIRED_FUNC(Iprocess),
#endif
	FUNCTION, "kill-next-word", WIRED_FUNC(DelNWord),
	FUNCTION, "kill-previous-word", WIRED_FUNC(DelPWord),
#ifdef IPROCS
	FUNCTION, "kill-process", WIRED_FUNC(ProcKill),
#endif
	FUNCTION, "kill-region", WIRED_FUNC(DelReg),
	FUNCTION, "kill-s-expression", WIRED_FUNC(KillExpr),
	FUNCTION, "kill-some-buffers", WIRED_FUNC(KillSome),
	FUNCTION, "kill-to-beginning-of-sentence", WIRED_FUNC(KillBos),
	FUNCTION, "kill-to-end-of-line", WIRED_FUNC(KillEOL),
	FUNCTION, "kill-to-end-of-sentence", WIRED_FUNC(KillEos),
	FUNCTION, "left-margin-here", WIRED_FUNC(SetLMargin),
#ifdef LISP
	DefMajor(LISPMODE), "lisp-mode", 0,
#endif
	FUNCTION, "list-buffers", WIRED_FUNC(BufList),
#ifdef IPROCS
	FUNCTION, "list-processes", WIRED_FUNC(ProcList),
#endif
	FUNCTION, "make-buffer-unmodified", WIRED_FUNC(NotModified),
	FUNCTION, "make-macro-interactive", WIRED_FUNC(MacInter),
	FUNCTION, "name-keyboard-macro", WIRED_FUNC(NameMac),
	FUNCTION, "newline", WIRED_FUNC(Newline),
	FUNCTION, "newline-and-backup", WIRED_FUNC(OpenLine),
	FUNCTION, "newline-and-indent", WIRED_FUNC(LineAI),
	FUNCTION, "next-error", WIRED_FUNC(NextError),
	FUNCTION, "next-line", WIRED_FUNC(NextLine),
	FUNCTION, "next-page", WIRED_FUNC(NextPage),
	FUNCTION, "next-window", WIRED_FUNC(NextWindow),
	FUNCTION, "number-lines-in-window", WIRED_FUNC(WNumLines),
	DefMinor(OverWrite), "over-write-mode", 0,
	FUNCTION, "page-next-window", WIRED_FUNC(PageNWind),
	FUNCTION, "paren-flash", WIRED_FUNC(DoParen),
	FUNCTION, "parse-errors", WIRED_FUNC(ParseAll),
	FUNCTION, "parse-special-errors", WIRED_FUNC(XParse),
#ifdef SPELL
	FUNCTION, "parse-spelling-errors-in-buffer", WIRED_FUNC(SpelWords),
#endif
#ifdef JOB_CONTROL
	FUNCTION, "pause-jove", WIRED_FUNC(PauseJove),
#else
	FUNCTION, "pause-jove", WIRED_FUNC(Push),
#endif
	FUNCTION, "pop-mark", WIRED_FUNC(PopMark),
#ifdef CHDIR
	FUNCTION, "popd", WIRED_FUNC(Popd),
#endif
	FUNCTION, "previous-error", WIRED_FUNC(PrevError),
	FUNCTION, "previous-line", WIRED_FUNC(PrevLine),
	FUNCTION, "previous-page", WIRED_FUNC(PrevPage),
	FUNCTION, "previous-window", WIRED_FUNC(PrevWindow),
	FUNCTION, "print", WIRED_FUNC(PrVar),
#ifdef IPROCS
	FUNCTION, "process-bind-to-key", WIRED_FUNC(ProcBind),
	FUNCTION, "process-newline", WIRED_FUNC(ProcNewline),
#endif
	FUNCTION, "push-shell", WIRED_FUNC(Push),
#ifdef CHDIR
	FUNCTION, "pushd", WIRED_FUNC(Pushd),
	FUNCTION, "pwd", WIRED_FUNC(prCWD),
#endif
	FUNCTION, "quadruple-numeric-argument", WIRED_FUNC(FourTime),
	FUNCTION, "query-replace-string", WIRED_FUNC(QRepSearch),
#ifdef IPROCS
	FUNCTION, "quit-process", WIRED_FUNC(ProcQuit),
#endif
	FUNCTION, "quoted-insert", WIRED_FUNC(QuotChar),
#ifdef ABBREV
	FUNCTION, "read-word-abbrev-file", WIRED_FUNC(RestAbbrevs),
#endif
	FUNCTION, "read-macros-from-file", WIRED_FUNC(ReadMacs),
	FUNCTION, "redraw-display", WIRED_FUNC(RedrawDisplay),
	FUNCTION, "recursive-edit", WIRED_FUNC(Recur),
	FUNCTION, "rename-buffer", WIRED_FUNC(ReNamBuf),
	FUNCTION, "replace-in-region", WIRED_FUNC(RegReplace),
	FUNCTION, "replace-string", WIRED_FUNC(RepSearch),
	FUNCTION, "right-margin-here", WIRED_FUNC(SetRMargin),
	FUNCTION, "save-file", WIRED_FUNC(SaveFile),
	FUNCTION, "scroll-down", WIRED_FUNC(DownScroll),
	FUNCTION, "scroll-up", WIRED_FUNC(UpScroll),
	FUNCTION, "search-forward", WIRED_FUNC(ForSearch),
	FUNCTION, "search-reverse", WIRED_FUNC(RevSearch),
	FUNCTION, "select-buffer", WIRED_FUNC(BufSelect),
	FUNCTION, "self-insert", WIRED_FUNC(SelfInsert),
	FUNCTION, "set", WIRED_FUNC(SetVar),
	FUNCTION, "set-mark", WIRED_FUNC(SetMark),
	FUNCTION, "shell-command", WIRED_FUNC(ShellCom),
	FUNCTION, "shell-command-to-buffer", WIRED_FUNC(ShToBuf),
	DefMinor(ShowMatch), "show-match-mode", 0,
	FUNCTION, "shrink-window", WIRED_FUNC(ShrWindow),
	FUNCTION, "source", WIRED_FUNC(Source),
#ifdef SPELL
	FUNCTION, "spell-buffer", WIRED_FUNC(SpelBuffer),
#endif
	FUNCTION, "split-current-window", WIRED_FUNC(SplitWind),
	FUNCTION, "start-remembering", WIRED_FUNC(Remember),
#ifdef IPROCS
#  ifndef PIPEPROCS
#    ifdef TIOCSLTC
	FUNCTION, "stop-process", WIRED_FUNC(ProcStop),
#    endif
#  endif
#endif
	FUNCTION, "stop-remembering", WIRED_FUNC(Forget),
	FUNCTION, "string-length", WIRED_FUNC(StrLength),
#ifdef JOB_CONTROL
	FUNCTION, "suspend-jove", WIRED_FUNC(PauseJove),
#endif
	DefMajor(TEXT), "text-mode", 0,
	FUNCTION, "transpose-characters", WIRED_FUNC(TransChar),
	FUNCTION, "transpose-lines", WIRED_FUNC(TransLines),
	FUNCTION, "unbind-key", WIRED_FUNC(UnbindC),
	FUNCTION, "version", WIRED_FUNC(ShowVersion),
	FUNCTION, "visible-spaces-in-window", WIRED_FUNC(WVisSpace),
	FUNCTION, "visit-file", WIRED_FUNC(ReadFile),
	FUNCTION, "window-find", WIRED_FUNC(WindFind),
#ifdef ABBREV
	DefMinor(Abbrev), "word-abbrev-mode", 0,
	FUNCTION, "write-word-abbrev-file", WIRED_FUNC(SaveAbbrevs),
#endif
	FUNCTION, "write-file", WIRED_FUNC(WriteFile),
	FUNCTION, "write-macros-to-file", WIRED_FUNC(WriteMacs),
	FUNCTION, "write-modified-files", WIRED_FUNC(WtModBuf),
	FUNCTION, "write-region", WIRED_FUNC(WrtReg),
	FUNCTION, "yank", WIRED_FUNC(Yank),
	FUNCTION, "yank-pop", WIRED_FUNC(YankPop),
	FUNCTION, 0, 0
};

#ifndef TXT_TO_C
data_obj *
findcom(prompt, flags)
char	*prompt;
{
	static char	*strings[(sizeof commands) / sizeof (struct funct)];
	static int	beenhere = 0;
	register int	com;

	if (beenhere == 0) {
		register char	**strs = strings;
		register struct funct	*f = commands;

		beenhere = 1;
		for (; f->Name; f++)
			*strs++ = f->Name;
		*strs = 0;
	}

	if ((com = complete(strings, prompt, flags)) < 0)
		return 0;
	return (data_obj *) &commands[com];
}
#endif
