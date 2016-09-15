
#pragma once
#include "resource.h"
//#include "dwmapi.h"
#include "windows.h"
#include "stdio.h"
using namespace std;


int func_pro(HWND hWnd,UINT msg,WPARAM wp,LPARAM lp)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			//设置窗口属性，打开自定义缩 略图和AeroPeek预览 
			/*BOOL truth = TRUE;
			DwmSetWindowAttribute(hWnd, DWMWA_HAS_ICONIC_BITMAP, &truth, sizeof(truth));
			DwmSetWindowAttribute(hWnd, DWMWA_FORCE_ICONIC_REPRESENTATION, &truth, sizeof(truth));*/
			break;
		}

		case WM_DWMSENDICONICTHUMBNAIL:
		{

			break;
		}
	}
	return DefDlgProc(hWnd,msg,wp,lp);
}

void CreateMyDialog()
{
	DialogBox(0, MAKEINTRESOURCE(IDD_DIALOG1), 0, (DLGPROC)func_pro);
}