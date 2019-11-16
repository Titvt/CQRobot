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

string HttpGet(string ip, string port, string url) {
	WORD word = MAKEWORD(2, 2);
	WSADATA ws;
	WSAStartup(word, &ws);
	SOCKET sockfd = socket(AF_INET, SOCK_STREAM, 0);
	int timeout = 2000;
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
	sockaddr_in clientAddr;
	clientAddr.sin_family = AF_INET;
	clientAddr.sin_port = htons(atoi(port.c_str()));
	inet_pton(AF_INET, ip.c_str(), &clientAddr.sin_addr);
	stringstream str;
	str << "GET " + url + " HTTP/1.1\r\nHost: " + ip + ":" + port + "\r\n\r\n";
	connect(sockfd, (sockaddr*)&clientAddr, sizeof(clientAddr));
	send(sockfd, str.str().c_str(), sizeof(str), 0);
	char recvbuf[2048] = { 0 };
	recv(sockfd, recvbuf, sizeof(recvbuf), 0);
	return string(recvbuf);
}

string AI(string message) {
	string ret = toUnicode(HttpGet("122.51.67.5", "80", "http://www.titvt.com/ai.php?question=" + UrlEncode(message)));
	size_t start = 0, end = ret.length() - 8;
	for (int i = 0; i < 9; i++)
		start = ret.find('\n', start + 1);
	return ret.substr(start + 1, end - start);
}

string XLAT(string message) {
	string ret = toUnicode(HttpGet("122.51.67.5", "80", "http://www.titvt.com/xlat.php?text=" + UrlEncode(message)));
	size_t start = 0, end = ret.length() - 8;
	for (int i = 0; i < 9; i++)
		start = ret.find('\n', start + 1);
	return ret.substr(start + 1, end - start);
}