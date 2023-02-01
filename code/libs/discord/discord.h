#pragma once

namespace libs {
namespace discord {

void init();
void shutdown();
void set_presence_string(SCP_string text);
void set_presence_gameplay();
}
} // namespace libs
