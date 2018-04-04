// Dui_RichEdit_Demo.cpp : 定义应用程序的入口点。
//

/*
	@Purpose:	仿QQ在应用中嵌入一个功能强大的Rich Edit;(支持多媒体对象、图文混排, 可与Office Word实现剪切板交互等)
	@Date:		2018/02/27;
	@Author:	Honkhat20;
	@Mail:		worksdata@163.com;
	
*/

/*
	需要实现的功能点:
	1. 能够判断出Rich Edit的版本, 比如MsftEdit.dll对应Rich Edit 4.1, Riched20.dll可能对应2.0等(好像后续的版本也沿用这个名称);
	2. 能够把RTF格式的图文混排数据放入剪切板, Ctrl+V可以粘贴到Word中;反之亦然;
	
*/

/*
	Problems:
		1. 如果使用Native CRichEditCtrl(含有窗口句柄),那么滚动条是个问题.
			有专门替换MFC滚动条的方法吗?
*/

/*
	1.	首先测试一下 RedRain版本的duilib的RichEdit控件, 都支持哪些富功能?
		Caret有问题吗?
		如果要支持RTF格式,肯定要实现OLE CALLBACK(TeamTalk版本的duilib这部分有可参考的价值);


	2.	RichEdit在office365(安装完成后显示office2016)中找到了8.0版本的riched20.dll;有8.1版本的吗?
		richedit.h有对应版本的吗? 安装完W

*/

#include "stdafx.h"
#include "Dui_RichEdit_Demo.h"
#include "MainWnd.h"
#include <Shlwapi.h> //for PathAppendW();

#pragma comment(lib, "shlwapi.lib")
//
//Application entry point!
//
int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	CPaintManagerUI::SetInstance(hInstance);
	WCHAR wsPath[MAX_PATH] = {0};
	::GetModuleFileNameW(NULL, wsPath, MAX_PATH);
	::PathAppendW(wsPath, L"..\\skin");	
	CPaintManagerUI::SetResourcePath(wsPath);
	//use SetResourceZip() if you use .zip resource;

	if(FAILED(::CoInitialize(NULL)))   //添加可能会用到的COM模块;
		return 0;

	CMainWnd* pMainWnd = new CMainWnd;
	pMainWnd->Create(NULL, _T("Dui_RichEdit"), UI_WNDSTYLE_FRAME, 0L, 800, 600); //DIALOG STYLE, TRY OTHERS!
	//UI_WNDSTYLE_DIALOG: 不能最大化;
	//UI_WNDSTYLE_FRAME: 可以最大化;

	pMainWnd->SetIcon(IDI_MSDN);


	pMainWnd->CenterWindow();
	pMainWnd->ShowWindow();
	CPaintManagerUI::MessageLoop();

	delete pMainWnd;
	::CoUninitialize(); //release COM resource;

	return 0;
}
