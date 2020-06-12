
#include "base.h"
#pragma comment(lib, "version.lib")

void modify_ver()
{
	auto lpRes = L"1.6.0.0";
	wstring strApp = L"D:\\Git\\esg-sfim-windows-std\\bin\\fslinkerpoc_.exe";
	HANDLE    hApp;
	if (NULL == (hApp = ::BeginUpdateResource(strApp.c_str(), NULL)))
	{
		return;
	}
	//标记更新资源
	::UpdateResource(hApp, RT_VERSION, L"ss", 0, (LPVOID)lpRes, wcslen(lpRes));
	//写入新资源
	int error = 0;
	if (!::EndUpdateResource(hApp, FALSE))
	{
		error = GetLastError();
		return;
	}
}
void ReplaceExeIco(LPCTSTR lpszApp, LPCTSTR lpszIco)
{
	
}

struct
{
	WORD wLanguage;
	WORD wCodePage;
} *lpTranslate;


void PaddingBinary(wstring & in_data)
{
	int len = in_data.length();
	int a = len % 4;
	if (a>0)
	{
		for (int i = 0; i < 4 - a; i++)
		{
			in_data.push_back((unsigned short)0);
		}
	}
}

wstring GenString(wstring key, wstring value)
{
	wstring tmp;
	tmp.push_back(0);
	tmp.push_back(value.length()+1);
	tmp.push_back(1);
	tmp += key;
	PaddingBinary(tmp);
	tmp += value;
	PaddingBinary(tmp);
	return tmp;
}

extern "C" __declspec(dllexport) void ModifyExeVerInfo(const char * lpszFile, LPVOID lpBuffer, DWORD dwSize)
{
	MessageBoxA(0, lpszFile, to_string((int)dwSize).c_str(), MB_OK);
	HANDLE hResource = BeginUpdateResourceA(lpszFile, FALSE);
	if (NULL != hResource)
	{
		if (UpdateResource(hResource, RT_VERSION, MAKEINTRESOURCE(VS_VERSION_INFO), 0,
			(LPVOID)lpBuffer, dwSize) != FALSE)
		{
			if (EndUpdateResource(hResource, FALSE) != FALSE)
				return;
		}
	}
}

BOOL UpdateDllFile()
{
	auto lpszFile = L"D:\\Git\\esg-sfim-windows-std\\bin\\fslinkerstd_.exe";

	BOOL bRet = FALSE;
	DWORD dwHandle = 0;
	DWORD dwSize = 0;

	dwSize = GetFileVersionInfoSize(lpszFile, &dwHandle);
	if (0 >= dwSize)
	{
		cout << GetLastError() << endl;
		return bRet;
	}

	LPBYTE lpBuffer = new BYTE[dwSize];
	memset(lpBuffer, 0, dwSize);

	if (GetFileVersionInfo(lpszFile, dwHandle, dwSize, lpBuffer) != FALSE)
	{
		HANDLE hResource = BeginUpdateResource(lpszFile, FALSE);
		if (NULL != hResource)
		{
			UINT uTemp;
			DWORD dwVer[4] = { 0 };

			if (VerQueryValue(lpBuffer, (L"\\VarFileInfo\\Translation"), (LPVOID *)&lpTranslate, &uTemp) != FALSE)
			{
				// 修改版本信息,给副版本号加1
				LPVOID lpFixedBuf = NULL;
				DWORD dwFixedLen = 0;
				if (FALSE != VerQueryValue(lpBuffer, (L"\\"), &lpFixedBuf, (PUINT)&dwFixedLen))
				{
					VS_FIXEDFILEINFO* pFixedInfo = (VS_FIXEDFILEINFO*)lpFixedBuf;

					pFixedInfo->dwFileVersionLS = pFixedInfo->dwFileVersionLS + 0x1;
					pFixedInfo->dwProductVersionLS = pFixedInfo->dwProductVersionLS + 0x1;

					dwVer[0] = HIWORD(pFixedInfo->dwFileVersionMS);
					dwVer[1] = LOWORD(pFixedInfo->dwFileVersionMS);
					dwVer[2] = HIWORD(pFixedInfo->dwFileVersionLS);
					dwVer[3] = LOWORD(pFixedInfo->dwFileVersionLS);
				}

				// 修改版本的文本信息
				LPVOID lpStringBuf = NULL;
				DWORD dwStringLen = 0;
				TCHAR szTemp[MAX_PATH] = { 0 };
				TCHAR szVersion[MAX_PATH] = { 0 };

				TCHAR * szCompanyName = L"顺丰科技a";;
				TCHAR * szFileDescription = L"丰 声s";
				TCHAR * szFileVersion = L"5.9.0.0";
				TCHAR * szInternalName = L"fslinkerstd_.exe";
				TCHAR * szLegalCopyright = L"顺丰科技公司 Copyright (C) 2017";
				TCHAR * szOriginalFilename = L"ksh.exe";
				TCHAR * szProductName = L"丰 声";
				TCHAR * szProductVersion = L"5.6.0.0";
				
				/*
				 BinaryWriter bw = CreateBinaryStream();
				 bw.Write((ushort)0);
				 bw.Write((ushort)0);
				 bw.Write((ushort)1);
				 AppendUnicodeString(bw, "000004b0");
				 PaddingBinary(bw);
				 bw.Write(GenString("Comments", Comments));
				 bw.Write(GenString("CompanyName", CompanyName));
				 bw.Write(GenString("FileDescription", FileDescription));
				 bw.Write(GenString("FileVersion", VersionString));
				 bw.Write(GenString("InternalName", InternalName));
				 bw.Write(GenString("LegalCopyright", LegalCopyright));
				 bw.Write(GenString("OriginalFilename", OriginalFilename));
				 bw.Write(GenString("ProductName", ProductName));
				 bw.Write(GenString("ProductVersion", VersionString));
				 bw.Write(GenString("Assembly Version", VersionString));*/

				wstring tmp;
				tmp.clear();
				tmp.push_back((unsigned short)0);
				tmp.push_back((unsigned short)0);
				tmp.push_back((unsigned short)1);
				tmp += L"000004b0";
				PaddingBinary(tmp);
				tmp.append(GenString(L"CompanyName", L"sss"));

// 				wsprintf(szTemp, L"\\StringFileInfo\\%04x%04x\\", lpTranslate[0].wLanguage, lpTranslate[0].wCodePage);
// 				wstring szBlockLang = szTemp;
// 
// 				if (FALSE != VerQueryValue(lpBuffer, (szBlockLang + L"CompanyName").c_str(), &lpStringBuf, (PUINT)&dwStringLen))
// 					memcpy(lpStringBuf, szCompanyName, (wcslen(szCompanyName) + 1) * sizeof(TCHAR));
// 
// 				if (FALSE != VerQueryValue(lpBuffer, (szBlockLang + L"FileDescription").c_str(), &lpStringBuf, (PUINT)&dwStringLen))
// 					memcpy(lpStringBuf, szFileDescription, (wcslen(szFileDescription) + 1) * sizeof(TCHAR));
// 
// 				if (FALSE != VerQueryValue(lpBuffer, (szBlockLang + L"FileVersion").c_str(), &lpStringBuf, (PUINT)&dwStringLen))
// 					memcpy(lpStringBuf, szFileVersion, (wcslen(szFileVersion) + 1) * sizeof(TCHAR));
// 
// 				if (FALSE != VerQueryValue(lpBuffer, (szBlockLang+L"InternalName").c_str(), &lpStringBuf, (PUINT)&dwStringLen))
// 					memcpy(lpStringBuf, szInternalName, (wcslen(szInternalName) + 1) * sizeof(TCHAR));
// 
// 				if (FALSE != VerQueryValue(lpBuffer, (szBlockLang + L"LegalCopyright").c_str(), &lpStringBuf, (PUINT)&dwStringLen))
// 					memcpy(lpStringBuf, szLegalCopyright, (wcslen(szLegalCopyright) + 1) * sizeof(TCHAR));
// 
// 				if (FALSE != VerQueryValue(lpBuffer, (szBlockLang + L"OriginalFilename").c_str(), &lpStringBuf, (PUINT)&dwStringLen))
// 					memcpy(lpStringBuf, szOriginalFilename, (wcslen(szOriginalFilename) + 1) * sizeof(TCHAR));
// 
// 				if (FALSE != VerQueryValue(lpBuffer, (szBlockLang + L"ProductName").c_str(), &lpStringBuf, (PUINT)&dwStringLen))
// 					memcpy(lpStringBuf, szProductName, (wcslen(szProductName) + 1) * sizeof(TCHAR));
// 
// // 				if (FALSE != VerQueryValue(lpBuffer, (szBlockLang + L"ProductVersion").c_str(), &lpStringBuf, (PUINT)&dwStringLen))
// // 					memcpy(lpStringBuf, szProductVersion, (wcslen(szProductVersion) + 1) * sizeof(TCHAR));
// 				
// 				memset(szTemp, 0, sizeof(szTemp));
// 				wsprintf(szVersion, (L"%d, %d, %d, %d"), dwVer[0], dwVer[1], dwVer[2], dwVer[3]);
// 
// 				memset(szTemp, 0, sizeof(szTemp));
// 				wsprintf(szTemp, (L"\\StringFileInfo\\%04x%04x\\FileVersion"), lpTranslate->wLanguage, lpTranslate->wCodePage);
// 				
// 				if (FALSE != VerQueryValue(lpBuffer, szTemp, &lpStringBuf, (PUINT)&dwStringLen))
// 					memcpy(lpStringBuf, szVersion, (wcslen(szVersion) + 1) * sizeof(TCHAR));
// 				else
// 				{
// 					cout << GetLastError()<<endl;
// 				}
				// 更新
				if (UpdateResource(hResource, RT_VERSION, MAKEINTRESOURCE(VS_VERSION_INFO), lpTranslate->wLanguage, 
					lpBuffer, dwSize) != FALSE)
				{
					if (EndUpdateResource(hResource, FALSE) != FALSE)
						bRet = TRUE;
				}
			}
		}
	}

	if (lpBuffer)
		delete[] lpBuffer;

	return bRet;
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


void test20200610()
{
	auto lpszFile = L"D:\\Git\\esg-sfim-windows-std\\bin\\fslinkerstd_.exe";
	GetPEResource(lpszFile, RT_VERSION, MAKEINTRESOURCE(VS_VERSION_INFO));
}