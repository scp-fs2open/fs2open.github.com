// Place all prepreprcoessor definitions that tuurn features on/off
// into this file.
// the purpose of this is to prevent what happens with FRED
// when DECALS_ENABLED was added to code.lib

/*
 * $Logfile: /Freespace2/code/PreProcDefines.h $
 * $Revision: 1.6 $
 * $Date: 2004-12-30 07:26:21 $
 * $Author: argv $
 *
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.5  2004/12/05 22:01:11  bobboau
 * sevral feature additions that WCS wanted,
 * and the foundations of a submodel animation system,
 * the calls to the animation triggering code (exept the procesing code,
 * wich shouldn't do anything without the triggering code)
 * have been commented out.
 *
 * Revision 1.4  2004/10/31 21:23:08  taylor
 * #define if !#defined, add USE_DEVIL and FS2_SPEECH commented out
 *
 * Revision 1.3  2004/10/06 22:02:53  Kazan
 * interface corruption fix - thanks taylor (MAX_BITMAPS upped to 7000)
 *
 * Revision 1.2  2004/08/20 05:13:07  Kazan
 * wakka wakka - fix minor booboo
 *
 * Revision 1.1  2004/08/11 05:06:17  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 *
 */

#if !defined(_pre_proc_defs_h_)
#define _pre_proc_defs_h_

#ifndef DECALS_ENABLED
#define DECALS_ENABLED		1
#endif

#ifndef HTL
#define HTL					1
#endif

#ifndef USE_OPENGL
#define USE_OPENGL			1
#endif

#ifndef MORE_SPECIES
#define MORE_SPECIES		1
#endif

#ifndef ENABLE_AUTO_PILOT
#define ENABLE_AUTO_PILOT	1
#endif

#ifndef _REPORT_MEM_LEAKS
#define _REPORT_MEM_LEAKS	1
#endif


/*
#ifndef USE_DEVIL
#define USE_DEVIL			1
#endif

#ifndef FS2_SPEECH
#define FS2_SPEECH			1
#endif
*/

// _argv[-1], 27 Dec 2004: Turns off the 50% rate-of-fire penalty
// when primary banks are linked. This is, arguably, stupid. So we
// turn it off if builder says to. This is not a command-line option,
// as that would conceivably be an excessively easy way of cheating in
// multi. Let's at least force cheaters to hack the code, right?
/*
#ifndef NO_LINKED_PRIMARY_PENALTY
#define NO_LINKED_PRIMARY_PENALTY 1
#endif
*/

#endif // _pre_proc_defs_h_
