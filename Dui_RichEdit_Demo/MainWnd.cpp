#include "MainWnd.h"
#include <Shlwapi.h> //PathAppend();

CMainWnd::CMainWnd(void):m_hRichEdit(NULL),m_pSkinWindow(NULL),m_pNativeRichEdit(NULL),m_pBtnREVisible(NULL),m_pBtnREMaxChars(NULL),m_pBtnREReadOnly(NULL),
	m_pBtnHorzBarVisible(NULL),m_pBtnEmptyRE(NULL),m_pBtnSelAllRE(NULL),m_pBtnCanPasteRE(NULL),m_pBtnCopyRE(NULL),m_pBtnCutRE(NULL),m_pBtnPasteRE(NULL),
	m_pBtnAutoDetectUrl(NULL),m_pBtnInsertImgRE(NULL),m_pBtnInsertFileRE(NULL),m_pBtnClearCacheRE(NULL),m_pBtnTestOleCallbackLeak(NULL),
	m_pBtnPasteSpecialRE(NULL),m_pCboClipboardFormat(NULL),m_pBtnTestCopyAsRtfRE(NULL),m_pBtnSnapRE(NULL),m_pBtnTraverseRE(NULL),m_pBtnGetScrollInfoRE(NULL),
	m_pBtnExportPicRE(NULL)
{
}


CMainWnd::~CMainWnd(void)
{
}

CDuiString CMainWnd::GetSkinFolder()
{
	return _T("MainWnd");
}

CDuiString CMainWnd::GetSkinFile() 
{ 
	return _T("MainWnd.xml"); 
}

LPCTSTR CMainWnd::GetWindowClassName() const 
{
	return _T("CMainWnd"); 
}

void CMainWnd::InitWindow()
{
	m_pSkinWindow = dynamic_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btnSkinWindow")));
	m_pNativeRichEdit = dynamic_cast<CNativeRichEditUI*>(m_PaintManager.FindControl(_T("edtNativeRichEdit")));
	m_pBtnREVisible = dynamic_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btnREVisible")));
	m_pBtnREMaxChars = dynamic_cast<CButtonUI*>(m_PaintManager.FindControl(_T("REMaxChars")));
	m_pBtnREReadOnly = dynamic_cast<CButtonUI*>(m_PaintManager.FindControl(_T("REReadOnly")));
	m_pBtnHorzBarVisible = dynamic_cast<CButtonUI*>(m_PaintManager.FindControl(_T("REHorzBar")));
	m_pBtnEmptyRE = dynamic_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btnEmptyRE")));
	m_pBtnSelAllRE = dynamic_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btnSelAllRE")));
	m_pBtnCanPasteRE = dynamic_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btnCanPasteRE")));
	m_pBtnCopyRE = dynamic_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btnCopyRE")));
	m_pBtnCutRE = dynamic_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btnCutRE")));
	m_pBtnPasteRE = dynamic_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btnPasteRE")));
	m_pBtnAutoDetectUrl = dynamic_cast<CButtonUI*>(m_PaintManager.FindControl(_T("AutoDetectUrl")));
	m_pBtnInsertImgRE = dynamic_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btnInsertImgRE")));
	m_pBtnInsertFileRE = dynamic_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btnInsertFileRE")));
	m_pBtnClearCacheRE = dynamic_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btnClearCacheRE")));
	m_pBtnTestOleCallbackLeak = dynamic_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btnTestOleCallbackLeak")));

	m_pCboClipboardFormat = dynamic_cast<CComboUI*>(m_PaintManager.FindControl(_T("cboClipboardFormat")));
	if(m_pCboClipboardFormat)
	{
		const int arTestClipboardFormat[] = {CF_TEXT, CF_BITMAP, CF_ENHMETAFILE};
		const TCHAR* szFormat[] = {_T("CF_TEXT"), _T("CF_BITMAP"), _T("CF_ENHMETAFILE")};
		const int iSize = sizeof(szFormat) / sizeof(TCHAR*);
		for(int i = 0; i < iSize; ++i)
		{
			CLabelUI* pLbl = new CLabelUI;
			m_pCboClipboardFormat->Add(pLbl);
			pLbl->SetTag(arTestClipboardFormat[i]);
			pLbl->SetText(szFormat[i]);
		}
	}
	m_pBtnPasteSpecialRE = dynamic_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btnPasteSpecialRE")));
	m_pBtnTestCopyAsRtfRE = dynamic_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btnCopyAsRtfRE")));
	m_pBtnSnapRE = dynamic_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btnSnapRE")));
	m_pBtnTraverseRE = dynamic_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btnTraverseRE")));
	m_pBtnGetScrollInfoRE = dynamic_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btnGetScrollInfoRE")));
	m_pBtnExportPicRE = dynamic_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btnExportPicRE")));
}


LRESULT CMainWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT bRet = S_FALSE; //S_FALSE:1; S_OK:0;

	//handle custom messages..
	//..

	if(S_FALSE == bRet)
		return __super::HandleMessage(uMsg, wParam, lParam);
	else
		return S_OK;
}

HWND CreateRichEdit(HWND hwndOwner,        // Dialog box handle.
	int x, int y,          // Location.
	int width, int height, // Dimensions.
	HINSTANCE hinst)       // Application or DLL instance.
{
	TCHAR szPath[MAX_PATH] = {0};
	::GetModuleFileName(NULL, szPath, MAX_PATH);
	::PathAppend(szPath, _T("..\\"));

	::PathAppend(szPath, _T("Msftedit.dll"));
	//::PathAppend(szPath, _T("..\\..\\..\\..\\RICHED20_DLL\\8.0\\RICHED20.DLL"));


	HMODULE hRichEditModule = LoadLibrary(szPath);
	if(NULL == hRichEditModule)
		return NULL;

	const TCHAR* arRichEditVersion[] = {_T("RICHEDIT81W"), _T("RICHEDIT80W"), _T("RICHEDIT70W"), _T("RICHEDIT60W"), _T("RICHEDIT50W")};
	const int iVerListCount = sizeof(arRichEditVersion) / sizeof(TCHAR*);

	HWND hEdit = NULL;
	for(int i = 0; i < iVerListCount; ++i)
	{
		try
		{
			hEdit = CreateWindowEx(0, arRichEditVersion[i]/*MSFTEDIT_CLASS*/, arRichEditVersion[i]/*TEXT("Type here")*/,
				ES_MULTILINE | WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_HSCROLL | WS_VSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_WANTRETURN, /*|WS_BORDER*/
				x, y, width, height, 
				hwndOwner, NULL, hinst, NULL);

			//::SetScrollPos(hEdit, SB_VERT, 20, TRUE);
			if(NULL != hEdit)
			{
				//::SetScrollPos(hEdit, SB_VERT|SB_HORZ, -160, TRUE);
				POINT ptPos;
				ptPos.x = 100;
				ptPos.y = 400;
				SendMessage(hEdit, EM_SETSCROLLPOS, 0,(LPARAM)&ptPos);


				SCROLLINFO info;
				info.cbSize = sizeof(SCROLLINFO);
				memset(&info, 0, sizeof(SCROLLINFO));
				info.fMask = SIF_ALL;
				::GetScrollInfo(hEdit, SB_VERT, &info);
				int iMin = info.nMin;
				int iMax = info.nMax;

				info.cbSize = sizeof(SCROLLINFO);
				memset(&info, 0, sizeof(SCROLLINFO));
				info.fMask = SIF_ALL;
				::GetScrollInfo(hEdit, SB_HORZ, &info);
				iMin = info.nMin;
				iMax = info.nMax;

				break;
			}
		}
		catch(std::exception& e)
		{
			OutputDebugStringA(e.what());
		}
	}

	return hEdit;
}


void CMainWnd::Notify(TNotifyUI& msg) //还可以使用MAKE_DELEGATE()的方法来关联控件与响应方法;
{
	if(msg.sType == DUI_MSGTYPE_WINDOWINIT)
	{
		//InitWindow()是在OnCreate()里面调用的, 此时控件的pos还没有确定(SetPos()在DoPaint()里面调用, 至少得画一次pos才确定);
		//这应该是DoPaint()之后的第一个消息;

		if(NULL == m_hRichEdit)
		{
			//m_hRichEdit = CreateRichEdit(GetHWND(), 150, 350, 200, 200, NULL);//VS2010 MFC对应的是RICHEDIT_CLASS,版本比较低! 所以可能需要在高版本上编译成.dll, VS2010再调用!jian.he 2018/02/28;

// 			CNativeWnd wndNative;
// 			RECT rcPos;
// 			rcPos.left = 200;
// 			rcPos.top = 200;
// 			rcPos.right = 400;
// 			rcPos.bottom = 400;
// 			HWND hNativeWnd = wndNative.CreateWnd(CNativeWnd::m_kSimplestStyle | WS_CHILD | WS_VISIBLE, rcPos, GetHWND());
// 			hNativeWnd;
// 			::ShowWindow(hNativeWnd, SW_SHOW);
			


		}
	}
	else if(msg.sType == DUI_MSGTYPE_CLICK) //奇葩!居然与"DUI_MSGTYPE_CLICK == msg.sType"不等价,可能是重载运算符的原因.
	{
		if( msg.pSender->GetName() ==  _T("closebtn"))
			PostQuitMessage(0);
		else if(msg.pSender->GetName() == _T("minbtn"))
			SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
		else if(msg.pSender->GetName() == _T("maxbtn"))
			SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0);
		else if(msg.pSender->GetName() == _T("restorebtn"))
			SendMessage(WM_SYSCOMMAND, SC_RESTORE, 0);
		else if(msg.pSender == m_pBtnREVisible)
		{
			static int g_iREVisibleCount = 0;
			if(m_pNativeRichEdit)
				m_pNativeRichEdit->SetVisible(++g_iREVisibleCount%2 == 0);
		}
		else if(msg.pSender == m_pBtnREMaxChars)
		{
			static int g_iREMaxCharsCount = 0;
			if(m_pNativeRichEdit)
				m_pNativeRichEdit->SetVisible(++g_iREMaxCharsCount%2 == 0);
		}
		else if(msg.pSender == m_pBtnREReadOnly)
		{
			static int g_iREReadOnlyCount = 0;
			if(m_pNativeRichEdit)
				m_pNativeRichEdit->SetReadOnly(++g_iREReadOnlyCount%2 == 0);
		}
		else if(msg.pSender == m_pBtnHorzBarVisible)
		{
			static int g_iREHorzBarCount = 0;
			if(m_pNativeRichEdit)
				m_pNativeRichEdit->SetHorzBarVisible(++g_iREHorzBarCount%2 == 0);
		}
		else if(msg.pSender == m_pBtnEmptyRE)
		{
			if(m_pNativeRichEdit)
				m_pNativeRichEdit->Empty();
		}
		else if(m_pBtnSelAllRE == msg.pSender)
		{
			if(m_pNativeRichEdit)
				m_pNativeRichEdit->SelectAll();
		}
		else if(m_pBtnCanPasteRE == msg.pSender)
		{
			if(m_pNativeRichEdit)
				m_pNativeRichEdit->CanPaste();
		}
		else if(m_pBtnCopyRE == msg.pSender)
		{
			if(m_pNativeRichEdit)
				m_pNativeRichEdit->Copy();
		}
		else if(m_pBtnCutRE == msg.pSender)
		{
			if(m_pNativeRichEdit)
				m_pNativeRichEdit->Cut();
		}
		else if(m_pBtnPasteRE == msg.pSender)
		{
			if(m_pBtnPasteRE)
				m_pNativeRichEdit->Paste();
		}
		else if(m_pBtnAutoDetectUrl == msg.pSender)
		{
			static unsigned int g_iAutoDetectUrlCount = 0;
			if(m_pNativeRichEdit)
				m_pNativeRichEdit->SetAutoDetectUrl(++g_iAutoDetectUrlCount%2==0);
		}
		else if(m_pBtnInsertImgRE == msg.pSender)
		{
			if(m_pNativeRichEdit)
				for(int i = 0; i <1; ++i)
				m_pNativeRichEdit->InsertImage(_T("1016.jpg"), 0);//.png .jpg可乎????????!!!! jian.he;pain
			//"D:\\pic\\test.bmp"
		}
		else if(m_pBtnInsertFileRE == msg.pSender)
		{
			if(m_pNativeRichEdit)
				for(int i = 0; i <10; ++i)
				{
					m_pNativeRichEdit->InsertFileObject(_T("D:\\pic\\pain.jpg"));//.png .jpg可乎????????!!!! jian.he;pain
					m_pNativeRichEdit->InsertText(_T("A"));
				}
		}
		else if(m_pBtnClearCacheRE == msg.pSender)
		{
			if(m_pNativeRichEdit)
				m_pNativeRichEdit->ResetUndoQueueCache();
		}
		else if(m_pBtnTestOleCallbackLeak == msg.pSender)
		{
			if(m_pNativeRichEdit)
				m_pNativeRichEdit->Test_FreeOleCallbackObject();
		}
		else if(m_pBtnPasteSpecialRE == msg.pSender)
		{
			if(m_pNativeRichEdit)
				m_pNativeRichEdit->PasteSpecial(CF_BITMAP);
		}
		else if(m_pBtnTestCopyAsRtfRE == msg.pSender)
		{
			if(m_pNativeRichEdit)
				m_pNativeRichEdit->Test_CopyAsRTF();
		}
		else if(m_pBtnSnapRE == msg.pSender)
		{
			if(m_pNativeRichEdit)
				m_pNativeRichEdit->Test_ConvertSelectionToPic();
		}
		else if(m_pBtnTraverseRE == msg.pSender)
		{
			if(m_pNativeRichEdit)
				m_pNativeRichEdit->Test_TraverseRE();
		}
		else if(m_pBtnGetScrollInfoRE == msg.pSender)
		{
			if(m_pNativeRichEdit)
				m_pNativeRichEdit->Test_GetScrollInfo();
		}
		else if(m_pBtnExportPicRE == msg.pSender)
		{
			if(m_pNativeRichEdit)
			{
				LPTSTR* arExportFile = NULL;
				int iExportCount = 0;
				DWORD* arUsrTag = NULL;
				int* arPos = NULL;
				m_pNativeRichEdit->ExportPicture(NULL, iExportCount, arPos, arExportFile, arUsrTag);
			}
		}
	}
	else if(msg.sType == DUI_MSGTYPE_ITEMSELECT)
	{
		if(m_pCboClipboardFormat == msg.pSender)
			m_pCboClipboardFormat->SetText(_T("what"));
	}

}
