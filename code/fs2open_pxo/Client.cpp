// Client.cpp
// Client Functions for FS2Open PXO
// Derek Meek
// 2-14-2003

#include "Client.h"
#include "protocol.h"
#include "udpsocket.h"
#include <iostream.h>
#include <time.h>
#include "network/multi_log.h"

// need to compile and link UDP_Server.cpp
//**************************************************************************************************************************************************

file_record* GetMissionsList(int &numMissions, const char *masterserver, UDP_Socket &Socket, int port, int timeout)
{
	fs2open_file_check fc_packet;
	fc_packet.pid = PCKT_MISSIONS_RQST;

	timeout = timeout * CLK_TCK;
	int starttime = clock();

	std::string sender = masterserver;

	// send Packet
	if (Socket.SendPacket((char *)&fc_packet, sizeof(fs2open_file_check), sender, port) == -1)
		return NULL;

	char PacketBuffer[16384]; // 4K should be enough i think..... I HOPE!
	fs2open_pxo_missreply *misreply_ptr = (fs2open_pxo_missreply *) PacketBuffer;
	
	file_record *frecs = NULL;
	numMissions = 0;

	while ((clock() - starttime) <= timeout)
	{

		if (Socket.GetPacket(PacketBuffer, 16384, sender) != -1)
		{
			if (misreply_ptr->pid != PCKT_MISSIONS_REPLY)
			{
				continue; // skip and ignore this packet
			}
			

			if ((misreply_ptr->num_files * sizeof(file_record)) + (sizeof(int) * 2) > 16384)
			{
				// WE"RE IN DEAP SHIT!

				ml_printf("Network (FS2OpenPXO): PCKT_MISSIONS_REPLY was larger than 16k!!!\n");
				return NULL;
			}
			frecs = new file_record[misreply_ptr->num_files];
			memcpy(frecs, PacketBuffer + 8, sizeof(file_record) * misreply_ptr->num_files); // packet buffer will be two ints then the array;
			numMissions = misreply_ptr->num_files;
			return frecs;

		}


	}

	return NULL;

}
//**************************************************************************************************************************************************


int Fs2OpenPXO_Login(const char* username, const char* password, UDP_Socket &Socket, const char* masterserver, int port, int timeout)
{
	timeout = timeout * CLK_TCK;
	int starttime = clock();

	std::string sender = masterserver;

	// create and send login packet
	fs2open_pxo_login loginpckt;
	memset(&loginpckt, 0, sizeof(loginpckt));
	loginpckt.pid = PCKT_LOGIN_AUTH;
	strncpy(loginpckt.username, username, 64);
	strncpy(loginpckt.password, password, 64);


	if (Socket.SendPacket((char *) &loginpckt, sizeof(loginpckt), sender, port) == -1)
	{
		cout << "Error Sending Packet" << endl;
	}
	
	// await reply
	fs2open_pxo_lreply loginreply;
	while ((clock() - starttime) <= timeout)
	{

		if (Socket.GetPacket((char *)&loginreply, sizeof(fs2open_pxo_lreply), sender) != -1)
		{
			if (loginreply.pid != PCKT_LOGIN_REPLY)
			{
				continue; // skip and ignore this packet
			}

			if (loginreply.login_status == false)
				return -1;
			else
				return loginreply.sid;
		}


	}



	return -1;
}


//**************************************************************************************************************************************************


void SendHeartBeat(const char* masterserver, int targetport, UDP_Socket &Socket, const char* myName, int myNetspeed, int myStatus, int myType, int numPlayers)
{
	// ---------- Prepair and send packet ------------	
	serverlist_hb_packet hbpack;
	memset(&hbpack, 0, sizeof(serverlist_hb_packet));

	hbpack.pid = PCKT_SLIST_HB;
	strncpy(hbpack.servername, myName, 64);
	hbpack.netspeed = myNetspeed;
	hbpack.status = myStatus; // forming
	hbpack.type = myType; // cooperative
	hbpack.players = numPlayers;


	Socket.SendPacket((char *) &hbpack, sizeof(serverlist_hb_packet), std::string(masterserver), targetport);
}

//**************************************************************************************************************************************************


net_server* GetServerList(const char* masterserver, int &numServersFound, UDP_Socket &Socket, int port, int timeout)
{
	numServersFound = 0;

	
	
	timeout = timeout * CLK_TCK;
	int starttime = clock();
	// ---------- Prepair and send request packet ------------	
	serverlist_request_packet rpack;
	rpack.pid = PCKT_SLIST_REQUEST;
	rpack.type = 0xFFFFFFFF;
	rpack.status = 0xFFFFFFFF;
	std::string sender = masterserver;	
	
	Socket.SendPacket((char *) &rpack, sizeof(serverlist_request_packet), sender, port);
	

	// ---------- Receive Reply ------------

	serverlist_reply_packet NewServer;
	net_server *retlist, templist[MAX_SERVERS];

	while ((clock() - starttime) <= timeout)
	{
		if (Socket.GetPacket((char *)&NewServer, sizeof(serverlist_reply_packet), sender) != -1)
		{

			if (!strcmp(NewServer.servername, "TERM") && NewServer.status == 0 && NewServer.players == 0 && NewServer.netspeed == 0 && NewServer.type == 0)
			{
				starttime = 0;
				break;
			}
		
			strncpy(templist[numServersFound].servername, NewServer.servername, 65);
			templist[numServersFound].netspeed = NewServer.netspeed;
			templist[numServersFound].status = NewServer.status;
			templist[numServersFound].players = NewServer.players;
			templist[numServersFound].type = NewServer.type;
			strncpy(templist[numServersFound].ip, NewServer.ip, 16);

			numServersFound++;
		}

		
	}


	// ---------- Prepair and return list ------------
	retlist = new net_server[numServersFound];
	memcpy(retlist, templist, sizeof(net_server) * numServersFound);

	return retlist;
}

//**************************************************************************************************************************************************


int Ping(const char* target, UDP_Socket &Socket)
{
	fs2open_ping ping;
	fs2open_pingreply rping;
	ping.pid = PCKT_PING;
	ping.time = time(0);
	std::string rctp = target;

	Socket.SendPacket((char *)&ping, sizeof(fs2open_ping), rctp, FS2OPEN_PXO_PORT);

	Socket.GetPacket((char *)&rping, sizeof(fs2open_pingreply), rctp);

	return int( float(time(0) - rping.time)/2.0 );
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

int Ping(const char* target)
{
	// ---------- Open and Bind Socket ------------
	UDP_Socket Socket(FS2OPEN_CLIENT_PORT);
	if (!Socket.InitSocket())
	{
		cout << "Initialization Error!" << endl;
		exit(1);
	}


	fs2open_ping ping;
	fs2open_pingreply rping;
	ping.pid = PCKT_PING;
	ping.time = time(0);
	std::string rctp = target;

	Socket.SendPacket((char *)&ping, sizeof(fs2open_ping), rctp, FS2OPEN_PXO_PORT);

	Socket.GetPacket((char *)&rping, sizeof(fs2open_pingreply), rctp);

	return int( float(time(0) - rping.time)/2.0 );
}