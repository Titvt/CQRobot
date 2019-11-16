#include <string>
#include <vector>
#include <WS2tcpip.h>
#include "UrlEncode.h"
#include "HTTP.h"

using namespace std;

int64_t OWNER, SELF;
vector<int64_t> GROUPS;

int32_t send(int32_t ac, int64_t group, string message) {
	return CQ_sendGroupMsg(ac, group, message.c_str());
}

int32_t sendTo(int32_t ac, int64_t qq, string message) {
	return CQ_sendPrivateMsg(ac, qq, message.c_str());
}

string renameReply(string reply) {
	size_t pos = 0;
	while ((pos = reply.find("小豪豪")) != string::npos)
		reply.replace(pos, 6, "麦萌萌");
	pos = 0;
	while ((pos = reply.find("豪豪")) != string::npos)
		reply.replace(pos, 4, "麦萌萌");
	return reply;
}

string removeAt(string str) {
	size_t start = 0, end;
	while ((start = str.find("[CQ:at,qq=")) != string::npos) {
		end = str.find("] ", start);
		str.replace(start, end - start + 2, "");
	}
	return str;
}

class Robot {
	int64_t bindedGroup;
	vector<int64_t> chatter;
	int32_t ac;

	bool isChatter(int64_t qq) {
		for (auto i = chatter.begin(); i < chatter.end(); i++)
			if ((*i) == qq)
				return true;
		return false;
	}

	void addChatter(int64_t qq) {
		chatter.push_back(qq);
	}

	void deleteChatter(int64_t qq) {
		for (auto i = chatter.begin(); i < chatter.end(); i++)
			if ((*i) == qq)
				chatter.erase(i);
	}

public:
	Robot(int64_t group, int32_t ac) {
		bindedGroup = group;
		this->ac = ac;
	}

	int64_t getBindedGroup() {
		return bindedGroup;
	}

	void processMessage(int64_t qq, string message) {
		if (qq == OWNER) {
			if (message.substr(0, 22) == "强制开始聊天[CQ:at,qq=") {
				int64_t q = stoll(message.substr(22, message.length() - 24));
				if (!isChatter(q))
					addChatter(q);
			}
			if (message.substr(0, 22) == "强制结束聊天[CQ:at,qq=") {
				int64_t q = stoll(message.substr(22, message.length() - 24));
				if (isChatter(q))
					deleteChatter(q);
			}
		}
		if (message == "麦萌萌") {
			send(ac, bindedGroup, "[CQ:at,qq=" + to_string(qq) + "]\n你好呀~\\(^0^)/\n我是麦萌萌小管家(￣▽￣～) ！\n咱在群里有以下功能哟~：\n教你百度：xxx\n百度：xxx\n翻译：xxx\n想和我单独聊天的话可以私聊我哦~\n(私聊时上述功能也可用>_<!)\n想在群里和我聊天的话有如下方式：\n开始聊天/结束聊天\n[CQ:at,qq=3340741722] xxx\n咱们愉快相处吧(●'o'●)！");
			return;
		}
		if (message.substr(0, 10) == "教你百度：") {
			send(ac, bindedGroup, "[CQ:at,qq=" + to_string(qq) + "]\n" + "http://iwo.im/?q=" + UrlEncode(message.substr(10)));
			return;
		}
		if (message.substr(0, 9) == "教你百度:") {
			send(ac, bindedGroup, "[CQ:at,qq=" + to_string(qq) + "]\n" + "http://iwo.im/?q=" + UrlEncode(message.substr(9)));
			return;
		}
		if (message.substr(0, 6) == "百度：") {
			send(ac, bindedGroup, "[CQ:at,qq=" + to_string(qq) + "]\n" + "https://www.baidu.com/s?wd=" + UrlEncode(message.substr(6)));
			return;
		}
		if (message.substr(0, 5) == "百度:") {
			send(ac, bindedGroup, "[CQ:at,qq=" + to_string(qq) + "]\n" + "https://www.baidu.com/s?wd=" + UrlEncode(message.substr(5)));
			return;
		}
		if (message.substr(0, 6) == "翻译：") {
			string str = XLAT(message.substr(6));
			if (str == "")
				send(ac, bindedGroup, "[CQ:at,qq=" + to_string(qq) + "] emmm...");
			else
				send(ac, bindedGroup, "[CQ:at,qq=" + to_string(qq) + "] 翻译结果为：\n" + str);
			return;
		}
		if (message.substr(0, 5) == "翻译:") {
			string str = XLAT(message.substr(5));
			if (str == "")
				send(ac, bindedGroup, "[CQ:at,qq=" + to_string(qq) + "] emmm...");
			else
				send(ac, bindedGroup, "[CQ:at,qq=" + to_string(qq) + "] 翻译结果为：\n" + str);
			return;
		}
		if (message.find("[CQ:at,qq=" + to_string(SELF) + "]") != string::npos) {
			string str = removeAt(message);
			if (str == "")
				send(ac, bindedGroup, "[CQ:at,qq=" + to_string(qq) + "]\n叫我干啥(⊙o⊙)？");
			else
				send(ac, bindedGroup, "[CQ:at,qq=" + to_string(qq) + "]\n" + renameReply(AI(str)));
			return;
		}
		if (message == "开始聊天") {
			if (isChatter(qq)) {
				send(ac, bindedGroup, "[CQ:at,qq=" + to_string(qq) + "]\n咱们已经在聊天啦~！");
				return;
			}
			else {
				addChatter(qq);
				send(ac, bindedGroup, "[CQ:at,qq=" + to_string(qq) + "]\n开始愉快的聊天吧~！");
				return;
			}
		}
		if (message == "结束聊天") {
			if (!isChatter(qq))
				return;
			else {
				deleteChatter(qq);
				send(ac, bindedGroup, "[CQ:at,qq=" + to_string(qq) + "]\n下次再聊吧~！");
				return;
			}
		}
		if (isChatter(qq)) {
			string str = removeAt(message);
			if (str == "")
				send(ac, bindedGroup, "[CQ:at,qq=" + to_string(qq) + "]\n叫我干啥(⊙o⊙)？");
			else
				send(ac, bindedGroup, "[CQ:at,qq=" + to_string(qq) + "]\n" + renameReply(AI(str)));
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
	RobotManager(int32_t ac, int64_t owner, int64_t self, vector<int64_t> groups) {
		this->ac = ac;
		OWNER = owner;
		SELF = self;
		GROUPS = groups;
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
			if (message == "查询列表") {
				string str = "已经开启的群如下：";
				for (auto i = robots.begin(); i < robots.end(); i++)
					str += "\n" + to_string((*i)->getBindedGroup());
				if (str == "已经开启的群如下：")
					sendTo(ac, OWNER, "目前没有开启的群");
				else
					sendTo(ac, OWNER, str);
				return true;
			}
			if (message == "全部开启") {
				for (auto i = GROUPS.begin(); i < GROUPS.end(); i++)
					if (!isBinded(*i))
						addRobot(*i);
				string str = "操作成功\n已经开启的群如下：";
				for (auto i = robots.begin(); i < robots.end(); i++)
					str += "\n" + to_string((*i)->getBindedGroup());
				if (str == "操作成功\n已经开启的群如下：")
					sendTo(ac, OWNER, "操作成功\n目前没有开启的群");
				else
					sendTo(ac, OWNER, str);
				return true;
			}
			if (message == "全部关闭") {
				for (auto i = robots.begin(); i < robots.end(); i++)
					free(*i);
				robots.clear();
				sendTo(ac, OWNER, "操作成功");
				return true;
			}
		}
		return false;
	}

	void processPrivateMessage(int64_t qq, string message) {
		if (message == "麦萌萌" || message == "麦萌萌 ") {
			sendTo(ac, qq, "你好呀~\\(^0^)/\n我是麦萌萌小管家(￣▽￣～) ！\n咱在群里有以下功能哟~：\n教你百度：xxx\n百度：xxx\n翻译：xxx\n想和我单独聊天的话可以私聊我哦~\n(私聊时上述功能也可用>_<!)\n想在群里和我聊天的话有如下方式：\n开始聊天/结束聊天\n[CQ:at,qq=3340741722] xxx\n咱们愉快相处吧(●'o'●)！");
			return;
		}
		if (message.substr(0, 10) == "教你百度：") {
			sendTo(ac, qq, "http://iwo.im/?q=" + UrlEncode(message.substr(10)));
			return;
		}
		if (message.substr(0, 9) == "教你百度:") {
			sendTo(ac, qq, "http://iwo.im/?q=" + UrlEncode(message.substr(9)));
			return;
		}
		if (message.substr(0, 6) == "百度：") {
			sendTo(ac, qq, "https://www.baidu.com/s?wd=" + UrlEncode(message.substr(6)));
			return;
		}
		if (message.substr(0, 5) == "百度:") {
			sendTo(ac, qq, "https://www.baidu.com/s?wd=" + UrlEncode(message.substr(5)));
			return;
		}
		if (message.substr(0, 6) == "翻译：") {
			string str = XLAT(message.substr(6));
			if (str == "")
				sendTo(ac, qq, "emmm...");
			else
				sendTo(ac, qq, "翻译结果为：\n" + str);
			return;
		}
		if (message.substr(0, 5) == "翻译:") {
			string str = XLAT(message.substr(5));
			if (str == "")
				sendTo(ac, qq, "emmm...");
			else
				sendTo(ac, qq, "翻译结果为：\n" + str);
			return;
		}
		string str = renameReply(AI(message));
		if (str == "")
			sendTo(ac, qq, "emmm...");
		else
			sendTo(ac, qq, str);
	}

	bool preProcessGroupMessage(int64_t group, int64_t qq, string message) {
		if (qq == OWNER) {
			if (message == "开启小管家") {
				if (!isBinded(group)) {
					addRobot(group);
					send(ac, group, "麦萌萌小管家上线啦~！\n有事请呼唤我哟~(*^_^*)\n(需要帮助的话请发送\"麦萌萌\"~(￣▽￣*)~)");
				}
				else
					send(ac, group, "我在呢！/(ㄒoㄒ)/~~");
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