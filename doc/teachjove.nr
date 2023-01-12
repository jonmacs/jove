.\"##########################################################################
.\"# This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    #
.\"# provided by Jonathan and Jovehacks without charge and without          #
.\"# warranty.  You may copy, modify, and/or distribute JOVE, provided that #
.\"# this notice is included in all the source files and documentation.     #
.\"##########################################################################
.TH TEACHJOVE 1 "24 June 1993"
.ad
.SH NAME
teachjove \- learn how to use the JOVE editor
.SH SYNOPSIS
.B teachjove
[
.BI \-s \ sharedir
]
.SH DESCRIPTION
.hy 0
TEACHJOVE (or TEACHJOV on MSDOS) is a simple program that
calls up the JOVE editor with the -T option to load an
interactive tutorial for the JOVE editor.  Once in JOVE all
you do is follow the instructions and by doing so you will
learn all about JOVE!  NOTE: If you use the
.I save-file
command (or the the keystrokes Control-X Control-S ),
TEACHJOVE saves a copy
of the tutorial in your home directory; if you ever want to
start over (if you save a trashed file by accident) all you need
to do is remove the file
.I ~/teach-jove
and run teachjove again.
.LP
TEACHJOVE passes switches to Jove, though some may not
be meaningful for the tutorial.  One that may be useful:
.TP
.BI \-s \ sharedir
Allows the user to specify the directory in which the special
teach-jove file can be found (default <SHAREDIR>).
.SH FILES
.I <SHAREDIR>/teach-jove
\(em THE special file.
.SH SEE ALSO
.I jove(1)
\(em to learn about JOVE in general.
.SH AUTHOR
Jonathan Payne
