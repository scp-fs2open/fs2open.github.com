// IRC.h
// IRC Client header
// Derek Meek
// 2-14-2003

// ############## ATTENTION ##########
// Licensed under the Academic Free License version 2.0
// View License at http://www.opensource.org/licenses/afl-2.0.php
// ###################################

/*
 * $Logfile: /Freespace2/code/irc/irc.h $
 * $Revision: 1.3 $
 * $Date: 2004-03-31 05:42:28 $
 * $Author: Goober5000 $
 * *
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2004/03/10 20:51:16  Kazan
 * irc
 *
 * Revision 1.1  2004/03/10 19:11:40  Kazan
 * generally it helps if i actually add and commit the files i want to modify on a different system
 *
 *
 *
 */



#if !defined(_IRC_H_)
#define _IRC_H_

#include "fs2open_pxo/TCP_Socket.h"

#pragma warning(push, 2)	// ignore all those warnings for Microsoft stuff
#include <string>
#include <vector>
#pragma warning(pop)

#include <fstream.h>

struct irc_user
{
	std::string user;
	std::string pass;
	std::string host;
	std::string ident;

	std::string modes;
};

class irc_channel
{
	public:
		std::string GetName() { return chan_name; }	
	private:
		std::string chan_name;
		std::string topic;
		std::vector<irc_user> channel_users;
		std::vector<std::string> channel_modes;
		std::vector<std::string> messages;

		int max_messages;

		// logging
		bool Log;
		std::vector<std::string> logfile;
		ofstream LogStream;
};


// used for incoming commands from the server
struct irc_command
{
	std::string source;
	std::string command;
	std::string params;
};

class irc_client
{
	public:
		irc_client() : bisConnected(false), current_channel(0) {}
		irc_client(std::string user, std::string pass, std::string server, int port) : bisConnected(false), current_channel(0)
				{ connect(user, pass, server, port); }
		~irc_client() { Disconnect(); }

		bool connect(std::string user, std::string pass, std::string server, int port);
		void Disconnect(std::string goodbye = "Bye!");
		bool isConnected() { return bisConnected; }

		void ParseForCommand(std::string UserInput);

		// IRC commands
		void Nick(std::string nick);
		void Pass(std::string pass);
		void User(std::string user, int mode, std::string realname);
		void Mode(std::string chan, std::string modes, std::string targets="");
		void Kick(std::string chan, std::string nick, std::string message);
		void Part(std::string chan, std::string message="");
		void Join(std::string chan);
		void PrivateMessage(std::string target, std::string message);
		void Notice(std::string target, std::string message);
		void Quit(std::string message="");
		void Oper(std::string user, std::string pass);
		void UserHost(std::string target);



		// Network Interface Commands
		void Interpret_Commands_Do();
		std::vector<std::string> Maybe_GetRawLines();

	private:
		TCP_Socket mySocket;
		irc_user MyUser;
		bool bisConnected;

		std::vector<irc_channel> channels;
		int current_channel;

		void PutRaw(std::string command);
		std::vector<std::string> ExtractParams(std::string UserInput, int params);
		std::vector<std::string> BreakLines(std::string HostInput);
		std::vector<std::string> SplitOnStr(std::string haystack, std::string divide);

		void Interpret_Command(std::string command);
		bool StrIcmp(std::string one, std::string two);
		
};



#endif