/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

/* Note: jove.spec must be updated when version number changes */

#define	jversion     "4.16.0.69" /* 2005 Sept 30 */
#define	jversion_lnum 4,16,0,69  /* list-of-numbers form for Win32 jjove.rc */

/*
 * $Log: version.h,v $
 * Revision 1.419  2005/10/01 00:34:41  hugh
 * 4.16.0.69: fix minor errors introduced in 4.16.0.67
 *
 * Revision 1.418  2005/09/28 18:41:49  hugh
 * Work around an xterm / termcap / terminfo bug involving SR.
 *
 * Revision 1.417  2005/05/14 20:29:03  hugh
 * Generalize tags code to handle bizarre the mutations of tagfile format.
 * Contributed by Mark Moraes; "improved" by DHR.
 *
 * Revision 1.416  2005/05/14 17:46:43  hugh
 * rpmbuild now requires "License" tag in place of "Copyright" tag in .spec
 *
 * Revision 1.415  2004/07/11 20:04:32  hugh
 * - dodge yet another xterm bug in hilite mode
 * - set default buffer size to 4kB (ups line length limit to same)
 * - tweak documentation
 *
 * Revision 1.414  2004/02/01 19:27:03  hugh
 * - introduce sysdef BSDPOSIX_STDC
 * - update README
 *
 * Revision 1.413  2003/05/05 01:37:05  hugh
 * refine modified flag in list-buffers output
 *
 * Revision 1.412  2003/03/09 23:49:55  hugh
 * 4.16.0.62: fix for WIN32; note tested environments [Jim Patterson]
 *
 * Revision 1.411  2003/02/01 02:02:08  hugh
 * 4.16.0.61: increase MESG_SIZE to support wider terminals
 *
 * Revision 1.410  2003/01/31 22:15:33  hugh
 * 4.16.0.60: fix screen update bug involving scrolling region
 * Surprise: cursor location undefined after setting scrolling region
 *
 * Revision 1.409  2003/01/31 22:01:40  hugh
 * 4.16.0.59: add "distrib" and "signeddistrib" Makefile targets
 * tar file will now unpack into jove<version> directory
 *
 * Revision 1.408  2003/01/09 02:07:15  hugh
 * adjust jove.spec for yet another new rpmbuild requirement
 *
 * Revision 1.407  2002/12/13 01:52:47  hugh
 * - allow $TERM for iprocs to be configured
 * - adjust types of parameters of truncstrsub and jamstrsub (lint)
 * - refine jove.spec (for RPM building); iproc TERM=vanilla
 *
 * Revision 1.406  2002/03/21 07:06:04  hugh
 * update Copyright to satisfy Debian
 *
 * Revision 1.405  2002/02/12 20:14:08  hugh
 * - add style.doc to describe some coding conventions
 * - add a Makefile target to support Exuberant Ctags more conveniently
 * - add a note about overflow in calculating percentage in buffer-position
 * - change TERM and TERMCAP environment variables in iproc
 *
 * Revision 1.404  2001/12/20 21:07:25  hugh
 * - fix Makefile problems with openpty configuration
 * - factor LIBS into TERMCAPLIB and EXTRALIBS
 *
 * Revision 1.403  2001/12/17 22:24:14  hugh
 * - fix a few buffer overruns
 * - add "Quick summary" to README
 * - use "jmode_t" where mode_t is appropriate (old systems must define as int)
 * - use uid_t; hope this is portable
 * - added GCC_LINT to make it more pleasant to use gcc warning options
 *
 * Revision 1.402  2001/07/15 19:14:23  hugh
 * improve jove.spec (for Redhat Package Manager)
 *
 * Revision 1.401  2001/03/22 07:17:26  hugh
 * 4.16.0.51: improve portability; use openpty on *BSD
 *
 * Revision 1.400  2001/02/04 21:01:50  hugh
 * 4.16.0.50 support groff; fix jove.spec for RPM creation
 *
 * Revision 1.399  2000/11/10 07:58:22  hugh
 * [BET+DHR] support installation into a playpen; exploit when building RPM
 *
 * Revision 1.398  2000/07/12 16:13:01  hugh
 * 4.16.0.48: use SVR4_PTYS (AKA UNIX98 PTYS) for LINUX
 *
 * Revision 1.397  1999/10/22 14:16:39  hugh
 * 4.16.0.47: get file creation mode right; predelete backup file
 * Thanks to Rob McMahon <cudcv@csv.warwick.ac.uk>
 *
 * Revision 1.396  1999/08/29 18:44:49  hugh
 * 4.16.0.46: add save-on-exit [Rob.McMahon@warwick.ac.uk]
 *
 * Revision 1.395  1999/08/25 19:47:30  hugh
 * 4.16.0.45: detect and report file close errors [Rob.McMahon@warwick.ac.uk]
 *
 * Revision 1.394  1999/08/19 02:14:13  hugh
 * 4.16.0.44: add .spec for RPM creation
 *
 * Revision 1.393  1999/08/18 23:17:20  hugh
 * 4.16.0.43: make mkstemp code less brittle (Moraes)
 *
 * Revision 1.392  1999/08/18 21:56:21  hugh
 * make USE_CTYPE implicit in BSDPOSIX (except for __convex__!)
 *
 * Revision 1.391  1999/08/15 02:24:48  hugh
 * Support for Cygwin32 environment on MS Win32
 * (from Arlindo da Silva and Dave Curry)
 *
 * Revision 1.390  1999/08/15 00:50:48  hugh
 * 4.16.0.40: fix horrible umask security hole
 *
 * Revision 1.389  1999/08/13 14:43:05  hugh
 * 4.16.0.39: reflect Jim's (Jim.Patterson@Cognos.COM) work on Win32 and HPUX
 *
 * Revision 1.388  1999/08/12 21:21:38  hugh
 * 4.16.0.37: update Makefile and README to reflect modern systems
 *
 * Revision 1.387  1999/08/12 19:14:11  hugh
 * 4.16.0.37: scatter "const"; update copyright; tidy comments
 *
 * Revision 1.386  1999/08/10 15:08:03  hugh
 * 4.16.0.36: switch to safe creation of tempfiles.
 * Note: mode_t isn't used because argument promotion rules mess things up.
 * Some additional tidying.
 * Tested on MSDOS, SunOS 4.0, Solaris 2.6, RedHat5.2.
 *
 * Revision 1.385  1999/08/09 05:50:51  hugh
 * 4.16.0.35: fix nits in documentation
 *
 * Revision 1.384  1999/08/06 19:02:27  hugh
 * 4.16.0.34: fixed a bug that caused justification of the following line
 * to hang JOVE.  set right-margin 70.  Note whitespace at end.
 *
 * Revision 1.383  1999/08/06 16:47:20  hugh
 * make xterm mouse code dodge metakey kludge (needed for xterms wider than 95)
 *
 * Revision 1.382  1999/02/11 18:41:51  hugh
 * fix indented #ifdef: would confuse old compilers
 *
 * Revision 1.381  1998/09/22 21:10:44  hugh
 * Rename SCO to SCO_ODT3 to be more specific.
 * Fix typo in SCO_ODT3 settings (cannot test!).
 * Thanks, Mark Moraes <moraes@staff.juno.com>
 *
 * Revision 1.380  1998/09/22 03:47:03  hugh
 * glibc-2 (the GNU C library, used in some LINUX systems) has a
 * a unique (but probably not wrong) feature: stat, when it fails,
 * scribbles over the stat buffer.  This change lets JOVE deal
 * with this feature.
 * Thanks to Steve Thompson <stevet@ultratech.net>.
 *
 * Revision 1.379  1998/09/21 21:19:05  hugh
 * Verify that other end of PTY can be opened before accepting it.
 * This is needed apparently needed under LINUX.  The test doesn't
 * work on old versions of BSDI/386, so BSDI_PTY_BUG will suppress.
 *
 * Revision 1.378  1998/09/21 17:57:12  hugh
 * fix error message construction
 * [originally done 1998 March 29]
 *
 * Revision 1.377  1998/09/21 17:52:15  hugh
 * avoid freeing already freed name list
 * [originally done 1998 January 25]
 *
 * Revision 1.376  1998/09/21 17:48:10  hugh
 * Change xterm-bug-workaround to avoid changes made for XFree 3.2.
 * Without this change, mouse usage under the xterm of XFree 3.2
 * will leave mysterious blanks on the screen.
 *
 * The relevant change to XFree 3.2's xterm is that the sequence ESC X is
 * now meaningful.  We had counted on this to be ignored.  Now we will
 * count on ESC DEL being ignored.
 * [originally done 1997 Sept 21]
 *
 * Revision 1.375  1997/07/10 06:06:30  hugh
 * Add support for different meaning if im and ic with ncurses' termcap
 * (used with LINUX and other free systems).
 *
 * Revision 1.374  1997/01/17 01:59:28  hugh
 * 4.16.0.24: little touches
 * - allow var internal-tabstop to be set to 0 (tab-width already could)
 * - notes for porting to Digital UNIX
 * - spelling improvement
 *
 * Revision 1.373  1997/01/16 22:08:26  hugh
 * 4.16.0.23: handle symlinks in path canonicalization (PathParse)
 *
 * Revision 1.372  1996/10/10 06:59:26  hugh
 * 4.16.0.22: more pr_name static buffer bugs
 *
 * Revision 1.371  1996/10/06 20:57:51  hugh
 * 4.16.0.21: port to Digital UNIX V4.0 -- one JOVE bug, one OS bug
 *
 * Revision 1.370  1996/09/19 02:09:39  hugh
 * 4.16.0.20: fix bug in window resizing by mouse
 * To demonstrate bug:
 * - split a window that is viewing a non-empty buffer
 * - make upper window have a different dot.
 * - enlarge the upper window by dragging its mode line down (button 2)
 * The upper window's dot will be a copy of the lower window's.
 *
 * Revision 1.369  1996/09/09 06:29:43  hugh
 * 4.16.0.19: eliminate tricky aliasing bug in Source/ask_ford/PathParse
 *
 * The bug to be fixed is the interaction of the following:
 * - PathParse must not have aliased args
 * - if ask_ford is called with def and buf aliased
 *   AND pr_name(def, YES) yields a pointer into def
 *   AND the user defaults the file name
 *   THEN PathParse will be called with aliased args
 * - Source calls ask_ford with def and buf aliased
 *
 * To demonstrate:
 * - set environment variable HOME to "" or "/"
 * - run jove in some other directory
 * - issue "source" command and hit return to default the file name
 * - jove will attempt to source the file named ""
 *
 * The fix: since pr_name sometimes returns a pointer to its static buffer,
 * the simplest change is to make it always return a pointer to its static
 * buffer.  At the same time, sprinkle a few comments about aliasing in the
 * relevant routines.
 *
 * Incidental improvements:
 * - pr_name now complains about too-long file names
 * - pr_name will now choose to use ~ over cwd-relative naming if the
 *   resulting name is shorter
 *
 * The apology: static buffers are evil (wasteful and error-prone) and
 * should be eliminated.  Unfortunately, this would require changes to
 * every use of pr_name.
 *
 * Revision 1.368  1996/08/19 21:05:09  hugh
 * 4.16.0.18: make dbx a minor mode (simplify, generalize)
 *
 * Revision 1.367  1996/07/11 05:09:06  hugh
 * 4.16.0.17: make jjove.rc use version.h
 *
 * Revision 1.366  1996/07/07 22:18:22  hugh
 * 4.16.0.16: fix bugs in commandline arg processing
 * There were several odd cases where the current buffer
 * would not be tied to the current window.
 * >> As a minor enhancement, the alternat buffer will now
 * be set to the last file not assigned a window.
 *
 * Revision 1.365  1996/07/02 00:15:13  hugh
 * 4.16.0.15: fix subtle screen maintenance bug
 * Without this, filename/command/etc completion would get
 * truncated when process windows were updated.
 *
 * Revision 1.364  1996/06/16 15:56:45  hugh
 * 4.16.0.14: fix MatchDir usage
 *
 * Revision 1.363  1996/06/13 06:19:29  hugh
 * 4.16.0.13: rename basename => jbasename (name taken by SVR4)
 *
 * Revision 1.362  1996/05/23 03:08:41  hugh
 * 4.16.0.12: fix nits in teach-jove
 *
 * Revision 1.361  1996/05/20 01:57:36  hugh
 * 4.16.0.11: in Makefile: use LDCC in appropriate places
 *
 * Revision 1.360  1996/05/09 02:40:21  hugh
 * 4.16.0.10: improve READMEs for DOS and WIN32
 *
 * Revision 1.359  1996/05/08 18:15:13  hugh
 * 4.16.0.9: support ConvexOS, istrip problems and all
 *
 * Revision 1.358  1996/05/08 03:44:25  hugh
 * 4.16.0.8: restore Zortech compatibility
 * Zortech 3.0 doesn't define EINTR; I don't think any MSDOS runtime
 * would generate it for a write().  (The code already reflects this
 * understanding for read().)
 *
 * Revision 1.357  1996/05/07 04:28:18  hugh
 * 4.16.0.7: fix backup-files code for MSFILESYSTEM
 *
 * Revision 1.356  1996/04/22 06:49:45  hugh
 * 4.16.0.6: remove unwarranted bug warning
 * The bug was caused by a virus on the test system.
 *
 * Revision 1.355  1996/03/30 00:59:29  hugh
 * 4.16.0.5: fix justification nits
 *
 * Revision 1.354  1996/03/21 19:14:48  hugh
 * 4.16.0.4: improve concatenation of pathname components
 * This change is needed to avoid accidental references to network files
 * under some Microsoft systems.
 * Took this oportunity to delete some obliquely related nonsense code
 * in scandir.c.
 *
 * Revision 1.353  1996/03/21 17:02:10  hugh
 * 4.16.0.3: correct case of OS in Makefile.msc
 *
 * Revision 1.352  1996/03/20 08:00:59  hugh
 * 4.16.0.2: Fix strange font usages in xjove (Charles)
 *
 * Revision 1.351  1996/03/20 07:40:19  hugh
 * 4.16.0.1: fix and use lint Makefile target
 */
