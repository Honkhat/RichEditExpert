#ifndef __UINATIVERICHEDIT_H__
#define __UINATIVERICHEDIT_H__
#pragma once
#include <unordered_map>
/*
	@Basic Introduction:
		1. 核心功能借助于Windows Native Control: CRichEditCtrl;
		2. 自绘Scroll Bar;
		3. 实现了OLE接口, 可与MS Word进行剪切板互操作;
		4. 只使用Win32基本窗口API, 不依赖MFC(多亏了微软将后来版本的CRichEditCtrl改成了COM模型);
	@Author:jian.he;
	@Mail:	worksdata@163.com;
	@Date:	2018/03/01;
	@Company: AnyMacro.com;
*/

namespace DuiLib
{
	class UILIB_API CNativeRichEditUI: public CControlUI
	{
	public:
		CNativeRichEditUI(void);
		~CNativeRichEditUI(void);

		virtual LPCTSTR GetClass() const;
		virtual LPVOID  GetInterface(LPCTSTR pstrName);
		virtual void    DoInit();
		virtual void    SetPos(RECT rc);
		virtual void    SetVisible(bool bVisible = true);
		virtual void    SetInternVisible(bool bVisible = true) override;
		virtual void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

		void* GetSkinFrameWndInstance();
		void SetDefaultFormat(LPCTSTR sFontName, int nSize, bool bBold, bool bUnderline, bool bItalic);
		void SetMaxChars(int iMaxChars);
		void SetReadOnly(bool bReadOnly);
		void SetHorzBarVisible(bool bHorzBarVisible);
		void Empty();
		void ResetUndoQueueCache();
		void SelectAll();
		void SetFocus();
		bool CanPaste();
		void Copy();
		void Cut();
		void Paste();
		void PasteSpecial(unsigned int iFormat);
		void SetAutoDetectUrl(bool bAuto);
		void InsertText(LPCTSTR sText);
		BOOL InsertFileObject(LPCTSTR szFileName);
		BOOL InsertImage(LPCTSTR szImagePath, int iDesiredPixelBox=0,DWORD dwUserTag=0);
		BOOL InsertBitmap(HBITMAP hBitmap, int iDesiredPixelX=0, int iDesiredPixelY=0, DWORD dwUserTag=0);
		BOOL ExportPicture(LPCTSTR szDir, int& iExportCount, int*& pPos, LPTSTR*& ppExportFile, DWORD*& pUsrTag);
		
		static BOOL CalcScaledSize(SIZE szIn, SIZE szLimit, SIZE& szOut);
		static int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
		static bool SaveImage(LPCTSTR szDir, HBITMAP hBmp);
		static bool AnsiToUnicode(LPCSTR asStr, std::wstring& wsStr);
		static bool Utf8ToUnicode(std::string& aStr, std::wstring& wStr);
		
#ifdef _DEBUG
		void Test_FreeOleCallbackObject();//test if memory leak;
		BOOL Test_CopyAsRTF();
		BOOL Test_ConvertSelectionToPic();
		void Test_TraverseRE();
		void Test_GetScrollInfo();
#endif

		typedef std::unordered_map<unsigned int, std::string> TIntToStrMap;	
		static const TCHAR* m_kNativeRichEditClass;

	private:
		void _InitClipboardFormatMap();
		void _MoveCaretToEnd(HWND hRE);

		static TIntToStrMap g_mpPredefinedClipboardFormat;
		static UINT m_uRtfCF;

		void* m_pSkinScrollFrame;
		int m_iTextPadding;
	};

}


#endif