
#include "pilotfile/pilotfile.h"
#include "ship/ship.h"
#include "stats/medals.h"


pilotfile Pilot;


pilotfile::pilotfile()
{
	m_have_flags = false;
	m_have_info = false;

	m_data_invalid = false;

	m_size_offset = 0;

	cfp = NULL;
	p = NULL;

	plr_ver = PLR_VERSION_INVALID;
	csg_ver = PLR_VERSION_INVALID;
}

pilotfile::~pilotfile()
{
	if (cfp) {
		cfclose(cfp);
	}
}

void pilotfile::startSection(Section section_id)
{
	Assert( cfp );

	const int zero = 0;

	cfwrite_ushort( (ushort)section_id, cfp );

	// to be updated when endSection() is called
	cfwrite_int(zero, cfp);

	// starting offset, for size of section
	m_size_offset = cftell(cfp);
}

void pilotfile::endSection()
{
	Assert( cfp );
	Assert( m_size_offset > 0 );

	size_t cur = cftell(cfp);

	Assert( cur >= m_size_offset );

	size_t section_size = cur - m_size_offset;

	if (section_size) {
		// go back to section size in file and write proper value
		cfseek(cfp, (int)(cur - section_size - sizeof(int)), CF_SEEK_SET);
		cfwrite_int((int)section_size, cfp);

		// go back to previous location for next section
		cfseek(cfp, (int)cur, CF_SEEK_SET);
	}
}

void pilotfile::update_stats(scoring_struct *stats, bool training)
{
	int idx, i, j, list_size;
	index_list_t ilist;
	scoring_special_t *p_stats = NULL;

	if (Game_mode & GM_MULTIPLAYER) {
		p_stats = &multi_stats;
	} else {
		p_stats = &all_time_stats;
	}

	// medals
	if (stats->m_medal_earned >= 0) {
		list_size = (int)p_stats->medals_earned.size();

		j = -1;

		for (idx = 0; idx < list_size; idx++) {
			if ( p_stats->medals_earned[idx].name == Medals[stats->m_medal_earned].name ) {
				j = idx;
				break;
			}
		}

		if (j >= 0) {
			p_stats->medals_earned[j].val++;
		} else {
			ilist.name = Medals[stats->m_medal_earned].name;
			ilist.index = stats->m_medal_earned;
			ilist.val = 1;

			p_stats->medals_earned.push_back(ilist);
		}
	}

	// early out if these stats are from a training mission
	if (training) {
		return;
	}

	if (stats->m_promotion_earned >= 0) {
		p_stats->rank = stats->m_promotion_earned;
	}

	p_stats->score += stats->m_score;

	p_stats->assists += stats->m_assists;
	p_stats->kill_count += stats->m_kill_count;
	p_stats->kill_count_ok += stats->m_kill_count_ok;
	p_stats->bonehead_kills += stats->m_bonehead_kills;

	p_stats->p_shots_fired += stats->mp_shots_fired;
	p_stats->p_shots_hit += stats->mp_shots_hit;
	p_stats->p_bonehead_hits += stats->mp_bonehead_hits;

	p_stats->s_shots_fired += stats->ms_shots_fired;
	p_stats->s_shots_hit += stats->ms_shots_hit;
	p_stats->s_bonehead_hits += stats->ms_bonehead_hits;

	p_stats->flight_time += (unsigned int)f2i(Missiontime);
	p_stats->last_backup = p_stats->last_flown;
	p_stats->last_flown = stats->last_flown;
	p_stats->missions_flown++;

	// badges
	if (stats->m_badge_earned.size()) {
		list_size = (int)p_stats->medals_earned.size();
		for (size_t medal = 0; medal < stats->m_badge_earned.size(); medal++) {
			j = -1;

			for (idx = 0; idx < list_size; idx++) {
				if ( p_stats->medals_earned[idx].name == Medals[stats->m_badge_earned[medal]].name ) {
					j = idx;
					break;
				}
			}

			if (j >= 0) {
				p_stats->medals_earned[j].val = 1;
			} else {
				ilist.name = Medals[stats->m_badge_earned[medal]].name;
				ilist.index = stats->m_badge_earned[medal];
				ilist.val = 1;

				p_stats->medals_earned.push_back(ilist);
				list_size++;
			}
		}
	}

	// ship kills
	idx = 0;
	for (auto it = Ship_info.cbegin(); it != Ship_info.cend(); idx++, ++it) {
		if (stats->m_okKills[idx] > 0) {
			list_size = (int)p_stats->ship_kills.size();

			j = -1;

			for (i = 0; i < list_size; i++) {
				if ( p_stats->ship_kills[i].name == it->name ) {
					j = i;
					break;
				}
			}

			if (j >= 0) {
				p_stats->ship_kills[j].val += stats->m_okKills[idx];
			} else {
				ilist.name = it->name;
				ilist.index = idx;
				ilist.val = stats->m_okKills[idx];

				p_stats->ship_kills.push_back(ilist);
			}
		}
	}
}

void pilotfile::update_stats_backout(scoring_struct *stats, bool training)
{
	int i, j;
	uint idx;
	size_t list_size;
	index_list_t ilist;
	scoring_special_t *p_stats = NULL;

	if (Game_mode & GM_MULTIPLAYER) {
		p_stats = &multi_stats;
	} else {
		p_stats = &all_time_stats;
	}

	// medals
	if (stats->m_medal_earned >= 0) {
		list_size = p_stats->medals_earned.size();

		j = -1;

		for (idx = 0; idx < list_size; idx++) {
			if ( p_stats->medals_earned[idx].name == Medals[stats->m_medal_earned].name ) {
				j = idx;
				break;
			}
		}

		if (j >= 0) {
			p_stats->medals_earned[j].val = std::max(0,p_stats->medals_earned[j].val--);
		} else {
			Assertion(true, "Medal '%s' not found, should have been added by pilotfile::update_stats.", Medals[stats->m_medal_earned].name);
		}
	}

	// only medals can be awarded in training missions
	if (training) {
		return;
	}

	p_stats->score -= stats->m_score;

	p_stats->assists -= stats->m_assists;
	p_stats->kill_count -= stats->m_kill_count;
	p_stats->kill_count_ok -= stats->m_kill_count_ok;
	p_stats->bonehead_kills -= stats->m_bonehead_kills;

	p_stats->p_shots_fired -= stats->mp_shots_fired;
	p_stats->p_shots_hit -= stats->mp_shots_hit;
	p_stats->p_bonehead_hits -= stats->mp_bonehead_hits;

	p_stats->s_shots_fired -= stats->ms_shots_fired;
	p_stats->s_shots_hit -= stats->ms_shots_hit;
	p_stats->s_bonehead_hits -= stats->ms_bonehead_hits;

	p_stats->flight_time -= (unsigned int)f2i(Missiontime);
	p_stats->last_flown = p_stats->last_backup;
	p_stats->missions_flown--;

	if (stats->m_promotion_earned >= 0) {
		// deal with a multi-rank promotion mission
		for (i = 0; i < MAX_FREESPACE2_RANK; ++i) {
			if (p_stats->score <= Ranks[i].points) {
				p_stats->rank = i-1;
				break;
			}
		}
		Assertion (p_stats->rank >= 0, "Rank became negative.");
	}

	// badges
	if (stats->m_badge_earned.size()) {
		list_size = p_stats->medals_earned.size();
		for (size_t medal = 0; medal < stats->m_badge_earned.size(); medal++) {
			j = -1;

			for (idx = 0; idx < list_size; idx++) {
				if ( p_stats->medals_earned[idx].name == Medals[stats->m_badge_earned[medal]].name ) {
					j = idx;
					break;
				}
			}

			if (j >= 0) {
				p_stats->medals_earned[j].val = 0;
			} else {
				Assertion (false, "Badge '%s' not found, should have been added by pilotfile::update_stats.", Medals[stats->m_badge_earned[medal]].name);
			}
		}
	}

	// ship kills
	i = 0;
	for (auto it = Ship_info.cbegin(); it != Ship_info.cend(); i++, ++it) {
		if (stats->m_okKills[i] > 0) {
			list_size = p_stats->ship_kills.size();

			j = -1;

			for (idx = 0; idx < list_size; idx++) {
				if ( p_stats->ship_kills[idx].name == it->name ) {
					j = idx;
					break;
				}
			}

			if (j >= 0) {
				p_stats->ship_kills[j].val -= stats->m_okKills[i];
			} else {
				UNREACHABLE("Ship kills of '%s' not found, should have been added by pilotfile::update_stats.", Ship_info[i].name);
			}
		}
	}
}

/**
 * @brief Clear and set multi stats from given scoring struct
 * @note This is really just for use by PXO to set multi stats from tracker
 * 
 * @param stats 
 */
void pilotfile::set_multi_stats(const scoring_struct *stats)
{
	size_t idx;
	index_list_t ilist;


	multi_stats.score = stats->score;
	multi_stats.rank = stats->rank;
	multi_stats.assists = stats->assists;
	multi_stats.kill_count = stats->kill_count;
	multi_stats.kill_count_ok = stats->kill_count_ok;
	multi_stats.bonehead_kills = stats->bonehead_kills;

	multi_stats.p_shots_fired = stats->p_shots_fired;
	multi_stats.p_shots_hit = stats->p_shots_hit;
	multi_stats.p_bonehead_hits = stats->p_bonehead_hits;

	multi_stats.s_shots_fired = stats->s_shots_fired;
	multi_stats.s_shots_hit = stats->s_shots_hit;
	multi_stats.s_bonehead_hits = stats->s_bonehead_hits;

	multi_stats.flight_time = stats->flight_time;
	multi_stats.missions_flown = stats->missions_flown;
	multi_stats.last_flown = stats->last_flown;
	multi_stats.last_backup = stats->last_backup;


	// ship kills
	multi_stats.ship_kills.clear();
	multi_stats.ship_kills.shrink_to_fit();

	auto kills_size = std::min(SDL_arraysize(stats->kills), Ship_info.size());

	for (idx = 0; idx < kills_size; ++idx) {
		if (stats->kills[idx] <= 0) {
			continue;
		}

		ilist.name = Ship_info[idx].name;
		ilist.index = static_cast<int>(idx);
		ilist.val = stats->kills[idx];

		multi_stats.ship_kills.push_back(ilist);
	}

	// medals
	multi_stats.medals_earned.clear();
	multi_stats.medals_earned.shrink_to_fit();

	auto medals_size = std::min(stats->medal_counts.size(), Medals.size());

	for (idx = 0; idx < medals_size; ++idx) {
		if (stats->medal_counts[idx] <= 0) {
			continue;
		}

		ilist.name = Medals[idx].name;
		ilist.index = static_cast<int>(idx);
		ilist.val = stats->medal_counts[idx];

		multi_stats.medals_earned.push_back(ilist);
	}
}

/**
 * Reset stats stored in the .plr file; all_time_stats & multi_stats
 */
void pilotfile::reset_stats()
{
	scoring_special_t *ss_stats[] = {&all_time_stats, &multi_stats};

	for (int i = 0; i < 2; ++i) {
		ss_stats[i]->score = 0;
		ss_stats[i]->assists = 0;
		ss_stats[i]->score = 0;
		ss_stats[i]->rank = 0;
		ss_stats[i]->assists = 0;
		ss_stats[i]->kill_count = 0;
		ss_stats[i]->kill_count_ok = 0;
		ss_stats[i]->bonehead_kills = 0;

		ss_stats[i]->p_shots_fired = 0;
		ss_stats[i]->p_shots_hit = 0;
		ss_stats[i]->p_bonehead_hits = 0;

		ss_stats[i]->s_shots_fired = 0;
		ss_stats[i]->s_shots_hit = 0;
		ss_stats[i]->s_bonehead_hits = 0;

		ss_stats[i]->missions_flown = 0;
		ss_stats[i]->flight_time = 0;
		ss_stats[i]->last_flown = 0;
		ss_stats[i]->last_backup = 0;

		ss_stats[i]->ship_kills.clear();
		ss_stats[i]->medals_earned.clear();
	}
}