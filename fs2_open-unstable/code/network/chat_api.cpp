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

#ifdef SCP_UNIX
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netdb.h>

#define WSAGetLastError()  (errno)
#else
#include <winsock.h>
#endif

#include <stdio.h>


#define MAXCHATBUFFER	500

SOCKET Chatsock;
SOCKADDR_IN Chataddr;
int Socket_connecting = 0;
char Nick_name[33];
char Orignial_nick_name[33];
int Nick_variety = 0;
char szChat_channel[33] = "";
char Input_chat_buffer[MAXCHATBUFFER] = "";
char Chat_tracker_id[33];
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
	Nick_name[0] = '\0';
	Orignial_nick_name[0] = '\0';
	Nick_variety = 0;
	szChat_channel[0] = '\0';
	Input_chat_buffer[0] = '\0';
	Chat_tracker_id[0] = '\0';
	Getting_user_channel_info_for[0] = '\0';
	Getting_user_tracker_info_for[0] = '\0';
	Getting_user_channel_error = 0;
	Getting_user_tracker_error = 0;
	User_req_tracker_id[0] = '\0';
	User_req_channel[0] = '\0';
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
	short chat_port;
	char chat_server[50];
	char *p;
	unsigned long argp = 1;
	char signon_str[100];

	//if(Socket_connected && ) return -2;

	if(!Socket_connecting)
	{
		unsigned long iaddr;

		strcpy(Nick_name,nickname);
		strcpy(Orignial_nick_name,nickname);
		strcpy(Chat_tracker_id,trackerid);
		
		Firstuser = NULL;
		Firstcommand = NULL;
		Chat_server_connected = 0;
		FlushChatCommandQueue();

		p = strchr(serveraddr,':');

		if(NULL==p)
		{
			//AfxMessageBox("Invalid chat server, must be host.com:port (ie. irc.dal.net:6667)");
			return -1;
		}
		strncpy(chat_server,serveraddr,(p-serveraddr));
		chat_server[p-serveraddr] = '\0';
		chat_port = (short)atoi(p+1);
		if(0==chat_port)
		{
			//AfxMessageBox("Invalid chat port, must be host.com:port (ie. irc.dal.net:6667)");
			return -1;
		}

		Chatsock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if(INVALID_SOCKET == Chatsock)
		{
			//AfxMessageBox("Unable to open socket!");
			return -1;
		}

		memset( &Chataddr, 0, sizeof(SOCKADDR_IN) );
		Chataddr.sin_family = AF_INET; 
		Chataddr.sin_addr.s_addr = INADDR_ANY; 
		Chataddr.sin_port = 0;
		
		if (SOCKET_ERROR==bind(Chatsock, (SOCKADDR*)&Chataddr, sizeof (sockaddr))) 
		{
			//AfxMessageBox("Unable to bind socket!");
			return -1;
		}

		ioctlsocket(Chatsock, FIONBIO, &argp);
		
		// first try and resolve by name
		iaddr = inet_addr( chat_server );
		if ( iaddr == INADDR_NONE ) {	
			HOSTENT *he;
			he = gethostbyname(chat_server);
			if(!he)
			{
				return 0;
				/*
				//AfxMessageBox("Unable to gethostbyname.\n");

				// try and resolve by address			
				unsigned int n_order = inet_addr(chat_server);
				he = gethostbyaddr((char*)&n_order,4,PF_INET);					

				if(!he){
					return -1;
				}
				*/
			}
			memcpy(&iaddr, he->h_addr_list[0],4);
		}
		
		memcpy(&Chataddr.sin_addr.s_addr, &iaddr,4); //&iaddr, 4);				

		
		// Chataddr.sin_addr.s_addr = inet_addr(chat_server);

		Chataddr.sin_port = htons( chat_port );

		if(SOCKET_ERROR == connect(Chatsock,(SOCKADDR *)&Chataddr,sizeof(SOCKADDR_IN)))
		{
#ifdef _WIN32
			if(WSAEWOULDBLOCK == WSAGetLastError())
#else
			int my_error = errno;
			if (my_error == EINPROGRESS)
#endif
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
			TIMEVAL timeout;   

			timeout.tv_sec=0;            
			timeout.tv_usec=0;

			FD_ZERO(&write_fds);
			FD_SET(Chatsock, &write_fds);    
			//Writable -- that means it's connected
#ifdef WIN32
			if ( select(0, NULL, &write_fds, NULL, &timeout) )
#else
			if ( select(Chatsock+1, NULL, &write_fds, NULL, &timeout) > 0 )
#endif
			{
				Socket_connected = 1;
				sprintf(signon_str,NOX("/USER %s %s %s :%s"),NOX("user"),NOX("user"),NOX("user"),Chat_tracker_id);
				SendChatString(signon_str,1);
				sprintf(signon_str,NOX("/NICK %s"),Nick_name);
				SendChatString(signon_str,1);
				return 0;
				//Now we are waiting for Chat_server_connected
			}
			FD_ZERO(&error_fds);
			FD_SET(Chatsock,&error_fds);    
			//error -- that means it's not going to connect
#ifdef WIN32
			if ( select(0, NULL, NULL, &error_fds, &timeout) )
#else
			if ( select(Chatsock+1, NULL, NULL, &error_fds, &timeout) )
#endif
			{
				return -1;
			}

			return 0;
		}
	}

	return 0;
}

// Call it to close the connection. It returns immediately
void DisconnectFromChatServer()
{
	if(!Socket_connected) return;
	SendChatString(NOX("/QUIT"),1);
	shutdown(Chatsock,2);
	closesocket(Chatsock);
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

// Send a string to be sent as chat, or scanned for messages (/msg <user>
// string)
char * SendChatString(char *line,int raw)
{
	char szCmd[200];
	char szTarget[50];
	if(!Socket_connected) return NULL;
	
	if(line[0]=='/')
	{
		
		//Start off by getting the command
		strcpy(szCmd,GetWordNum(0,line+1));
		if(stricmp(szCmd,NOX("msg"))==0)
		{
			strcpy(szTarget,GetWordNum(1,line+1));
			sprintf(szCmd,NOX("PRIVMSG %s :%s\n\r"),szTarget,line+strlen(NOX("/msg "))+strlen(szTarget)+1);
			send(Chatsock,szCmd,strlen(szCmd),0);
			szCmd[strlen(szCmd)-2] = '\0';
			return ParseIRCMessage(szCmd,MSG_LOCAL);

		}
		if(stricmp(szCmd,NOX("me"))==0)
		{
			sprintf(szCmd,NOX("PRIVMSG %s :\001ACTION %s\001\n\r"),szChat_channel,line+strlen(NOX("/me ")));
			send(Chatsock,szCmd,strlen(szCmd),0);
			szCmd[strlen(szCmd)-2] = '\0';
			return ParseIRCMessage(szCmd,MSG_LOCAL);

		}
		if(stricmp(szCmd,NOX("xyz"))==0)
		{
			//Special command to send raw irc commands
			sprintf(szCmd,"%s\n\r",line+strlen(NOX("/xyz ")));
			send(Chatsock,szCmd,strlen(szCmd),0);
			return NULL;
		}
		if(stricmp(szCmd,NOX("list"))==0)
		{
			sprintf(szCmd,"%s\n\r",line+1);
			send(Chatsock,szCmd,strlen(szCmd),0);
			return NULL;
		}
		if(raw)
		{
			sprintf(szCmd,"%s\n\r",line+1);
			send(Chatsock,szCmd,strlen(szCmd),0);
			return NULL;
		}
		return XSTR("Unrecognized command",634);
		
	}
	else
	{
		if(szChat_channel[0])
		{
			/*
			CString sndstr;
			sndstr.Format("PRIVMSG %s :%s\n\r",szChat_channel,line);
			send(Chatsock,LPCSTR(sndstr),sndstr.GetLength(),0);
			sndstr = sndstr.Left(sndstr.GetLength()-2);
			return ParseIRCMessage((char *)LPCSTR(sndstr),MSG_LOCAL);
			*/

			sprintf(szCmd,NOX("PRIVMSG %s :%s\n\r"),szChat_channel,line);
			send(Chatsock,szCmd,strlen(szCmd),0);			
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
	int iuser_list_length = 0;;
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
// another channel, or the user wont be able to chat
//-1 Failed to join
// 0 joining
// 1 successfully joined
int SetNewChatChannel(char *channel)
{
	char partstr[100];
	if(!Socket_connected) return -1;
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
			strcpy(szChat_channel,"");
			return -1;
		}
	}
	else
	{
		if(szChat_channel[0])
		{
			sprintf(partstr,NOX("/PART %s"),szChat_channel);
			SendChatString(partstr,1);
		}
		strcpy(szChat_channel,channel);
		sprintf(partstr,NOX("/JOIN %s"),szChat_channel);
		SendChatString(partstr,1);
		Joining_channel = 1;
		Joined_channel = 0;
	}
	
	return 0;
}


char *ChatGetString(void)
{
	fd_set read_fds;	           
	TIMEVAL timeout; 
	char ch[2];
	char *p;
	int bytesread;
	static char return_string[MAXCHATBUFFER];
	
	timeout.tv_sec=0;            
	timeout.tv_usec=1;
	
	FD_ZERO(&read_fds);
	FD_SET(Chatsock,&read_fds);    
	//Writable -- that means it's connected
#ifdef WIN32
	while ( select(0, &read_fds, NULL, NULL, &timeout) )
#else
	while ( select(Chatsock+1, &read_fds, NULL, NULL, &timeout) )
#endif
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
				strcpy(return_string,Input_chat_buffer);
				Input_chat_buffer[0] = '\0';
				
				p = ParseIRCMessage(return_string,MSG_REMOTE);
				
				return p;
			}
			Assert(strlen(Input_chat_buffer) < MAXCHATBUFFER-1);
			strcat(Input_chat_buffer,ch);
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


char * GetWordNum(int num, char * l_String)
{
	static char strreturn[600];
	static char ptokstr[600];
	char seps[10] = NOX(" \n\r\t");
	char *token,*strstart;

	strstart = ptokstr;

	strcpy(ptokstr,l_String);

	token=strtok(ptokstr,seps);

	for(int i=0;i!=num;i++)
	{
		token=strtok(NULL,seps);
	}
	if(token)
	{
		strcpy(strreturn,token);
	}
	else
	{
		return "";
	}
	//check for the ':' char....
	if(token[0]==':')
	{
		//Its not pretty, but it works, return the rest of the string
		strcpy(strreturn,l_String+((token-strstart)+1));
	}

	//return the appropriate response.
	return strreturn;
}

int AddChatUser(char *nickname)
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
		strcpy(Firstuser->nick_name,nickname);
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
		strcpy(Curruser->nick_name,nickname);
		Curruser->next = NULL;
		AddChatCommandToQueue(CC_USER_JOINING,nickname,strlen(nickname)+1);
		return 1;
	}

}

int RemoveChatUser(char *nickname)
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
	char *pszTempStr;
	char szPrefix[MAXLOCALSTRING] = "";
	char szHackPrefix[MAXLOCALSTRING] = "";
	char szTarget[MAXLOCALSTRING] = "";
	char szNick[MAXLOCALSTRING] = "";
	char szCmd[MAXLOCALSTRING] = "";
	char szCTCPCmd[MAXLOCALSTRING] = "";

	static char szResponse[MAXLOCALSTRING] = "";

	int iNickLen;
	int iPrefixLen = 0;	// JAS: Get rid of optimized warning

	szPrefix[0] = '\0';
	szNick[0] = '\0';

	if(strlen(Line)>=MAXLOCALSTRING)
	{
		return NULL; 
	}
	//Nick included....
	if(iMode==MSG_REMOTE)
	{
		strcpy(szRemLine,Line);
		//Start by getting the prefix
		if(Line[0]==':')
		{
			//
			pszTempStr=GetWordNum(0,Line+1);
			strcpy(szPrefix,pszTempStr);
			strcpy(szHackPrefix,pszTempStr);
			strcpy(szRemLine,Line+1+strlen(szPrefix));
		}
		//Next, get the Nick
		pszTempStr=strtok(szHackPrefix,"!");
		if(pszTempStr)
		{
			strcpy(szNick,pszTempStr);
		}
		else
		{
			strncpy(szNick,szPrefix,31);
         szNick[31]=0;
		}
		//strcpy(NewMsg.Nickname,szNick);
		iNickLen=strlen(szNick);
		iPrefixLen=strlen(szPrefix);
	}
	else if(iMode==MSG_LOCAL)
	{
		strcpy(szRemLine,Line);
		strcpy(szNick,Nick_name);
		strcpy(szPrefix,Nick_name);
		//strcpy(NewMsg.Nickname,szNick);
		iNickLen=-2;
		iPrefixLen=-2;
	}
	//Next is the command
	pszTempStr=GetWordNum(0,szRemLine);
	if(pszTempStr[0])
	{
		strcpy(szCmd,pszTempStr);
	}
	else
	{
		//Shouldn't ever happen, but we can't be sure of what the host will send us.
		return NULL;
	}

	//Move the szRemLine string up
	strcpy(szRemLine,Line+iPrefixLen+strlen(szCmd)+2);
	//Now parse the commands!
	//printf("%s",szCmd);
	if(stricmp(szCmd,NOX("PRIVMSG"))==0)
	{
		pszTempStr=GetWordNum(0,szRemLine);
		strcpy(szTarget,pszTempStr);
		strcpy(szRemLine,Line+iPrefixLen+strlen(szCmd)+strlen(szTarget)+4);
		if(szRemLine[0]==':')
		{
			strcpy(szCTCPCmd,GetWordNum(0,szRemLine+1));
			if(szCTCPCmd[strlen(szCTCPCmd)-1]==0x01) szCTCPCmd[strlen(szCTCPCmd)-1]=0x00;

		}
		else
		{
			strcpy(szCTCPCmd,GetWordNum(0,szRemLine));
			if(szCTCPCmd[strlen(szCTCPCmd)-1]==0x01) szCTCPCmd[strlen(szCTCPCmd)-1]=0x00;
		}
		if(szCTCPCmd[0]==0x01)
		{
			//Handle ctcp message
			strcpy(szRemLine,Line+iPrefixLen+strlen(szCmd)+strlen(szTarget)+strlen(szCTCPCmd)+6);
			szRemLine[strlen(szRemLine)-1] = '\0';//null out the ending 0x01
			if(stricmp(szCTCPCmd+1,NOX("ACTION"))==0)
			{
				//Posture
				sprintf(szResponse,"* %s %s",szNick,szRemLine);								
				return szResponse;
			}
			if(iMode==MSG_LOCAL)
			{
				strcpy(szHackPrefix,Line+iPrefixLen+strlen(szCmd)+strlen(szTarget)+4);
				szRemLine[strlen(szRemLine)-1] = '\0';
				sprintf(szResponse,NOX("** CTCP %s %s %s"),szTarget,szCTCPCmd+1,szRemLine);
				return szResponse;
			}
			if(stricmp(szCTCPCmd+1,NOX("PING"))==0)
			{
				sprintf(szResponse,NOX("/NOTICE %s :\001PING %s\001"),szNick,szRemLine);//Don't need the trailing \001 because szremline has it.
				SendChatString(szResponse,1);
				return NULL;
			}
			if(stricmp(szCTCPCmd+1,NOX("VERSION"))==0)
			{
				//reply with a notice version & copyright
				//sprintf(szTempLine,"NOTICE %s :\001VERSION Copyright(c)\001\n",szNick);

				return NULL;
			}
			strcpy(szRemLine,1 + GetWordNum(0,Line+iPrefixLen+strlen(szCmd)+strlen(szTarget)+4));
			szRemLine[strlen(szRemLine)-1] = '\0';
			sprintf(szResponse,NOX("** CTCP Message from %s (%s)"),szNick,szRemLine);
			return szResponse;

		}
		//differentiate between channel and private
		if(szTarget[0]=='#')
		{
			pszTempStr=GetWordNum(0,szRemLine);
			sprintf(szResponse,"[%s] %s",szNick,pszTempStr);			
			return szResponse;
		}
		else
		{
			if(iMode == MSG_LOCAL)
			{
				pszTempStr=GetWordNum(0,szRemLine);
				sprintf(szResponse,NOX("Private Message to <%s>: %s"),szNick,pszTempStr);			
			}
			else
			{
				pszTempStr=GetWordNum(0,szRemLine);
				sprintf(szResponse,NOX("Private Message from <%s>: %s"),szNick,pszTempStr);			
			}
			return szResponse;
		}

	}
	//don't handle any other messages locally.
	if(iMode==MSG_LOCAL)
	{
		return NULL;
	}

	if(stricmp(szCmd,NOX("NOTICE"))==0)
	{
		

		pszTempStr=GetWordNum(0,szRemLine);
		strcpy(szTarget,pszTempStr);
		strcpy(szRemLine,Line+iPrefixLen+strlen(szCmd)+strlen(szTarget)+4);
		if(szRemLine[0]==':')
		{
			strcpy(szCTCPCmd,GetWordNum(0,szRemLine+1));
			if(szCTCPCmd[strlen(szCTCPCmd)-1]==0x01) szCTCPCmd[strlen(szCTCPCmd)-1]=0x00;

		}
		else
		{
			strcpy(szCTCPCmd,GetWordNum(0,szRemLine));
			if(szCTCPCmd[strlen(szCTCPCmd)-1]==0x01) szCTCPCmd[strlen(szCTCPCmd)-1]=0x00;
		}
		if(szCTCPCmd[0]==0x01)
		{
			//Handle ctcp message
			strcpy(szRemLine,Line+iPrefixLen+strlen(szCmd)+strlen(szTarget)+strlen(szCTCPCmd)+6);
			szRemLine[strlen(szRemLine)-1] = '\0';//null out the ending 0x01
			if(stricmp(szCTCPCmd+1,NOX("PING"))==0)
			{
				//This is a ping response, figure out time and print
				//sprintf(NewMsg.Message,"** Ping Response from %s: %ums",szNick,ulping);
				return NULL;
			}
			
			//Default message
			strcpy(szRemLine,1 + GetWordNum(0,Line+iPrefixLen+strlen(szCmd)+strlen(szTarget)+4));
			szRemLine[strlen(szRemLine)-1] = '\0';
			sprintf(szResponse,XSTR("** CTCP Message from %s (%s)",635),szNick,szRemLine);
			return szResponse;
			
		}
		sprintf(szResponse,"%s",szRemLine);
		return NULL;
	}
	if(stricmp(szCmd,NOX("JOIN"))==0)
	{
		//see if it is me!
		if(stricmp(Nick_name,szNick)==0)
		{
			//Yup, it's me!
			//if(stricmp(szChat_channel,GetWordNum(0,szRemLine))==0)
			//{
				Joined_channel = 1;
				if(stricmp(szChat_channel,NOX("#autoselect"))==0)
				{
					strcpy(szChat_channel,GetWordNum(0,szRemLine));
					AddChatCommandToQueue(CC_YOURCHANNEL,szChat_channel,strlen(szChat_channel)+1);

				}
				//CC_YOURCHANNEL
			//}
		}
		//		AddChatUser(szNick);

		
		pszTempStr=GetWordNum(0,szRemLine);
		strcpy(szTarget,pszTempStr);
		//strcpy(szRemLine,Line+iPrefixLen+strlen(szCmd)+strlen(szTarget)+3);

		//strcpy(NewMsg.Channel,szTarget);

		AddChatUser(szNick);
		sprintf(szResponse,XSTR("** %s has joined %s",636),szNick,szTarget);
		return NULL;//szResponse;
		//Add them to the userlist too!
	}
	if(stricmp(szCmd,NOX("PART"))==0)
	{
		pszTempStr=GetWordNum(0,szRemLine);
		strcpy(szTarget,pszTempStr);
		strcpy(szRemLine,Line+iPrefixLen+strlen(szCmd)+strlen(szTarget)+3);
		//see if it is me!
		if(stricmp(Nick_name,szNick)==0)
		{
			//Yup, it's me!
			//szChat_channel[0]=NULL;
			RemoveAllChatUsers();
		}
		
		RemoveChatUser(szNick);
		return NULL;
		//Remove them to the userlist too!
	}
	if(stricmp(szCmd,NOX("KICK"))==0)
	{
		pszTempStr=GetWordNum(0,szRemLine);
		strcpy(szTarget,pszTempStr);
		pszTempStr=GetWordNum(1,szRemLine);
		strcpy(szHackPrefix,pszTempStr);
		pszTempStr=GetWordNum(2,szRemLine);
		//see if it is me!
		if(stricmp(Nick_name,GetWordNum(1,szRemLine))==0)
		{
			//Yup, it's me!
			szChat_channel[0] = '\0';
			//bNewStatus=1;
			AddChatCommandToQueue(CC_KICKED,NULL,0);			
			RemoveAllChatUsers();
		}
		sprintf(szResponse,XSTR("*** %s has kicked %s from channel %s (%s)",637),szNick,szHackPrefix,szTarget,pszTempStr);
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
			strcpy(Nick_name,GetWordNum(0,szRemLine));
		}
		char nicks[70];
		sprintf(nicks,"%s %s",szNick,GetWordNum(0,szRemLine));
		AddChatCommandToQueue(CC_NICKCHANGED,nicks,strlen(nicks)+1);
		RemoveChatUser(szNick);
		AddChatUser(GetWordNum(0,szRemLine));
      sprintf(szResponse,XSTR("*** %s is now known as %s",638),szNick,GetWordNum(0,szRemLine));
		return szResponse;
	}
	if(stricmp(szCmd,NOX("PING"))==0)
	{
		//respond with pong (GetWordNum(0,szRemLine))
		sprintf(szResponse,NOX("/PONG :%s"),GetWordNum(0,szRemLine));
		SendChatString(szResponse,1);
		return NULL;
	}
	if(stricmp(szCmd,NOX("MODE"))==0)
	{
		//Channel Mode info
		return NULL;
	}


	if(stricmp(szCmd,"401")==0)
	{
		//This is whois user info, we can get their tracker info from here.  -5
		char szWhoisUser[33];
		strcpy(szWhoisUser,GetWordNum(1,szRemLine));
		Getting_user_tracker_error = 1;			
		Getting_user_channel_error = 1;				
						
		sprintf(szResponse,XSTR("**Error: %s is not online!",639),szWhoisUser);
		return szResponse;

	}
	if(stricmp(szCmd,"311")==0)
	{
		char szWhoisUser[33];
		strcpy(szWhoisUser,GetWordNum(1,szRemLine));
		//This is whois user info, we can get their tracker info from here.  -5
		//if(stricmp(Getting_user_tracker_info_for,szWhoisUser)==0)
		//{
			strcpy(User_req_tracker_id,GetWordNum(5,szRemLine));
		//}
		return NULL;
	}
	if(stricmp(szCmd,"319")==0)
	{
		char szWhoisUser[33];
		strcpy(szWhoisUser,GetWordNum(1,szRemLine));
		//This is whois channel info -- what channel they are on		-2
		//if(stricmp(Getting_user_channel_info_for,szWhoisUser)==0)
		//{
			strcpy(User_req_channel,GetWordNum(2,szRemLine));
		//}
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
			strcpy(sztopic,GetWordNum(3,szRemLine));
			strcpy(channel_list_name,GetWordNum(1,szRemLine));
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
			//strncpy(szChanTopic,GetWordNum(2,szRemLine),70);
		}
		//sprintf(NewMsg.Message,"*** %s has changed the topic to: %s",szNick,GetWordNum(2,szRemLine));

		return NULL;
	}
	if(stricmp(szCmd,NOX("TOPIC"))==0)
	{
		//Channel Topic, update status bar.
		if(stricmp(szChat_channel,szTarget)==0)
		{
			//strncpy(szChanTopic,GetWordNum(1,szRemLine),70);
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
		strcpy(szResponse, PXO_CHAT_END_OF_MOTD_PREFIX);
		return szResponse;
	}
	if((stricmp(szCmd,"377")==0)||
		(stricmp(szCmd,"372")==0)||
		(stricmp(szCmd,"372")==0)
		
		)
	{
		//Stip the message, and display it.
		pszTempStr=GetWordNum(3,Line);		
		strcpy(szResponse, PXO_CHAT_MOTD_PREFIX);
		strcat(szResponse, pszTempStr);
		return szResponse;
	}
	//Ignore these messages
	if(((stricmp(szCmd,"366")==0))||
	    (stricmp(szCmd,"333")==0) || //Who set the topic
		 (stricmp(szCmd,"329")==0))    //Time Channel created
		 /*
		 (stricmp(szCmd,"305")==0) ||
		 (stricmp(szCmd,"306")==0) ||
		 (stricmp(szCmd,"311")==0) || //WHOIS stuff
		 (stricmp(szCmd,"312")==0) ||
		 (stricmp(szCmd,"313")==0) ||
		 (stricmp(szCmd,"317")==0) ||
		 (stricmp(szCmd,"318")==0) ||
		 (stricmp(szCmd,"319")==0) ||
		 */

	{
		return NULL;
	}
	if(stricmp(szCmd,"353")==0)
	{

		//Names in the channel.
		pszTempStr = GetWordNum(3,Line+iPrefixLen+strlen(szCmd)+2);
		strcpy(szRemLine,pszTempStr);
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
	   (stricmp(szCmd,"375")==0)||
	   (stricmp(szCmd,"372")==0)|| 
	   (stricmp(szCmd,"375")==0)
	   )
	{
		// Stip the message, and display it.
		// pszTempStr = GetWordNum(3, Line);
		// strcpy(szResponse, PXO_CHAT_MOTD_PREFIX);
		// strcat(szResponse, pszTempStr);
		return NULL;
		// return szResponse;
	}
	if(stricmp(szCmd,"432")==0)
	{
		//Channel Mode info
		sprintf(szResponse,XSTR("Your nickname contains invalid characters",640));
		AddChatCommandToQueue(CC_DISCONNECTED,NULL,0);
		return szResponse;
	}
	if(stricmp(szCmd,"433")==0)
	{
		//Channel Mode info
		char new_nick[33];
		sprintf(new_nick,"%s%d",Orignial_nick_name,Nick_variety);
		strcpy(Nick_name,new_nick);
		Nick_variety++;
		sprintf(szResponse,NOX("/NICK %s"),new_nick);
		SendChatString(szResponse,1);
		return NULL;
	}
	//Default print
	strcpy(szResponse,Line);
	//return szResponse;
	return NULL;

}


void AddChatCommandToQueue(int command,void *data,int len)
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
	int ichan_list_length = 0;
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
	Chan_list[0] = '\0';
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
		strcpy(Firstchannel->channel_name,channel);
		strcpy(Firstchannel->topic,topic);
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
		strcpy(Currchannel->channel_name,channel);
		strcpy(Currchannel->topic,topic);
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
		strcpy(Getting_user_tracker_info_for,nickname);
		sprintf(szWhoisCmd,NOX("/WHOIS %s"),nickname);
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
		strcpy(Getting_user_channel_info_for,nickname);
		User_req_channel[0] = '\0';
		sprintf(szWhoisCmd,NOX("/WHOIS %s"),nickname);
		SendChatString(szWhoisCmd,1);
		GettingUserChannel = 1;
	}
	return NULL;
}
