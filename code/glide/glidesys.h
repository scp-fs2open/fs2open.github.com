/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
**
** $Header: /cvs/cvsroot/fs2open/fs2_open/code/glide/glidesys.h,v 2.0 2002-06-03 04:02:22 penguin Exp $
** $Log: not supported by cvs2svn $
** Revision 1.1  2002/05/02 18:03:07  mharris
** Initial checkin - converted filenames and includes to lower case
**
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:48a Dave
 * 
 * 2     2/27/98 3:25p Frank
 * (John/Frank K.) updated to 2.43 headers
 * 
 * 1     10/16/97 11:51a Klier
*/
#ifndef __GLIDESYS_H__
#define __GLIDESYS_H__

/*
n** -----------------------------------------------------------------------
** COMPILER/ENVIRONMENT CONFIGURATION
** -----------------------------------------------------------------------
*/

/* Endianness is stored in bits [30:31] */
#define GLIDE_ENDIAN_SHIFT      30
#define GLIDE_ENDIAN_LITTLE     (0x1 << GLIDE_ENDIAN_SHIFT)
#define GLIDE_ENDIAN_BIG        (0x2 << GLIDE_ENDIAN_SHIFT)

/* OS is stored in bits [0:6] */
#define GLIDE_OS_SHIFT          0
#define GLIDE_OS_UNIX           0x1
#define GLIDE_OS_DOS32          0x2
#define GLIDE_OS_WIN32          0x4
#define GLIDE_OS_SYSTEM7        0x8
#define GLIDE_OS_OS2            0x10
#define GLIDE_OS_OTHER          0x20 /* For Proprietary Arcade HW */

/* Sim vs. Hardware is stored in bits [7:8] */
#define GLIDE_SST_SHIFT         7
#define GLIDE_SST_SIM           (0x1 << GLIDE_SST_SHIFT)
#define GLIDE_SST_HW            (0x2 << GLIDE_SST_SHIFT )

/* Hardware Type is stored in bits [9:12] */
#define GLIDE_HW_SHIFT          9
#define GLIDE_HW_SST1           (0x1 << GLIDE_HW_SHIFT)
#define GLIDE_HW_SST96          (0x2 << GLIDE_HW_SHIFT)
#define GLIDE_HW_SSTH3          (0x4 << GLIDE_HW_SHIFT)
#define GLIDE_HW_SST2           (0x8 << GLIDE_HW_SHIFT)

/*
** Make sure we handle all instances of WIN32
*/
#ifndef __WIN32__
#  if defined ( _WIN32 ) || defined (WIN32) || defined(__NT__)
#    define __WIN32__
#  endif
#endif

/* We need two checks on the OS: one for endian, the other for OS */
/* Check for endianness */
#if defined(__IRIX__) || defined(__sparc__) || defined(MACOS)
#  define GLIDE_ENDIAN    GLIDE_ENDIAN_BIG
#else
#  define GLIDE_ENDIAN GLIDE_ENDIAN_LITTLE
#endif

/* Check for OS */
#if defined(__IRIX__) || defined(__sparc__) || defined(__linux__)
#  define GLIDE_OS        GLIDE_OS_UNIX
#elif defined(__DOS__)
#  define GLIDE_OS        GLIDE_OS_DOS32
#elif defined(__WIN32__)
#  define GLIDE_OS        GLIDE_OS_WIN32
#endif

/* Check for Simulator vs. Hardware */
#ifdef GLIDE_SIMULATOR
#  define GLIDE_SST       GLIDE_SST_SIM
#else
#  define GLIDE_SST     GLIDE_SST_HW
#endif

/* Check for type of hardware */
#ifdef SST96
#  define GLIDE_HW        GLIDE_HW_SST96
#elif defined(SSTH3)
#  define GLIDE_HW        GLIDE_HW_SSTH3
#elif defined(SST2)
#  define GLIDE_HW        GLIDE_HW_SST2
#else /* Default to SST1 */
#  define GLIDE_HW        GLIDE_HW_SST1
#endif


#define GLIDE_PLATFORM (GLIDE_ENDIAN | GLIDE_OS | GLIDE_SST | GLIDE_HW)

/*
** Control the number of TMUs
*/
#ifndef GLIDE_NUM_TMU
#  define GLIDE_NUM_TMU 2
#endif


#if ( ( GLIDE_NUM_TMU < 0 ) && ( GLIDE_NUM_TMU > 3 ) )
#  error "GLIDE_NUM_TMU set to an invalid value"
#endif

#endif /* __GLIDESYS_H__ */
