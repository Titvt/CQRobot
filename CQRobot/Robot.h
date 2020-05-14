#include <string>
#include <fstream>
#include <vector>
#include <ctime>
#include <io.h>
#include <thread>
#include <regex>
#include <WS2tcpip.h>
#include "cqp.h"
#include "UrlEncode.h"
#include "HTTP.h"
#include "Pica.h"
#include "libssh2.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "libssh2.lib")

using namespace std;

struct autoGroup {
	int64_t group;
	bool allowRepeat;
};

int64_t OWNER, SELF;
vector<autoGroup> GROUPS;

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

string engu(string str) {
	string ret;
	for (int ch = 0; ch < str.length(); ch++)
		for (int i = 7; i >= 0; i--)
			if (str[ch] & 1 << i)
				ret += "~";
			else
				ret += "咕";
	return ret;
}

string degu(string str) {
	string ret;
	char ch = '\0';
	int n = 7;
	for (int i = 0; i < str.length(); i++) {
		if (str[i] == '~')
			ch |= 1 << n;
		else
			i++;
		if (n == 0) {
			ret += ch;
			ch = '\0';
			n = 7;
		}
		else
			n--;
	}
	return ret;
}

void sendImage(int32_t ac, int64_t group) {
	_finddata_t fileinfo;
	int handle = _findfirst("./CQRobot/image/*", &fileinfo);
	int total = -1;
	while (!_findnext(handle, &fileinfo))
		total++;
	_findclose(handle);
	srand(time(0));
	int random = rand() % total + 2;
	handle = _findfirst("./CQRobot/image/*", &fileinfo);
	while (random--)
		_findnext(handle, &fileinfo);
	_findclose(handle);
	CopyFileA(("./CQRobot/image/" + string(fileinfo.name)).c_str(), "./data/image/image", false);
	send(ac, group, "[CQ:image,file=image]");
}

void timer(int32_t ac, int64_t group, int* interval) {
	int old = *interval;
	thread sendImage(sendImage, ac, group);
	sendImage.detach();
	Sleep(*interval * 1000);
	if (*interval == old) {
		thread timer(timer, ac, group, interval);
		timer.detach();
	}
}

void reader(LIBSSH2_CHANNEL *channel, int32_t ac, int64_t bindedGroup, string *lastShell) {
	string reg;
	char buf[4096];
	while (true)
	{
		memset(buf, 0, 4096);
		if (libssh2_channel_read(channel, buf, 4096) == 0)
			return;
		reg = regex_replace(buf, regex("\\r\\n"), "\n");
		reg = regex_replace(reg, regex("\\x1b\\]0;.*?\\x1b\\[\\?1034h"), "");
		reg = regex_replace(reg, regex("\\x1b\\[.*?[mhlHJK]"), "");
		reg = regex_replace(reg, regex("\\x1b\\].*?[\\x07]"), "");
		reg = regex_replace(reg, regex("\\x1b(\\(B|\\=|\\>)"), "");
		reg = regex_replace(reg, regex("\\xe2\\x80[\\x98\\x99]"), "'");
		reg = regex_replace(reg, regex("\\xe2\\x96\\xbd"), "");
		if (!reg.empty() && reg.find("Last failed login: ") == string::npos && reg.find(" failed login attempt since the last successful login.") == string::npos && reg.find("Last login: ") == string::npos && reg != *lastShell && reg != "\n")
			send(ac, bindedGroup, reg);
	}
}

class Robot {
	int32_t ac;
	int64_t bindedGroup;
	vector<int64_t> chatters;
	bool allowRepeat;
	int repeatNumber = 0;
	string repeatText = "";
	vector<int64_t> admins;
	bool enableAI = true;
	bool enableDUI = false;
	vector<int64_t> duiers;
	int interval = 0;
	vector<int64_t> shellers;
	string lastShell = "";
	int sock = 0;
	LIBSSH2_SESSION *session;
	LIBSSH2_CHANNEL *channel;
	Pica pica;

	bool isChatter(int64_t qq) {
		for (auto i = chatters.begin(); i < chatters.end(); i++)
			if ((*i) == qq)
				return true;
		return false;
	}

	void addChatter(int64_t qq) {
		chatters.push_back(qq);
	}

	void deleteChatter(int64_t qq) {
		for (auto i = chatters.begin(); i < chatters.end(); i++)
			if (*i == qq)
				chatters.erase(i);
	}

	bool isAdmin(int64_t qq) {
		if (qq == OWNER)
			return true;
		for (auto i = admins.begin(); i < admins.end(); i++) {
			if (*i == qq)
				return true;
		}
		return false;
	}

	void addAdmin(int64_t qq) {
		admins.push_back(qq);
	}

	void deleteAdmin(int64_t qq) {
		for (auto i = admins.begin(); i < admins.end(); i++)
			if (*i == qq)
				admins.erase(i);
	}

	bool isDuier(int64_t qq) {
		for (auto i = duiers.begin(); i < duiers.end(); i++) {
			if (*i == qq)
				return true;
		}
		return false;
	}

	void addDuier(int64_t qq) {
		duiers.push_back(qq);
	}

	void deleteDuier(int64_t qq) {
		for (auto i = duiers.begin(); i < duiers.end(); i++)
			if (*i == qq)
				duiers.erase(i);
	}

	bool isSheller(int64_t qq) {
		for (auto i = shellers.begin(); i < shellers.end(); i++) {
			if (*i == qq)
				return true;
		}
		return false;
	}

	void addSheller(int64_t qq) {
		shellers.push_back(qq);
	}

	void deleteSheller(int64_t qq) {
		for (auto i = shellers.begin(); i < shellers.end(); i++)
			if (*i == qq)
				shellers.erase(i);
	}

public:
	Robot(int32_t ac, int64_t group, bool allowRepeat) :pica(ac, group) {
		this->ac = ac;
		bindedGroup = group;
		this->allowRepeat = allowRepeat;
	}

	int64_t getBindedGroup() {
		return bindedGroup;
	}

	void setRepeat(bool status) {
		allowRepeat = status;
	}

	void processMessage(int64_t qq, string message) {
		if (qq == OWNER) {
			if (message.substr(0, 14) == "提权[CQ:at,qq=") {
				int64_t q = stoll(message.substr(14, message.length() - 16));
				if (!isAdmin(q))
					addAdmin(q);
				return;
			}
			if (message.substr(0, 14) == "废掉[CQ:at,qq=") {
				int64_t q = stoll(message.substr(14, message.length() - 16));
				if (isAdmin(q))
					deleteAdmin(q);
				return;
			}
			if (message == "开启终端")
			{
				send(ac, bindedGroup, "滴~~~终端开启中……");
				fstream data("./CQRobot/shell.txt", ios::in);
				string ip, username, password;
				if (data.peek() != EOF)
					data >> ip >> username >> password;
				else {
					data.close();
					send(ac, bindedGroup, "当前无终端");
					return;
				}
				data.close();
				WSAStartup(1, &WSADATA());
				sock = socket(AF_INET, SOCK_STREAM, 0);
				sockaddr_in sin;
				sin.sin_family = AF_INET;
				sin.sin_port = htons(22);
				inet_pton(AF_INET, ip.c_str(), &sin.sin_addr.s_addr);
				connect(sock, (sockaddr*)&sin, sizeof(sockaddr_in));
				libssh2_init(0);
				session = libssh2_session_init();
				libssh2_session_handshake(session, sock);
				libssh2_userauth_password(session, username.c_str(), password.c_str());
				channel = libssh2_channel_open_session(session);
				libssh2_channel_request_pty(channel, "xterm");
				libssh2_channel_shell(channel);
				thread reader(reader, channel, ac, bindedGroup, &lastShell);
				reader.detach();
				addSheller(qq);
				return;
			}
			if (message == "关闭终端")
			{
				send(ac, bindedGroup, "叮！终端已关闭~");
				libssh2_channel_free(channel);
				libssh2_session_disconnect(session, "");
				libssh2_session_free(session);
				closesocket(sock);
				libssh2_exit();
				sock = 0;
				return;
			}
			if (message.substr(0, 18) == "添加用户[CQ:at,qq=")
			{
				int64_t q = stoll(message.substr(18, message.length() - 20));
				if (!isSheller(q))
					addSheller(q);
				return;
			}
			if (message.substr(0, 18) == "删除用户[CQ:at,qq=")
			{
				int64_t q = stoll(message.substr(18, message.length() - 20));
				if (isSheller(q))
					deleteSheller(q);
				return;
			}
		}
		if (isAdmin(qq)) {
			if (message == "开启AI" || message == "开启ai") {
				enableAI = true;
				return;
			}
			if (message == "关闭AI" || message == "关闭ai") {
				enableAI = false;
				return;
			}
			if (message == "开启对线") {
				enableDUI = true;
				return;
			}
			if (message == "关闭对线") {
				enableDUI = false;
				return;
			}
			if (message.substr(0, 18) == "关小黑屋[CQ:at,qq=") {
				int64_t q = stoll(message.substr(18, message.length() - 20));
				if (q == OWNER)
					return;
				srand(time(0));
				int thousand = rand() % 2, hundred = rand() % 10, ten = rand() % 10, one = rand() % 10;
				CQ_setGroupBan(ac, bindedGroup, q, (thousand * 1000 + hundred * 100 + ten * 10 + one + 1) * 60);
				send(ac, bindedGroup, "好了你不要说了.jpg");
				return;
			}
			if (message.substr(0, 14) == "捞人[CQ:at,qq=") {
				int64_t q = stoll(message.substr(14, message.length() - 16));
				CQ_setGroupBan(ac, bindedGroup, q, 0);
				send(ac, bindedGroup, "你复活啦！！");
				return;
			}
			if (message == "禁止复读") {
				setRepeat(false);
				send(ac, bindedGroup, "大家不要复读嗷┗|｀O′|┛ ~~");
				return;
			}
			if (message == "允许复读") {
				setRepeat(true);
				send(ac, bindedGroup, "大家可以复读啦┗|｀O′|┛ ~~");
				return;
			}
			if (message.substr(0, 8) == "发福利：") {
				int t = stoi(message.substr(8, message.length() - 8));
				if (t < 0 || t > 86400) {
					send(ac, bindedGroup, "输入错误！\n输入范围为0~86400，单位为秒\n设置为0表示停止发福利");
					return;
				}
				interval = t;
				if (t == 0)
					send(ac, bindedGroup, "停止发福利");
				else {
					send(ac, bindedGroup, "当前的发福利时间被设置为" + message.substr(8, message.length() - 8) + "秒");
					thread timer(timer, ac, bindedGroup, &interval);
					timer.detach();
				}
				return;
			}
			if (message.substr(0, 7) == "发福利:") {
				int t = stoi(message.substr(7, message.length() - 7));
				if (t < 0 || t>86400) {
					send(ac, bindedGroup, "输入错误！\n输入范围为0~86400，单位为秒\n设置为0表示停止发福利");
					return;
				}
				interval = t;
				if (t == 0)
					send(ac, bindedGroup, "停止发福利");
				else {
					send(ac, bindedGroup, "当前的发福利时间被设置为" + message.substr(7, message.length() - 7) + "秒");
					thread timer(timer, ac, bindedGroup, &interval);
					timer.detach();
				}
				return;
			}
		}
		if (message == "麦萌萌") {
			send(ac, bindedGroup, "[CQ:at,qq=" + to_string(qq) + "]\n你好呀~\\(^0^)/\n我是麦萌萌小管家(￣▽￣～) ！\n咱在群里有以下功能哟~：\n教你百度：xxx\n百度：xxx\n翻译：xxx\n咕咕加密：xxx\n咕咕解密：xxx\n想和我聊天的话有如下方式：\n开始聊天/结束聊天\n[CQ:at,qq=" + to_string(SELF) + "] xxx\n想和我对线的话有如下方式：\n来对线/我怂了\n对线状态也支持@，不可与聊天状态并存\n想睡觉的话可以发送\"睡觉\"嗷~~\n若正在发福利的话，发送\"我要福利\"可以立即获得一份福利哟~！\n用户组中的群员在终端开启时可以向终端发送命令哦：\n#C/#c：发送Ctrl-C\n#Z/#z：发送Ctrl-Z\n# xxx：发送xxx（注意空格）\n发送\"哔咔哔咔\"可以获得5张本子推荐哦~\n咱们愉快相处吧(●'o'●)！");
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
		if (message.substr(0, 10) == "咕咕加密：") {
			send(ac, bindedGroup, engu(message.substr(10)));
			return;
		}
		if (message.substr(0, 9) == "咕咕加密:") {
			send(ac, bindedGroup, engu(message.substr(9)));
			return;
		}
		if (message.substr(0, 10) == "咕咕解密：") {
			send(ac, bindedGroup, degu(message.substr(10)));
			return;
		}
		if (message.substr(0, 9) == "咕咕解密:") {
			send(ac, bindedGroup, degu(message.substr(9)));
			return;
		}
		if (message == "睡觉") {
			if (qq == OWNER)
				return;
			srand(time(0));
			int thousand = rand() % 2, hundred = rand() % 10, ten = rand() % 10, one = rand() % 10;
			CQ_setGroupBan(ac, bindedGroup, qq, (thousand * 1000 + hundred * 100 + ten * 10 + one + 1) * 60);
			send(ac, bindedGroup, "[CQ:at,qq=" + to_string(qq) + "]\nお休み (￣▽￣*)~");
			return;
		}
		if (message == "我要福利") {
			if (interval != 0)
				sendImage(ac, bindedGroup);
			else
				send(ac, bindedGroup, "不给，哼！(￣▽￣～)");
			return;
		}
		if (message == "哔咔哔咔") {
			pica.getRandom();
			return;
		}
		if (sock != 0 && isSheller(qq))
		{
			if (message.substr(0, 2) == "#C" || message.substr(0, 2) == "#c")
			{
				libssh2_channel_write(channel, "\x03", 1);
				return;
			}
			if (message.substr(0, 2) == "#Z" || message.substr(0, 2) == "#z")
			{
				libssh2_channel_write(channel, "\x1a", 1);
				return;
			}
			if (message.substr(0, 2) == "# ")
			{
				lastShell = string(message.substr(2));
				libssh2_channel_write(channel, lastShell.c_str(), lastShell.length());
				libssh2_channel_write(channel, "\n", 1);
				return;
			}
		}
		if (message.find("[CQ:at,qq=" + to_string(SELF) + "]") != string::npos) {
			if (enableDUI && isDuier(qq)) {
				send(ac, bindedGroup, "[CQ:at,qq=" + to_string(qq) + "]\n" + DUI());
				return;
			}
			if (enableAI) {
				string str = removeAt(message);
				if (str == "")
					send(ac, bindedGroup, "[CQ:at,qq=" + to_string(qq) + "]\n叫我干啥(⊙o⊙)？");
				else
					send(ac, bindedGroup, "[CQ:at,qq=" + to_string(qq) + "]\n" + renameReply(AI(str)));
				return;
			}
			return;
		}
		if (enableAI && message == "开始聊天" && !isChatter(qq)) {
			addChatter(qq);
			deleteDuier(qq);
			send(ac, bindedGroup, "[CQ:at,qq=" + to_string(qq) + "]\n开始愉快的聊天吧~！");
			return;
		}
		if (enableAI && message == "结束聊天" && isChatter(qq)) {
			deleteChatter(qq);
			send(ac, bindedGroup, "[CQ:at,qq=" + to_string(qq) + "]\n下次再聊吧~！");
			return;
		}
		if (enableDUI && message == "来对线" && !isDuier(qq)) {
			addDuier(qq);
			deleteChatter(qq);
			send(ac, bindedGroup, "[CQ:at,qq=" + to_string(qq) + "]\n来啊！谁怕谁！");
			return;
		}
		if (enableDUI && message == "我怂了" && isDuier(qq)) {
			deleteDuier(qq);
			send(ac, bindedGroup, "[CQ:at,qq=" + to_string(qq) + "]\n知道我的厉害了吧！");
			return;
		}
		if (enableAI && isChatter(qq)) {
			string str = removeAt(message);
			if (str == "")
				send(ac, bindedGroup, "[CQ:at,qq=" + to_string(qq) + "]\n叫我干啥(⊙o⊙)？");
			else
				send(ac, bindedGroup, "[CQ:at,qq=" + to_string(qq) + "]\n" + renameReply(AI(str)));
			return;
		}
		if (enableDUI && isDuier(qq)) {
			send(ac, bindedGroup, "[CQ:at,qq=" + to_string(qq) + "]\n" + DUI());
			return;
		}
		if (!allowRepeat) {
			if (message == repeatText) {
				if (repeatNumber == 2) {
					if (qq == OWNER)
						return;
					srand(time(0));
					int thousand = rand() % 2, hundred = rand() % 10, ten = rand() % 10, one = rand() % 10;
					CQ_setGroupBan(ac, bindedGroup, qq, (thousand * 1000 + hundred * 100 + ten * 10 + one + 1) * 60);
					send(ac, bindedGroup, "[CQ:at,qq=" + to_string(qq) + "]\n哼哼！复读的下场就是被禁言" + to_string(thousand * 1000 + hundred * 100 + ten * 10 + one + 1) + "分钟 (￣▽￣*)！");
				}
				else
					repeatNumber++;
			}
			else {
				repeatText = message;
				repeatNumber = 1;
			}
		}
	}
};

class RobotManager {
	int32_t ac;
	vector<Robot*> robots;

	bool isBinded(int64_t group) {
		for (auto i = robots.begin(); i < robots.end(); i++) {
			if ((*i)->getBindedGroup() == group)
				return true;
		}
		return false;
	}

	void addRobot(int64_t group, bool allowRepeat) {
		robots.push_back(new Robot(ac, group, allowRepeat));
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
	RobotManager(int32_t ac, int64_t owner, int64_t self, vector<autoGroup> groups, bool autoStart) {
		this->ac = ac;
		OWNER = owner;
		SELF = self;
		GROUPS = groups;
		if (autoStart) {
			for (auto i = GROUPS.begin(); i < GROUPS.end(); i++)
				if (!isBinded((*i).group))
					addRobot((*i).group, (*i).allowRepeat);
		}
	}

	void privateMessage(int64_t qq, string message) {
		if (qq == OWNER) {
			if (message == "查询列表") {
				string str = "已经开启的群如下：";
				for (auto i = robots.begin(); i < robots.end(); i++)
					str += "\n" + to_string((*i)->getBindedGroup());
				if (str == "已经开启的群如下：")
					sendTo(ac, OWNER, "目前没有开启的群");
				else
					sendTo(ac, OWNER, str);
				return;
			}
			if (message == "全部开启") {
				for (auto i = GROUPS.begin(); i < GROUPS.end(); i++)
					if (!isBinded((*i).group))
						addRobot((*i).group, (*i).allowRepeat);
				sendTo(ac, OWNER, "操作成功");
				return;
			}
			if (message == "全部关闭") {
				for (auto i = robots.begin(); i < robots.end(); i++)
					free(*i);
				robots.clear();
				sendTo(ac, OWNER, "操作成功");
				return;
			}
		}
		sendTo(ac, qq, "你好呀~\\(^0^)/\n我是麦萌萌小管家(￣▽￣～) ！\n咱们愉快相处吧(●'o'●)！");
	}

	bool preProcessGroupMessage(int64_t group, int64_t qq, string message) {
		if (qq == OWNER) {
			if (message.substr(0, 10) == "开启小管家") {
				if (message == "开启小管家") {
					if (!isBinded(group)) {
						addRobot(group, true);
						send(ac, group, "麦萌萌小管家上线啦~！\n有事请呼唤我哟~(*^_^*)\n(发送\"麦萌萌\"获取帮助嗷~(￣▽￣*)~)");
					}
					else
						send(ac, group, "我在呢！/(ㄒoㄒ)/~~");
				}
				else {
					if (!isBinded(group)) {
						addRobot(group, false);
						send(ac, group, "麦萌萌小管家上线啦~！\n有事请呼唤我哟~(*^_^*)\n(发送\"麦萌萌\"获取帮助嗷~(￣▽￣*)~)");
					}
					else
						send(ac, group, "我在呢！/(ㄒoㄒ)/~~");
				}
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