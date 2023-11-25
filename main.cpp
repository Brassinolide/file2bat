#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <windows.h>
#include <string>

using namespace std;

namespace console_API {
	void console_YN(IN const char* label, IN const char* error, OUT bool* result) {
		string i;
		HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);

		while (1) {
			printf("%s [y/n]: ", label);
			getline(cin, i);

			if (i[0] == 'y' || i[0] == 'Y') {
				*result = true;
				break;
			}
			else if (i[0] == 'n' || i[0] == 'N') {
				*result = false;
				break;
			}
			else {
				SetConsoleTextAttribute(h, 4);
				printf("%s\n", error);
				SetConsoleTextAttribute(h, 7);
			}
		}
	}

	void console_getstring(IN const char* label, IN const char* _default, OUT string* result) {
		printf("%s\n>", label);
		getline(cin, *result);
		if (result->length() == 0)*result = _default;
	}
}

namespace path_API {
	string GetExtensionWithDot(IN string s) {
		int dot = s.find_last_of('.');
		if (dot == -1)return "";
		return s.substr(dot);
	}

	string GetBasenameWithExtension(IN string s) {
		return s.substr(s.find_last_of('\\') + 1);
	}
}

//代码有点复杂，vs的编辑器复制粘贴时会多加空格，干脆直接使用二进制数据
/*
@ECHO OFF&(PUSHD "%~DP0")&(REG QUERY "HKU\S-1-5-19">NUL 2>&1)||(powershell -Command "Start-Process '%~sdpnx0' -Verb RunAs"&&EXIT)
*/
const unsigned char admin[] = {
	0x40,0x45,0x43,0x48,0x4f,0x20,0x4f,0x46,0x46,0x26,0x28,0x50,0x55,0x53,0x48,0x44,0x20,0x22,0x25,0x7e,0x44,0x50,0x30,0x22,0x29,0x26,0x28,0x52,0x45,0x47,0x20,0x51,
	0x55,0x45,0x52,0x59,0x20,0x22,0x48,0x4b,0x55,0x5c,0x53,0x2d,0x31,0x2d,0x35,0x2d,0x31,0x39,0x22,0x3e,0x4e,0x55,0x4c,0x20,0x32,0x3e,0x26,0x31,0x29,0x7c,0x7c,0x28,
	0x70,0x6f,0x77,0x65,0x72,0x73,0x68,0x65,0x6c,0x6c,0x20,0x2d,0x43,0x6f,0x6d,0x6d,0x61,0x6e,0x64,0x20,0x22,0x53,0x74,0x61,0x72,0x74,0x2d,0x50,0x72,0x6f,0x63,0x65,
	0x73,0x73,0x20,0x27,0x25,0x7e,0x73,0x64,0x70,0x6e,0x78,0x30,0x27,0x20,0x2d,0x56,0x65,0x72,0x62,0x20,0x52,0x75,0x6e,0x41,0x73,0x22,0x26,0x26,0x45,0x58,0x49,0x54,
	0x29,0xd,0xa,0
};

void print_usage() {
	MessageBoxA(0, "Usage: file2bat.exe file", 0, 0);
	exit(1);
}

int main(int argc, char* argv[]) {
	SetConsoleTitleA("exe2bat");

	if (argc != 2) {
		print_usage();
	}

	DWORD dwAttrib = GetFileAttributesA(argv[1]);
	if (dwAttrib == INVALID_FILE_ATTRIBUTES || dwAttrib & FILE_ATTRIBUTE_DIRECTORY) {
		print_usage();
	}

	string outfile;
	string _default = path_API::GetBasenameWithExtension(argv[1]);
	console_API::console_getstring("输出文件名 (可空)", _default.c_str(), &outfile);

	bool isAdmin;
	console_API::console_YN("是否以管理员身份运行", "错误输入", &isAdmin);

	bool isTemp;
	console_API::console_YN("是否释放到%Temp%中 (为否则释放到运行目录)", "错误输入", &isTemp);

	ofstream fout("result.bat", ios_base::binary);
	fout << "@ECHO OFF & CD /D %~DP0\n";

	if (isAdmin) {
		fout << admin;
	}

	if (isTemp) {
		outfile.insert(0, "%Temp%\\");
	}

	fout << "certutil -f -decode \"%0\" \"" << outfile << "\" >nul\n";
	fout << "start \"\" \"" << outfile << "\"\nexit\n\n";

	string sys = "certutil -f -encode \"";
	sys += argv[1];
	sys += "\" tmpfile>nul";
	std::system(sys.c_str());

	ifstream fin("tmpfile", ios_base::binary);
	string b64((std::istreambuf_iterator<char>(fin)),{});
	fout << b64;

	fin.close();
	fout.close();

	DeleteFileA("tmpfile");
}
