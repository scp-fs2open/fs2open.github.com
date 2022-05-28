/*
 * Copyright (C) Volition, Inc. 2005.  All rights reserved.
 * 
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#include "globalincs/pstypes.h"
#include "network/chat_api.h"
#include "network/psnet2.h"
#include "network/multi_log.h"

#ifdef SCP_UNIX
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#ifdef SCP_SOLARIS
#include <sys/filio.h>
#endif
#include <netinet/in.h>
#include <sys/select.h>
#include <cerrno>
#include <arpa/inet.h>
#include <netdb.h>
#else
#include <winsock2.h>
typedef int socklen_t;
#endif

#include <cstdio>


#define MAXCHATBUFFER	500

SOCKET Chatsock = INVALID_SOCKET;
static SOCKADDR_STORAGE Chataddr;
int Socket_connecting = 0;
char Nick_name[33];
char Original_nick_name[33];
int Nick_variety = 0;
char szChat_channel[33] = "";
char Input_chat_buffer[MAXCHATBUFFER] = "";
char Chat_tracker_id[65];
char Getting_user_channel_info_for[33] = "";
char Getting_user_tracker_info_for[33] = "";
int Getting_user_channel_error = 0;
int Getting_user_tracker_error = 0;
char User_req_tracker_id[100] = ""; //These are oversized for saftey
char User_req_channel[100] = "";
char *User_list = NULL;
char *Chan_list = NULL;
int Socket_connected = 0;
int Chat_server_connected = 0;
int Joining_channel = 0;
int Joined_channel = 0;
int GettingChannelList = 0;
int GettingUserTID = 0;
int GettingUserChannel = 0;

Chat_user *Firstuser,*Curruser;
Chat_command *Firstcommand,*Currcommand;
Chat_channel *Firstchannel,*Currchannel;

void ChatInit(void)
{
	Socket_connecting = 0;
	memset(Nick_name, 0, sizeof(Nick_name));
	memset(Original_nick_name, 0, sizeof(Original_nick_name));
	Nick_variety = 0;
	memset(szChat_channel, 0, sizeof(szChat_channel));
	memset(Input_chat_buffer, 0, sizeof(Input_chat_buffer));
	memset(Chat_tracker_id, 0, sizeof(Chat_tracker_id));
	memset(Getting_user_channel_info_for, 0, sizeof(Getting_user_channel_info_for));
	memset(Getting_user_tracker_info_for, 0, sizeof(Getting_user_tracker_info_for));
	Getting_user_channel_error = 0;
	Getting_user_tracker_error = 0;
	memset(User_req_tracker_id, 0, sizeof(User_req_tracker_id));
	memset(User_req_channel, 0, sizeof(User_req_channel));
	User_list = NULL;
	Chan_list = NULL;
	Socket_connected = 0;
	Chat_server_connected = 0;
	Joining_channel = 0;
	Joined_channel = 0;
	GettingChannelList = 0;
	GettingUserTID = 0;
	GettingUserChannel = 0;

}


// Return codes:
//-2 Already connected
//-1 Failed to connect
// 0 Connecting
// 1 Connected
// Call it once with the server IP address, and it will return immediately
// with 0. Keep calling it until it returns something other than 0
// note: the nickname may be changed if someone with that name already
// exists (Scourge1 for instance)
int ConnectToChatServer(char *serveraddr, char *nickname, char *trackerid)
{
	uint16_t chat_port;
	char chat_server[50];
	char *p;
	unsigned long argp = 1;
	char signon_str[100];

	if(!Socket_connecting)
	{
		strncpy_s(Nick_name, nickname, sizeof(Nick_name)-1);
		strncpy_s(Original_nick_name, nickname, sizeof(Original_nick_name)-1);
		strncpy_s(Chat_tracker_id, trackerid, sizeof(Chat_tracker_id)-1);
		
		Firstuser = NULL;
		Firstcommand = NULL;
		Chat_server_connected = 0;
		FlushChatCommandQueue();

		p = strchr(serveraddr,':');

		if(NULL==p)
		{
			return -1;
		}

		strncpy_s(chat_server,serveraddr,(p-serveraddr));
		chat_port = static_cast<uint16_t>(atoi(p+1));
		if(0==chat_port)
		{
			return -1;
		}

		Chatsock = socket(AF_INET6, SOCK_STREAM, 0);

		if(INVALID_SOCKET == Chatsock)
		{
			return -1;
		}

		// make sure we are in dual-stack mode (not the default on Windows)
		int i_opt = 0;
		setsockopt(Chatsock, IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<const char *>(&i_opt), sizeof(i_opt));

		if ( !psnet_get_addr(nullptr, static_cast<uint16_t>(0), &Chataddr) )
		{
			return -1;
		}
		
		if (SOCKET_ERROR==bind(Chatsock, reinterpret_cast<LPSOCKADDR>(&Chataddr), psnet_get_sockaddr_len(&Chataddr)))
		{
			return -1;
		}

		ioctlsocket(Chatsock, FIONBIO, &argp);

		if ( !psnet_get_addr(chat_server, chat_port, &Chataddr) )
		{
			return -2;
		}

		if(SOCKET_ERROR == connect(Chatsock, reinterpret_cast<LPSOCKADDR>(&Chataddr), psnet_get_sockaddr_len(&Chataddr)))
		{
			if(WSAEWOULDBLOCK == WSAGetLastError())
			{
				Socket_connecting = 1;
				return 0;
			}
		}
		else
		{
			//This should never happen, connect should always return WSAEWOULDBLOCK
			Socket_connecting = 1;
			Socket_connected = 1;
			return 1;
		}
	}
	else
	{
		if(Chat_server_connected)
		{
			return 1;
		}

		if(!Socket_connected)
		{
			//Do a few select to check for an error, or to see if we are writeable (connected)
			fd_set write_fds,error_fds;	           
			struct timeval timeout;   

			timeout.tv_sec=0;            
			timeout.tv_usec=0;

			FD_ZERO(&write_fds);
			FD_SET(Chatsock, &write_fds);    
			//Writable -- that means it's connected
			if (select(static_cast<int>(Chatsock+1), NULL, &write_fds, NULL, &timeout) > 0)
			{
				// make sure that we don't have any connect() errors (since it's non-blocking)
				int err_val = 0;
				size_t err_val_size = sizeof(err_val);
				getsockopt(Chatsock, SOL_SOCKET, SO_ERROR, (char*)&err_val, (socklen_t*)&err_val_size);

				if (err_val)
				{
					if (err_val != WSAEWOULDBLOCK)
					{
						return -1;
					}

					return 0;
				}

				// check for server error messages
				const char *msg = ChatGetString();

				if (msg && !strcmp(msg, "ERROR"))
				{
					return -1;
				}

				Socket_connected = 1;
				memset(signon_str, 0, sizeof(signon_str));
				sprintf_safe(signon_str, NOX("/USER %s %s %s :%s"), NOX("user"), NOX("user"), NOX("user"), Chat_tracker_id);
				SendChatString(signon_str, 1);
				sprintf_safe(signon_str, NOX("/NICK %s"), Nick_name);
				SendChatString(signon_str,1);
				return 0;
				//Now we are waiting for Chat_server_connected
			}
			FD_ZERO(&error_fds);
			FD_SET(Chatsock,&error_fds);    
			//error -- that means it's not going to connect
			if ( select(static_cast<int>(Chatsock + 1), NULL, NULL, &error_fds, &timeout) )
			{
				return -1;
			}

			return 0;
		}
	}

	return 0;
}

/**
 * Call it to close the connection. It returns immediately
 */
void DisconnectFromChatServer()
{
	if (Socket_connected)
	{
		SendChatString(NOX("/QUIT"), 1);
	}

	if (Chatsock != INVALID_SOCKET)
	{
		shutdown(Chatsock, 2);
		closesocket(Chatsock);
	}

	Chatsock = INVALID_SOCKET;
	Socket_connecting = 0;
	Socket_connected = 0;
	Input_chat_buffer[0] = '\0';
	if(User_list)
	{
		vm_free(User_list);
		User_list = NULL;
	}
	if(Chan_list)
	{
		vm_free(Chan_list);
		Chan_list = NULL;
	}
	
	Chat_server_connected = 0;
	Joining_channel = 0;
	Joined_channel = 0;
	RemoveAllChatUsers();
	FlushChatCommandQueue();
	return;
}

// returns NULL if no line is there to print, otherwise returns a string to
// print (all preformatted of course)
char * GetChatText()
{

	if(!Socket_connected) return NULL;

	//ChatGetString will do the formatting
	return ChatGetString();

}

/**
 * Send a string to be sent as chat, or scanned for messages (/msg \<user\> string)
 */
const char * SendChatString(const char *line,int raw)
{
	char szCmd[200];
	char szTarget[50];
	if(!Socket_connected) return NULL;

	if(line[0]=='/')
	{
		//Start off by getting the command
		strncpy_s(szCmd, GetWordNum(0, line+1), sizeof(szCmd)-1);
		if(stricmp(szCmd,NOX("msg"))==0)
		{
			strncpy_s(szTarget, GetWordNum(1, line+1), sizeof(szTarget)-1);
			sprintf_safe(szCmd, NOX("PRIVMSG %s :%s\n\r"), szTarget, line+strlen(NOX("/msg "))+strlen(szTarget)+1);
			send(Chatsock,szCmd,(int)strlen(szCmd),0);
			szCmd[strlen(szCmd)-2] = '\0';
			return ParseIRCMessage(szCmd,MSG_LOCAL);

		}
		if(stricmp(szCmd,NOX("me"))==0)
		{
			sprintf_safe(szCmd, NOX("PRIVMSG %s :\001ACTION %s\001\n\r"), szChat_channel,line+strlen(NOX("/me ")));
			send(Chatsock,szCmd,(int)strlen(szCmd),0);
			szCmd[strlen(szCmd)-2] = '\0';
			return ParseIRCMessage(szCmd,MSG_LOCAL);

		}
		if(stricmp(szCmd,NOX("xyz"))==0)
		{
			//Special command to send raw irc commands
			sprintf_safe(szCmd, "%s\n\r", line+strlen(NOX("/xyz ")));
			send(Chatsock,szCmd,(int)strlen(szCmd),0);
			return NULL;
		}
		if(stricmp(szCmd,NOX("list"))==0)
		{
			sprintf_safe(szCmd, "%s\n\r", line+1);
			send(Chatsock,szCmd,(int)strlen(szCmd),0);
			return NULL;
		}
		if(raw)
		{
			sprintf_safe(szCmd, "%s\n\r", line+1);
			send(Chatsock,szCmd,(int)strlen(szCmd),0);
			return NULL;
		}
		return XSTR("Unrecognized command",634);
		
	}
	else
	{
		if(szChat_channel[0])
		{
			sprintf_safe(szCmd, NOX("PRIVMSG %s :%s\n\r"), szChat_channel, line);
			send(Chatsock,szCmd,(int)strlen(szCmd),0);
			if(strlen(szCmd) >= 2){
				szCmd[strlen(szCmd)-2] = '\0';
				return ParseIRCMessage(szCmd,MSG_LOCAL);
			} 			

			return NULL;
		}
	}
	
	return NULL;
}


// Returns a structure which contains a command and possible some data (like
// a user joining or leaving) if one is waiting
// This tells you if you need to add a user from the userlist, remove a user,
// etc. Also for status messages, like if you get knocked
// off the server for some reason.
Chat_command *GetChatCommand()
{
	if(!Socket_connected) return NULL;
	return GetChatCommandFromQueue();
}

// This function returns a list of users in the current channel, in one
// string, separated by spaces, terminated by a null
// (Spaces aren't allowed as part of a nickname)
char *GetChatUserList()
{
	size_t iuser_list_length = 0;;
	if(User_list)
	{
		vm_free(User_list);
		User_list = NULL;
	}
	if(!Socket_connected) return NULL;
	
	Curruser = Firstuser;
	while(Curruser) 
	{
		iuser_list_length += strlen(Curruser->nick_name)+1;
		Curruser = Curruser->next;
	}
	Curruser = Firstuser;
	User_list = (char *)vm_malloc(iuser_list_length+1);
	User_list[0] = '\0';
	while(Curruser) 
	{
		strcat(User_list,Curruser->nick_name);
		strcat(User_list," ");
		Curruser = Curruser->next;
	}

	return User_list;
}

// Call this to set/join a channel. Since we can't be sure that we will be
// able to join that channel, check it for completion
// You can't be in more than one channel at a time with this API, so you
// leave the current channel before trying to join
// a new one. Because of this if the join fails, make sure you try to join
// another channel, or the user won't be able to chat
//-1 Failed to join
// 0 joining
// 1 successfully joined
int SetNewChatChannel(char *channel)
{
	char partstr[100];
	if(!Socket_connected) return -1;

	partstr[sizeof(partstr)-1] = '\0';

	if(Joining_channel==1) 
	{
		if(Joined_channel==1) 
		{
			//We made it in!
			Joining_channel = 0;
			return 1;
		}
		else if(Joined_channel==-1) 
		{
			//Error -- we got a message that the channel was invite only, or we were banned or something
			Joining_channel = 0;
			strcpy_s(szChat_channel, "");
			return -1;
		}
	}
	else
	{
		if(szChat_channel[0])
		{
			sprintf_safe(partstr, NOX("/PART %s"), szChat_channel);
			SendChatString(partstr,1);
		}
		strncpy_s(szChat_channel, channel, sizeof(szChat_channel)-1);
		sprintf_safe(partstr, NOX("/JOIN %s"), szChat_channel);
		SendChatString(partstr,1);
		Joining_channel = 1;
		Joined_channel = 0;
	}
	
	return 0;
}


char *ChatGetString(void)
{
	fd_set read_fds;	           
	struct timeval timeout; 
	char ch[2];
	char *p;
	int bytesread;
	static char return_string[MAXCHATBUFFER];
	
	timeout.tv_sec=0;            
	timeout.tv_usec=1;
	
	FD_ZERO(&read_fds);
	FD_SET(Chatsock,&read_fds);    
	//Writable -- that means it's connected
	while ( select(static_cast<int>(Chatsock+1), &read_fds, nullptr, nullptr, &timeout) > 0 )
	{
		bytesread = recv(Chatsock,ch,1,0);
		if(bytesread)
		{
			ch[1] = '\0';
			
			if((ch[0] == 0x0a)||(ch[0]==0x0d))
			{
				if( !strlen(Input_chat_buffer) )
				{
					//Blank line, ignore it
					return NULL;
				}
				strncpy_s(return_string, Input_chat_buffer, sizeof(return_string)-1);
				Input_chat_buffer[0] = '\0';
				
				p = ParseIRCMessage(return_string,MSG_REMOTE);
				
				return p;
			}
			Assert(strlen(Input_chat_buffer) < MAXCHATBUFFER-1);
			strcat_s(Input_chat_buffer,ch);
		}
		else
		{
			//Select said we had read data, but 0 bytes read means disconnected
			AddChatCommandToQueue(CC_DISCONNECTED,NULL,0);
			return NULL;
		}
		
	}
	return NULL;
}


const char * GetWordNum(int num, const char * l_String)
{
	static char strreturn[600];
	static char ptokstr[600];
	char seps[10] = NOX(" \n\r\t");
	char *token,*strstart;

	strstart = ptokstr;

	strncpy_s(ptokstr, l_String, sizeof(ptokstr)-1);

	token=strtok(ptokstr,seps);

	for(int i=0;i!=num;i++)
	{
		token=strtok(NULL,seps);
	}
	if(token)
	{
		strncpy_s(strreturn, token, sizeof(strreturn)-1);
	}
	else
	{
		return "";
	}
	//check for the ':' char....
	if(token[0]==':')
	{
		//Its not pretty, but it works, return the rest of the string
		strncpy_s(strreturn, l_String+((token-strstart)+1), sizeof(strreturn)-1);
	}

	//return the appropriate response.
	return strreturn;
}

int AddChatUser(const char *nickname)
{
	Curruser = Firstuser;
	while(Curruser) 
	{
		if(stricmp(nickname,Curruser->nick_name)==0) return 0;
		Curruser = Curruser->next;
	}

	Curruser = Firstuser;
	if(Firstuser==NULL)
	{
		Firstuser = (Chat_user *)vm_malloc(sizeof(Chat_user));
		Assert(Firstuser);
		strncpy_s(Firstuser->nick_name, nickname, sizeof(Firstuser->nick_name)-1);
		Firstuser->next = NULL;
		AddChatCommandToQueue(CC_USER_JOINING,nickname,strlen(nickname)+1);
		return 1;
	}
	else
	{
		while(Curruser->next) 
		{
			Curruser = Curruser->next;
		}
		Curruser->next = (Chat_user *)vm_malloc(sizeof(Chat_user));
		Curruser = Curruser->next;
		Assert(Curruser);
		strncpy_s(Curruser->nick_name, nickname, sizeof(Curruser->nick_name)-1);
		Curruser->next = NULL;
		AddChatCommandToQueue(CC_USER_JOINING,nickname,strlen(nickname)+1);
		return 1;
	}

}

int RemoveChatUser(const char *nickname)
{
	Chat_user *prv_user = NULL;
	
	Curruser = Firstuser;
	while(Curruser) 
	{
		if(stricmp(nickname,Curruser->nick_name)==0)
		{
			if(prv_user)
			{
				prv_user->next = Curruser->next;

			}
			else
			{
				Firstuser = Curruser->next;
			}
			AddChatCommandToQueue(CC_USER_LEAVING,Curruser->nick_name,strlen(Curruser->nick_name)+1);
			vm_free(Curruser);
			return 1;
		}		
		prv_user = Curruser;
		Curruser = Curruser->next;
	}
	return 0;

}

void RemoveAllChatUsers(void)
{
	Chat_user *tmp_user = NULL;
	Curruser = Firstuser;
	while(Curruser) 
	{
		tmp_user = Curruser->next;
		AddChatCommandToQueue(CC_USER_LEAVING,Curruser->nick_name,strlen(Curruser->nick_name)+1);
		vm_free(Curruser);
		Curruser = tmp_user;
	}
	Firstuser = NULL;
}


char * ParseIRCMessage(char *Line, int iMode)
{
	char szRemLine[MAXLOCALSTRING] ="";
	const char *pszTempStr;
	char szPrefix[MAXLOCALSTRING] = "";
	char szHackPrefix[MAXLOCALSTRING] = "";
	char szTarget[MAXLOCALSTRING] = "";
	char szNick[MAXLOCALSTRING] = "";
	char szCmd[MAXLOCALSTRING] = "";
	char szCTCPCmd[MAXLOCALSTRING] = "";

	static char szResponse[MAXLOCALSTRING] = "";

	ptrdiff_t iPrefixLen = 0;	// JAS: Get rid of optimized warning

	if(strlen(Line)>=MAXLOCALSTRING)
	{
		return NULL; 
	}
	//Nick included....
	if(iMode==MSG_REMOTE)
	{
		strncpy_s(szRemLine, Line, sizeof(szRemLine)-1);
		//Start by getting the prefix
		if(Line[0]==':')
		{
			//
			pszTempStr=GetWordNum(0,Line+1);
			strncpy_s(szPrefix, pszTempStr, sizeof(szPrefix)-1);
			strncpy_s(szHackPrefix, pszTempStr, sizeof(szHackPrefix)-1);
			strncpy_s(szRemLine, Line+1+strlen(szPrefix), sizeof(szRemLine)-1);
		}
		//Next, get the Nick
		pszTempStr=strtok(szHackPrefix,"!");
		if(pszTempStr)
		{
			strncpy_s(szNick, pszTempStr, sizeof(szNick)-1);
		}
		else
		{
			strncpy_s(szNick,szPrefix,31);
		}
		iPrefixLen=strlen(szPrefix);
	}
	else if(iMode==MSG_LOCAL)
	{
		strncpy_s(szRemLine, Line, sizeof(szRemLine)-1);
		strncpy_s(szNick, Nick_name, sizeof(szNick)-1);
		strncpy_s(szPrefix, Nick_name, sizeof(szPrefix)-1);
		iPrefixLen=-2;
	}
	//Next is the command
	pszTempStr=GetWordNum(0,szRemLine);
	if(pszTempStr[0])
	{
		strncpy_s(szCmd, pszTempStr, sizeof(szCmd)-1);
	}
	else
	{
		//Shouldn't ever happen, but we can't be sure of what the host will send us.
		return NULL;
	}

	//Move the szRemLine string up
	strncpy_s(szRemLine, Line+iPrefixLen+strlen(szCmd)+2, sizeof(szRemLine)-1);
	//Now parse the commands!
	if(stricmp(szCmd,NOX("PRIVMSG"))==0)
	{
		pszTempStr=GetWordNum(0,szRemLine);
		strncpy_s(szTarget, pszTempStr, sizeof(szTarget)-1);
		strncpy_s(szRemLine, Line+iPrefixLen+strlen(szCmd)+strlen(szTarget)+4, sizeof(szRemLine)-1);
		if(szRemLine[0]==':')
		{
			strncpy_s(szCTCPCmd, GetWordNum(0,szRemLine+1), sizeof(szCTCPCmd)-1);
			if(szCTCPCmd[strlen(szCTCPCmd)-1]==0x01) szCTCPCmd[strlen(szCTCPCmd)-1]=0x00;

		}
		else
		{
			strncpy_s(szCTCPCmd, GetWordNum(0,szRemLine), sizeof(szCTCPCmd)-1);
			if(szCTCPCmd[strlen(szCTCPCmd)-1]==0x01) szCTCPCmd[strlen(szCTCPCmd)-1]=0x00;
		}
		if(szCTCPCmd[0]==0x01)
		{
			//Handle ctcp message
			strncpy_s(szRemLine, Line+iPrefixLen+strlen(szCmd)+strlen(szTarget)+strlen(szCTCPCmd)+6, sizeof(szRemLine)-1);
			szRemLine[strlen(szRemLine)-1] = '\0';//null out the ending 0x01
			if(stricmp(szCTCPCmd+1,NOX("ACTION"))==0)
			{
				//Posture
				sprintf_safe(szResponse, "* %s %s", szNick, szRemLine);
				return szResponse;
			}
			if(iMode==MSG_LOCAL)
			{
				strncpy_s(szHackPrefix, Line+iPrefixLen+strlen(szCmd)+strlen(szTarget)+4, sizeof(szHackPrefix)-1);
				szRemLine[strlen(szRemLine)-1] = '\0';
				sprintf_safe(szResponse, NOX("** CTCP %s %s %s"), szTarget, szCTCPCmd+1, szRemLine);
				return szResponse;
			}
			if(stricmp(szCTCPCmd+1,NOX("PING"))==0)
			{
				sprintf_safe(szResponse, NOX("/NOTICE %s :\001PING %s\001"), szNick, szRemLine);//Don't need the trailing \001 because szremline has it.
				SendChatString(szResponse,1);
				return NULL;
			}
			if(stricmp(szCTCPCmd+1,NOX("VERSION"))==0)
			{
				return NULL;
			}
			strncpy_s(szRemLine, 1 + GetWordNum(0,Line+iPrefixLen+strlen(szCmd)+strlen(szTarget)+4), sizeof(szRemLine)-1);
			szRemLine[strlen(szRemLine)-1] = '\0';
			sprintf_safe(szResponse, NOX("** CTCP Message from %s (%s)"), szNick, szRemLine);
			return szResponse;

		}
		//differentiate between channel and private
		if(szTarget[0]=='#')
		{
			pszTempStr=GetWordNum(0,szRemLine);
			sprintf_safe(szResponse, "[%s] %s", szNick, pszTempStr);
			return szResponse;
		}
		else
		{
			if(iMode == MSG_LOCAL)
			{
				pszTempStr=GetWordNum(0,szRemLine);
				sprintf_safe(szResponse, NOX("Private Message to <%s>: %s"), szNick, pszTempStr);
			}
			else
			{
				pszTempStr=GetWordNum(0,szRemLine);
				sprintf_safe(szResponse, NOX("Private Message from <%s>: %s"), szNick, pszTempStr);
			}
			return szResponse;
		}

	}
	//don't handle any other messages locally.
	if(iMode==MSG_LOCAL)
	{
		return NULL;
	}

	if(stricmp(szCmd,NOX("ERROR"))==0)
	{
		// Server reported errors
		ml_printf("Chat ERROR:  %s", szRemLine);
		AddChatCommandToQueue(CC_DISCONNECTED, nullptr, 0);
		strcpy_s(szResponse, "ERROR");
		return szResponse;
	}

	if(stricmp(szCmd,NOX("NOTICE"))==0)
	{
		pszTempStr=GetWordNum(0,szRemLine);
		strncpy_s(szTarget, pszTempStr, sizeof(szTarget)-1);
		strncpy_s(szRemLine, Line+iPrefixLen+strlen(szCmd)+strlen(szTarget)+4, sizeof(szRemLine)-1);
		if(szRemLine[0]==':')
		{
			strncpy_s(szCTCPCmd, GetWordNum(0,szRemLine+1), sizeof(szCTCPCmd)-1);
			if(szCTCPCmd[strlen(szCTCPCmd)-1]==0x01) szCTCPCmd[strlen(szCTCPCmd)-1]=0x00;
		}
		else
		{
			strncpy_s(szCTCPCmd, GetWordNum(0,szRemLine), sizeof(szCTCPCmd)-1);
			if(szCTCPCmd[strlen(szCTCPCmd)-1]==0x01) szCTCPCmd[strlen(szCTCPCmd)-1]=0x00;
		}
		if(szCTCPCmd[0]==0x01)
		{
			//Handle ctcp message
			strncpy_s(szRemLine, Line+iPrefixLen+strlen(szCmd)+strlen(szTarget)+strlen(szCTCPCmd)+6, sizeof(szRemLine)-1);
			szRemLine[strlen(szRemLine)-1] = '\0';//null out the ending 0x01
			if(stricmp(szCTCPCmd+1,NOX("PING"))==0)
			{
				return NULL;
			}
			
			//Default message
			strncpy_s(szRemLine, 1 + GetWordNum(0,Line+iPrefixLen+strlen(szCmd)+strlen(szTarget)+4), sizeof(szRemLine)-1);
			szRemLine[strlen(szRemLine)-1] = '\0';
			sprintf_safe(szResponse, XSTR("** CTCP Message from %s (%s)", 635), szNick, szRemLine);
			return szResponse;
			
		}
		sprintf_safe(szResponse, "%s", szRemLine);
		return NULL;
	}
	if(stricmp(szCmd,NOX("JOIN"))==0)
	{
		//see if it is me!
		if(stricmp(Nick_name,szNick)==0)
		{
			Joined_channel = 1;
			if(stricmp(szChat_channel,NOX("#autoselect"))==0)
			{
				strncpy_s(szChat_channel, GetWordNum(0,szRemLine), sizeof(szChat_channel)-1);
				AddChatCommandToQueue(CC_YOURCHANNEL,szChat_channel,strlen(szChat_channel)+1);

			}
		}
		
		pszTempStr=GetWordNum(0,szRemLine);
		strncpy_s(szTarget, pszTempStr, sizeof(szTarget)-1);

		AddChatUser(szNick);
		sprintf_safe(szResponse, XSTR("** %s has joined %s", 636), szNick, szTarget);
		return NULL;//szResponse;
		//Add them to the userlist too!
	}
	if(stricmp(szCmd,NOX("PART"))==0)
	{
		pszTempStr=GetWordNum(0,szRemLine);
		strncpy_s(szTarget, pszTempStr, sizeof(szTarget)-1);
		strncpy_s(szRemLine, Line+iPrefixLen+strlen(szCmd)+strlen(szTarget)+3, sizeof(szRemLine)-1);
		//see if it is me!
		if(stricmp(Nick_name,szNick)==0)
		{
			RemoveAllChatUsers();
		}
		
		RemoveChatUser(szNick);
		return NULL;
		//Remove them to the userlist too!
	}
	if(stricmp(szCmd,NOX("KICK"))==0)
	{
		pszTempStr=GetWordNum(0,szRemLine);
		strncpy_s(szTarget, pszTempStr, sizeof(szTarget)-1);
		pszTempStr=GetWordNum(1,szRemLine);
		strncpy_s(szHackPrefix, pszTempStr, sizeof(szHackPrefix)-1);
		pszTempStr=GetWordNum(2,szRemLine);
		//see if it is me!
		if(stricmp(Nick_name,GetWordNum(1,szRemLine))==0)
		{
			//Yup, it's me!
			szChat_channel[0] = '\0';
			AddChatCommandToQueue(CC_KICKED,NULL,0);			
			RemoveAllChatUsers();
		}
		sprintf_safe(szResponse, XSTR("*** %s has kicked %s from channel %s (%s)", 637), szNick, szHackPrefix, szTarget, pszTempStr);
		//Remove them to the userlist too!
		RemoveChatUser(szNick);
		return szResponse;
		
	}
	if(stricmp(szCmd,NOX("NICK"))==0)
	{
      //see if it is me!
		if(stricmp(Nick_name,szNick)==0)
		{
			//Yup, it's me!
			strncpy_s(Nick_name, GetWordNum(0,szRemLine), sizeof(Nick_name)-1);
		}
		char nicks[70];
		sprintf_safe(nicks, "%s %s", szNick, GetWordNum(0, szRemLine));
		AddChatCommandToQueue(CC_NICKCHANGED,nicks,strlen(nicks)+1);
		RemoveChatUser(szNick);
		AddChatUser(GetWordNum(0,szRemLine));
		sprintf_safe(szResponse, XSTR("*** %s is now known as %s", 638), szNick, GetWordNum(0, szRemLine));
		return szResponse;
	}
	if(stricmp(szCmd,NOX("PING"))==0)
	{
		//respond with pong (GetWordNum(0,szRemLine))
		sprintf_safe(szResponse, NOX("/PONG :%s"), GetWordNum(0, szRemLine));
		SendChatString(szResponse,1);
		return NULL;
	}
	if(stricmp(szCmd,NOX("MODE"))==0)
	{
		//Channel Mode info
		return NULL;
	}


	if(stricmp(szCmd, "403")==0)
	{
		// ERR_NOSUCHCHANNEL - Used to indicate the given channel name is invalid.
		Joined_channel = -1;
		return NULL;
	}

	if(stricmp(szCmd,"401")==0)
	{
		//This is whois user info, we can get their tracker info from here.  -5
		char szWhoisUser[33];
		strncpy_s(szWhoisUser, GetWordNum(1,szRemLine), sizeof(szWhoisUser)-1);
		Getting_user_tracker_error = 1;			
		Getting_user_channel_error = 1;

		sprintf_safe(szResponse, XSTR("**Error: %s is not online!", 639), szWhoisUser);
		return szResponse;

	}
	if(stricmp(szCmd,"311")==0)
	{
		char szWhoisUser[33];
		strncpy_s(szWhoisUser, GetWordNum(1,szRemLine), sizeof(szWhoisUser)-1);
		//This is whois user info, we can get their tracker info from here.  -5
		strncpy_s(User_req_tracker_id, GetWordNum(5,szRemLine), sizeof(User_req_tracker_id)-1);
		return NULL;
	}
	if(stricmp(szCmd,"319")==0)
	{
		char szWhoisUser[33];
		strncpy_s(szWhoisUser, GetWordNum(1,szRemLine), sizeof(szWhoisUser)-1);
		//This is whois channel info -- what channel they are on		-2
		strncpy_s(User_req_channel, GetWordNum(2,szRemLine), sizeof(User_req_channel)-1);
		return NULL;
	}
	
	//End of whois and we didn't get a channel means they aren't in a channel.
	if(stricmp(szCmd,"318")==0)
	{
		if(!*User_req_channel)
		{
			User_req_channel[0] = '*';
		}
	}


	if(stricmp(szCmd,"321")==0)
	{
		//start of channel list
		FlushChannelList();
		GettingChannelList = 1;
		return NULL;
	}
	if(stricmp(szCmd,"322")==0)
	{
		//channel list data
		if(GettingChannelList == 1)
		{
			char channel_list_name[33];
			char sztopic[200];
			strncpy_s(sztopic, GetWordNum(3,szRemLine), sizeof(sztopic)-1);
			strncpy_s(channel_list_name, GetWordNum(1,szRemLine), sizeof(channel_list_name)-1);
			AddChannel(channel_list_name,(short)atoi(GetWordNum(2,szRemLine)),sztopic);
		}
		return NULL;
	}
	if(stricmp(szCmd,"323")==0)
	{
		//end of channel list
		GettingChannelList = 2;
		return NULL;
	}
	if(stricmp(szCmd,"324")==0)
	{
		//Channel Mode info
		return NULL;
	}

	if(stricmp(szCmd,"332")==0)
	   	{
		//Channel Topic, update status bar.
		if(stricmp(szChat_channel,szTarget)==0)
		{
			//strncpy_s(szChanTopic,GetWordNum(2,szRemLine),70);
		}
		//sprintf(NewMsg.Message,"*** %s has changed the topic to: %s",szNick,GetWordNum(2,szRemLine));

		return NULL;
	}
	if(stricmp(szCmd,NOX("TOPIC"))==0)
	{
		//Channel Topic, update status bar.
		if(stricmp(szChat_channel,szTarget)==0)
		{
			//strncpy_s(szChanTopic,GetWordNum(1,szRemLine),70);
		}
		//sprintf(NewMsg.Message,"*** %s has changed the topic to: %s",szNick,GetWordNum(1,szRemLine));
		return NULL;
	}
	if(stricmp(szCmd,NOX("QUIT"))==0)
	{
		//Remove the user!
		RemoveChatUser(szNick);
		return NULL;
	}
	if(stricmp(szCmd,"376")==0) //end of motd, trigger autojoin...
	{
		if (!Chat_server_connected)
		{
			Chat_server_connected=1;
		}

		// end of motd
		strncpy_s(szResponse, PXO_CHAT_END_OF_MOTD_PREFIX, sizeof(szResponse)-1);
		return szResponse;
	}
	if((stricmp(szCmd,"377")==0)||
		(stricmp(szCmd,"372")==0))
	{
		//Stip the message, and display it.
		pszTempStr=GetWordNum(3,Line);
		sprintf_safe(szResponse, "%s%s", PXO_CHAT_MOTD_PREFIX, pszTempStr);
		return szResponse;
	}
	//Ignore these messages
	if(((stricmp(szCmd,"366")==0))||
	    (stricmp(szCmd,"333")==0) || //Who set the topic
		 (stricmp(szCmd,"329")==0))    //Time Channel created

	{
		return NULL;
	}
	if(stricmp(szCmd,"353")==0)
	{
		//Names in the channel.
		pszTempStr = GetWordNum(3,Line+iPrefixLen+strlen(szCmd)+2);
		strncpy_s(szRemLine, pszTempStr, sizeof(szRemLine)-1);
		pszTempStr = strtok(szRemLine," ");

		while(pszTempStr)
		{
			if(pszTempStr[0]=='@')
			{
				AddChatUser(pszTempStr+1);
			}
			else if(pszTempStr[0]=='+')
			{
				AddChatUser(pszTempStr+1);
			}
			else
			{
				AddChatUser(pszTempStr);
			}
			pszTempStr=strtok(NULL," ");
		}
		return NULL;
	}
	//MOTD Codes
	if((stricmp(szCmd,"001")==0)||
	   (stricmp(szCmd,"002")==0)||
	   (stricmp(szCmd,"003")==0)||
	   (stricmp(szCmd,"004")==0)||
	   (stricmp(szCmd,"251")==0)||
	   (stricmp(szCmd,"254")==0)||
	   (stricmp(szCmd,"255")==0)||
	   (stricmp(szCmd,"265")==0)||
	   (stricmp(szCmd,"372")==0)||
	   (stricmp(szCmd,"375")==0)
	   )
	{
		return NULL;
		// return szResponse;
	}
	if(stricmp(szCmd,"432")==0)
	{
		//Channel Mode info
		sprintf_safe(szResponse, "%s", XSTR("Your nickname contains invalid characters", 640));
		AddChatCommandToQueue(CC_DISCONNECTED,NULL,0);
		return szResponse;
	}
	if(stricmp(szCmd,"433")==0)
	{
		//Channel Mode info
		char new_nick[33];
		sprintf_safe(new_nick, "%s%d", Original_nick_name, Nick_variety);
		strncpy_s(Nick_name, new_nick, sizeof(Nick_name)-1);
		Nick_variety++;
		sprintf_safe(szResponse, NOX("/NICK %s"), new_nick);
		SendChatString(szResponse,1);
		return NULL;
	}
	//Default print
	strncpy_s(szResponse, Line, sizeof(szResponse)-1);
	return NULL;
}


void AddChatCommandToQueue(int command,const void *data,size_t len)
{
	Currcommand = Firstcommand;
	if(Firstcommand==NULL)
	{
		Firstcommand = (Chat_command *)vm_malloc(sizeof(Chat_command));
		Assert(Firstcommand);
		Firstcommand->next = NULL;
		Currcommand = Firstcommand;
	}
	else
	{
		while(Currcommand->next) 
		{
			Currcommand = Currcommand->next;
		}
		Currcommand->next = (Chat_command *)vm_malloc(sizeof(Chat_command));
		Assert(Currcommand->next);
		Currcommand = Currcommand->next;
	}
	Currcommand->command = (short)command;
	if(len&&data) memcpy(&Currcommand->data,data,len);
	Currcommand->next = NULL;
	return;
}

Chat_command *GetChatCommandFromQueue(void)
{
	static Chat_command response_cmd;
	Chat_command *tmp_cmd;
	if(!Firstcommand) return NULL;
	Currcommand = Firstcommand;
	memcpy(&response_cmd,Currcommand,sizeof(Chat_command));
	tmp_cmd = Currcommand->next;
	vm_free(Firstcommand);
	Firstcommand = tmp_cmd;
	return &response_cmd;
}

void FlushChatCommandQueue(void)
{
	Chat_command *tmp_cmd;
	Currcommand = Firstcommand;
	
	while(Currcommand) 
	{
		tmp_cmd = Currcommand->next;
		vm_free(Currcommand);
		Currcommand = tmp_cmd;
	}
	Firstcommand = NULL;
}


void FlushChannelList(void)
{
	Chat_channel *tmp_chan;
	Currchannel = Firstchannel;
	
	while(Currchannel) 
	{
		tmp_chan = Currchannel->next;
		vm_free(Currchannel);
		Currchannel = tmp_chan;
	}
	Firstchannel = NULL;


}
char *GetChannelList(void)
{
	size_t ichan_list_length = 0;
	char sznumusers[10];
	
	if(GettingChannelList != 2) return NULL;
	if(!Socket_connected) return NULL;

	if(Chan_list)
	{
		vm_free(Chan_list);
		Chan_list = NULL;
	}
	
	
	Currchannel = Firstchannel;
	while(Currchannel) 
	{
		ichan_list_length += strlen(Currchannel->topic)+1+strlen(Currchannel->channel_name)+1+5;//1 for the space, and 4 for the number of users 0000-9999 + space
		Currchannel = Currchannel->next;
	}
	Currchannel = Firstchannel;
	Chan_list = (char *)vm_malloc(ichan_list_length+1);
	memset(Chan_list, 0, ichan_list_length+1);
	while(Currchannel) 
	{
		strcat(Chan_list,"$");
		strcat(Chan_list,Currchannel->channel_name);
		strcat(Chan_list," ");
		sprintf(sznumusers,"%d ",Currchannel->users);
		strcat(Chan_list,sznumusers);
		strcat(Chan_list,Currchannel->topic);//fgets
		strcat(Chan_list," ");
		Currchannel = Currchannel->next;
	}
	FlushChannelList();
	GettingChannelList = 0;
	return Chan_list;
}

void AddChannel(char *channel,unsigned short numusers,char *topic)
{
	Currchannel = Firstchannel;
	if(Firstchannel==NULL)
	{
		Firstchannel = (Chat_channel *)vm_malloc(sizeof(Chat_channel));
		Assert(Firstchannel);
		strncpy_s(Firstchannel->channel_name, channel, sizeof(Firstchannel->channel_name)-1);
		strncpy_s(Firstchannel->topic, topic, sizeof(Firstchannel->topic)-1);
		Firstchannel->users = numusers;
		Firstchannel->next = NULL;
		Currchannel = Firstchannel;
	}
	else
	{
		while(Currchannel->next) 
		{
			Currchannel = Currchannel->next;
		}
		Currchannel->next = (Chat_channel *)vm_malloc(sizeof(Chat_channel));
		Assert(Currchannel->next);
		Currchannel = Currchannel->next;
		strncpy_s(Currchannel->channel_name, channel, sizeof(Currchannel->channel_name)-1);
		strncpy_s(Currchannel->topic, topic, sizeof(Currchannel->topic)-1);
		Currchannel->users = numusers;
	}
	Currchannel->next = NULL;
	return;
}


char *GetTrackerIdByUser(char *nickname)
{
	char szWhoisCmd[100];
	
	if(GettingUserTID)
	{
		if(Getting_user_tracker_error)
		{
			Getting_user_tracker_error = 0;
			Getting_user_channel_error = 0;
			GettingUserTID = 0;
			return (char *)-1;
		}
		
		if(*User_req_tracker_id)
		{
			GettingUserTID = 0;
			return User_req_tracker_id;
		}
	}
	else
	{
		strncpy_s(Getting_user_tracker_info_for, nickname, sizeof(Getting_user_tracker_info_for)-1);
		sprintf_safe(szWhoisCmd, NOX("/WHOIS %s"), nickname);
		User_req_tracker_id[0] = '\0';
		SendChatString(szWhoisCmd,1);		
		GettingUserTID = 1;
	}
	return NULL;
}

char *GetChannelByUser(char *nickname)
{
	char szWhoisCmd[100];
	
	if(GettingUserChannel)
	{
		if(Getting_user_channel_error)
		{
			Getting_user_channel_error = 0;
			Getting_user_tracker_error = 0;
			GettingUserChannel = 0;
			return (char *)-1;
		}
		if(*User_req_channel)
		{
			GettingUserChannel = 0;
			return User_req_channel;
		}
	}
	else
	{
		strncpy_s(Getting_user_channel_info_for, nickname, sizeof(Getting_user_channel_info_for)-1);
		User_req_channel[0] = '\0';
		sprintf_safe(szWhoisCmd, NOX("/WHOIS %s"), nickname);
		SendChatString(szWhoisCmd,1);
		GettingUserChannel = 1;
	}
	return NULL;
}
