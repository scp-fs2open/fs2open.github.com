#ifndef _WIN32

#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <streambuf>
#include <algorithm>
#include <iterator>
#include <cstddef>

// Copy-Paste from http://www.cplusplus.com/faq/sequences/strings/split/#c-tokenizer
struct split_struct {
    enum empties_t {
        empties_ok, no_empties
    };
};

template<typename Container>
Container& split(Container& result, const typename Container::value_type& s,
        const typename Container::value_type& delimiters, split_struct::empties_t empties = split_struct::empties_ok) {
    result.clear();
    size_t current;
    size_t next = -1;
    do {
        if (empties == split_struct::no_empties) {
            next = s.find_first_not_of(delimiters, next + 1);
            if (next == Container::value_type::npos)
                break;
            next -= 1;
        }
        current = next + 1;
        next = s.find_first_of(delimiters, current);
        result.push_back(s.substr(current, next - current));
    } while (next != Container::value_type::npos);
    return result;
}
// Copy-Paste end

// Copy-Paste from http://www.adp-gmbh.ch/cpp/common/base64.html
/*
 base64.cpp and base64.h

 Copyright (C) 2004-2008 René Nyffenegger

 This source code is provided 'as-is', without any express or implied
 warranty. In no event will the author be held liable for any damages
 arising from the use of this software.

 Permission is granted to anyone to use this software for any purpose,
 including commercial applications, and to alter it and redistribute it
 freely, subject to the following restrictions:

 1. The origin of this source code must not be misrepresented; you must not
 claim that you wrote the original source code. If you use this source code
 in a product, an acknowledgment in the product documentation would be
 appreciated but is not required.

 2. Altered source versions must be plainly marked as such, and must not be
 misrepresented as being the original source code.

 3. This notice may not be removed or altered from any source distribution.

 René Nyffenegger rene.nyffenegger@adp-gmbh.ch

 */

static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

static inline bool is_base64(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; (i < 4); i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];

        while ((i++ < 3))
            ret += '=';

    }

    return ret;

}

std::string base64_decode(std::string const& encoded_string) {
    int in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_];
        in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++)
                char_array_4[i] = base64_chars.find(char_array_4[i]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret += char_array_3[i];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; j++)
            char_array_4[j] = 0;

        for (j = 0; j < 4; j++)
            char_array_4[j] = base64_chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++)
            ret += char_array_3[j];
    }

    return ret;
}
// Copy-Paste end


/*
 Copyright (c) 2013 Eli2
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "network/stand_gui.h"

#include "globalincs/pstypes.h"
#include "gamesequence/gamesequence.h"
#include "playerman/player.h"
#include "mission/missiongoals.h"
#include "ship/ship.h"

#include "network/multi.h"
#include "network/multiutil.h"
#include "network/multimsgs.h"
#include "network/multi_pmsg.h"
#include "network/multi_kick.h"
#include "network/multi_endgame.h"

#include "fs2netd/fs2netd_client.h"

#include "mongoose.h"
#include "jansson.h"

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))



class WebapiCommand {
public:
    virtual ~WebapiCommand() {
    }
    ;
    virtual void execute() = 0;
};

class KickPlayerCommand: public WebapiCommand {
public:
    KickPlayerCommand(int playerId)
            : mPlayerId(playerId) {
    }

    virtual void execute() {
        size_t foundPlayerIndex;
        for (size_t idx = 0; idx < MAX_PLAYERS; idx++) {
            if (MULTI_CONNECTED(Net_players[idx])) {
                if (Net_players[idx].player_id == mPlayerId) {
                    foundPlayerIndex = idx;
                }
            }
        }
        multi_kick_player(foundPlayerIndex, 0);
    }
private:
    int mPlayerId;
};

class ShutdownServerCommand: public WebapiCommand {
public:
    virtual void execute() {
        gameseq_post_event(GS_EVENT_QUIT_GAME);
    }
};

class UpdateMissionsCommand: public WebapiCommand {
public:
    virtual void execute() {
        if (MULTI_IS_TRACKER_GAME) {
            // delete mvalid.cfg if it exists
            cf_delete(MULTI_VALID_MISSION_FILE, CF_TYPE_DATA);

            // refresh missions
            multi_update_valid_missions();
        }
    }
};

class ResetGameCommand: public WebapiCommand {
public:
    virtual void execute() {
        multi_quit_game(PROMPT_NONE);
    }
};

class ResetFs2NetCommand: public WebapiCommand {
public:
    virtual void execute() {
        fs2netd_reset_connection();
    }
};

SDL_mutex* webapiCommandQueueMutex = SDL_CreateMutex();
SCP_vector<WebapiCommand*> webapiCommandQueue;

void webapiAddCommand(WebapiCommand *command) {
    SDL_mutexP(webapiCommandQueueMutex);

    webapiCommandQueue.push_back(command);

    SDL_mutexV(webapiCommandQueueMutex);
}

void webapiExecuteCommands() {
    SDL_mutexP(webapiCommandQueueMutex);

    for (SCP_vector<WebapiCommand*>::iterator iter = webapiCommandQueue.begin(); iter != webapiCommandQueue.end();
            ++iter) {
        (*iter)->execute();
    }

    webapiCommandQueue.clear();

    SDL_mutexV(webapiCommandQueueMutex);
}

struct LogResource {
    struct LogResourceEntry {
        long timestamp;
        json_t *entity;
    };

    long currentTimestamp;
    SCP_list<LogResourceEntry> entries;

    void addEntity(json_t* entity) {
        LogResourceEntry entry;
        entry.timestamp = currentTimestamp;
        entry.entity = entity;

        entries.push_back(entry);
        currentTimestamp++;

        if (entries.size() > 500) {
            json_delete(entries.front().entity);
            entries.pop_front();
        }
    }

    json_t* getEntriesAfter(long after) {
        json_t *msgs = json_array();

        for (std::list<LogResourceEntry>::iterator iter = entries.begin(); iter != entries.end(); ++iter) {
            if (iter->timestamp >= after) {
                json_t *msg = json_copy(iter->entity);
                json_object_set_new(msg, "timestamp", json_integer(iter->timestamp));
                json_array_append(msgs, msg);
            }
        }

        return msgs;
    }
};


SDL_mutex *webapi_dataMutex = SDL_CreateMutex();
netgame_info webapi_netgameInfo;
std::map<short, net_player> webapiNetPlayers;
float webui_fps;
float webui_missiontime;
std::list<mission_goal> webuiMissionGoals;
LogResource webapi_chatLog;
LogResource webapi_debugLog;

enum HttpStatuscode {
    HTTP_200_OK, HTTP_401_UNAUTHORIZED, HTTP_404_NOT_FOUND, HTTP_500_INTERNAL_SERVER_ERROR
};

static void sendResponse(mg_connection *conn, std::string const& data, HttpStatuscode status) {
    std::stringstream headerStream;

    headerStream << "HTTP/1.0 ";
    switch (status) {
    case HTTP_200_OK:
        headerStream << "200 OK";
        break;
    case HTTP_401_UNAUTHORIZED:
        headerStream << "401 Unauthorized";
        break;
    case HTTP_404_NOT_FOUND:
        headerStream << "404 Not Found";
        break;
    case HTTP_500_INTERNAL_SERVER_ERROR:
        headerStream << "500 Internal Server Error";
        break;
    }
    headerStream << "\r\n";

    if (data.length() > 0) {
        headerStream << "Content-Length: " << data.length() << "\r\n";
        headerStream << "Content-Type: application/json\r\n\r\n";
    }

    std::string resultString;
    resultString += headerStream.str();
    resultString += data;

    mg_write(conn, resultString.c_str(), (int) resultString.length());
}

static void sendJsonResponse(mg_connection *connection, json_t *responseEntity) {
    char* result = json_dumps(responseEntity, 0);
    std::string resultString(result);
    free(result);
    json_delete(responseEntity);

    sendResponse(connection, resultString, HTTP_200_OK);
}

// =============================================================================

struct ResourceContext {
    json_t *requestEntity;
    SCP_map<SCP_string, SCP_string> parameters;
};

typedef json_t* (*resourceHandler)(ResourceContext *);

struct Resource {
    SCP_string path;
    SCP_string method;

    resourceHandler handler;
};

json_t* emptyResource(ResourceContext *context) {
    return json_object();
}

json_t* serverGet(ResourceContext *context) {
    json_t *result = json_object();

    json_object_set_new(result, "name", json_string(Multi_options_g.std_pname));
    json_object_set_new(result, "password", json_string(Multi_options_g.std_passwd));
    json_object_set_new(result, "framecap", json_integer(Multi_options_g.std_framecap));

    return result;
}

json_t* serverPut(ResourceContext *context) {
    const char* name = json_string_value(json_object_get(context->requestEntity, "name"));
    if (name) {
        strcpy(Multi_options_g.std_pname, name);
    }
    const char* passwd = json_string_value(json_object_get(context->requestEntity, "password"));
    if (passwd) {
        strcpy(Multi_options_g.std_passwd, passwd);
    }

    return json_object();
}

json_t* serverDelete(ResourceContext *context) {
    webapiAddCommand(new ShutdownServerCommand());
    return json_object();
}

json_t* refreshMissions(ResourceContext *context) {
    webapiAddCommand(new UpdateMissionsCommand());
    return json_object();
}

json_t* serverResetGame(ResourceContext *context) {
    webapiAddCommand(new ResetGameCommand());
    return json_object();
}

json_t* fs2netReset(ResourceContext *context) {
    webapiAddCommand(new ResetFs2NetCommand());
    return json_object();
}

json_t* netgameInfoGet(ResourceContext *context) {
    json_t *obj = json_object();

    json_object_set_new(obj, "name", json_string(webapi_netgameInfo.name));
    json_object_set_new(obj, "mission", json_string(webapi_netgameInfo.mission_name));
    json_object_set_new(obj, "campaign", json_string(webapi_netgameInfo.campaign_name));

    json_object_set_new(obj, "maxPlayers", json_integer(webapi_netgameInfo.max_players));
    json_object_set_new(obj, "maxObservers", json_integer(webapi_netgameInfo.options.max_observers));
    json_object_set_new(obj, "respawn", json_integer(webapi_netgameInfo.respawn));

    json_object_set_new(obj, "gameState", json_integer(webapi_netgameInfo.game_state));

    json_object_set_new(obj, "security", json_integer(webapi_netgameInfo.security));
    return obj;
}

json_t* missionGet(ResourceContext *context) {
    json_t *fpsEntity = json_object();

    json_object_set_new(fpsEntity, "fps", json_real(webui_fps));
    json_object_set_new(fpsEntity, "time", json_real(webui_missiontime));

    return fpsEntity;
}

json_t* missionGoalsGet(ResourceContext *context) {
    json_t *goals = json_array();

    for (std::list<mission_goal>::iterator iter = webuiMissionGoals.begin(); iter != webuiMissionGoals.end(); ++iter) {
        mission_goal goal = *iter;

        json_t *goalEntity = json_object();

        json_object_set_new(goalEntity, "name", json_string(goal.name));
        json_object_set_new(goalEntity, "message", json_string(goal.message));
        json_object_set_new(goalEntity, "score", json_integer(goal.score));
        json_object_set_new(goalEntity, "team", json_integer(goal.team));

        char *typeString;
        switch (goal.type) {
        case PRIMARY_GOAL:
            typeString = "primary";
            break;
        case SECONDARY_GOAL:
            typeString = "secondary";
            break;
        case BONUS_GOAL:
            typeString = "bonus";
            break;
        default:
            typeString = "error";
            break;
        };
        json_object_set_new(goalEntity, "type", json_string(typeString));

        char *statusString;
        switch (goal.satisfied) {
        case GOAL_FAILED:
            statusString = "failed";
            break;
        case GOAL_COMPLETE:
            statusString = "complete";
            break;
        case GOAL_INCOMPLETE:
            statusString = "incomplete";
            break;
        default:
            statusString = "error";
            break;
        };
        json_object_set_new(goalEntity, "status", json_string(statusString));

        json_array_append(goals, goalEntity);
    }

    return goals;
}

json_t* playerGet(ResourceContext *context) {
    json_t *playerList = json_array();

    for (std::map<short, net_player>::iterator iter = webapiNetPlayers.begin(); iter != webapiNetPlayers.end();
            ++iter) {
        net_player p = iter->second;

        char address[256];
        sprintf(address, "%u.%u.%u.%u:%u", p.p_info.addr.addr[0], p.p_info.addr.addr[1], p.p_info.addr.addr[2],
                p.p_info.addr.addr[3], p.p_info.addr.port);

        json_t *obj = json_object();

        json_object_set(obj, "id", json_integer(p.player_id));
        json_object_set(obj, "address", json_string(address));
        json_object_set(obj, "ping", json_integer(p.s_info.ping.ping_avg));
        json_object_set(obj, "host", (MULTI_HOST(p)) ? json_true() : json_false());
        json_object_set(obj, "observer", (MULTI_OBSERVER(p)) ? json_true() : json_false());
        json_object_set(obj, "callsign", json_string(p.m_player->callsign));
        json_object_set(obj, "ship", json_string(Ship_info[p.p_info.ship_class].name));

        json_array_append(playerList, obj);
    }

    return playerList;
}

json_t* playerDelete(ResourceContext *context) {
    int playerId = atoi(context->parameters["playerId"].c_str());
    webapiAddCommand(new KickPlayerCommand(playerId));
    return json_object();
}

net_player *playerForId(int playerId) {
    net_player *foundPlayer = NULL;
    for (size_t idx = 0; idx < MAX_PLAYERS; idx++) {
        if (MULTI_CONNECTED(Net_players[idx])) {
            if (Net_players[idx].player_id == playerId) {
                foundPlayer = &Net_players[idx];
            }
        }
    }
    return foundPlayer;
}

json_t* playerMissionScoreAlltimeGet(ResourceContext *context) {
    net_player *p = playerForId(atoi(context->parameters["playerId"].c_str()));

    json_t *obj2 = json_object();
    if (p == NULL || p->m_player == NULL)
        return obj2;

    scoring_struct *scores = &(p->m_player->stats);

    json_object_set_new(obj2, "score", json_integer(scores->m_score));
    json_object_set_new(obj2, "kills", json_integer(scores->m_kill_count));
    json_object_set_new(obj2, "kills-enemy", json_integer(scores->m_kill_count_ok));
    json_object_set_new(obj2, "kills-friendly", json_integer(scores->m_bonehead_kills));
    json_object_set_new(obj2, "assists", json_integer(scores->m_assists));
    json_object_set_new(obj2, "shots-primary", json_integer(scores->mp_shots_fired));
    json_object_set_new(obj2, "shots-secondary", json_integer(scores->ms_shots_fired));
    json_object_set_new(obj2, "hits-primary", json_integer(scores->mp_shots_hit));
    json_object_set_new(obj2, "hits-secondary", json_integer(scores->ms_shots_hit));
    json_object_set_new(obj2, "hits-friendly-primary", json_integer(scores->mp_bonehead_hits));
    json_object_set_new(obj2, "hits-friendly-secondary", json_integer(scores->ms_bonehead_hits));

    return obj2;
}

json_t* playerMissionScoreMissionGet(ResourceContext *context) {
    net_player *p = playerForId(atoi(context->parameters["playerId"].c_str()));

    json_t *obj = json_object();
    if (p == NULL || p->m_player == NULL)
        return obj;

    scoring_struct *scores = &(p->m_player->stats);

    json_object_set_new(obj, "score", json_integer(scores->score));
    json_object_set_new(obj, "kills", json_integer(scores->kill_count));
    json_object_set_new(obj, "kills-enemy", json_integer(scores->kill_count_ok));
    json_object_set_new(obj, "kills-friendly", json_integer(scores->bonehead_kills));
    json_object_set_new(obj, "assists", json_integer(scores->assists));
    json_object_set_new(obj, "shots-primary", json_integer(scores->p_shots_fired));
    json_object_set_new(obj, "shots-secondary", json_integer(scores->s_shots_fired));
    json_object_set_new(obj, "hits-primary", json_integer(scores->p_shots_hit));
    json_object_set_new(obj, "hits-secondary", json_integer(scores->s_shots_hit));
    json_object_set_new(obj, "hits-friendly-primary", json_integer(scores->p_bonehead_hits));
    json_object_set_new(obj, "hits-friendly-secondary", json_integer(scores->s_bonehead_hits));

    return obj;
}

int afterTimestamp(ResourceContext *context) {
    SCP_map<SCP_string, SCP_string>::iterator iter = context->parameters.find("after");
    if (iter != context->parameters.end()) {
        return atoi(iter->second.c_str());
    }

    return 0;
}

json_t* chatGet(ResourceContext *context) {
    int after = afterTimestamp(context);
    return webapi_chatLog.getEntriesAfter(after);
}

json_t* chatPost(ResourceContext *context) {
    const char* message = json_string_value(json_object_get(context->requestEntity, "message"));
    if (message) {
        send_game_chat_packet(Net_player, const_cast<char*>(message), MULTI_MSG_ALL, NULL);
    }

    return emptyResource(context);
}

json_t* debugGet(ResourceContext *context) {
    int after = afterTimestamp(context);
    return webapi_debugLog.getEntriesAfter(after);
}

struct Resource resources[] = {
    { "api/1/auth", "GET", &emptyResource },
    { "api/1/server", "GET", &serverGet },
    { "api/1/server", "PUT", &serverPut },
    { "api/1/server", "DELETE", &serverDelete },
    { "api/1/server/refreshMissions", "GET", &refreshMissions },
    { "api/1/server/resetGame", "GET", &serverResetGame },
    { "api/1/server/fs2net/reset", "GET", &fs2netReset },
    { "api/1/netgameInfo", "GET", &netgameInfoGet },
    { "api/1/mission", "GET", &missionGet },
    { "api/1/mission/goals", "GET", &missionGoalsGet },
    { "api/1/player", "GET", &playerGet },
    { "api/1/player/*", "DELETE", &playerDelete },
    { "api/1/player/*/score/mission", "GET", &playerMissionScoreMissionGet },
    { "api/1/player/*/score/alltime", "GET", &playerMissionScoreAlltimeGet },
    { "api/1/chat", "GET", &chatGet },
    { "api/1/chat", "POST", &chatPost },
    { "api/1/debug", "GET", &debugGet } };

static bool webserverApiRequest(mg_connection *conn, const mg_request_info *ri) {
    SCP_string resourcePath(ri->uri);

    resourcePath.erase(0, 1);
    SCP_vector<SCP_string> pathParts;
    split(pathParts, resourcePath, "/", split_struct::no_empties);

    json_t *result = NULL;

    SCP_string method(ri->request_method);

    for (size_t i = 0; i < ARRAY_SIZE(resources); i++) {
        Resource* r = &resources[i];
        SCP_vector<SCP_string> resourcePathParts;
        split(resourcePathParts, r->path, "/", split_struct::no_empties);

        if (resourcePathParts.size() == pathParts.size()) {
            ResourceContext context;

            bool pathMatch = true;
            for (size_t u = 0; u < resourcePathParts.size(); u++) {
                //TODO this is kind of a hack
                if (resourcePathParts.at(u) == "*") {
                    context.parameters["playerId"] = pathParts.at(u);
                } else if (resourcePathParts.at(u) != pathParts.at(u)) {
                    pathMatch = false;
                    break;
                }
            }

            if (pathMatch && r->method == method) {

                std::string userNameAndPassword;
                
                userNameAndPassword += Multi_options_g.webapiUsername.c_str();
                userNameAndPassword += ":";
                userNameAndPassword += Multi_options_g.webapiPassword.c_str();
                
                std::string basicAuthValue = "Basic ";
                
                basicAuthValue += base64_encode(reinterpret_cast<const unsigned char*>(userNameAndPassword.c_str()), userNameAndPassword.length());

                const char* authValue = mg_get_header(conn, "Authorization");
                if (authValue == NULL || strcmp(authValue, basicAuthValue.c_str()) != 0) {
                    sendResponse(conn, std::string(), HTTP_401_UNAUTHORIZED);
                    return true;
                }

                if (ri->query_string) {
                    SCP_string query(ri->query_string);
                    SCP_vector<SCP_string> queryPairs;
                    split(queryPairs, query, "&", split_struct::no_empties);

                    for (SCP_vector<SCP_string>::const_iterator iter = queryPairs.begin(); iter != queryPairs.end();
                            ++iter) {
                        SCP_vector<SCP_string> temp;

                        split(temp, *iter, "=", split_struct::no_empties);

                        if (temp.size() == 2) {
                            context.parameters[temp[0]] = temp[1];
                        }
                    }
                }

                char entityBuffer[1024];
                /*int readBytes = */mg_read(conn, &entityBuffer, sizeof(entityBuffer));

                json_error_t parseError;
                context.requestEntity = json_loads((const char*) &entityBuffer, JSON_DISABLE_EOF_CHECK, &parseError);


                SDL_mutexP(webapi_dataMutex);
                result = r->handler(&context);
                SDL_mutexV(webapi_dataMutex);

                break;
            }
        }
    }

    if (result) {
        sendJsonResponse(conn, result);
        return true;
    }

    return false;
}

static void *webserverCallback(enum mg_event event, struct mg_connection *conn) {
    const struct mg_request_info *ri = mg_get_request_info(conn);

    switch (event) {
    case MG_EVENT_LOG: {
        const char *msg = (const char *) ri->ev_data;
        mprintf(("Webapi error: %s\n", msg));
        break;
    }
    case MG_NEW_REQUEST: {
        bool requestHandled = webserverApiRequest(conn, ri);
        return requestHandled ? const_cast<char*>("") : NULL;
    }
    default:
        break;
    };
    return NULL;
}

struct mg_context *webserverContext = NULL;

void webapi_shutdown() {
    if (webserverContext) {
        mprintf(("Webapi shutting down\n"));
        mg_stop(webserverContext);
    }
}

void std_init_standalone() {
    atexit(webapi_shutdown);
}

void std_configLoaded(multi_global_options *options) {

    webapi_shutdown();

    char buffer[16];
    sprintf(buffer, "%d", options->webapiPort);

    const char *mgOptions[] = {
        "listening_ports", buffer,
        "document_root", options->webuiRootDirectory.c_str(),
        "num_threads", "4",
        NULL };

    mprintf(("Webapi starting on port: %d with document root at: %s\n", options->webapiPort, options->webuiRootDirectory.c_str()));

    webserverContext = mg_start(&webserverCallback, NULL, mgOptions);
}

void std_add_chat_text(const char *text, int player_index, int add_id) {

    json_t *msg = json_object();
    //json_object_set_new(msg, "source",    json_string(Net_players[player_index].m_player->callsign));
    json_object_set_new(msg, "message", json_string(text));

    webapi_chatLog.addEntity(msg);
}

void std_debug_multilog_add_line(const char *str) {

    SCP_vector<SCP_string> debugMessages;
    split(debugMessages, SCP_string(str), "\n", split_struct::no_empties);

    for (SCP_vector<SCP_string>::const_iterator iter = debugMessages.begin(); iter != debugMessages.end(); ++iter) {
        json_t *msg = json_object();
        json_object_set_new(msg, "message", json_string(str));
        webapi_debugLog.addEntity(msg);
    }
}

// =============================================================================

std::vector<std::string> bannedPlayers;

void std_add_ban(const char *name) {
    bannedPlayers.push_back(std::string(name));
}

int std_player_is_banned(const char *name) {
    return std::find(bannedPlayers.begin(), bannedPlayers.end(), std::string(name)) != bannedPlayers.end() ? 1 : 0;
}

/**
 * return 1 if the standalone is host password protected, otherwise 0
 */
int std_is_host_passwd() {
    return (Multi_options_g.std_passwd[0] != '\0') ? 1 : 0;
}

void std_set_standalone_fps(float fps) {
    webui_fps = fps;
}

void std_do_gui_frame() {
    SDL_mutexP(webapi_dataMutex);

    webapi_netgameInfo = Netgame;

    // Update player data
    webapiNetPlayers.clear();

    for (size_t idx = 0; idx < MAX_PLAYERS; idx++) {
        if (MULTI_CONNECTED(Net_players[idx]) && (Net_player != &Net_players[idx])) {
            net_player* p = &Net_players[idx];

            webapiNetPlayers[p->player_id] = *p;
        }
    }

    // Update mission data
    webui_missiontime = f2fl(Missiontime);

    webuiMissionGoals.clear();
    for (int idx = 0; idx < Num_goals; idx++) {
        webuiMissionGoals.push_back(Mission_goals[idx]);
    }

    SDL_mutexV(webapi_dataMutex);

    webapiExecuteCommands();
}

/**
 * Unused methods from the original API below,
 * most of this stuff is now done in std_do_gui_frame
 */
void std_add_player(net_player *p) {}
/* The return value does nothing, except cause the two functions below to be called*/
int std_remove_player(net_player *p) {return 0;}
void std_connect_set_host_connect_status() {}
int std_connect_set_connect_count() {return 0;}
void std_update_player_ping(net_player *p) {}
void std_multi_setup_goal_tree() {}
void std_multi_add_goals() {}
void std_multi_update_goals() {}
void std_connect_set_gamename(char *name) {}
void std_multi_update_netgame_info_controls() {}
void std_multi_set_standalone_mission_name(char *mission_name) {}
void std_gen_set_text(char *str, int field_num) {}
void std_create_gen_dialog(char *title) {}
void std_destroy_gen_dialog() {}
int std_gen_is_active() {return 0;}
void std_debug_set_standalone_state_string(char *str) {}
void std_reset_standalone_gui() {}
void std_reset_timestamps() {}
void std_multi_set_standalone_missiontime(float mission_time) {}

#endif
