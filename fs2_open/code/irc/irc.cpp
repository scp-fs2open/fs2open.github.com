// IRC.cpp
// IRC Client 
// Derek Meek
// 2-14-2003

// ############## ATTENTION ##########
// Licensed under the Academic Free License version 2.0
// View License at http://www.opensource.org/licenses/afl-2.0.php
// ###################################

/*
 * $Logfile: /Freespace2/code/irc/irc.cpp $
 * $Revision: 1.23 $
 * $Date: 2006-04-16 05:28:10 $
 * $Author: taylor $
 * *
 * $Log: not supported by cvs2svn $
 * Revision 1.22  2006/01/26 03:23:29  Goober5000
 * pare down the pragmas some more
 * --Goober5000
 *
 * Revision 1.21  2006/01/20 07:10:33  Goober5000
 * reordered #include files to quash Microsoft warnings
 * --Goober5000
 *
 * Revision 1.20  2005/10/10 17:21:05  taylor
 * remove NO_NETWORK
 *
 * Revision 1.19  2005/07/16 06:33:16  wmcoolmon
 * Commented out lines 554 and 555 so the code would compile
 *
 * Revision 1.18  2005/07/14 13:25:03  mrduckman
 * Restored messages to on-focus channel. If it doesn't work, we'll work around with a pseudo-cmd.
 *
 * Revision 1.17  2005/07/13 03:15:52  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 1.16  2005/06/09 14:57:38  mrduckman
 * Added LIST command and added comment about PRIVMSG for Channels
 *
 * Revision 1.15  2005/05/02 15:33:17  mrduckman
 * Added /NAMES command (Very needed to know who's in a channel :P)
 *
 * Revision 1.14  2005/03/27 21:13:05  mrduckman
 * Syntax correction
 *
 * Revision 1.13  2005/03/27 21:06:31  mrduckman
 * Added /kill IRC Command
 *
 * Revision 1.12  2005/03/02 21:18:19  taylor
 * better support for Inferno builds (in PreProcDefines.h now, no networking support)
 * make sure NO_NETWORK builds are as friendly on Windows as it is on Linux/OSX
 * revert a timeout in Client.h back to the original value before Linux merge
 *
 * Revision 1.11  2005/02/04 20:06:04  taylor
 * merge with Linux/OSX tree - p0204-2
 *
 * Revision 1.10  2004/11/21 11:31:02  taylor
 * move warning fixes from il_func.cpp to irc.cpp since that's where they are coming from
 *
 * Revision 1.9  2004/11/18 00:05:37  Goober5000
 * #pragma'd a bunch of warnings
 * --Goober5000
 *
 * Revision 1.8  2004/07/26 20:47:35  Kazan
 * remove MCD complete
 *
 * Revision 1.7  2004/07/12 16:32:51  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 1.6  2004/05/25 00:24:00  wmcoolmon
 * Updated to use <fstream> instead of <fstream.h> and fixed an un/signed disagreement
 *
 * Revision 1.5  2004/04/03 18:11:21  Kazan
 * FRED fixes
 *
 * Revision 1.4  2004/04/03 06:22:33  Goober5000
 * fixed some stub functions and a bunch of compile warnings
 * --Goober5000
 *
 * Revision 1.3  2004/03/31 05:42:28  Goober5000
 * got rid of all those nasty warnings from xlocale and so forth; also added comments
 * to indicate which warnings were being disabled
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

#include "irc/irc.h"
#include "globalincs/pstypes.h"

// 4097 = something used as synonym for class
#pragma warning(disable: 4097)

#ifdef _WIN32
#include <direct.h>
#endif

#include "cfile/cfile.h"
#include "cfile/cfilesystem.h"

static ubyte ServerChanCreated = 0;


//************************************************************************************
// irc_channel implementation
//************************************************************************************
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

irc_channel::irc_channel(std::string logfile, int nummessages) : messages(nummessages), max_messages(nummessages),  cur_messages(0)
{

	LogStream.open(logfile.c_str(), std::ios::out | std::ios::app);  

	if (!LogStream)
		Log = false;
	else
		Log = true;
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

void irc_channel::RemoveFirstMessage()
{
	for (int i = 0; i < max_messages-1; i++)
		messages[i] = messages[i+1];

	cur_messages--;
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

void irc_channel::AddMessage(const std::string& message)
{
	while (cur_messages >= max_messages-1)
	{
		RemoveFirstMessage();
	}

	messages[cur_messages] = message;

	if (Log)
		LogStream << message.c_str() << std::endl;

	cur_messages++;
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

bool irc_channel::StartLogging(std::string file)
{
	if (!Log)
	{
	
		
		/*ofstream test;
		test.open(file.c_str(), std::ios_base::out | std::ios_base::app);

		if (!test)
			return false;*/

		LogStream.open(file.c_str(), std::ios_base::out | std::ios_base::app);

		if (!LogStream)
			return false;

		Log = true;
	}

	return true;
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

void irc_channel::EndLogging()
{
	if (Log)
	{
		LogStream.close();
		Log = false;
	}
}


//************************************************************************************
// irc_client implementation
//************************************************************************************



void irc_client::SetCurrentChannel(std::string chan)
{
	irc_chan_link *cur = channels;

	while (cur)
	{
		if (cur->chan->GetName() == chan)
		{
			current_channel = cur;
			return;
		}
		cur = cur->next;
	}
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

int irc_client::NumChans()
{
	int i = 0;

	irc_chan_link *cur = channels;

	while (cur)
	{
		i++;
		cur = cur->next;
	}

	return i;
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
	
irc_channel* irc_client::GetChan(std::string chan)
{
	irc_chan_link *c = FindChan(chan);

	if (c)
		return c->chan;

	return NULL;


}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

void irc_client::AddChan(std::string chan)
{
	char root_path[255];
	char filename[32];
	irc_chan_link *nchan = new irc_chan_link;

	memset( root_path, 0, sizeof(root_path) );
	memset( filename, 0, sizeof(filename) );

	// create the full path to where we want to write these channel logs
	strncpy(filename, chan.c_str(), sizeof(filename)-1);
	strncat(filename, ".txt", sizeof(filename) - strlen(filename) - 1);
	cf_create_default_path_string( root_path, sizeof(root_path)-1, CF_TYPE_MULTI_CACHE, filename );

	nchan->next = nchan->prev = NULL;
	std::string logfile;
	logfile.append(root_path);

	nchan->chan = new irc_channel(logfile);
	nchan->chan->SetName(chan);

	if (channels == NULL)
	{
		channels = nchan;
	}
	else
	{
		nchan->next = channels;
		channels->prev = nchan;
		channels = nchan;
	}

	current_channel = nchan;
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

void irc_client::PartChan(std::string chan)
{
	irc_chan_link *pchan = FindChan(chan);

	if (pchan)
	{
		if (pchan == channels)
		{
			channels = pchan->next;
		}
		else
		{
			if (pchan->prev)
				pchan->prev->next = pchan->next;

			if (pchan->next)
				pchan->next->prev = pchan->prev;

		}

		
		delete pchan->chan;
		delete pchan;
	}
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

irc_chan_link* irc_client::FindChan(std::string chan)
{
	irc_chan_link *cur = channels;

	while (cur)
	{
 		if (cur->chan->GetName() == chan)
			return cur;
		cur = cur->next;
	}
	

	return NULL;
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

void irc_client::UnloadChanList()
{
	irc_chan_link *cur = channels, *prev;

	while (cur)
	{
		prev = cur;
		cur = cur->next;
		delete prev->chan;
		delete prev;
	}

	channels = NULL;
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-


bool irc_client::StrIcmp(std::string one, std::string two)
{
	return !stricmp(one.c_str(), two.c_str());
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
/*struct irc_command
{
	std::string source;
	std::string target;
	std::string command;

	std::string params;
};*/
void irc_client::Interpret_Command(std::string command)
{
	// [:sourcemask] <command> [params]
	std::vector<std::string> parts = ExtractParams(command, 2), params;
	irc_command cmd;

	// we should always make sure that "server" exists
	if ( !ServerChanCreated ) {
		AddChan("server");
		ServerChanCreated = 1;
	}

	if (parts[0][0] == ':')
	{
		// prefix specified (prefix=source)
		parts[0].erase(0, 1);
		cmd.source = parts[0];

		params = ExtractParams(parts[1], 3);
		cmd.command = params[0];
		cmd.target = params[1];
		cmd.params = params[2];

	}
	else
	{
		// prefix _not_ specified, assume it was from the server
		cmd.source = ""; // fix this with the server's ID
		cmd.command = parts[0];

		params = ExtractParams(parts[1], 2);
		cmd.target = params[1];
		cmd.params = params[1];
	}

	// let's do something with this command
	if (StrIcmp(cmd.command, "001")) //RPL_WELCOME
	{
		//:irc.maxgaming.net 001 kazan :Welcome to the MGOirc IRC Network kazan!~kazan@12-216-14-64.client.mchsi.com

		params = SplitOnStr(cmd.params, " ");
		MyUser.hostmask = params[params.size()-1];

		cmd.params.erase(0,1);

		command="";

		// it's safe to assume "server" exists
		GetChan("server")->AddMessage(cmd.params);
		//channels[0].AddMessage(cmd.params);
	}
	else if (StrIcmp(cmd.command, "002") || StrIcmp(cmd.command, "003") || StrIcmp(cmd.command, "004") || StrIcmp(cmd.command, "005"))
	{
		if (cmd.params[0] == ':')
			cmd.params.erase(0, 1);
		
		command="";
		GetChan("server")->AddMessage(cmd.params);
	}
	else if (StrIcmp(cmd.command, "PING"))
	{
		if (cmd.params[0] == ':')
		{
			cmd.params.erase(0,1);
			std::string pong = "PONG " + cmd.params;
			PutRaw(pong);
		}
	}
	else if (StrIcmp(cmd.command, "302")) //RPL_USERHOST
	{
		// :ircd.stealth.net 302 yournick :syrk=+syrk@millennium.stealth.net
		// cmd.source = server
		// cmd.cmd = "302"
		// cmd.params = "yournick :user!ident@host

		params = ExtractParams(cmd.params, 2);
		// params[0] = MyUser.user
		// params[1] = :user!ident@host

		params[1].erase(0,1);
		// params[1] = nick=ident@host


		
	}

	// for debugging
	if (command != "")
		current_channel->chan->AddMessage(command);
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-


void irc_client::Interpret_Commands_Do()
{
	std::vector<std::string> raw_input = Maybe_GetRawLines();

	for (unsigned int i = 0; i < raw_input.size(); i++)
	{
		Interpret_Command(raw_input[i]);
	}
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

bool irc_client::connect(std::string user, std::string pass, std::string server, int port)
{
	mySocket.InitSocket(server, port);

	if (mySocket.isInitialized())
	{
		Pass(pass);
		Nick(user);
		User(user, 8, "FS2Open User");
		bisConnected = true;

		MyUser.user = user;
		MyUser.pass = pass;
		MyUser.modes = "+i";
		
		//channels[0].StartLogging("irc_server_log.txt");
		return true;
	}
	return false;
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

void irc_client::Disconnect(std::string goodbye)
{
	Quit(goodbye);
	//mySocket.Close();
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

void irc_client::ParseForCommand(std::string UserInput)
// parses a line from the input for the command then it's parameters
{
	std::vector<std::string> parts = ExtractParams(UserInput, 2), params;
	// parts[0] should now have the command
	// and parts[1] all the parameters

	if (!stricmp(parts[0].c_str(), "/nick"))
	{
		//NICK <nick>
		Nick(parts[1]);
	}
	else if (StrIcmp(parts[0], "/mode"))
	{
		//MODE <channel> <modes> [targets]
		// May also be /MODE <NICK> <MODES> -- mrduckman
		params = ExtractParams(parts[1], 3);
		Mode(params[0], params[1], params[2]);
		
	} 
	else if (StrIcmp(parts[0], "/kick"))
	{
		//KICK <channel> <nick> <message>
		params = ExtractParams(parts[1], 3);
		Kick(params[0], params[1], params[2]);
	} 
	else if (StrIcmp(parts[0], "/part"))
	{
		//PART <channel> :[message]
		params = ExtractParams(parts[1], 2);
		Part(params[0], params[1]);
	} 
	else if (StrIcmp(parts[0], "/join"))
	{
		//JOIN <channel>
		Nick(parts[1]);
	} 
	else if (StrIcmp(parts[0], "/msg"))
	{
		//PRIVMSG <target> :<message>
		params = ExtractParams(parts[1], 2);
		PrivateMessage(params[0], params[1]);
	} 
	else if (StrIcmp(parts[0], "/notice"))
	{
		//NOTICE <target> :<message>
		params = ExtractParams(parts[1], 2);
		Notice(params[0], params[1]);
	} 
	else if (StrIcmp(parts[0], "/quit"))
	{
		//QUIT :[message]
		Quit(parts[1]);
	} 
	else if (StrIcmp(parts[0], "/oper"))
	{
		//OPER <user> <pass>
		params = ExtractParams(parts[1], 2);
		Oper(params[0], params[1]);
	} 
	else if (StrIcmp(parts[0], "/kill"))
	{
		//KILL <nick> <cause>
		params = ExtractParams(parts[1], 2);
		Kill(params[0], params[1]);
	}
	else if (StrIcmp(parts[0], "/names"))
	{
		//NAMES <#channel>
		Names(parts[1]);
	}
	else if (StrIcmp(parts[0], "/list"))
	{
		//LIST (May take parameters, but I won't do pattern matching yet (mrduckman)
		ListChan();
	}
	else if (StrIcmp(parts[0], "/raw"))
	{
		//RAW command
		PutRaw(parts[1]);
	} 
	else
	{
		// it's a message to the current channel
//		if (NumChannels() >= 1)
//			PrivateMessage(channels[current_channel].GetName(), UserInput);
		/* mrduckman (09/06/2005)

		Here you should pass a privmsg to the chan and also print it in the UI
		cmd is PRIVMSG <channel> :<text>
		The ':' is the delimeter for PRIVMSG

		*/				
	}

}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-


void irc_client::UserHost(std::string target)
{
	std::string command = "USERHOST " + target;
	PutRaw(command);
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-


void irc_client::Nick(std::string nick)
//NICK <nick>
{
	std::string command = "NICK " + nick;
	PutRaw(command);
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

void irc_client::Pass(std::string pass)
//PASS <pass>
{
	std::string command = "PASS " + pass;
	PutRaw(command);
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-


void irc_client::User(std::string user, int mode, std::string realname)
//USER Kazan * 0 :Derek
//USER <username> <unused> <mode> :<realname>
{
	char modetext[4];

	std::string command = "USER " + user + " * ";
	command = command + itoa(mode, modetext, 10);
	command = command + " :" + realname;
	PutRaw(command);
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-


void irc_client::Mode(std::string chan, std::string modes, std::string targets)
//MODE <channel> <modes> [targets]
{
	std::string command = "MODE " + chan + " " + modes;

	if (targets != "")
		command = command + " " + targets;
	PutRaw(command);
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-


void irc_client::Kick(std::string chan, std::string nick, std::string message)
//KICK <channel> <nick> <message>
{
	std::string command = "KICK " + chan + " " + nick + " :" + message;
	PutRaw(command);
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-


void irc_client::Part(std::string chan, std::string message)
//PART <channel> :[message]
{
	std::string command = "PART " + chan;

	if (message != "")
		command = command + " :" + message;

	PutRaw(command);
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-


void irc_client::Join(std::string chan)
//JOIN <channel>
{
	std::string command = "JOIN " + chan;
	PutRaw(command);
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-


void irc_client::PrivateMessage(std::string target, std::string message)
//PRIVMSG <target> :<message>
{
	std::string command = "PRIVMSG " + target + " :" + message;
	PutRaw(command);
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-


void irc_client::Notice(std::string target, std::string message)
//NOTICE <target> :<message>
{
	std::string command = "NOTICE " + target + " :" + message;
	PutRaw(command);
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-


void irc_client::Quit(std::string message)
//QUIT :[message]
{

	std::string command = "QUIT";

	if (message != "")
		command += " :" + message;

	PutRaw(command);
	//mySocket.Close();
	bisConnected = false;
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-


void irc_client::Oper(std::string user, std::string pass)
//OPER <user> <pass>
{
	std::string command = "OPER " + user + " " + pass;
	PutRaw(command);
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

void irc_client::Kill(std::string nick, std::string message)
//KILL <nick> <cause>
{
	std::string command = "KILL " + nick + " " + message;
	PutRaw(command);
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
void irc_client::Names(std::string channel)
//NAMES <#channel>
{
	std::string command = "NAMES " + channel;
	PutRaw(command);
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
void irc_client::ListChan()
//LIST [pattern|chan name, chan name], but not doing it with them yet 
// 'cause I don't know C/C++ pattern matching (mrduckman)
{
	std::string command = "LIST";
	PutRaw(command);
}
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

void irc_client::PutRaw(std::string command)
//PutRaw actually sends the command to the server
//All Commands end in \r\n
{
	std::string to_put = command + "\r\n";
	if (mySocket.isInitialized())
		mySocket.SendData((char *)to_put.c_str(), to_put.length());
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-


std::vector<std::string> irc_client::ExtractParams(std::string UserInput, int params)
{
	int space;
	std::vector<std::string> StrParams(params);

	for (int i = 0; i < params-1; i++)
	{
		space = UserInput.find(' ');
		StrParams[i] = UserInput.substr(0, space);
		UserInput.erase(0, space+1); // get the space too
	}
	StrParams[params-1] = UserInput; // last param is the trailing string;

	return StrParams;
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

std::vector<std::string> irc_client::Maybe_GetRawLines()
{
	std::vector<std::string> lines;
	if (mySocket.DataReady())
	{
		char *buf = new char[32*1024]; // 32K should be PLENTY
		memset(buf, 0, 32*1024);

		mySocket.GetData(buf, 32*1024);
		lines = BreakLines(buf);

		delete[] buf;
	}
	return lines;
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

std::vector<std::string> irc_client::BreakLines(std::string HostInput)
{
	return SplitOnStr(HostInput, "\r\n");
}

std::vector<std::string> irc_client::SplitOnStr(std::string haystack, std::string divide)
{
	int curline = 0, break_index;
	std::vector<std::string> Lines(5); // 5 is a good starting point;

	break_index = haystack.find(divide);
	while (break_index != (int)std::string::npos)
	{
		if (curline >= (int)Lines.size())
			Lines.resize(Lines.size()*2);

		// copy the line
		Lines[curline] = haystack.substr(0, break_index);
		curline++;

		// remove the line we just got
		haystack.erase(0, break_index+divide.length()); // get the \r\n as well otherwise we'll continuously loop!
		break_index = haystack.find(divide);
	}

	// catch any trailing
	if (haystack.length() > 0)
	{
		while (curline >= (int)Lines.size())
			Lines.resize(Lines.size()+1);

		Lines[curline] = haystack;
		curline++;
	}

	Lines.resize(curline);
	

	return Lines;
}
