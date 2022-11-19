#include "network/multi_turret_manager.h"

multiplayer_turret_manager Multi_Turret_Manager;

bool multiplayer_turret_manager::get_latest_packet(ushort net_signature, int subsys_index, turret_packet* packet) 
{
    TIMESTAMP current_time = _timestamp(Multi_Timing_Info.get_current_time() + Multi_Timing_Info.get_mission_start_time());

    auto it = _packets_in.find(turret_packet_address(net_signature, subsys_index));

    // return if we have no packets for this turret and subsystem.
    if (it == _packets_in.end() || it->second.empty()){
        return false;
    }

    TIMESTAMP winning_time = TIMESTAMP::immediate();
    int winning_index = -1;
    
    SCP_vector<turret_packet> retained;
    
    for (uint index = 0; index < it->second.size(); index++) {

        // check that this packet has already elapsed.
        if (timestamp_compare(current_time, it->second[index].time) > 0) {

            // only the most recent packet for which time has elapsed matters.
            if (timestamp_compare(it->second[index].time, winning_time) > 0){
                winning_time = it->second[index].time;
                winning_index = static_cast<int>(index);
            }

        } // retain packets that have not yet elapsed
        else {
            retained.push_back(it->second[index]);
        }
    }

    // Sanity check. We were supposed to deal with an empty incoming vector above.
    if (winning_index < 0 && retained.empty()) {
        mprintf(("There seems to be a non-fatal mistake in multiplayer code.  Please report to Cyborg or check that multiplayer_turret_manager::find() has correct logic.\n"));
        return false;

    } else if (winning_index > -1) {
        // we found an appropriate packet.
        *packet = it->second[winning_index];

        // restore only the elements that we need to keep around.
        it->second = std::move(retained);
        return true;

    // here none of the received packets are yet available.
    } else {
        return false;
    }
}

// clear things manually so that we don't rely on netsted containers emptying correctly automatically.
void multiplayer_turret_manager::reset()
{
    for (auto& vec : _packets_in){
        vec.second.clear();
    }

    _packets_in.clear();

    for (auto& vec : _packets_out){
        vec.second.clear();
    }

    _packets_out.clear();
}

