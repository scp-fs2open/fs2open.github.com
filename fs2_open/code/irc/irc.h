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
 * $Revision: 1.7 $
 * $Date: 2005-02-04 20:06:04 $
 * $Author: taylor $
 * *
 * $Log: not supported by cvs2svn $
 * Revision 1.6  2004/08/11 05:06:25  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 1.5  2004/05/25 00:24:00  wmcoolmon
 * Updated to use <fstream> instead of <fstream.h> and fixed an un/signed disagreement
 *
 * Revision 1.4  2004/04/03 18:11:21  Kazan
 * FRED fixes
 *
 * Revision 1.3  2004/03/31 05:42:28  Goober5000
 * got rid of all those nasty warnings from xlocale and so forth; also added comments
 * for #pragma warning disable to indicate the message being disabled
 * --Goober5000
 *
 * Revision 1.2  2004/03/10 20:51:16  Kazan
 * irc
 *
 * Revision 1.1  2004/03/10 19:11:40  Kazan
 * generally it helps if i actually add and commit the files i want to modify on a different system
 *
 *
 *
 */


#include "PreProcDefines.h"
#if !defined(_IRC_H_)
#define _IRC_H_


#pragma warning(push, 2)	// ignore all those warnings for Microsoft stuff
#include <string>
#include <vector>
#pragma warning(pop)

#include <fstream>

#include "fs2open_pxo/TCP_Socket.h"


struct irc_user
{
	std::string user;
	std::string pass;

	std::string hostmask;

	std::string modes;
};


class irc_channel
// generic channel holder - will also be used for the server window
{
	public:
		irc_channel(std::string logfile, int nummessages=512);
		~irc_channel() { EndLogging(); }

		// Accessors
		const std::string& GetName() const { return chan_name; }	
		const std::string& GetTopic() const { return topic; }

		const std::string& GetModeline(int i) const { return channel_modes[i]; }
		int NumModlines() const { return channel_modes.size(); }
		

		const std::string& GetMessage(int i) const { return messages[i]; }
		int NumMessages() const{ return cur_messages; }
		int MaxMessages() const { return max_messages; }

		// Modifiers
		void SetName(const std::string& name) { chan_name = name; }
		void SetTopic(const std::string& atopic) { topic = atopic; }
		void AddMessage(const std::string& message);
		

		

	private:
		std::string chan_name;
		std::string topic;
		std::vector<irc_user> channel_users;
		std::vector<std::string> channel_modes;
		std::vector<std::string> messages;

		int max_messages;
		int cur_messages;

		// logging
		bool Log;
		std::vector<std::string> logfile;
		std::ofstream LogStream;

		// utility functions
		void RemoveFirstMessage();

		bool StartLogging(std::string file);
		void EndLogging();
};


// used for incoming commands from the server
struct irc_command
{
	std::string source;
	std::string target;
	std::string command;

	std::string params;
};

// used for the channels linked list
struct irc_chan_link
{
	irc_channel *chan;
	irc_chan_link* next;
	irc_chan_link* prev;

};


class irc_client
{
	public:
		irc_client() : bisConnected(false), channels(NULL), current_channel(NULL) { AddChan("server"); }
		irc_client(std::string user, std::string pass, std::string server, int port) : bisConnected(false), channels(NULL), current_channel(NULL)
				{ connect(user, pass, server, port); }
		~irc_client() { Disconnect(); UnloadChanList(); }

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

		// channel functions
		irc_channel* GetCurrentChannel() { return current_channel->chan; }
		
		void SetCurrentChannel(std::string chan);

		int NumChans();
		irc_channel* GetChan(std::string chan);

	private:
		TCP_Socket mySocket;
		irc_user MyUser;
		bool bisConnected;

		// index 0 is _always_ the server window
		irc_chan_link *channels;
		irc_chan_link *current_channel;

		void PutRaw(std::string command);
		std::vector<std::string> ExtractParams(std::string UserInput, int params);
		std::vector<std::string> BreakLines(std::string HostInput);
		std::vector<std::string> SplitOnStr(std::string haystack, std::string divide);

		void Interpret_Command(std::string command);
		bool StrIcmp(std::string one, std::string two);

		// Misc internal functions
		void AddChan(std::string chan);
		void PartChan(std::string chan);
		irc_chan_link* FindChan(std::string chan);
		void UnloadChanList();
};



#endif
