#pragma once

#include "globalincs/pstypes.h"

#include "mission/missionlog.h"

/**
 * @file
 * The event journal. With -event_journal, the engine appends mission events to
 * <config dir>/journal/Journal.<UTC time of the first event>.log, one JSON object per line.
 *
 * Every event has "timestamp" (UTC, ISO 8601) and "event":
 * - "MissionStart": "Mission" (file name), "Title"
 * - one event per mission log entry, named after its type ("ShipDestroyed", "WingArrived", "GoalSatisfied", ...):
 *   "Mission", "MissionTime" (seconds), "Primary", "PrimaryTeam", "Secondary", "SecondaryTeam", "Index", "Hidden".
 *   The team fields are IFF names and are left out when the entry has no team.
 * - "MissionEnd": "Mission", "MissionTime"
 *
 * Only the host writes it; multiplayer clients write nothing.
 */

/**
 * @brief Writes the MissionStart event for the mission that just loaded, unless it was already written
 */
void mission_journal_mission_start();

/**
 * @brief Writes the event for a mission log entry that was just added
 *
 * @param[in] entry  The log entry
 */
void mission_journal_log_entry(const log_entry& entry);

/**
 * @brief Writes the MissionEnd event for the mission being closed, if its MissionStart was written
 */
void mission_journal_mission_end();

/**
 * @brief Formats a mission log entry as one journal line, without the trailing newline
 *
 * @param[in] entry         The log entry
 * @param[in] mission_name  The mission file name
 * @param[in] timestamp     The UTC time to record
 *
 * @return The JSON text
 */
SCP_string mission_journal_format_entry(const log_entry& entry, const char* mission_name, const char* timestamp);
