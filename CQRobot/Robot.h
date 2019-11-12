#include <string>
#include <vector>
#include "url.h"

using namespace std;

int32_t send(int32_t ac, int64_t group, string message) {
	return CQ_sendGroupMsg(ac, group, message.c_str());
}

class Robot {
	int64_t bindedGroup;
	int32_t ac;

public:
	Robot(int64_t group, int32_t ac) {
		bindedGroup = group;
		this->ac = ac;
	}

	int64_t getBindedGroup() {
		return bindedGroup;
	}

	void processMessage(int64_t qq, string message) {
		if (message.find("麦萌萌") != message.npos) {
			send(ac, bindedGroup, "[CQ:at,qq=" + to_string(qq) + "]\n叫我干啥(⊙o⊙)？");
		}
		if (message.substr(0, 6) == "百度：") {
			send(ac, bindedGroup, "http://iwo.im/?q=" + url(message.substr(6)));
		}
	}
};

class RobotManager {
	vector<Robot*> robots;
	int32_t ac;

	bool isBinded(int64_t group) {
		for (auto i = robots.begin(); i < robots.end(); i++) {
			if ((*i)->getBindedGroup() == group)
				return true;
		}
		return false;
	}

	void addRobot(int64_t group) {
		robots.push_back(new Robot(group, ac));
	}

	void deleteRobot(int64_t group) {
		for (auto i = robots.begin(); i < robots.end(); i++) {
			if ((*i)->getBindedGroup() == group) {
				robots.erase(i);
				free(*i);
			}
		}
	}

public:
	RobotManager(int32_t ac) {
		this->ac = ac;
	}

	bool preProcessMessage(int64_t group, int64_t qq, string message) {
		if (qq == 1044805408LL) {
			if (message == "开启小管家") {
				if (!isBinded(group)) {
					addRobot(group);
					send(ac, group, "麦萌萌小管家上线啦~！\n有事请呼唤我哟~(*^_^*)");
				}
				else
					send(ac, group, "我在呢！,,ԾㅂԾ,,");
				return true;
			}
			if (message == "关闭小管家") {
				if (isBinded(group)) {
					deleteRobot(group);
					send(ac, group, "哼，咱不理你们了！ε=( o｀ω′)ノ");
				}
				else
					send(ac, group, "（咱并不在哦~）");
				return true;
			}
		}
		return false;
	}

	void processMessage(int64_t group, int64_t qq, string message) {
		for (auto i = robots.begin(); i < robots.end(); i++) {
			if ((*i)->getBindedGroup() == group)
				(*i)->processMessage(qq, message);
		}
	}
};