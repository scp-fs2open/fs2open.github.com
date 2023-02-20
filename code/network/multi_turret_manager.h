#pragma once

#include "globalincs/pstypes.h"
#include "globalincs/systemvars.h"
#include "network/multi_time_manager.h"
#include "network/multiutil.h"
#include "ship/ship.h"

typedef std::pair<ushort,int> turret_packet_address;

struct turret_packet {
    TIMESTAMP time;
    int target_objnum;
    std::pair<bool, float> angle1;
    std::pair<bool, float> angle2;

    turret_packet() = default;
    
    turret_packet(int time_in, ushort target_netsig, std::pair<bool, float> ang1_in, std::pair<bool, float> ang2_in) 
    {
        time = _timestamp(time_in + Multi_Timing_Info.get_mission_start_time());

        object* temp = (multi_get_network_object(target_netsig));
        target_objnum = (temp != nullptr) ? OBJ_INDEX(temp) : -1;

        angle1.first = ang1_in.first;

        if (ang1_in.first){
            angle1.second = ang1_in.second;
        } else {
            angle1.second = 0.0f;
        }

        angle2.first = ang2_in.first;

        if (ang2_in.first){
            angle2.second = ang2_in.second;
        } else {
            angle2.second = 0.0f;
        }
    }
};

// basic hash function to allow us to use turret_packet_address as a key for unordered map
struct turret_packet_address_hash // <table_entry_option_address>
{
    std::size_t operator()(const turret_packet_address& k) const noexcept
    {
        // Compute individual hash values and combine them using XOR and bit shifting:
        return ( (std::hash<int>{}(static_cast<int>(k.first)))
            ^ (std::hash<int>{}(k.second) << 1));
    }
};

class multiplayer_turret_manager {
    SCP_unordered_map<turret_packet_address, SCP_vector<turret_packet>, turret_packet_address_hash> _packets_in;
    SCP_unordered_map<ushort, SCP_vector<int>> _packets_out;

public:
    void add_incoming_packet(int time_in, int parent_ship, short subsys_index, ushort target_netsig, std::pair<bool, float> ang1_in, std::pair<bool, float> ang2_in) 
    {
        // if the data is nonsense, return
        if (parent_ship < 0 || parent_ship > MAX_OBJECTS || Objects[parent_ship].net_signature == 0){
            return;
        }

        _packets_in[turret_packet_address(Objects[parent_ship].net_signature, static_cast<int>(subsys_index))].emplace_back(time_in, target_netsig, ang1_in, ang2_in);
    }

    bool get_latest_packet(ushort net_signature, int subsys_index, turret_packet* packet);

    void reset();

    void add_packet_to_queue(ushort net_signature, int subsys_index) {

        // if the subsysem actually can't be sent, don't queue it.
        if (net_signature == 0  || subsys_index > INT16_MAX) {
            nprintf(("network", "Trying to queue a turret tracking packet with unsendable info: net_signature %d and subsys_index %d.", net_signature, subsys_index));
            return;
        }
        
        // if this one was already added, there's no need to add it again.  
        if (std::find(_packets_out[net_signature].begin(), _packets_out[net_signature].end(), subsys_index) != _packets_out[net_signature].end()){
            return;
        }

        _packets_out[net_signature].push_back(subsys_index);
    }

    void send_queued_packets(){
        for (auto& queued : _packets_out) {
            if (!queued.second.empty()){
                extern SCP_vector<int> send_turret_tracking_packet(ushort parent_sig, SCP_vector<int>& subsys_indexes);
                queued.second = send_turret_tracking_packet(queued.first, queued.second);
            }
        }
    };
};

extern multiplayer_turret_manager Multi_Turret_Manager;
