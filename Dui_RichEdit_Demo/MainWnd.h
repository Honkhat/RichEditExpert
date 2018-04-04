#pragma once

#include "stdafx.h"

class CMainWnd: public WindowImplBase
{
public:
	CMainWnd(void);
	~CMainWnd(void);


protected:
	virtual CDuiString GetSkinFolder();
	virtual CDuiString GetSkinFile();
	virtual LPCTSTR    GetWindowClassName() const;
	virtual void       InitWindow();
	virtual LRESULT    HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void       Notify(TNotifyUI& msg);


private:
	HWND m_hRichEdit;
	CButtonUI* m_pSkinWindow;
	CNativeRichEditUI* m_pNativeRichEdit;
	CButtonUI* m_pBtnREVisible;
	CButtonUI* m_pBtnREMaxChars;
	CButtonUI* m_pBtnREReadOnly;
	CButtonUI* m_pBtnHorzBarVisible;
	CButtonUI* m_pBtnEmptyRE;
	CButtonUI* m_pBtnSelAllRE;
	CButtonUI* m_pBtnCanPasteRE;
	CButtonUI* m_pBtnCopyRE;
	CButtonUI* m_pBtnCutRE;
	CButtonUI* m_pBtnPasteRE;
	CButtonUI* m_pBtnAutoDetectUrl;
	CButtonUI* m_pBtnInsertImgRE;
	CButtonUI* m_pBtnInsertFileRE;
	CButtonUI* m_pBtnClearCacheRE;
	CButtonUI* m_pBtnTestOleCallbackLeak;
	
	CComboUI* m_pCboClipboardFormat;
	CButtonUI* m_pBtnPasteSpecialRE;
	CButtonUI* m_pBtnTestCopyAsRtfRE;
	CButtonUI* m_pBtnSnapRE;
	CButtonUI* m_pBtnTraverseRE;
	CButtonUI* m_pBtnGetScrollInfoRE;
	CButtonUI* m_pBtnExportPicRE;
};
