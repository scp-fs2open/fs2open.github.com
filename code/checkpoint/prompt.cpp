#include "checkpoint/prompt.h"

#include "checkpoint/checkpoint.h"

#include "camera/photomode.h"
#include "freespace.h"
#include "gamesequence/gamesequence.h"
#include "localization/localize.h"
#include "mission/missionmessage.h"
#include "mission/missionparse.h"
#include "popup/popup.h"
#include "popup/popupdead.h"
#include "sound/audiostr.h"
#include "weapon/weapon.h"

void game_start_subspace_ambient_sound();

namespace checkpoint {

namespace {

// the popup holds "Start from beginning" and this many checkpoints
constexpr int MAX_OFFERED = 5;

enum class Offer { None, MissionStart, Death };

Offer Pending_offer = Offer::None;
int Frames_until_offer = 0;

bool mission_offers_checkpoints()
{
	return The_mission.flags[Mission::Mission_Flags::Offer_checkpoints] && !(Game_mode & GM_MULTIPLAYER);
}

// Returns the chosen checkpoint, 0 to start from the beginning, or -1 if the popup was cancelled or not shown.
int choose_checkpoint()
{
	auto checkpoints = list_checkpoints(MAX_OFFERED);
	mprintf(("Checkpoint: offering %d checkpoints\n", static_cast<int>(checkpoints.size())));
	if (checkpoints.empty())
		return -1;

	SCP_vector<SCP_string> choices;
	choices.emplace_back(XSTR("Start from beginning", -1));
	for (const auto& checkpoint : checkpoints)
		choices.push_back(checkpoint.second);

	// as for the end-mission popup
	game_stop_looped_sounds();
	audiostream_pause_all();
	weapon_pause_sounds();
	message_pause_all();

	SCP_string text = SCP_string(XSTR("Checkpoints", -1)) + "\n" + XSTR("Continue from a checkpoint?", -1);
	int choice = popup(PF_TITLE_BIG | PF_TITLE_WHITE | PF_BODY_BIG | PF_NO_SPECIAL_BUTTONS, choices, text.c_str());

	if (Game_subspace_effect)
		game_start_subspace_ambient_sound();
	if (!game_is_photo_mode_active()) {
		audiostream_unpause_all();
		message_resume_all();
	}
	weapon_unpause_sounds();
	game_flush();

	if (choice <= 0)
		return choice;
	return checkpoints[choice - 1].first;
}

} // namespace

bool can_offer_checkpoints()
{
	return mission_offers_checkpoints() && !list_checkpoints(1).empty();
}

void player_entered()
{
	Pending_offer = Offer::None;
	if (!mission_offers_checkpoints() || loaded_index() != 0 || is_loading()) {
		mprintf(("Checkpoint: no offer at mission start (flag %d, loaded %d)\n",
			mission_offers_checkpoints() ? 1 : 0, loaded_index()));
		return;
	}
	Pending_offer = Offer::MissionStart;
	Frames_until_offer = 1;
}

void offer_after_death()
{
	Pending_offer = Offer::Death;
	Frames_until_offer = 0;
}

void process_offer()
{
	if (Pending_offer == Offer::None)
		return;
	if (Frames_until_offer > 0) {
		--Frames_until_offer;
		return;
	}

	auto offer = Pending_offer;
	Pending_offer = Offer::None;

	int choice = choose_checkpoint();
	if (choice > 0) {
		request_load(choice);
		return;
	}

	if (offer == Offer::Death) {
		if (choice == 0)
			gameseq_post_event(GS_EVENT_ENTER_GAME);	// as Quick Start Mission
		else
			popupdead_start();
	}
}

} // namespace checkpoint
