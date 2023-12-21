#pragma once

extern SCP_vector<std::pair<SCP_string, bool>> Option_categories;

void ingame_options_init();
void ingame_options_close();
void ingame_options_do_frame();