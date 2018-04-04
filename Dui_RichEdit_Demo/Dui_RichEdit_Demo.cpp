// Dui_RichEdit_Demo.cpp : ����Ӧ�ó������ڵ㡣
//

/*
	@Purpose:	��QQ��Ӧ����Ƕ��һ������ǿ���Rich Edit;(֧�ֶ�ý�����ͼ�Ļ���, ����Office Wordʵ�ּ��а彻����)
	@Date:		2018/02/27;
	@Author:	Honkhat20;
	@Mail:		worksdata@163.com;
	
*/

/*
	��Ҫʵ�ֵĹ��ܵ�:
	1. �ܹ��жϳ�Rich Edit�İ汾, ����MsftEdit.dll��ӦRich Edit 4.1, Riched20.dll���ܶ�Ӧ2.0��(��������İ汾Ҳ�����������);
	2. �ܹ���RTF��ʽ��ͼ�Ļ������ݷ�����а�, Ctrl+V����ճ����Word��;��֮��Ȼ;
	
*/

/*
	Problems:
		1. ���ʹ��Native CRichEditCtrl(���д��ھ��),��ô�������Ǹ�����.
			��ר���滻MFC�������ķ�����?
*/

/*
	1.	���Ȳ���һ�� RedRain�汾��duilib��RichEdit�ؼ�, ��֧����Щ������?
		Caret��������?
		���Ҫ֧��RTF��ʽ,�϶�Ҫʵ��OLE CALLBACK(TeamTalk�汾��duilib�ⲿ���пɲο��ļ�ֵ);


	2.	RichEdit��office365(��װ��ɺ���ʾoffice2016)���ҵ���8.0�汾��riched20.dll;��8.1�汾����?
		richedit.h�ж�Ӧ�汾����? ��װ��W

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

	if(FAILED(::CoInitialize(NULL)))   //��ӿ��ܻ��õ���COMģ��;
		return 0;

	CMainWnd* pMainWnd = new CMainWnd;
	pMainWnd->Create(NULL, _T("Dui_RichEdit"), UI_WNDSTYLE_FRAME, 0L, 800, 600); //DIALOG STYLE, TRY OTHERS!
	//UI_WNDSTYLE_DIALOG: �������;
	//UI_WNDSTYLE_FRAME: �������;

	pMainWnd->SetIcon(IDI_MSDN);


	pMainWnd->CenterWindow();
	pMainWnd->ShowWindow();
	CPaintManagerUI::MessageLoop();

	delete pMainWnd;
	::CoUninitialize(); //release COM resource;

	return 0;
}
