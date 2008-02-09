// TCP_Socket.h
// TCP_Socket for FS2Open PXO
// Derek Meek
// 2-14-2003

// ############## ATTENTION ##########
// Licensed under the Academic Free License version 2.0
// View License at http://www.opensource.org/licenses/afl-2.0.php
// ###################################


/*
 * $Logfile: /Freespace2/code/fs2open_pxo/TCP_Socket.h $
 * $Revision: 1.1.2.1 $
 * $Date: 2007-10-15 06:43:11 $
 * $Author: taylor $
 *
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.13  2006/01/26 03:23:29  Goober5000
 * pare down the pragmas some more
 * --Goober5000
 *
 * Revision 1.12  2006/01/20 07:10:33  Goober5000
 * reordered #include files to quash Microsoft warnings
 * --Goober5000
 *
 * Revision 1.11  2005/07/13 02:50:49  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 1.10  2005/06/29 18:49:37  taylor
 * various FS2NetD fixes:
 *  - replace timer stuff with something that more accurately works cross-platform and without being affected by load
 *  - better sanity checking for the server list
 *  - working Linux compatibility that's not dog slow
 *  - when calling DataReady() make sure that the data is properly valid
 *  - fix messed up cvs merge cleanup from the Linux merge which did nasty things
 *
 * Revision 1.9  2005/06/21 00:12:11  taylor
 * add virtual destructor so that GCC4 can shut up
 *
 * Revision 1.8  2005/02/04 20:06:03  taylor
 * merge with Linux/OSX tree - p0204-2
 *
 * Revision 1.7  2004/08/11 05:06:23  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 1.6  2004/03/31 05:42:26  Goober5000
 * got rid of all those nasty warnings from xlocale and so forth; also added comments
 * to indicate which warnings were being disabled
 * --Goober5000
 *
 * Revision 1.5  2004/03/09 17:59:01  Kazan
 * Disabled multithreaded TCP_Socket in favor of safer single threaded
 * FS2NetD doesn't kill the game on connection failure now - just gives warning message and effectively dsiables itself until they try to connect again
 *
 * Revision 1.4  2004/02/21 00:59:43  Kazan
 * FS2NETD License Comments
 *
 * Revision 1.3  2003/10/13 06:02:50  Kazan
 * Added Log Comment Thingy to these files
 *
 *
 *
 */

#if !defined(__TCP_SOCKET_H_)
#define __TCP_SOCKET_H_

#include "globalincs/pstypes.h" // make sure _cdecl is defined correctly on *nix


int FS2NetD_ConnectToServer(char *host, ushort port);
void FS2NetD_Disconnect();

int FS2NetD_GetData(char *buffer, int blen, bool OOB = false);
int FS2NetD_SendData(char *buffer, int msg_len, bool OOB = false);
void FS2NetD_IgnorePackets();


#endif // __TCP_SOCKET_H_
