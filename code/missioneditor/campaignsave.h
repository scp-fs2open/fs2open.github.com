#pragma once
#include "missionsave.h"

typedef struct campaign_link {
	int from;                         // index of source mission
	int to;                           // index of mission link leads to
	int sexp;                         // sexp index of condition that allows this branch
	int node;                         // node tracker when link is in sexp tree window
	bool is_mission_loop;             // whether link leads to mission loop
	bool is_mission_fork;             // whether link leads to mission fork
	char* mission_branch_txt;         // text describing mission loop
	char* mission_branch_brief_anim;  // filename of anim to play in the brief
	char* mission_branch_brief_sound; // filename of anim to play in the brief
} campaign_link;

class Fred_campaign_save : public Fred_mission_save {
  public:
	/**
	 * @brief Saves the campaign file to the given full pathname
	 *
	 * @param[in] pathname The full pathname to save to
	 *
	 * @details Returns the value of CFred_mission_save::err, which is:
	 *
	 * @returns 0 for no error, or
	 * @returns A negative value if an error occurred
	 *
	 * @see save_mission_internal()
	 */
	int save_campaign_file(const char* pathname, const SCP_vector<campaign_link>& links);

  private:
	/**
	 * @brief Save the campaign sexp to file
	 *
	 * @param[in] node Index of the sexp node
	 * @param[in] link Mission index of the next mission. Is -1 if this is the last link
	 */
	void save_campaign_sexp(int node, int link_num);
};