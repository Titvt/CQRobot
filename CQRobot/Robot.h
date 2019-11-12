#include <string>
#include <vector>
#include <WS2tcpip.h>
#include "UrlEncode.h"
#include "HTTP.h"

using namespace std;

const int64_t OWNER = 1044805408LL;

int32_t sendTo(int32_t ac, int64_t qq, string message) {
	return CQ_sendPrivateMsg(ac, qq, message.c_str());
}

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
		if (message == "麦萌萌") {
			send(ac, bindedGroup, "[CQ:at,qq=" + to_string(qq) + "]\n叫我干啥(⊙o⊙)？");
			return;
		}
		if (message.substr(0, 10) == "教你百度：") {
			send(ac, bindedGroup, "http://iwo.im/?q=" + UrlEncode(message.substr(10)));
			return;
		}
		if (message.substr(0, 6) == "百度：") {
			send(ac, bindedGroup, "https://www.baidu.com/s?wd=" + UrlEncode(message.substr(6)));
			return;
		}
		if (message.substr(0, 22) == "[CQ:at,qq=3340741722] " && message.substr(22).length() != 0) {
			string reply = HttpGet(message.substr(22));
			string::size_type pos = 0;
			while ((pos = reply.find("菲菲")) != string::npos)
				reply.replace(pos, 4, "麦萌萌");
			send(ac, bindedGroup, "[CQ:at,qq=" + to_string(qq) + "]\n" + reply);
			return;
		}
		if (message.find("[CQ:at,qq=3340741722]") != string::npos) {
			send(ac, bindedGroup, "[CQ:at,qq=" + to_string(qq) + "]\n叫我干啥(⊙o⊙)？");
			return;
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

	bool preProcessPrivateMessage(int64_t qq, string message) {
		if (qq == OWNER) {
			if (message.substr(0, 12) == "开启小管家：") {
				int64_t group = stoll(message.substr(12));
				if (!isBinded(group)) {
					addRobot(group);
					sendTo(ac, OWNER, "开启成功");
				}
				else
					sendTo(ac, OWNER, "已经开启");
				return true;
			}
			if (message.substr(0, 12) == "关闭小管家：") {
				int64_t group = stoll(message.substr(12));
				if (isBinded(group)) {
					deleteRobot(group);
					sendTo(ac, OWNER, "关闭成功");
				}
				else
					sendTo(ac, OWNER, "已经关闭");
				return true;
			}
		}
		return false;
	}

	void processPrivateMessage(int64_t qq, string message) {
		if (message == "麦萌萌") {
			sendTo(ac, qq, "你好！我是麦萌萌小管家(*^_^*)");
			return;
		}
		if (message.substr(0, 10) == "教你百度：") {
			sendTo(ac, qq, "http://iwo.im/?q=" + UrlEncode(message.substr(10)));
			return;
		}
		if (message.substr(0, 6) == "百度：") {
			sendTo(ac, qq, "https://www.baidu.com/s?wd=" + UrlEncode(message.substr(6)));
			return;
		}
		string reply = HttpGet(message);
		string::size_type pos = 0;
		while ((pos = reply.find("菲菲")) != string::npos)
			reply.replace(pos, 4, "麦萌萌");
		send(ac, qq, reply);
	}

	bool preProcessGroupMessage(int64_t group, int64_t qq, string message) {
		if (qq == OWNER) {
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

	void processGroupMessage(int64_t group, int64_t qq, string message) {
		for (auto i = robots.begin(); i < robots.end(); i++) {
			if ((*i)->getBindedGroup() == group)
				(*i)->processMessage(qq, message);
		}
	}
};