// TCP_Socket.h
// TCP_Socket for FS2Open PXO
// Derek Meek
// 2-14-2003

// ############## ATTENTION ##########
// Licensed under the Academic Free License version 2.0
// View License at http://www.opensource.org/licenses/afl-2.0.php
// ###################################




#if !defined(__TCP_SOCKET_H_)
#define __TCP_SOCKET_H_

#include "globalincs/pstypes.h" // make sure _cdecl is defined correctly on *nix


int FS2NetD_ConnectToServer(const char *host, const char *port);
void FS2NetD_Disconnect();

int FS2NetD_GetData(char *buffer, int blen);
int FS2NetD_SendData(char *buffer, int blen);
bool FS2NetD_DataReady();


#endif // __TCP_SOCKET_H_
