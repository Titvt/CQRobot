#include <Windows.h>

using namespace std;

void UnicodeToUTF_8(char* pOut, WCHAR* pText) {
	char* pchar = (char *)pText;
	pOut[0] = (0xE0 | ((pchar[1] & 0xF0) >> 4));
	pOut[1] = (0x80 | ((pchar[1] & 0x0F) << 2)) + ((pchar[0] & 0xC0) >> 6);
	pOut[2] = (0x80 | (pchar[0] & 0x3F));
}

void Gb2312ToUnicode(WCHAR* pOut, char* gbBuffer) {
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, gbBuffer, 2, pOut, 1);
}

void GB2312ToUTF_8(string& pOut, char* pText, int pLen) {
	char buf[4];
	memset(buf, 0, 4);
	pOut.clear();
	int i = 0;
	while (i < pLen) {
		if (pText[i] >= 0) {
			char asciistr[2] = { 0 };
			asciistr[0] = (pText[i++]);
			pOut.append(asciistr);
		}
		else {
			WCHAR pbuffer;
			Gb2312ToUnicode(&pbuffer, pText + i);
			UnicodeToUTF_8(buf, &pbuffer);
			pOut.append(buf);
			i += 2;
		}
	}
}

string UrlEncode(string str) {
	string tmp, ret;
	GB2312ToUTF_8(tmp, (char*)str.c_str(), strlen(str.c_str()));
	int len = tmp.length();
	for (int i = 0; i < len; i++) {
		if (isalnum((BYTE)tmp.at(i))) {
			char tempbuff[2] = { 0 };
			sprintf_s(tempbuff, "%c", (BYTE)tmp.at(i));
			ret.append(tempbuff);
		}
		else if (isspace((BYTE)tmp.at(i)))
			ret.append("+");
		else {
			char tempbuff[4];
			sprintf_s(tempbuff, "%%%X%X", ((BYTE)tmp.at(i)) >> 4, ((BYTE)tmp.at(i)) % 16);
			ret.append(tempbuff);
		}
	}
	return ret;
}