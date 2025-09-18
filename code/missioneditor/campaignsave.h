#pragma once
#include "missionsave.h"

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
	int save_campaign_file(const char* pathname);

  private:
	/**
	 * @brief Save the campaign sexp to file
	 *
	 * @param[in] node Index of the sexp node
	 * @param[in] link Mission index of the next mission. Is -1 if this is the last link
	 */
	void save_campaign_sexp(int node, int link_num);
};