#include "util.h"

#include <QtGlobal>
#include <QtCore/QProcess>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace fso {
namespace fred {

SCP_string getUsername() {
#ifdef Q_OS_WIN
    char acUserName[256];
    DWORD nUserName = sizeof(acUserName);
    if (GetUserNameA(acUserName, &nUserName))
        return SCP_string(acUserName);
    else
    	return "";
#elif defined(Q_OS_UNIX)
	QProcess process;
	QString username;
	QObject::connect(&process,
					 static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
					 [&process, &username](int, QProcess::ExitStatus) {
						 username = process.readAllStandardOutput();
					 });
	process.start("whoami", QStringList());
	process.waitForFinished();

	return username.toUtf8().constData();
#else
#error No username implementation on this platform!
#endif
}

}
}
