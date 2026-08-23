/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

/* The tmp file stores lines of text for JOVE buffers as contiguous
 * sequences of chars, terminated by NUL.  New lines are added to
 * the end of the tmp file.  The file is not garbage collected
 * because that would be too painful.  As a result, commands like
 * Yank and Kill are really easy; basically all we do is make copies
 * of the disk addresses of the lines (as opposed to the contents).
 * So, jputline(char *buf) writes buf to the disk and returns a new
 * disk address.  jgetline(daddr addr, char *buf) is the opposite of
 * jputline().  f_getputl(LinePtr line, File fp) reads from open fp
 * directly into the tmp file (into the buffer cache (see below))
 * and stores the address in line.  This is used during read_file to
 * minimize copying.
 *
 * To reduce the size of a tmp file disk address, each disk address
 * is limited to point to positions that are the start of a chunk.
 * A chunk has exactly CHNK_CHARS characters, so that an disk
 * address can be reduced by log2(CHNK_CHARS) bits.  These extra
 * bits allow for a larger tmp file.  This is exploited on machines
 * where the disk address (daddr) is stored in a 16-bit type (for other
 * machines, a chunk is simply one character).  Using chunks wastes
 * some tmp file (and b_cache) space since lines must be aligned on
 * CHNK_CHARS boundaries. A CHNK_CHARS value large enough to hold
 * the vast majority of lines inside a single chunk means that each
 * unique daddr value refers to a chunk/line, making best use of
 * precious daddr bits (lines larger than CHNK_CHARS mean that daddr
 * values are skipped, which is not a big deal for 32bit or 64it
 * machines but really cuts into the max# of lines on PCDOS, for example.
 * Tmp file space, on the other hand, is relatively cheap, and wasting
 * b_cache only causes a lot more I/O slowness, but is not as painful
 * as running out of lines. As a point of reference, JOVE 4.17 compiled
 * for large model on PCDOS with Open Watcom 1.9 (space-optimized -os)
 * can fit around 32000 lines in memory from a 1.4MB text file before
 * starting to hit limits, and/or it can hold about 30 JOVE source files
 * simultaenously before running out of memory.
 *
 * The tmp file is accessed by blocks of size JBUFSIZ (typically
 * 512 bytes or larger).  Lines do NOT cross block boundaries in the
 * tmp file so that accessing the contents of lines can be much
 * faster.  Pointers into the interior of disk buffers are returned
 * instead of copying the contents into local arrays and then using
 * them.  This cuts down on the amount of copying a great deal, at
 * the expense of space efficiency.
 *
 * The high bit of each disk address is used for marking the line as
 * needing redisplay (DDIRTY).  In theory, this has nothing to do
 * with a disk address, but it is the cheapest place to hide a bit
 * in struct line (this is important since instances of this struct
 * are the major consumer of memory).
 *
 * The type daddr is used to represent the disk address of a line.
 * It is an integer containing three packed fields: the dirty bit,
 * the block number (within the tmp file), and the chunk number
 * (within the block).  Many of the following definitions are for
 * packing and unpacking daddr values.
 * On a system which limits JOVE to a very small data segment,
 * it may be worthwhile limiting daddr to a short.  This reduces
 * the size of a Line descriptor (so more line references fit in memory),
 * but reduces the addressable size of the temp file (so fewer
 * line contents can be stored on disk), which is reasonable
 * on 16-bit or very-small-memory machines (e.g. PDP-11 and iAPX 8088/8086).
 * NOTE: logically, daddr is unsigned, but a signed type will work
 * if you cannot use an unsigned type.
 *
 * There is a buffer cache of NBUF buffers (3 on SMALL machine,
 * and some larger value (e.g. 64) on !SMALL machines.  For 8086
 * memory models, NBUF*buffersize must be less than 64K).  The
 * blocks are stored in LRU order and each block is also stored
 * in a hash table by block #.  When a block is requested,
 * it can quickly be looked up in the hash table.  If it's not there the
 * LRU block is assigned the new block #.  If it finds that the LRU block
 * is dirty (i.e., has pending IO) it syncs the WHOLE tmp file, i.e.,
 * does all the pending writes.  This works much better on floppy disk
 * systems, like the IBM PC, if the blocks are sorted before sync'ing.
 *
 */

#ifndef LG_CHNK_CHARS
# ifdef JSMALL
#  define LG_CHNK_CHARS		7	/* save bits in daddr at cost of space in tempfile */
# else
# define LG_CHNK_CHARS		0
# endif
#endif

/* MAX_BLOCKS is the number of distinct block numbers that
 * can be represented in a daddr (a power of two!).
 * In fact, we don't allow block number MAX_BLOCKS-1 to be
 * used because NOWHERE_DADDR and UNSAVED_CURLINE_DADDR (disp.c) must not
 * be valid disk references, and we want to prevent space overflow
 * from being undetected through arithmetic overflow.
 *
 * MAX_BLOCKS is based on the number of bits remaining in daddr to
 * represent it, after space for the offset (in chunks) and the
 * DDIRTY bit is accounted for.
 */
#define MAX_BLOCKS		((daddr) 1 << (sizeof(daddr)*CHAR_BIT - LG_JBUFSIZ + LG_CHNK_CHARS - 1))

/* CHNK_CHARS is how big each chunk is.  For each 1 the DFree pointer
 * is incremented we extend the tmp file by CHNK_CHARS characters.
 * BLK_CHNKS is the # of chunks per block.
 *
 * NOTE:  It's pretty important that these numbers be powers of 2.
 * Be careful if you change things.
 */
#define CHNK_CHARS		((daddr)1 << LG_CHNK_CHARS)
#define REQ_CHNKS(chars)	(((daddr)(chars) + (CHNK_CHARS - 1)) >> LG_CHNK_CHARS)
#define BLK_CHNKS		((daddr)JBUFSIZ / CHNK_CHARS)
#define blk_chop(addr)		((daddr)(addr) & ~(BLK_CHNKS - 1))
#define da_to_bno(addr)		(((daddr)(addr) >> (LG_JBUFSIZ - LG_CHNK_CHARS)) & (MAX_BLOCKS - 1))
#define da_to_off(addr)		(((daddr)(addr) << LG_CHNK_CHARS) & ((daddr)JBUFSIZ - 1))
#define bno_to_seek_off(bno)	(((off_t)bno) << LG_JBUFSIZ)
