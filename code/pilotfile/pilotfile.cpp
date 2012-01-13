
#include "globalincs/pstypes.h"
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
}

pilotfile::~pilotfile()
{
	if (cfp) {
		cfclose(cfp);
	}
}

void pilotfile::startSection(Section::id section_id)
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
		cfseek(cfp, cur - section_size - sizeof(int), CF_SEEK_SET);
		cfwrite_int((int)section_size, cfp);

		// go back to previous location for next section
		cfseek(cfp, cur, CF_SEEK_SET);
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
			if ( p_stats->medals_earned[idx].name.compare(Medals[stats->m_medal_earned].name) ) {
				j = idx;
				break;
			}
		}

		if (j >= 0) {
			p_stats->medals_earned[j].val++;
		} else {
			ilist.name = Medals[stats->m_medal_earned].name;
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
	if (stats->m_badge_earned >= 0) {
		list_size = (int)p_stats->medals_earned.size();

		j = -1;

		for (idx = 0; idx < list_size; idx++) {
			if ( p_stats->medals_earned[idx].name.compare(Medals[stats->m_badge_earned].name) ) {
				j = idx;
				break;
			}
		}

		if (j >= 0) {
			p_stats->medals_earned[j].val = 1;
		} else {
			ilist.name = Medals[stats->m_badge_earned].name;
			ilist.val = 1;

			p_stats->medals_earned.push_back(ilist);
		}
	}

	// ship kills
	for (idx = 0; idx < Num_ship_classes; idx++) {
		if (stats->m_okKills[idx] > 0) {
			list_size = (int)p_stats->ship_kills.size();

			j = -1;

			for (i = 0; i < list_size; i++) {
				if ( p_stats->ship_kills[i].name.compare(Ship_info[idx].name) ) {
					j = i;
					break;
				}
			}

			if (j >= 0) {
				p_stats->ship_kills[j].val += stats->m_okKills[idx];
			} else {
				ilist.name = Ship_info[idx].name;
				ilist.val = stats->m_okKills[idx];

				p_stats->ship_kills.push_back(ilist);
			}
		}
	}
}
