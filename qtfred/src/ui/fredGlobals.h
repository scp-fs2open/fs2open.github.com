
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
	bool _initializeEmitted;

	QtFredGlobals();

	static QtFredGlobals instance;
public:
	QtFredGlobals(const QtFredGlobals&) = delete;
	QtFredGlobals& operator=(const QtFredGlobals&) = delete;

	QtFredGlobals(QtFredGlobals&&) = delete;
	QtFredGlobals& operator=(QtFredGlobals&&) = delete;

	static QtFredGlobals& getInstance() { return instance; }

	bool isInitializeComplete() const { return _initializeEmitted; }

	void qtInit();

	void runAfterInit(std::function<void()>&& action);
signals:
	void initializeComplete();
};

extern QtFredGlobals* fredGlobals;

}
}
