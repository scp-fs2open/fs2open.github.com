#pragma once

#include "network/multi_pxo.h"
#include "scripting/ade_api.h"

namespace scripting {
namespace api {

struct channel_h {
	int channel;
	channel_h();
	explicit channel_h(int l_channel);
	pxo_channel* getChannel() const;
	bool isCurrent() const;
	bool isValid() const;
};

DECLARE_ADE_OBJ(l_Channel, channel_h);

} // namespace api
} // namespace scripting