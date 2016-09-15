
// MFCApplication1Dlg.cpp : 实现文件
//

#include "stdafx.h"
#include "MFCApplication1.h"
#include "MFCApplication1Dlg.h"
#include "afxdialogex.h"
#include "dwmapi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMFCApplication1Dlg 对话框



CMFCApplication1Dlg::CMFCApplication1Dlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_MFCAPPLICATION1_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCApplication1Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMFCApplication1Dlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()


// CMFCApplication1Dlg 消息处理程序

BOOL CMFCApplication1Dlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	//设置窗口属性，打开自定义缩 略图和AeroPeek预览 
	BOOL truth = TRUE;
	DwmSetWindowAttribute(m_hWnd, DWMWA_HAS_ICONIC_BITMAP, &truth, sizeof(truth));
	DwmSetWindowAttribute(m_hWnd, DWMWA_FORCE_ICONIC_REPRESENTATION, &truth, sizeof(truth));
	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMFCApplication1Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMFCApplication1Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

#include "atlimage.h"
BOOL CMFCApplication1Dlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_DWMSENDICONICTHUMBNAIL)
	{
		CImage img;
		img.LoadFromResource(GetModuleHandle(0), IDB_BITMAP1);
		int w = img.GetWidth();
		int h = img.GetHeight();

		CDC dc;
		CDC * pdc = GetWindowDC();
		dc.CreateCompatibleDC(pdc);
		CBitmap bmp;
		bmp.CreateCompatibleBitmap(pdc, w, h);
		HBITMAP hBmpOld = (HBITMAP)dc.SelectObject(bmp);
		img.Draw(dc.m_hDC, 0, 0);
		HBITMAP hBmpNew = (HBITMAP)dc.SelectObject(hBmpOld);
		
		//HBITMAP hBmp = LoadBitmap(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDB_BITMAP1));
		DwmSetIconicThumbnail(m_hWnd, hBmpNew, 0);

	}
	return CDialog::PreTranslateMessage(pMsg);
}
