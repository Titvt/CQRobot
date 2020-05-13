#define CURL_STATICLIB

#include "libcurl\curl.h"
#include <openssl\hmac.h>
#include <urlmon.h>

#pragma comment(lib, "libcurl.lib")
#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "wldap32.lib")
#pragma comment(lib, "libssl.lib")
#pragma comment(lib, "libcrypto.lib")
#pragma comment(lib, "urlmon.lib")

size_t callback(void *ptr, size_t size, size_t nmemb, void *stream);

int32_t sendPica(int32_t ac, int64_t group, string message) {
	return CQ_sendGroupMsg(ac, group, message.c_str());
}

class Pica {
	string key;

	string HmacSha256(string str)
	{
		transform(str.begin(), str.end(), str.begin(), ::tolower);
		HMAC_CTX *hmac = HMAC_CTX_new();
		HMAC_Init_ex(hmac, "~d}$Q7$eIni=V)9\\RK/P.RM4;9[7|@/CA}b~OW!3?EV`:<>M7pddUBL5n|0/*Cn", 63, EVP_sha256(), NULL);
		unsigned char hash[32];
		unsigned int len = 32;
		HMAC_Update(hmac, (unsigned char *)str.c_str(), str.length());
		HMAC_Final(hmac, hash, &len);
		HMAC_CTX_free(hmac);
		string hexdigest = "";
		char buf[3];
		for (int i = 0; i < 32; i++)
		{
			sprintf_s(buf, "%02x", hash[i]);
			hexdigest += buf;
		}
		return hexdigest;
	}

public:
	int32_t ac;
	int64_t bindedGroup;
	int num;
	bool downloading;

	Pica(int32_t ac, int64_t group) {
		this->ac = ac;
		bindedGroup = group;
		fstream data("./CQRobot/key.txt", ios::in);
		if (data.peek() != EOF)
			data >> key;
		data.close();
	}

	void getRandom() {
		CURL *curl;
		CURLcode res;
		curl = curl_easy_init();
		if (curl) {
			curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
			curl_easy_setopt(curl, CURLOPT_URL, "https://picaapi.picacomic.com/comics/random");
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
			curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
			curl_slist *headers = NULL;
			char t[11];
			sprintf_s(t, "%d", time(0));
			string s = "signature: " + HmacSha256("comics/random" + string(t) + "cb69a7aab9a833208cf174347e9ee970GETC69BAF41DA5ABD1FFEDC6D2FEA56B");
			headers = curl_slist_append(headers, "api-key: C69BAF41DA5ABD1FFEDC6D2FEA56B");
			headers = curl_slist_append(headers, "accept: application/vnd.picacomic.com.v1+json");
			headers = curl_slist_append(headers, ("authorization: " + key).c_str());
			headers = curl_slist_append(headers, s.c_str());
			headers = curl_slist_append(headers, ("time: " + string(t)).c_str());
			headers = curl_slist_append(headers, "nonce: cb69a7aab9a833208cf174347e9ee970");
			headers = curl_slist_append(headers, "app-version: 2.2.1.3.3.4");
			headers = curl_slist_append(headers, "app-uuid: cb69a7aa-b9a8-3320-8cf1-74347e9ee970");
			headers = curl_slist_append(headers, "app-platform: android");
			headers = curl_slist_append(headers, "app-build-version: 45");
			headers = curl_slist_append(headers, "app-channel: 2");
			headers = curl_slist_append(headers, "user-agent: okhttp/3.8.1");
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
			curl_easy_setopt(curl, CURLOPT_PROXY, "127.0.0.1:10809");
			curl_easy_setopt(curl, CURLOPT_HTTPPROXYTUNNEL, 1);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
			num = 0;
			downloading = false;
			curl_easy_perform(curl);
		}
		curl_easy_cleanup(curl);
	}
};

size_t callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
	Pica *pica = (Pica*)stream;
	if (pica->num >= 5)
		return 0;
	string str((char*)ptr);
	vector<string> names;
	vector<wstring> urls;
	string::const_iterator b = str.begin();
	string::const_iterator e = str.end();
	smatch s;
	regex r(R"("title": ".*?(?=",))");
	while (regex_search(b, e, s, r))
	{
		names.push_back(toUnicode(string(s[0]).substr(10)));
		b = s[0].second;
	}
	b = str.begin();
	r = regex(R"("path": ".*?(?=",))");
	while (regex_search(b, e, s, r))
	{
		string surl = "https://storage1.picacomic.com/static/" + string(s[0]).substr(9);
		int len = MultiByteToWideChar(CP_UTF8, 0, surl.c_str(), surl.length(), 0, 0);
		wchar_t* buf = new wchar_t[len + 1];
		buf[len] = 0;
		MultiByteToWideChar(CP_UTF8, 0, surl.c_str(), surl.length(), buf, len);
		urls.push_back(wstring(buf));
		b = s[0].second;
	}
	for (int i = 0; i < names.size() && pica->num < 5; i++)
	{
		while (pica->downloading)
			Sleep(100);
		pica->downloading = true;
		if (URLDownloadToFile(0, urls[i].c_str(), L"./data/image/pica", 0, 0) == S_OK) {
			sendPica(pica->ac, pica->bindedGroup, "[CQ:image,file=pica]" + names[i]);
			pica->num++;
		}
		pica->downloading = false;
	}
	return size * nmemb;
}