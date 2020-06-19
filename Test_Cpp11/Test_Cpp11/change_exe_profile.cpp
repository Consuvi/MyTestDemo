
#include "windows.h"
#include "string"
#include <iostream>
#include <regex>
using namespace std;
#pragma comment(lib, "version.lib")

struct VerInfo
{
	wstring VersionString;
	wstring Comments;
	wstring CompanyName;
	wstring FileDescription;
	wstring InternalName;
	wstring LegalCopyright;
	wstring OriginalFilename;
	wstring ProductName;
} gVerInfo;

wstring GetKeyValue(const wstring & key, const wstring & file_path)
{
	TCHAR buf[1024] = L"";
	GetPrivateProfileString(L"Ver", key.c_str(), L"", buf, 1023, file_path.c_str());
	return buf;
}

void ReadVerInfo(const wstring & file_path)
{
	gVerInfo.VersionString = GetKeyValue(L"VersionString", file_path);
	gVerInfo.Comments = GetKeyValue(L"Comments", file_path);
	gVerInfo.CompanyName = GetKeyValue(L"CompanyName", file_path);
	gVerInfo.FileDescription = GetKeyValue(L"FileDescription", file_path);
	gVerInfo.InternalName = GetKeyValue(L"InternalName", file_path);
	gVerInfo.LegalCopyright = GetKeyValue(L"LegalCopyright", file_path);
	gVerInfo.OriginalFilename = GetKeyValue(L"OriginalFilename", file_path);
	gVerInfo.ProductName = GetKeyValue(L"ProductName", file_path);
}

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

void write_size(wstring & tmp)
{
	unsigned short sz = tmp.size();
	if (sz > 0 && tmp.back() == L'\0' && tmp.at(sz - 2) == 0)
	{
		sz -= 1;
	}

	*tmp.begin() = sz*2;
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
	//tmp1.push_back(L'\0');
	PaddingBinary(tmp1);
	unsigned short sz = tmp1.size()*2;
	if (tmp1.back() == L'\0')
	{
		sz -= 2;
	}
	//*tmp1.begin() = get_wchar(sz);
	write_size(tmp1);
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
		push_shorts(tmp1, wstring{ (unsigned short)0x804, (unsigned short)0x4b0 });
		//*tmp1.begin() = get_wchar(tmp1.size() * 2);
		write_size(tmp1);
		tmp.append(tmp1);
	}
	//*tmp.begin() = get_wchar(tmp.size() * 2);
	write_size(tmp);
	return tmp;
}

std::vector<unsigned short> SplitVer(const wstring & ver)
{
	std::wregex reg(L"\\.");
	std::vector<wstring> vec(std::wsregex_token_iterator(ver.begin(),
		ver.end(), reg, -1), std::wsregex_token_iterator());
	std::vector<unsigned short> tmp;
	for (auto a : vec)
	{
		tmp.push_back((unsigned short)stoul(a));
	}
	//不足4个，补足4个
	for (auto i = 0; i < (4 - tmp.size() % 4); i++)
	{
		tmp.push_back(0);
	}
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
	auto iVerList = SplitVer(gVerInfo.VersionString);
	tmp.append(wstring{ iVerList[1], iVerList[0], iVerList[3], iVerList[2] });
	tmp.append(wstring{ iVerList[1], iVerList[0], iVerList[3], iVerList[2] });
	write_int(tmp, 0x3F);
	write_int(tmp, 0);
	write_int(tmp, 0x40004);
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
		write_value(tmp1, L"CompanyName", gVerInfo.CompanyName);
		write_value(tmp1, L"FileDescription", gVerInfo.FileDescription);
		write_value(tmp1, L"FileVersion", gVerInfo.VersionString);
		write_value(tmp1, L"InternalName", gVerInfo.InternalName);
		write_value(tmp1, L"LegalCopyright", gVerInfo.LegalCopyright);
		write_value(tmp1, L"OriginalFilename", gVerInfo.OriginalFilename);
		write_value(tmp1, L"ProductName", gVerInfo.ProductName);
		write_value(tmp1, L"ProductVersion", gVerInfo.VersionString);
		//write_value(tmp1, L"Assembly Version", L"1.6.0.88");
		//*tmp1.begin() = get_wchar(tmp1.size() * 2);
		write_size(tmp1);
		tmp.append(tmp1);
	}
	//*tmp.begin() = get_wchar(tmp.size() * 2);
	write_size(tmp);
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
	data += GenStringFileInfo();
	data += GenVarFileInfo();

 	DWORD sz = data.size() * 2;
// 	*data.begin() = get_wchar(sz);
	write_size(data);
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
		if (UpdateResource(hResource, RT_VERSION, MAKEINTRESOURCE(VS_VERSION_INFO), 0,
			(LPVOID)lpBuffer, dwSize) != FALSE)
		{
			if (EndUpdateResource(hResource, FALSE) != FALSE)
			{
				cout << "update success"<<endl;
				return;
			}
		}
	}
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

int wmain(int argc, wchar_t * argv[])
{
	ReadVerInfo(L"e:\\ver_info.ini");
	wstring lpFilePath = L"D:\\Git\\esg-sfim-windows-std\\bin\\fslinkerstd.exe";
	string data;
	ConstructVerInfo(data);
	ModifyExeVerInfo(lpFilePath.c_str(), (LPVOID)data.c_str(), data.size());
	return 0;
}