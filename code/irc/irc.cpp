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
 * $Revision: 1.1 $
 * $Date: 2004-03-10 19:11:40 $
 * $Author: Kazan $
 * *
 * $Log: not supported by cvs2svn $
 *
 *
 */


#pragma warning(disable:4786)
#include "irc.h"




//************************************************************************************
// irc_client implementation
//************************************************************************************

bool irc_client::connect(std::string user, std::string pass, std::string server, int port)
{
	mySocket.InitSocket(server, port);

	if (mySocket.isInitialized())
	{
		Pass(pass);
		Nick(user);
		User(user, 8, "FS2Open User");
		bisConnected = true;
		return true;
	}
	return false;
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

void irc_client::Disconnect(std::string goodbye)
{
	Quit(goodbye);
	mySocket.Close();
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
	else if (!stricmp(parts[0].c_str(), "/Mode"))
	{
		//MODE <channel> <modes> [targets]
		params = ExtractParams(parts[1], 3);
		Mode(params[0], params[1], params[2]);
		
	} 
	else if (!stricmp(parts[0].c_str(), "/kick"))
	{
		//KICK <channel> <nick> <message>
		params = ExtractParams(parts[1], 3);
		Kick(params[0], params[1], params[2]);
	} 
	else if (!stricmp(parts[0].c_str(), "/part"))
	{
		//PART <channel> :[message]
		params = ExtractParams(parts[1], 2);
		Part(params[0], params[1]);
	} 
	else if (!stricmp(parts[0].c_str(), "/join"))
	{
		//JOIN <channel>
		Nick(parts[1]);
	} 
	else if (!stricmp(parts[0].c_str(), "/msg"))
	{
		//PRIVMSG <target> :<message>
		params = ExtractParams(parts[1], 2);
		PrivateMessage(params[0], params[1]);
	} 
	else if (!stricmp(parts[0].c_str(), "/"))
	{
		//NOTICE <target> :<message>
		params = ExtractParams(parts[1], 2);
		Notice(params[0], params[1]);
	} 
	else if (!stricmp(parts[0].c_str(), "/"))
	{
		//QUIT :[message]
		Quit(parts[1]);
	} 
	else if (!stricmp(parts[0].c_str(), "/"))
	{
		//OPER <user> <pass>
		params = ExtractParams(parts[1], 2);
		Oper(params[0], params[1]);
	} 
	else if (!stricmp(parts[0].c_str(), "/raw"))
	{
		//RAW command
		PutRaw(parts[1]);
	} 
	else
	{
		// it's a message to the current channel
		if (channels.size() >= 1)
			PrivateMessage(channels[current_channel].GetName(), UserInput);
	}

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
	mySocket.Close();
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

void irc_client::PutRaw(std::string command)
//PutRaw actually sends the command to the server
//All Commands end in \r\n
{
	std::string to_put = command + "\r\n";
	if (mySocket.isInitialized())
		mySocket.SendData((char *)command.c_str(), command.length());
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
	int curline = 0, break_index;
	std::vector<std::string> Lines(5); // 5 is a good starting point;

	break_index = HostInput.find("\r\n");
	while (break_index != std::string::npos)
	{
		if (curline >= Lines.size())
			Lines.resize(Lines.size()*2);

		// copy the line
		Lines[curline] = HostInput.substr(0, break_index);
		curline++;

		// remove the line we just got
		HostInput.erase(0, break_index+2); // get the \r\n as well otherwise we'll continuously loop!
		break_index = HostInput.find("\r\n");
	}

	// catch any trailing
	if (HostInput.length() > 0)
	{
		while (curline >= Lines.size())
			Lines.resize(Lines.size()+1);

		Lines[curline] = HostInput;
		curline++;
	}

	Lines.resize(curline);
	

	return Lines;
}

