#include <fstream>
#include "string"
#include "cqp.h"
#include "appmain.h"
#include "Robot.h"

using namespace std;

RobotManager* manager;

CQEVENT(const char*, AppInfo, 0)() {
	return CQAPPINFO;
}

CQEVENT(int32_t, Initialize, 4)(int32_t AuthCode) {
	fstream data;
	bool autoStart = false;
	int64_t owner = 1044805408LL, self = 3340741722LL;
	autoGroup group;
	vector<autoGroup> groups;
	data.open("./CQRobot/owner.txt", ios::in);
	if (data.peek() != EOF)
		data >> owner;
	data.close();
	data.open("./CQRobot/self.txt", ios::in);
	if (data.peek() != EOF)
		data >> self;
	data.close();
	data.open("./CQRobot/groups.txt", ios::in);
	if (data.peek() != EOF)
		data >> autoStart;
	while (data.peek() != EOF) {
		data >> group.group >> group.allowRepeat;
		groups.push_back(group);
	}
	data.close();
	manager = new RobotManager(AuthCode, owner, self, groups, autoStart);
	return 0;
}

CQEVENT(int32_t, __eventStartup, 0)() {
	return 0;
}

CQEVENT(int32_t, __eventExit, 0)() {
	return 0;
}

CQEVENT(int32_t, __eventPrivateMsg, 24)(int32_t subType, int32_t msgId, int64_t fromQQ, const char *msg, int32_t font) {
	manager->privateMessage(fromQQ, string(msg));
	return EVENT_BLOCK;
}

CQEVENT(int32_t, __eventGroupMsg, 36)(int32_t subType, int32_t msgId, int64_t fromGroup, int64_t fromQQ, const char *fromAnonymous, const char *msg, int32_t font) {
	if (!(manager->preProcessGroupMessage(fromGroup, fromQQ, string(msg))))
		manager->processGroupMessage(fromGroup, fromQQ, string(msg));
	return EVENT_BLOCK;
}