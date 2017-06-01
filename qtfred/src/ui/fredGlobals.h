
#include <QObject>

#include <functional>

namespace fso
{
namespace fred
{

class QtFredGlobals : public QObject
{
	Q_OBJECT

private:
	bool _initializeEmitted = false;

public:
	QtFredGlobals();
	~QtFredGlobals();

	QtFredGlobals(const QtFredGlobals&) = delete;
	QtFredGlobals& operator=(const QtFredGlobals&) = delete;

	QtFredGlobals(QtFredGlobals&&) = delete;
	QtFredGlobals& operator=(QtFredGlobals&&) = delete;

	bool isInitializeComplete() const { return _initializeEmitted; }

	void runAfterInit(std::function<void()>&& action);
signals:
	void initializeComplete();
};

extern QtFredGlobals* fredGlobals;

}
}
