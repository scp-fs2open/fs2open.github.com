// Protocol.h
// Protocol Definitions for FS2Open PXO
// Derek Meek
// 2-14-2003

/*
 * $Logfile: /Freespace2/code/fs2open_pxo/protocol.h $
 * $Revision: 1.6 $
 * $Date: 2003-10-13 06:02:50 $
 * $Author: Kazan $
 *
 *
 * $Log: not supported by cvs2svn $
 *
 *
 */

#if !defined(__pxo_protocol_h_)
#define __pxo_protocol_h_


#define PXO_PROTO_VER "1.1"
// PUT the Protocol into TCP mode
#define PXO_TCP

// Packet IDs
#define PCKT_SLIST_REQUEST	0x1
#define PCKT_SLIST_REPLY	0x2
#define PCKT_SLIST_HB		0x3
#define PCKT_LOGIN_AUTH		0x4
#define PCKT_LOGIN_REPLY	0x5
#define PCKT_MISSIONS_RQST	0x5
#define PCKT_MISSIONS_REPLY	0x6
#define PCKT_TABLES_RQST	0x7
#define PCKT_TABLES_REPLY	0x8
#define PCKT_PILOT_GET		0x9
#define PCKT_PILOT_REPLY	0xA
#define PCKT_PILOT_UPDATE	0xB
#define PCKT_PILOT_UREPLY	0xC
#define PCKT_PING			0xD
#define PCKT_PINGREPLY		0xE

#define PCKT_MISSION_CHECK	0xF
#define PCKT_MCHECK_REPLY	0x10

#define FS2OPEN_PXO_PORT	12000
#define FS2OPEN_CLIENT_PORT	FS2OPEN_PXO_PORT + 1


//***********************************************************************************************************************
// Packet Structures
//***********************************************************************************************************************

// ----- Server List -----

//this one is sent Client->Server and when the Server receives it sends back the server list to the sender of this packet
struct serverlist_request_packet
{
     int pid; //  0x1 : serverlist request (PCKT_SLIST_REQUEST)
     int type; // so you can request only servers of a certain time
     int status; // so you can request only servers of a certain status
};

//the server list will be sent one server at a time - each server being on UDP packet containing this
// a terminator will be sent with the following values
// servername = "TERM"
// netspeed = status = players = type = 0

struct serverlist_reply_packet
{
      int pid; // 0x2 : serverlist reply (PCKT_SLIST_REPLY)
      char servername[65];
      int netspeed;
      int status;
      short players;
      int type; // binary bitmask for type and dedicated server
	  char  ip[16]; // "255.255.255.255"
	  int port;
};

//a server sends this UDP packet to the server every 60 seconds has a "Heartbeat" telling the server it's here -
// if one isn't received for 120 seconds the server is dropped from the list
struct serverlist_hb_packet
{
      int pid; // 0x3 : serverlist register (PCKT_SLIST_HB)
      char servername[65];
      int netspeed;
      int status;
      short players;
      int type; // binary bitmask for type and dedicated server
	  int port;
};


//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+


// ----- Logins -----

//login request
struct fs2open_pxo_login
{
         int pid; // 0x4 : fs2open_pxo_login (PCKT_LOGIN_AUTH)
         char username[65];
         char password[65];         
};

//and the reply
struct fs2open_pxo_lreply
{
         int pid; // 0x5 (PCKT_LOGIN_REPLY)
         bool login_status; // true if successful, false if failed
         int sid; // if (login_status) sid = session id, ip associated and expires after 1 hour
         int pilots; // if (login_status) pilots = number of pilots for this account
};


//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+


// ----- Missions & Tables -----

struct fs2open_file_check_single
{
	int pid; //PCKT_MISSION_CHECK
	char name[60];
	unsigned int crc32;
};

struct fs2open_fcheck_reply
{
	int pid; //PCKT_MCHECK_REPLY
	int status; // 1 = valid, 0 = invalid
};

//request

struct fs2open_file_check
{
          int pid; // 0x5 for missions 0x7 for tables ( PCKT_MISSIONS_RQST / PCKT_TABLES_RQST )
};


//reply

struct file_record
{
         char name[60];
         unsigned int crc32;
};

struct fs2open_pxo_missreply
{
         int pid; // 0x6 for missions 0x8 for tables ( PCKT_MISSIONS_REPLY / PCKT_TABLES_REPLY )
         int num_files;
         file_record *files;
};

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+


// ----- Pilot Management -----



struct fs2open_get_pilot
{
          int pid; // 0x9 ( PCKT_PILOT_GET )
          int sid; // session id returned upon login
          char pilotname[65];
          bool create; // create pilot
};

struct fs2open_ship_typekill
{
          char name[65];
          unsigned int kills;
};

struct fs2open_pilot_reply
{
         int pid; // 0x0A (PCKT_PILOT_REPLY)
         int replytype; // 0 = pilot retrieved, 1 = pilot created, 2 = invalid pilot, 3 = invalid (expired?) sid, 4 = pilot already exists

         // if and only if (replytype == 0) then the rest of this data

         // unsigned __int64 points; -- i'd use this except DUMBASS MICROSOFT DOESN'T SUPPORT INT64 COPRRECTLY! DUMBFUCKS
         unsigned int points;
		 unsigned int missions;
         //unsigned __int64 flighttime;
		 unsigned int flighttime;
         unsigned int LastFlight;
         unsigned int Kills;
         unsigned int Assists;
         unsigned int FriendlyKills;
         unsigned int PriShots; 
         unsigned int PriHits; 
         unsigned int PriFHits; 
         unsigned int SecShots; 
         unsigned int SecHits; 
         unsigned int SecFHits;
         
         unsigned int ship_types;
         fs2open_ship_typekill *type_kills; 
};



struct fs2open_pilot_update
{
         int pid; // 0x0B (PCKT_PILOT_UPDATE)
         int sid; // session id

		 char name[65];
		 char user[65];
         // unsigned __int64 points; -- i'd use this except DUMBASS MICROSOFT DOESN'T SUPPORT INT64 COPRRECTLY! DUMBFUCKS
         unsigned int points;
		 unsigned int missions;
         //unsigned __int64 flighttime;
		 unsigned int flighttime;
         unsigned int LastFlight;
         unsigned int Kills;
         unsigned int Assists;
         unsigned int FriendlyKills;
         unsigned int PriShots; 
         unsigned int PriHits; 
         unsigned int PriFHits; 
         unsigned int SecShots; 
         unsigned int SecHits; 
         unsigned int SecFHits;
         
         unsigned int ship_types;
         fs2open_ship_typekill *type_kills;          

         
};

struct fs2open_pilot_updatereply
{
         int pid; // 0x0C (PCKT_PILOT_UREPLY)
         int replytype; // 0 = pilot updated, 1  = invalid pilot, 2 = invalid (expired?) sid
         
};


// ---------------- Generic Ping -----------------

// ping time is in ticks returned by clock()
struct fs2open_ping
{
	int pid; // 0xD (PCKT_PING)
	int time;
};

struct fs2open_pingreply
{
	int pid; // 0xE (PCKT_PINGREPLY
	int time;
};


#endif
