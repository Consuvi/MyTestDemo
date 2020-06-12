
#include "windows.h"
#include "string"
#include <iostream>
using namespace std;
#pragma comment(lib, "version.lib")

std::string GetPEResource(const TCHAR* exepath, const TCHAR* type,
	const TCHAR* name, int language = 0)
{
	std::string r = "";

	if (!exepath)
		return r;

	//加载可执行文件
	HMODULE hexe = LoadLibrary(exepath);
	if (!hexe)
		return r;

	//查找资源
	HRSRC src = FindResourceEx(hexe, type, name, language);
	if (src)
	{
		HGLOBAL glb = LoadResource(hexe, src);
		int sz = SizeofResource(hexe, src);
		r = std::string((char *)LockResource(glb), sz);
		UnlockResource(glb);
		FreeResource(glb);
	}

	//释放可执行文件
	FreeLibrary(hexe);

	return r;
}

void PaddingBinary(wstring & in_data)
{
	int len = in_data.length();
	if (len%2>0)
	{
		in_data.push_back((unsigned short)0);
	}
	if (in_data.back() != L'\0')
	{
		in_data.push_back(L'\0');
		in_data.push_back(L'\0');
	}
}


auto write_int = [](wstring & tmp, unsigned int v){
	unsigned short wcs[3]{0};
	//unsigned char cs[5]{0};
// 	for (auto i = 0; i < 4; i++)
// 	{
// 		if ((cs[i] & 0x80) != 0)
// 		{
// 			cs[i] = 0x3f;
// 		}
// 	}
	memcpy(wcs, &v, 4);
	tmp.push_back(wcs[0]);
	tmp.push_back(wcs[1]);
};

auto get_wchar = [](unsigned short us){
	unsigned char cs[3]{0};
	memcpy(cs, &us, 2);
	wchar_t wc = 0;
// 	for (auto i = 0; i < 2; i++)
// 	{
// 		if ((cs[i] & 0x80) != 0)
// 		{
// 			cs[i] = 0x3f;
// 		}
// 	}
	memcpy(&wc, cs, 2);
	return wc;
};

auto write_value = [](wstring & tmp, const wstring & key, const wstring & v){
	wstring tmp1;
	int len = v.length();
	tmp1.append(wstring{ 0, (unsigned short)(len + 1), 1 });
	tmp1.append(key);
	PaddingBinary(tmp1);
	tmp1.append(v);
	tmp1.push_back(L'\0');
	PaddingBinary(tmp1);
	*tmp1.begin() = get_wchar(tmp1.size() * 2);
	tmp.append(tmp1);
};

auto push_shorts = [](wstring & tmp, const wstring & str_in){
	tmp.append(str_in);
};


void write_str(wstring & tmp, const wstring & v)
{
	tmp.append(v);
	tmp.push_back(L'\0');
}

wstring GenVarFileInfo()
{
	wstring tmp;
	//GenVarFileInfo
	tmp.append(wstring{ 0, 0, 1 });
	tmp += L"VarFileInfo";
	PaddingBinary(tmp);
	{
		wstring tmp1;
		//GenVar
		push_shorts(tmp1, wstring{ 0, (unsigned short)4, 0 });
		tmp1.append(L"Translation");
		PaddingBinary(tmp1);
		push_shorts(tmp1, wstring{ (unsigned short)0x4b0, (unsigned short)0x804 });
		*tmp1.begin() = get_wchar(tmp1.size() * 2);
		tmp.append(tmp1);
	}
	*tmp.begin() = get_wchar(tmp.size() * 2);
	return tmp;
}

wstring GenFixedFileInfo()
{
	wstring tmp;
	//GenFixedFileInfo
	//tmp.append(wstring{ (unsigned short)0x04BD, (unsigned short)0xFEEF });
	tmp.push_back(0x04BD);
	tmp.push_back(0xFEEF);
	write_int(tmp, 0x10000);
	unsigned short v1 = 1, v2 = 6, v3 = 0, v4 = 88;
	tmp.append(wstring{ v2, v1, v4, v3 });
	tmp.append(wstring{ v2, v1, v4, v3 });
	write_int(tmp, 0x3F);
	write_int(tmp, 0);
	write_int(tmp, 4);
	write_int(tmp, 1);
	write_int(tmp, 0);
	write_int(tmp, 0);
	write_int(tmp, 0);
	return tmp;
}

wstring GenStringFileInfo()
{
	wstring tmp;
	//GenStringFileInfo
	tmp.append(wstring{ 0, 0, 1 });
	tmp.append(L"StringFileInfo");
	PaddingBinary(tmp);
	{
		wstring tmp1;
		//GenStringTable
		tmp1.append(wstring{ 0, 0, 1 });
		tmp1.append(L"080404b0");
		PaddingBinary(tmp1);
		//write_value(tmp1, L"Comments", L"123");
		write_value(tmp1, L"CompanyName", L"顺丰科技");
		write_value(tmp1, L"FileDescription", L"FileDescription");
		write_value(tmp1, L"FileVersion", L"1.6.0.88");
		write_value(tmp1, L"InternalName", L"InternalName");
		write_value(tmp1, L"LegalCopyright", L"LegalCopyright");
		write_value(tmp1, L"OriginalFilename", L"ksh.exe");
		write_value(tmp1, L"ProductName", L"ProductName");
		write_value(tmp1, L"ProductVersion", L"1.6.0.88");
		write_value(tmp1, L"Assembly Version", L"1.6.0.88");
		*tmp1.begin() = get_wchar(tmp1.size() * 2);
		tmp.append(tmp1);
	}
	*tmp.begin() = get_wchar(tmp.size() * 2);
	return tmp;
}

void ConstructVerInfo(string & cdata)
{
	wstring data;
	data.append(wstring{ 0, (unsigned short)0x34, 0 });
	data.append(L"VS_VERSION_INFO");
	PaddingBinary(data);

	data += GenFixedFileInfo();
	PaddingBinary(data);
	data += GenVarFileInfo();
	data += GenStringFileInfo();

	DWORD sz = data.size() * 2;
	*data.begin() = get_wchar(sz);
	auto buf = new BYTE[sz+1]{0};
	memcpy(buf, data.c_str(), sz);
	cdata = string((char *)buf, sz);
	delete[]buf;
}


void ModifyExeVerInfo(const wchar_t * lpszFile, LPVOID lpBuffer, DWORD dwSize)
{
	HANDLE hResource = BeginUpdateResource(lpszFile, FALSE);
	if (NULL != hResource)
	{
		if (UpdateResource(hResource, RT_VERSION, MAKEINTRESOURCE(VS_VERSION_INFO), 0x804,
			(LPVOID)lpBuffer, dwSize) != FALSE)
		{
			if (EndUpdateResource(hResource, FALSE) != FALSE)
				return;
		}
	}
}

string read_data()
{
	string data;
	auto f = fopen("e:\\rt_ver1", "rb");
	if (f)
	{
		while (true)
		{
			char buf[1024]{0};
			auto nRead = fread(buf, 1, 1023, f);
			if (nRead == 0)
			{
				fclose(f);
				break;
			}
			else
			{
				data += string(buf, nRead);
			}
		}
	}
	return data;
}

void write_to_file(const wstring & file_name, const string & data)
{
	if (!data.empty())
	{
		auto f = _wfopen(file_name.c_str(), L"wb+");
		if (f)
		{
			fwrite(data.c_str(), 1, data.size(), f);
		}
		fclose(f);
	}
}


void modify_lang(LPWSTR lpszFile)
{
	DWORD dwHandle = 0;
	auto dwSize = GetFileVersionInfoSize(lpszFile, &dwHandle);
	if (0 >= dwSize)
	{
		cout << GetLastError() << endl;
		return;
	}

	LPBYTE lpBuffer = new BYTE[dwSize];
	memset(lpBuffer, 0, dwSize);

	if (GetFileVersionInfo(lpszFile, dwHandle, dwSize, lpBuffer) != FALSE)
	{
		if (VerQueryValue(lpBuffer, (L"\\VarFileInfo\\Translation"), (LPVOID *)&lpTranslate, &uTemp) != FALSE)
		{

		}
		HANDLE hResource = BeginUpdateResource(lpszFile, FALSE);
		if (NULL != hResource)
		{
			UINT uTemp;
			DWORD dwVer[4] = { 0 };

		}
	}
}

int wmain(int argc, wchar_t * argv[])
{
	auto lpFilePath = L"D:\\Git\\esg-sfim-windows-std\\bin\\fslinkerpoc_.exe";
	string data;	//data = read_data();
	auto data1 = GetPEResource(L"D:\\Git\\esg-sfim-windows-std\\bin\\fslinkerstd_.exe",
		RT_VERSION, MAKEINTRESOURCE(VS_VERSION_INFO), 0);
	ConstructVerInfo(data);
	cout << hex<<data.size() << endl;
// 	unsigned int sz = data.size();
// 	*(++data.begin()) = 0;
// 	*data.begin() = sz;
// 	BYTE buf[500]{0};
// 	memcpy(buf, data.c_str(), data.size());
// 	memcpy(buf, &sz, 4);
// 	return 0;
	//auto lpszFile = L"D:\\Git\\esg-sfim-windows-std\\bin\\fslinkerpoc_d.exe";
	//data = GetPEResource(lpszFile, RT_VERSION, MAKEINTRESOURCE(VS_VERSION_INFO));

	write_to_file(L"e:\\rt_ver1", data);
	write_to_file(L"e:\\rt_ver2", data1);

	ModifyExeVerInfo(lpFilePath, (LPVOID)data.c_str(), data.size());
	system("pause");
	return 0;
}