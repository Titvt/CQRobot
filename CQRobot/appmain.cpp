#include "string"
#include "cqp.h"
#include "appmain.h"
#include "Robot.h"

using namespace std;

int ac = -1;
RobotManager* manager;

CQEVENT(const char*, AppInfo, 0)() {
	return CQAPPINFO;
}

CQEVENT(int32_t, Initialize, 4)(int32_t AuthCode) {
	ac = AuthCode;
	manager = new RobotManager(ac);
	return 0;
}

CQEVENT(int32_t, __eventStartup, 0)() {
	return 0;
}

CQEVENT(int32_t, __eventExit, 0)() {
	return 0;
}

CQEVENT(int32_t, __eventPrivateMsg, 24)(int32_t subType, int32_t msgId, int64_t fromQQ, const char *msg, int32_t font) {
	if (!(manager->preProcessPrivateMessage(fromQQ, string(msg))))
		manager->processPrivateMessage(fromQQ, string(msg));
	return EVENT_BLOCK;
}

CQEVENT(int32_t, __eventGroupMsg, 36)(int32_t subType, int32_t msgId, int64_t fromGroup, int64_t fromQQ, const char *fromAnonymous, const char *msg, int32_t font) {
	if (!(manager->preProcessGroupMessage(fromGroup, fromQQ, string(msg))))
		manager->processGroupMessage(fromGroup, fromQQ, string(msg));
	return EVENT_BLOCK;
}