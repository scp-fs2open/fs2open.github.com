#pragma once

#include <QSignalBlocker>
#include <vector>

namespace fso {
namespace fred {
namespace util {

class SignalBlockers {
public:
	SignalBlockers(QObject* parent);
	~SignalBlockers() = default;
	SignalBlockers() = delete;
	SignalBlockers(const SignalBlockers&) = delete;
	SignalBlockers& operator=(const SignalBlockers&) = delete;
private:
	std::vector<QSignalBlocker> _blockers;
};

}
}
}
