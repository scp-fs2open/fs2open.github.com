// Place all prepreprcoessor definitions that tuurn features on/off
// into this file.
// the purpose of this is to prevent what happens with FRED
// when DECALS_ENABLED was added to code.lib

/*
 * $Logfile: /Freespace2/code/PreProcDefines.h $
 * $Revision: 1.2 $
 * $Date: 2004-08-20 05:13:07 $
 * $Author: Kazan $
 *
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2004/08/11 05:06:17  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 *
 */

#if !defined(_pre_proc_defs_h_)
#define _pre_proc_defs_h_

#define DECALS_ENABLED		1
#define HTL					1
#define USE_OPENGL			1
#define MORE_SPECIES		1
#define ENABLE_AUTO_PILOT	1

#endif // _pre_proc_defs_h_