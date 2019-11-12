#include <iostream>
#include <sstream>

#pragma comment(lib,"ws2_32.lib")

string toUnicode(string str) {
	int wcsLen = MultiByteToWideChar(CP_UTF8, NULL, str.c_str(), strlen(str.c_str()), NULL, 0);
	wchar_t* wszString = new wchar_t[wcsLen + 1];
	MultiByteToWideChar(CP_UTF8, NULL, str.c_str(), strlen(str.c_str()), wszString, wcsLen);
	wszString[wcsLen] = '\0';
	char *m_char;
	int len = WideCharToMultiByte(CP_ACP, 0, wszString, wcslen(wszString), NULL, 0, NULL, NULL);
	m_char = new char[len + 1];
	WideCharToMultiByte(CP_ACP, 0, wszString, wcslen(wszString), m_char, len, NULL, NULL);
	m_char[len] = '\0';
	return string(m_char);
}

string HttpGet(string msg) {
	WORD word = MAKEWORD(2, 2);
	WSADATA ws;
	WSAStartup(word, &ws);
	SOCKET sockfd = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in clientAddr;
	clientAddr.sin_family = AF_INET;
	clientAddr.sin_port = htons(80);
	inet_pton(AF_INET, "47.107.120.234", &clientAddr.sin_addr);
	stringstream str;
	str << "GET http://api.qingyunke.com/api.php?key=free&appid=0&msg=" + UrlEncode(msg) + " HTTP/1.1\r\n";
	str << "Host: 47.107.120.234:80\r\n";
	str << "Connection: close\r\n\r\n";
	connect(sockfd, (sockaddr*)&clientAddr, sizeof(clientAddr));
	send(sockfd, str.str().c_str(), sizeof(str), 0);
	char recvbuf[2048] = { 0 };
	recv(sockfd, recvbuf, sizeof(recvbuf), 0);
	string ret = recvbuf;
	ret = ret.substr(ret.find("content\":\"") + 10);
	ret = ret.substr(0, ret.length() - 2);
	return toUnicode(ret);
}