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
	int64_t owner = 1044805408LL, self = 3340741722LL, tmp;
	vector<int64_t> groups;
	data.open("owner.txt");
	if (data.peek() != EOF)
		data >> owner;
	data.close();
	data.open("self.txt");
	if (data.peek() != EOF)
		data >> self;
	data.close();
	data.open("groups.txt");
	while (data.peek() != EOF) {
		data >> tmp;
		groups.push_back(tmp);
	}
	data.close();
	if (groups.empty())
		groups.push_back(775980353LL);
	manager = new RobotManager(AuthCode, owner, self, groups);
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