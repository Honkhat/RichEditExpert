#include "StdAfx.h"
#include "UINativeRichEdit.h"
#include <RichOle.h>
#include <atlconv.h>
#include <atlcomcli.h>
#include <GdiPlus.h>

//for test..
#include <string>
#include <fstream>
#include <sstream>
using namespace std;

namespace DuiLib
{
	//======================================================================================================
	//	CSkinScrollBar class
	//======================================================================================================
	class CSkinScrollBar: public WindowImplBase
	{
	public:
		CSkinScrollBar(bool bVertical):m_bVertical(bVertical),m_pBar(nullptr)
		{
		}
		~CSkinScrollBar(void){}

		void SetScrollInfo(LPSCROLLINFO info)
		{
			if(info && m_pBar)
			{
				//更新滚动范围和位置;
				int iRange = info->nMax - info->nMin;
				int iScrollRange = iRange - info->nPage;
				if(iScrollRange != m_pBar->GetScrollRange())//需要处理nTrackPos吗?
					m_pBar->SetScrollRange(iScrollRange);

				if(info->nPos != m_pBar->GetScrollPos())
					m_pBar->SetScrollPos(info->nPos);

				//更新可见性;(ScrollBarUI控件一直是可见的,外部控制的是ScrollBarWnd的可见性)
// 				bool bVisibleOld = m_pBar->IsVisible();
// 				bool bVisible = iRange > info->nPage;
// 				if(bVisible != bVisibleOld)
// 					m_pBar->SetVisible(bVisible);
			}
		}

		void ApplyAttributeList(LPCTSTR pstrList)
		{
			if(m_pBar)
				m_pBar->ApplyAttributeList(pstrList);
		}

		//返回滚动条窗体与滚动条控件的内边距(为了完全覆盖原生的滚动条而留白的区域的宽度);
		int GetInternalBarWidth()
		{
			int iUIBarW = 0;
			if(m_pBar)
				iUIBarW = m_bVertical ? m_pBar->GetFixedWidth() : m_pBar->GetFixedHeight();
			return iUIBarW;
		}

// 		int GetScrollBarInternalHeight()
// 		{
// 			if(m_pBar)
// 				return m_pBar->GetThumbWidth();
// 			else
// 				return 0;
// 		}

	protected:
		virtual CDuiString GetSkinFile()
		{
			if(m_bVertical)
				return _T("<?xml version=\"1.0\" encoding=\"utf-8\"?>\
						  <Window size=\"1,1\">\
						  <HorizontalLayout>\
						  <Control bkcolor=\"#ffffffff\"/>\
						  <ScrollBar width=\"8\" name=\"bar\" hor=\"false\" showbutton1=\"false\" showbutton2=\"false\" />\
						  </HorizontalLayout>\
						  </Window>\
						  ");
			else
				return _T("<?xml version=\"1.0\" encoding=\"utf-8\"?>\
						  <Window size=\"1,1\">\
						  <VerticalLayout bkcolor=\"#ffffffff\">\
						  <Control bkcolor=\"#ffffffff\"/>\
						  <ScrollBar height=\"8\" name=\"bar\" hor=\"true\" showbutton1=\"false\" showbutton2=\"false\"/>\
						  </VerticalLayout>\
						  </Window>\
						  ");
		}

		virtual LPCTSTR GetWindowClassName() const
		{
			return _T("CSkinScrollBar");
		}

		virtual void InitWindow()
		{
			m_pBar = dynamic_cast<CScrollBarUI*>(m_PaintManager.FindControl(_T("bar")));
		}

		virtual void Notify(TNotifyUI& msg)
		{
			if(msg.sType == DUI_MSGTYPE_SCROLL)
			{
				if(msg.pSender)
				{
					CScrollBarUI* pBar = dynamic_cast<CScrollBarUI*>(msg.pSender);
					if(pBar)
						::SendMessage(::GetParent(m_hWnd), pBar->IsHorizontal()?WM_HSCROLL:WM_VSCROLL, (WPARAM)pBar->GetScrollPos(), (LPARAM)this);
				}
			}
		}


	private:
		CSkinScrollBar(void){}//forbidden to use;

		bool m_bVertical;
		CScrollBarUI* m_pBar;
	};

	//======================================================================================================
	//	CPlaceholderWnd class (the placeholder window for native ctrl)
	//======================================================================================================
	class CPlaceholderWnd: public WindowImplBase
	{
	public:
		CPlaceholderWnd(){}
		~CPlaceholderWnd(void){}

	protected:
		virtual CDuiString GetSkinFile()
		{
			return _T("<?xml version=\"1.0\" encoding=\"utf-8\"?>\
						  <Window size=\"1,1\">\
						  <VerticalLayout>\
						  <Control bkcolor=\"#FFffffff\"/>\
						  </VerticalLayout>\
						  </Window>\
						  ");
		}

		virtual LPCTSTR GetWindowClassName() const
		{
			return _T("CPlaceholderWnd");
		}
	};

	//======================================================================================================
	//	IExRichEditOleCallback class (implement the OLE callback for CRichEditCtrl)
	//======================================================================================================

	interface IExRichEditOleCallback : public IRichEditOleCallback
	{
	public:
		IExRichEditOleCallback()
		{
			m_pStorage = NULL;
			m_iNumStorages = 0;
			m_dwRef = 0;

			// set up OLE storage
			HRESULT hResult = ::StgCreateDocfile(NULL, STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE /*| STGM_DELETEONRELEASE */|STGM_CREATE, 0, &m_pStorage );
			//The caller is responsible for deleting the doc file when finished with it, unless STGM_DELETEONRELEASE was specified for the grfMode parameter.
		}

		virtual ~IExRichEditOleCallback()
		{
			if(m_pStorage)
				m_pStorage->Release();
		}

		int m_iNumStorages;
		IStorage* m_pStorage;
		DWORD m_dwRef;

		virtual HRESULT STDMETHODCALLTYPE GetNewStorage(LPSTORAGE* lplpstg)
		{
			m_iNumStorages++;
			WCHAR tName[50];
			swprintf(tName, L"REOLEStorage%d", m_iNumStorages);

			HRESULT hResult = m_pStorage->CreateStorage(tName, 
				STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE ,
				0, 0, lplpstg );

			return hResult;
		}

		virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void ** ppvObject)
		{
			HRESULT hr = S_OK;
			*ppvObject = NULL;

			if ( iid == IID_IUnknown ||
				iid == IID_IRichEditOleCallback )
			{
				*ppvObject = this;
				AddRef();
				hr = NOERROR;
			}
			else
			{
				hr = E_NOINTERFACE;
			}

			return hr;
		}

		virtual ULONG STDMETHODCALLTYPE AddRef()
		{
			return ++m_dwRef;
		}

		virtual ULONG STDMETHODCALLTYPE Release()
		{
			if ( --m_dwRef < 1 )
			{
				delete this;
				return 0;
			}

			return m_dwRef;
		}

		virtual HRESULT STDMETHODCALLTYPE GetInPlaceContext(LPOLEINPLACEFRAME FAR *lplpFrame, LPOLEINPLACEUIWINDOW FAR *lplpDoc, LPOLEINPLACEFRAMEINFO lpFrameInfo)
		{
			return S_OK;
		}

		virtual HRESULT STDMETHODCALLTYPE ShowContainerUI(BOOL fShow)
		{
			return S_OK;
		}

		virtual HRESULT STDMETHODCALLTYPE QueryInsertObject(LPCLSID lpclsid, LPSTORAGE lpstg, LONG cp)
		{
			return S_OK;
		}

		virtual HRESULT STDMETHODCALLTYPE DeleteObject(LPOLEOBJECT lpoleobj)
		{
			return S_OK;
		}

		virtual HRESULT STDMETHODCALLTYPE QueryAcceptData(LPDATAOBJECT lpdataobj, CLIPFORMAT FAR *lpcfFormat,DWORD reco, BOOL fReally, HGLOBAL hMetaPict)
		{
			return S_OK;
		}

		virtual HRESULT STDMETHODCALLTYPE ContextSensitiveHelp(BOOL fEnterMode)
		{
			return S_OK;
		}

		virtual HRESULT STDMETHODCALLTYPE GetClipboardData(CHARRANGE FAR *lpchrg, DWORD reco, LPDATAOBJECT FAR *lplpdataobj)
		{
			//如果自己处理, 设置剪切板的数据, 那么 return S_OK;
			//如果让RichEdit内置功能自己处理, 那么 return S_FALSE;
			return S_FALSE;
		}

		virtual HRESULT STDMETHODCALLTYPE GetDragDropEffect(BOOL fDrag, DWORD grfKeyState, LPDWORD pdwEffect)
		{
			return S_OK;
		}

		virtual HRESULT STDMETHODCALLTYPE GetContextMenu(WORD seltyp, LPOLEOBJECT lpoleobj, CHARRANGE FAR *lpchrg,HMENU FAR *lphmenu)
		{
			return S_OK;
		}
	};

	//=========================
	//	CImageDataObject class
	//=========================
	class CImageDataObject:IDataObject
	{
	public:
		CImageDataObject():m_ulRefCnt(0),m_pStgMdm(nullptr),m_pFmtEtc(nullptr)
		{
		}

		~CImageDataObject()
		{
		}

		// Methods of the IUnknown interface
		STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject)
		{
			if (iid == IID_IUnknown || iid == IID_IDataObject)
			{
				*ppvObject = this;
				AddRef();
				return S_OK;
			}
			else
				return E_NOINTERFACE;
		}
		STDMETHOD_(ULONG, AddRef)(void)
		{
			++m_ulRefCnt;
			return m_ulRefCnt;
		}
		STDMETHOD_(ULONG, Release)(void)
		{
			if (--m_ulRefCnt < 1)
				delete this;

			return m_ulRefCnt;
		}

		// Methods of the IDataObject Interface
		STDMETHOD(GetData)(FORMATETC *pformatetcIn, STGMEDIUM *pmedium)
		{
			if(!m_pStgMdm)
				return E_HANDLE;

			HANDLE hDst = ::OleDuplicateData(m_pStgMdm->hBitmap, CF_BITMAP, NULL);
			if(NULL == hDst)
				return E_HANDLE;

			pmedium->tymed = TYMED_GDI;
			pmedium->hBitmap = (HBITMAP)hDst;
			pmedium->pUnkForRelease = NULL;

			return S_OK;
		}
		STDMETHOD(GetDataHere)(FORMATETC* pformatetc, STGMEDIUM*  pmedium ) {
			return E_NOTIMPL;
		}
		STDMETHOD(QueryGetData)(FORMATETC*  pformatetc ) {
			return E_NOTIMPL;
		}
		STDMETHOD(GetCanonicalFormatEtc)(FORMATETC*  pformatectIn ,FORMATETC* pformatetcOut ) 	{
			return E_NOTIMPL;
		}
		STDMETHOD(SetData)(FORMATETC* pformatetc , STGMEDIUM*  pmedium , BOOL  fRelease ) {
			m_pFmtEtc = pformatetc;
			m_pStgMdm = pmedium;

			return S_OK;
		}
		STDMETHOD(EnumFormatEtc)(DWORD  dwDirection , IEnumFORMATETC**  ppenumFormatEtc ) {
			return E_NOTIMPL;
		}
		STDMETHOD(DAdvise)(FORMATETC *pformatetc, DWORD advf, IAdviseSink *pAdvSink,
			DWORD *pdwConnection) {
				return E_NOTIMPL;
		}
		STDMETHOD(DUnadvise)(DWORD dwConnection) {
			return E_NOTIMPL;
		}
		STDMETHOD(EnumDAdvise)(IEnumSTATDATA **ppenumAdvise) {
			return E_NOTIMPL;
		}

		// Some Other helper functions;
		IOleObject *GetOleObject(IOleClientSite *pOleClientSite, IStorage *pStorage)
		{
			IOleObject *pOleObject = nullptr;
			if(!m_pFmtEtc)
				return pOleObject;

			SCODE sc = ::OleCreateStaticFromData(this, IID_IOleObject, OLERENDER_FORMAT, 
				m_pFmtEtc, pOleClientSite, pStorage, (void **)&pOleObject);
			if (sc != S_OK)
			{
				if(!pOleObject)
				{
					pOleObject->Release();
					pOleObject = nullptr;
				}
			}			
			return pOleObject;
		}

	private:
		ULONG	m_ulRefCnt;
		STGMEDIUM* m_pStgMdm;//only reference;
		FORMATETC* m_pFmtEtc;//only reference;
	};

	//======================================================================================================
	//	CSkinScrollFrame class
	//  Ref: https://www.codeproject.com/Articles/14724/Replace-a-Window-s-Internal-Scrollbar-with-a-custo
	//======================================================================================================
#define WM_UPDATENATIVECTRLPOS (WM_USER + 101)

	class CSkinScrollFrame: public WindowImplBase
	{
	public:
		CSkinScrollFrame(void):m_funOldProc(NULL),m_bOp(FALSE),m_sbVert(nullptr),m_sbHorz(false),m_pWndLimit(nullptr),m_hNativeCtrl(NULL),m_iTextPadding(0),
			m_bHorzBarVisible(true)
		{
		}

		~CSkinScrollFrame(void)
		{
			if(m_hNativeCtrl)
			{
				IRichEditOle* pRichEditOle = NULL;
				::SendMessage(m_hNativeCtrl, EM_GETOLEINTERFACE, 0, (LPARAM)&pRichEditOle);
				pRichEditOle->Release();
				pRichEditOle->Release();//finally delete memory;
			}
			if(m_pWndLimit)
				delete m_pWndLimit;
			if(m_sbVert)
				delete m_sbVert;
			if(m_sbHorz)
				delete m_sbHorz;
		}

		virtual CDuiString GetSkinFile()
		{
			return _T("<?xml version=\"1.0\" encoding=\"utf-8\"?>\
						  <Window size=\"1,1\">\
						  <VerticalLayout >\
						  <Control bkcolor=\"#ffffffff\" />\
						  </VerticalLayout>\
						  </Window>\
						  ");
		}

		virtual LPCTSTR GetWindowClassName() const
		{
			return _T("CSkinScrollFrame");
		}

		BOOL SkinWindow(HWND hMain, RECT rcCtrl, LPCTSTR vertScrollBarAttrList, LPCTSTR horzScrollBarAttrList)
		{
			this->Create(hMain, _T("SkinScrollFrame"), UI_WNDSTYLE_CHILD, 0L);
			if(!m_hWnd)
				return FALSE;
			if(!m_pWndLimit)
			{
				m_pWndLimit = new CPlaceholderWnd;
				m_pWndLimit->Create(m_hWnd, _T("NativeLimitWnd"), UI_WNDSTYLE_CHILD, 0L, 1, 1);
			}
			
			m_hNativeCtrl = CreateRichEditCtrl(m_pWndLimit->GetHWND(), rcCtrl);
			if(!m_hNativeCtrl)
				return FALSE;
			SetWindowLong(m_hNativeCtrl, GWL_USERDATA, (LONG)this);
			m_funOldProc = (WNDPROC)SetWindowLong(m_hNativeCtrl, GWL_WNDPROC, (LONG)CSkinScrollFrame::HookWndProc);
			//通过一个Timer实现CustomScrollBar与RichEdit的滚动条位置保持一致;
			::SetTimer(m_hWnd, CSkinScrollFrame::m_kTimerID_UpdateScrollBarPos, CSkinScrollFrame::m_kTimerInterval_UpdateScrollBarPos, NULL);		
			
			if(!m_sbVert)
			{
				m_sbVert = new CSkinScrollBar(true);
				m_sbVert->Create(m_hWnd, _T("VertScrollWnd"), WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, 1, 1);
				if(vertScrollBarAttrList)
					m_sbVert->ApplyAttributeList(vertScrollBarAttrList);
 			}
			if(!m_sbHorz)
			{
				m_sbHorz = new CSkinScrollBar(false);
				m_sbHorz->Create(m_hWnd, _T("HorzScrollWnd"), WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, 1, 1);
				if(horzScrollBarAttrList)
					m_sbHorz->ApplyAttributeList(horzScrollBarAttrList);
			}	
			
			return TRUE;
		}

		void SetPos(RECT rc)
		{
			::MoveWindow(GetHWND(), rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, TRUE);
			if(m_pWndLimit)
				::MoveWindow(m_pWndLimit->GetHWND(), 0, 0, rc.right-rc.left, rc.bottom-rc.top, TRUE);
			if(m_hNativeCtrl)
				::MoveWindow(m_hNativeCtrl, 0, 0, rc.right-rc.left, rc.bottom-rc.top, TRUE);			
		}

		void SetVisible(bool bVisible)
		{
			::ShowWindow(m_hWnd, bVisible?SW_SHOW:SW_HIDE);
		}

		virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			LRESULT lr = __super::HandleMessage(uMsg, wParam, lParam);
			BOOL bHandled = FALSE;
			switch(uMsg)
			{
			case WM_TIMER: OnTimer(uMsg, wParam, lParam, bHandled);break;
			case WM_UPDATENATIVECTRLPOS: OnUpdateNativeCtrlPos(uMsg, wParam, lParam, bHandled);break;
			case WM_VSCROLL: OnVScroll(uMsg, wParam, lParam, bHandled);break;
			case WM_HSCROLL: OnHScroll(uMsg, wParam, lParam, bHandled);break;

			default:break;
			}

			return lr;
		}

		HWND CreateRichEditCtrl(HWND hParent, RECT rc)
		{
			HMODULE hRichEditModule = LoadLibrary(_T("Msftedit.dll"));
			//HMODULE hRichEditModule = LoadLibrary(_T("RICHED20.DLL"));//specially for 8.0/8.1 Version;
			if(!hRichEditModule)
				return NULL;

			//Create Rich Edit native control;
			const TCHAR* arRichEditVersion[] = {_T("RICHEDIT81W"), _T("RICHEDIT80W"), _T("RICHEDIT70W"), _T("RICHEDIT60W"), _T("RICHEDIT50W")};
			const int iVerListCount = sizeof(arRichEditVersion) / sizeof(TCHAR*);
			 
			HWND hEdit = NULL;
			for(int i = 0; i < iVerListCount; ++i)
			{
			 	try
			 	{
					DWORD dwStyle = ES_MULTILINE | WS_VISIBLE | WS_CHILD | WS_TABSTOP /*| WS_HSCROLL*/ | WS_VSCROLL /*| ES_AUTOHSCROLL*/ | ES_AUTOVSCROLL | ES_WANTRETURN /*|WS_BORDER*/;
					if(m_bHorzBarVisible)
						dwStyle |= (WS_HSCROLL | ES_AUTOHSCROLL);

			 		hEdit = CreateWindowEx(0, arRichEditVersion[i]/*MSFTEDIT_CLASS*/, _T(""), dwStyle, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, hParent, NULL, CPaintManagerUI::GetInstance(), NULL);			 
			 		if(NULL != hEdit)
					{
						//Set the implemented OLE callback!
						IExRichEditOleCallback* pOleCallback = new IExRichEditOleCallback;
						::SendMessage(hEdit, EM_SETOLECALLBACK, 0, (LPARAM)pOleCallback);
						//设置左右边距, 但是不能设置上下边距, 不符合需求, 弃之;
						//::SendMessage(hEdit, EM_SETMARGINS, (WPARAM)(EC_LEFTMARGIN|EC_RIGHTMARGIN), (LPARAM)MAKELONG(20,20));
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

		inline HWND GetNativeRichEditCtrl()
		{
			return m_hNativeCtrl;
		}

		BOOL GetWindowScrollInfo(HWND hWnd, int nBar, LPSCROLLINFO info, UINT nMask = SIF_ALL)
		{
			if(!info)
				return FALSE;

			info->cbSize = sizeof(SCROLLINFO);
			info->fMask = nMask;
			return ::GetScrollInfo(hWnd, nBar, info);
		}

		void UpdateScrollBarPos()
		{
			if(m_hNativeCtrl)
			{
				SCROLLINFO si = {0};

				//更新垂直滚动条;
				if(GetWindowScrollInfo(m_hNativeCtrl, SB_VERT, &si, SIF_ALL))
				{
					if(m_sbVert)
						m_sbVert->SetScrollInfo(&si);
				}

				//更新水平滚动条;
				memset(&si, 0, sizeof(SCROLLINFO));
				if(GetWindowScrollInfo(m_hNativeCtrl, SB_HORZ, &si, SIF_ALL))
				{
					if(m_sbHorz)
						m_sbHorz->SetScrollInfo(&si);
				}
			}
		}

		LRESULT OnTimer(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
		{
			if(CSkinScrollFrame::m_kTimerID_UpdateScrollBarPos == wParam)
			{
				UpdateScrollBarPos();
				bHandled = TRUE;
			}

			return bHandled?1:0;
		}

		LRESULT OnUpdateNativeCtrlPos(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
		{
			m_bOp = TRUE;
			BOOL bVScroll = wParam;
			BOOL bLeftScroll = lParam;
			RECT rcWnd;
			::GetWindowRect(m_hWnd, &rcWnd);			
			::OffsetRect(&rcWnd, -rcWnd.left, -rcWnd.top);
			RECT rcLimit = rcWnd;

			//更新底部坐标;
			bool bHScroll = ::IsWindowVisible(m_sbHorz->GetHWND());
			if(bHScroll)
			{
				RECT rcWindow;
				::GetWindowRect(m_sbHorz->GetHWND(), &rcWindow);
				rcLimit.bottom -= (rcWindow.bottom-rcWindow.top);
			}

			//更新左右坐标;
			if(bLeftScroll)
			{
				if(bVScroll)
				{
					RECT rcWindow;
					::GetWindowRect(m_sbVert->GetHWND(), &rcWindow);
					int iBarW = rcWindow.right - rcWindow.left;
					rcLimit.left += iBarW;
				}
			}
			else
			{
				if(bVScroll)
				{
					RECT rcWindow;
					::GetWindowRect(m_sbVert->GetHWND(), &rcWindow);
					int iBarW = rcWindow.right - rcWindow.left;
					rcLimit.right -= iBarW;
				}
			}

			::MoveWindow(m_pWndLimit->GetHWND(), rcLimit.left, rcLimit.top, rcLimit.right-rcLimit.left, rcLimit.bottom-rcLimit.top, TRUE);

			//设置RichEdit内容区域的大小;
			//当有滚动条时,已经有了边距,所以就不必设了.
			//结论: left top必须设置; right仅当无VertScrollBar时设置; bottom仅当无HorzScrolBar时设置;			
			RECT rcContent = rcLimit;
			rcContent.top += m_iTextPadding;
			if(bVScroll)
			{
				if(bLeftScroll)
					rcContent.right -= m_iTextPadding;
				else
					rcContent.left += m_iTextPadding;
			}
			else
			{
				rcContent.left += m_iTextPadding;
				rcContent.right -= m_iTextPadding;
			}
			if(!bHScroll)
				rcContent.bottom -= m_iTextPadding;
			RECT rcOldContent;
			::SendMessage(m_hNativeCtrl, EM_GETRECT, 0, (LPARAM)&rcOldContent);
			if((rcOldContent.left != rcContent.left) || (rcOldContent.top != rcContent.top) || (rcOldContent.right != rcContent.right) || (rcOldContent.bottom != rcContent.bottom))
				::SendMessage(m_hNativeCtrl, EM_SETRECT, 0, (LPARAM)&rcContent);

			m_bOp = FALSE;

			return 0;
		}

		LRESULT OnVScroll(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
		{
			m_bOp = TRUE;
			if(m_hNativeCtrl)
			{
				//不能使用发送WM_VSCROLL/WM_HSCROLL的方法来设置滚动, 因为duilib的CScrollBarUI控件发送DUI_MSGTYPE_SCROLL的时候已经更新了ScrollPos;
				//::SetScrollPos(m_hNativeCtrl, SB_VERT, wParam, TRUE);//应该只为CScrollBar控件设计的? 需要测试！！！jian.he;

				int xPos = 0;
				SCROLLINFO info;
				if(GetWindowScrollInfo(m_hNativeCtrl, SB_HORZ, &info))
					xPos = info.nPos;

				POINT ptPos;
				ptPos.x = xPos;
				ptPos.y = wParam;
				::SendMessage(m_hNativeCtrl, EM_SETSCROLLPOS, 0,(LPARAM)&ptPos);
				bHandled = TRUE;
			}
			m_bOp = FALSE;

			return bHandled?1:0;
		}

		LRESULT OnHScroll(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
		{
			m_bOp = TRUE;
			if(m_hNativeCtrl)
			{
				//不能使用发送WM_VSCROLL/WM_HSCROLL的方法来设置滚动, 因为duilib的CScrollBarUI控件发送DUI_MSGTYPE_SCROLL的时候已经更新了ScrollPos;
				int yPos = 0;
				SCROLLINFO info;
				if(GetWindowScrollInfo(m_hNativeCtrl, SB_VERT, &info))
					yPos = info.nPos;

				POINT ptPos;
				ptPos.x = wParam;
				ptPos.y = yPos;
				::SendMessage(m_hNativeCtrl, EM_SETSCROLLPOS, 0,(LPARAM)&ptPos);
				bHandled = TRUE;
			}
			m_bOp = FALSE;

			return bHandled?1:0;
		}

		static LRESULT CALLBACK HookWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
		{
			CSkinScrollFrame *pSkinFrame = (CSkinScrollFrame*)GetWindowLong(hwnd, GWL_USERDATA);
			if(!pSkinFrame)
				return 0;

			if(WM_DESTROY == msg)//还原被HOOK的WNDPROC;
			{
				WNDPROC procOld = pSkinFrame->m_funOldProc;
				SetWindowLong(hwnd, GWL_WNDPROC, (LONG)procOld);
				SetWindowLong(hwnd, GWL_USERDATA, 0);
				return ::CallWindowProc(procOld, hwnd, msg, wp, lp);
			}

			LRESULT lr = ::CallWindowProc(pSkinFrame->m_funOldProc, hwnd, msg, wp, lp);

			if(pSkinFrame->m_bOp)
				return lr;

			if(WM_ERASEBKGND == msg)
			{
				SCROLLINFO si;
				DWORD dwStyle = ::GetWindowLong(hwnd, GWL_STYLE);
				if(dwStyle & WS_VSCROLL)
				{
					if(pSkinFrame->m_sbVert)
					{
						memset(&si, 0, sizeof(si));
						si.cbSize = sizeof(si);
						si.fMask = SIF_ALL;
						::GetScrollInfo(hwnd, SB_VERT, &si);
						pSkinFrame->m_sbVert->SetScrollInfo(&si);
					}
				}
				if(dwStyle & WS_HSCROLL)
				{
					if(pSkinFrame->m_sbHorz)
					{
						memset(&si, 0, sizeof(si));
						si.cbSize = sizeof(si);
						si.fMask = SIF_ALL;
						::GetScrollInfo(hwnd, SB_HORZ, &si);
						pSkinFrame->m_sbHorz->SetScrollInfo(&si);
					}
				}
			}
			else if((WM_NCCALCSIZE == msg) && wp)
			{
				LPNCCALCSIZE_PARAMS pNcCalcSizeParam = (LPNCCALCSIZE_PARAMS)lp;
				DWORD dwStyle = ::GetWindowLong(hwnd, GWL_STYLE);
				DWORD dwExStyle = ::GetWindowLong(hwnd, GWL_EXSTYLE);
				BOOL  bLeftScroll = dwExStyle & WS_EX_LEFTSCROLLBAR;
				int iNativeBarWidth = ::GetSystemMetrics(SM_CXVSCROLL);
				
				//这里, 我不打算更改RichEdit的内容区域的大小, 因为改变后, 在原滚动条区域和自定义滚动条区域的差域会出现鼠标形状在箭头和竖线之间的频繁闪烁!
// 				if(dwStyle & WS_VSCROLL)
// 				{
// 					if(bLeftScroll)
// 						pNcCalcSizeParam->rgrc[0].left -= nWid - pSkinFrame->m_sbVert->GetScrollBarWidth();
// 					else
// 						pNcCalcSizeParam->rgrc[0].right += nWid - pSkinFrame->m_sbVert->GetScrollBarWidth();
// 				}
// 				if(dwStyle & WS_HSCROLL)
// 					pNcCalcSizeParam->rgrc[0].bottom += nWid - pSkinFrame->m_sbHorz->GetScrollBarHeight();

				//当水平和竖直滚动条同时出现时, 计算滚动条末端应该增加的长度, 使得水平滚动条、竖直滚动条都滚动到底时能够"碰头".
				int iCustomBarWndAdd = 0;
				if( (dwStyle&WS_VSCROLL) && (dwStyle&WS_HSCROLL) && pSkinFrame->m_sbVert )
					iCustomBarWndAdd = iNativeBarWidth - pSkinFrame->m_sbVert->GetInternalBarWidth();

				RECT rc,rcVert,rcHorz;
				::GetWindowRect(hwnd, &rc);
				::OffsetRect(&rc, -rc.left, -rc.top);

				if(bLeftScroll)
				{
					int nLeft = pNcCalcSizeParam->rgrc[0].left;
					int nBottom = pNcCalcSizeParam->rgrc[0].bottom;
					rcVert.right = nLeft;
					rcVert.left = nLeft-iNativeBarWidth;
					rcVert.top = 0;
					rcVert.bottom = nBottom + iCustomBarWndAdd;
					rcHorz.left = nLeft - iCustomBarWndAdd;
					rcHorz.right = pNcCalcSizeParam->rgrc[0].right;
					rcHorz.top = nBottom;
					rcHorz.bottom = nBottom+iNativeBarWidth;
				}
				else
				{
					int nRight = pNcCalcSizeParam->rgrc[0].right;
					int nBottom = pNcCalcSizeParam->rgrc[0].bottom;
					rcVert.left = nRight;
					rcVert.right = nRight+iNativeBarWidth;
					rcVert.top = 0;
					rcVert.bottom = nBottom + iCustomBarWndAdd;
					rcHorz.left = 0;
					rcHorz.right = nRight + iCustomBarWndAdd;
					rcHorz.top = nBottom;
					rcHorz.bottom = nBottom+iNativeBarWidth;
				}

				if(dwStyle&WS_VSCROLL)
				{
					::MoveWindow(pSkinFrame->m_sbVert->GetHWND(), rcVert.left, rcVert.top, rcVert.right-rcVert.left, rcVert.bottom-rcVert.top, TRUE);
					::ShowWindow(pSkinFrame->m_sbVert->GetHWND(), SW_SHOW);
				}
				else
				{
					::ShowWindow(pSkinFrame->m_sbVert->GetHWND(), SW_HIDE);
				}
				if(dwStyle&WS_HSCROLL)
				{
					::MoveWindow(pSkinFrame->m_sbHorz->GetHWND(), rcHorz.left, rcHorz.top, rcHorz.right-rcHorz.left, rcHorz.bottom-rcHorz.top, TRUE);
					::ShowWindow(pSkinFrame->m_sbHorz->GetHWND(), SW_SHOW);
				}
				else
				{
					::ShowWindow(pSkinFrame->m_sbHorz->GetHWND(), SW_HIDE);
				}
				::PostMessage(pSkinFrame->m_hWnd, WM_UPDATENATIVECTRLPOS, dwStyle&WS_VSCROLL, bLeftScroll);
			}
			return lr;
		}

		void SetTextPadding(int iPadding)
		{
			if(iPadding >= 0)
				m_iTextPadding = iPadding;
		}

		void SetHorzBarVisible(bool bHorzBarVisible)
		{
			m_bHorzBarVisible = bHorzBarVisible;
			//设置RichEdit的水平滚动条是否可见, 只能在创建前指定确定的风格;
			//如果运行时设置, 达不到预期效果;
		}
		

	private:
		WNDPROC		m_funOldProc;
		BOOL		m_bOp;//operation is busy;
		CSkinScrollBar* m_sbVert;
		CSkinScrollBar* m_sbHorz;
		CPlaceholderWnd*	m_pWndLimit;
		const static unsigned int m_kTimerID_UpdateScrollBarPos;
		const static unsigned int m_kTimerInterval_UpdateScrollBarPos;
		int m_iTextPadding;
		HWND m_hNativeCtrl;
		bool m_bHorzBarVisible;
	};
	const unsigned int CSkinScrollFrame::m_kTimerID_UpdateScrollBarPos = 101;
	const unsigned int CSkinScrollFrame::m_kTimerInterval_UpdateScrollBarPos = 200;
	

	//======================================================================================================
	//	CNativeRichEditUI class
	//======================================================================================================

	//类静态变量;
	const TCHAR* CNativeRichEditUI::m_kNativeRichEditClass = _T("CNativeRichEditUI");
	UINT CNativeRichEditUI::m_uRtfCF = 0;

	CNativeRichEditUI::CNativeRichEditUI(void):m_pSkinScrollFrame(nullptr),m_iTextPadding(4)
	{
		_InitClipboardFormatMap();
	}


	CNativeRichEditUI::~CNativeRichEditUI(void)
	{
		if(m_pSkinScrollFrame)
			delete m_pSkinScrollFrame;
	}

	LPCTSTR CNativeRichEditUI::GetClass() const
	{
		return m_kNativeRichEditClass;
	}

	LPVOID CNativeRichEditUI::GetInterface(LPCTSTR pstrName)
	{
		if (0 == _tcscmp(pstrName, DUI_CTR_NATIVERICHEDIT))
			return this;
		else
			return CControlUI::GetInterface(pstrName);
	}

	void CNativeRichEditUI::DoInit()
	{
		HMODULE hRichEditModule = LoadLibrary(_T("Msftedit.dll"));
		//HMODULE hRichEditModule = LoadLibrary(_T("RICHED20.dll"));//specially for 8.0/8.1 Version;
		if(NULL == hRichEditModule)
			return;

		if(m_pManager && m_pManager->GetPaintWindow() && CPaintManagerUI::GetInstance())
		{
			RECT rc = {0,0,1,1};//此时没有准确的ClientRect;

			CSkinScrollFrame* pSkinFrame = (CSkinScrollFrame*)GetSkinFrameWndInstance();
			if(pSkinFrame)
			{
				pSkinFrame->SetTextPadding(m_iTextPadding);
				pSkinFrame->SkinWindow(m_pManager->GetPaintWindow(), rc, m_pManager->GetDefaultAttributeList(_T("VScrollBar")), m_pManager->GetDefaultAttributeList(_T("HScrollBar")));
			}
		}

		//字体设置;
		SetDefaultFormat(NULL, 0, false, false, false);
// 		m_RichEditCtrlTS.SetSelectionCharFormat(cf); //为选定的内容设定所选字体
// 		else 
// 			m_RichEditCtrlTS.SetWordCharFormat(cf); //为将要输入的内容设定字体
// 		m_RichEditCtrlTS.SetDefaultCharFormat(cf);

		//SetMaxChars(200);

		if(m_uRtfCF < 1)
			m_uRtfCF = RegisterClipboardFormat(CF_RTF);
	}

	void CNativeRichEditUI::SetPos(RECT rc)
	{
		__super::SetPos(rc);

		if(m_pSkinScrollFrame)
			((CSkinScrollFrame*)m_pSkinScrollFrame)->SetPos(rc);
	}

	void CNativeRichEditUI::SetVisible(bool bVisible /*= true*/)
	{
		__super::SetVisible(bVisible);

		if(m_pSkinScrollFrame)
			((CSkinScrollFrame*)m_pSkinScrollFrame)->SetVisible(bVisible);
	}

	void CNativeRichEditUI::SetInternVisible(bool bVisible)
	{
		__super::SetInternVisible(bVisible);

		if(m_pSkinScrollFrame)
			((CSkinScrollFrame*)m_pSkinScrollFrame)->SetVisible(bVisible);
	}

	void CNativeRichEditUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if(0 == _tcscmp(pstrName, _T("horzbar")))
			SetHorzBarVisible(0 == _tcscmp(pstrValue, _T("true")));
		else
			__super::SetAttribute(pstrName, pstrValue);
	}

	void* CNativeRichEditUI::GetSkinFrameWndInstance()
	{
		if(!m_pSkinScrollFrame)
			m_pSkinScrollFrame = new CSkinScrollFrame;
		return m_pSkinScrollFrame;
	}

	void CNativeRichEditUI::SetDefaultFormat(LPCTSTR sFontName, int nSize, bool bBold, bool bUnderline, bool bItalic)
	{
		if(!m_pSkinScrollFrame)
			return;

		HWND hRichEdit = ((CSkinScrollFrame*)m_pSkinScrollFrame)->GetNativeRichEditCtrl();
		
		//给空值设定默认值;
		if(!sFontName)
		{
			TFontInfo* pDuiFontInfo = m_pManager->GetDefaultFontInfo();
			if(pDuiFontInfo)
			{
				sFontName = pDuiFontInfo->sFontName.GetData();
				nSize = pDuiFontInfo->iSize;
				bBold = pDuiFontInfo->bBold;
				bUnderline = pDuiFontInfo->bUnderline;
				bItalic = pDuiFontInfo->bItalic;
			}
		}
		
		//目前只选择性的设置几个传进来的参数标识的变量(基本需求), 实际可以设置的参数还有很多.(后续慢慢实现, 扩展空间本身属性即可)
		CHARFORMAT2 fmt2;
		memset(&fmt2, 0, sizeof(CHARFORMAT2));
		fmt2.cbSize = sizeof(CHARFORMAT2);
		//::SendMessage(hRichEdit, EM_GETCHARFORMAT, 0, (LPARAM)&fmt2);

		if(sFontName)
		{
			fmt2.dwMask |= CFM_FACE;
			_tcscpy(fmt2.szFaceName, sFontName);
		}
		if(nSize > 0)
		{
			fmt2.dwMask |= CFM_SIZE;
			fmt2.yHeight = nSize*nSize;
		}
		fmt2.dwMask |= CFM_BOLD;
		if(bBold)
			fmt2.dwEffects |= CFE_BOLD;
		else
			fmt2.dwEffects &= (~CFE_BOLD);
		
		::SendMessage(hRichEdit, EM_SETCHARFORMAT, 0, (LPARAM)&fmt2);
	}

	void CNativeRichEditUI::SetMaxChars(int iMaxChars)
	{
		if(iMaxChars > 0 && m_pSkinScrollFrame)
		{
			HWND hRichEdit = ((CSkinScrollFrame*)m_pSkinScrollFrame)->GetNativeRichEditCtrl();
			if(hRichEdit)
				::SendMessage(hRichEdit, EM_SETLIMITTEXT, iMaxChars, 0);//also EM_EXLIMITTEXT;
		}
	}
	
	void CNativeRichEditUI::SetReadOnly(bool bReadOnly)
	{
		if(m_pSkinScrollFrame)
		{
			HWND hRichEdit = ((CSkinScrollFrame*)m_pSkinScrollFrame)->GetNativeRichEditCtrl();
			if(hRichEdit)
			{
				::SendMessage(hRichEdit, EM_SETLIMITTEXT, bReadOnly, 0);
				::EnableWindow(hRichEdit, bReadOnly?FALSE:TRUE);
			}
		}
	}

	void CNativeRichEditUI::SetHorzBarVisible(bool bHorzBarVisible)
	{
		CSkinScrollFrame* pSkinFrame = (CSkinScrollFrame*)GetSkinFrameWndInstance();
		if(pSkinFrame)
			pSkinFrame->SetHorzBarVisible(bHorzBarVisible);
	}

	void CNativeRichEditUI::Empty()
	{
		if(m_pSkinScrollFrame)
		{
			HWND hRichEdit = ((CSkinScrollFrame*)m_pSkinScrollFrame)->GetNativeRichEditCtrl();
			if(hRichEdit)
			{
				::SendMessage(hRichEdit, EM_SETSEL, 0, -1);
				::SendMessage(hRichEdit, EM_REPLACESEL, FALSE, (WPARAM)_T(""));//wParam set to FALSE, indicates that this operation can't undo.
				::SendMessage(hRichEdit, WM_SETTEXT, 0, (LPARAM)_T(""));
			}
		}
	}

	const unsigned int g_kRichEditUndoLimit = 10;//RichEdit's default value:100;
	void CNativeRichEditUI::ResetUndoQueueCache()
	{
		if(m_pSkinScrollFrame)
		{
			HWND hRichEdit = ((CSkinScrollFrame*)m_pSkinScrollFrame)->GetNativeRichEditCtrl();
			if(hRichEdit)
			{
				::SendMessage(hRichEdit, EM_SETUNDOLIMIT, 0, 0);
				::SendMessage(hRichEdit, EM_SETUNDOLIMIT, g_kRichEditUndoLimit, 0);
			}
		}
	}

	void CNativeRichEditUI::SelectAll()
	{
		if(m_pSkinScrollFrame)
		{
			HWND hRichEdit = ((CSkinScrollFrame*)m_pSkinScrollFrame)->GetNativeRichEditCtrl();
			if(hRichEdit)
			{
				::SendMessage(hRichEdit, EM_SETSEL, 0, -1);
				::SetFocus(hRichEdit);
			}
		}
	}

	void CNativeRichEditUI::SetFocus()
	{
		if(m_pSkinScrollFrame)
		{
			HWND hRichEdit = ((CSkinScrollFrame*)m_pSkinScrollFrame)->GetNativeRichEditCtrl();
			if(hRichEdit)
				::SetFocus(hRichEdit);
		}
	}

	CNativeRichEditUI::TIntToStrMap CNativeRichEditUI::g_mpPredefinedClipboardFormat;
	void CNativeRichEditUI::_InitClipboardFormatMap()
	{
		g_mpPredefinedClipboardFormat[1] = "CF_TEXT";
		g_mpPredefinedClipboardFormat[2] = "CF_BITMAP";
		g_mpPredefinedClipboardFormat[3] = "CF_METAFILEPICT";
		g_mpPredefinedClipboardFormat[4] = "CF_SYLK";
		g_mpPredefinedClipboardFormat[5] = "CF_DIF";
		g_mpPredefinedClipboardFormat[6] = "CF_TIFF";
		g_mpPredefinedClipboardFormat[7] = "CF_OEMTEXT";
		g_mpPredefinedClipboardFormat[8] = "CF_DIB";
		g_mpPredefinedClipboardFormat[9] = "CF_PALETTE";
		g_mpPredefinedClipboardFormat[10] = "CF_PENDATA";
		g_mpPredefinedClipboardFormat[11] = "CF_RIFF";
		g_mpPredefinedClipboardFormat[12] = "CF_WAVE";
		g_mpPredefinedClipboardFormat[13] = "CF_UNICODETEXT";
		g_mpPredefinedClipboardFormat[14] = "CF_ENHMETAFILE";
		g_mpPredefinedClipboardFormat[15] = "CF_HDROP";
		g_mpPredefinedClipboardFormat[16] = "CF_LOCALE";
		g_mpPredefinedClipboardFormat[17] = "CF_DIBV5";
		g_mpPredefinedClipboardFormat[18] = "CF_MAX TAG";
		g_mpPredefinedClipboardFormat[0x0080] = "CF_OWNERDISPLAY";
		g_mpPredefinedClipboardFormat[0x0081] = "CF_DSPTEXT";
		g_mpPredefinedClipboardFormat[0x0082] = "CF_DSPBITMAP";
		g_mpPredefinedClipboardFormat[0x0083] = "CF_DSPMETAFILEPICT";
		g_mpPredefinedClipboardFormat[0x008E] = "CF_DSPENHMETAFILE";
		g_mpPredefinedClipboardFormat[0x0200] = "CF_PRIVATEFIRST";
		g_mpPredefinedClipboardFormat[0x02FF] = "CF_PRIVATELAST";
		g_mpPredefinedClipboardFormat[0x0300] = "CF_GDIOBJFIRST";
		g_mpPredefinedClipboardFormat[0x03FF] = "CF_GDIOBJLAST";
	}


	bool CNativeRichEditUI::CanPaste()
	{
		bool bCanPaste = false;
		if(m_pSkinScrollFrame)
		{
			HWND hRichEdit = ((CSkinScrollFrame*)m_pSkinScrollFrame)->GetNativeRichEditCtrl();
			if(!hRichEdit)
				return false;

			//[wParam]
			//	Specifies the clipboard format to try. Set this parameter to zero to try any format currently on the clipboard.
			bCanPaste = !!::SendMessage(hRichEdit, EM_CANPASTE, 0, 0);
			/*
			if(::CountClipboardFormats() < 1)
				return false;

			if(!::OpenClipboard(hRichEdit))
				return false;
			
			//获取当前剪切板中的数据可用的格式;
			TIntToStrMap mpFormat;
			char szFormatName[80] = {0};
			UINT uFormat = 0;
			while(1)
			{
				uFormat = ::EnumClipboardFormats(uFormat);
				if(0 == uFormat)
					break;

				memset(szFormatName, 0, sizeof(szFormatName));
				if(::GetClipboardFormatNameA(uFormat, szFormatName, sizeof(szFormatName)))
					mpFormat[uFormat] = szFormatName;
				else
					mpFormat[uFormat] = "(Unknown)";
				
				//追加格式定义描述;
				if(g_mpPredefinedClipboardFormat.end() != g_mpPredefinedClipboardFormat.find(uFormat))
				{
					std::string sFormatDesc = mpFormat[uFormat];
					sFormatDesc += "-";
					sFormatDesc += g_mpPredefinedClipboardFormat[uFormat];
					mpFormat[uFormat] = sFormatDesc;
				}
			}

			//获取RichEdit可用的格式;
			for(TIntToStrMap::iterator it1 = mpFormat.begin(); it1 != mpFormat.end();)
			{
				if(!::SendMessage(hRichEdit, EM_CANPASTE, it1->first, 0))
					it1 = mpFormat.erase(it1);
				else
					++it1;
			}
			::CloseClipboard();
			*/
		}
		
		return bCanPaste;
	}

	void CNativeRichEditUI::Copy()
	{
		if(m_pSkinScrollFrame)
		{
			HWND hRichEdit = ((CSkinScrollFrame*)m_pSkinScrollFrame)->GetNativeRichEditCtrl();
			if(hRichEdit)
				::SendMessageA(hRichEdit, WM_COPY, 0, 0);
		}
	}

	void CNativeRichEditUI::Cut()
	{
		if(m_pSkinScrollFrame)
		{
			HWND hRichEdit = ((CSkinScrollFrame*)m_pSkinScrollFrame)->GetNativeRichEditCtrl();
			if(hRichEdit)
				::SendMessageA(hRichEdit, WM_CUT, 0, 0);
		}
	}

	void CNativeRichEditUI::Paste()
	{
		if(m_pSkinScrollFrame)
		{
			HWND hRichEdit = ((CSkinScrollFrame*)m_pSkinScrollFrame)->GetNativeRichEditCtrl();
			if(hRichEdit)
				::SendMessageA(hRichEdit, WM_PASTE, 0, 0);
		}
	}

	void CNativeRichEditUI::PasteSpecial(unsigned int iFormat)
	{
		if(iFormat < CF_TEXT)
			return;

		if(m_pSkinScrollFrame)
		{
			HWND hRichEdit = ((CSkinScrollFrame*)m_pSkinScrollFrame)->GetNativeRichEditCtrl();
			if(hRichEdit)
			{
				REPASTESPECIAL rePS;
				rePS.dwAspect = DVASPECT_CONTENT;//also support DVASPECT_ICON;
				rePS.dwParam = NULL;
				::SendMessage(hRichEdit, EM_PASTESPECIAL, iFormat, (LPARAM)&rePS);
			}
		}
	}
	
	void CNativeRichEditUI::SetAutoDetectUrl(bool bAuto)
	{
		if(m_pSkinScrollFrame)
		{
			HWND hRichEdit = ((CSkinScrollFrame*)m_pSkinScrollFrame)->GetNativeRichEditCtrl();
			if(hRichEdit)
			{
				unsigned int uMask = SendMessage(hRichEdit, EM_GETEVENTMASK, 0, 0);
				SendMessage(hRichEdit, EM_SETEVENTMASK, 0, uMask | ENM_LINK);
				SendMessage(hRichEdit, EM_AUTOURLDETECT, bAuto, 0);//AURL_ENABLEURL Win8以后才有;
			}
		}
	}

	void CNativeRichEditUI::InsertText(LPCTSTR sText)
	{
		if(!sText || _tcslen(sText)<1)
			return;

		if(m_pSkinScrollFrame)
		{
			HWND hRichEdit = ((CSkinScrollFrame*)m_pSkinScrollFrame)->GetNativeRichEditCtrl();
			if(hRichEdit)
				::SendMessage(hRichEdit, EM_REPLACESEL, TRUE, (LPARAM)sText);
		}
	}

	BOOL CNativeRichEditUI::InsertFileObject(LPCTSTR szFileName)
	{
		HWND hRichEdit = NULL;
		if(m_pSkinScrollFrame)
			hRichEdit = ((CSkinScrollFrame*)m_pSkinScrollFrame)->GetNativeRichEditCtrl();
		if(!hRichEdit)
			return FALSE;

		//Variables indicate process;
		HRESULT hr = S_FALSE;
		CComPtr<IRichEditOle> pRichEditOle = NULL;
		CComPtr<ILockBytes> lpLockBytes = NULL;
		CComPtr<IStorage> pStorage = NULL;
		CComPtr<IOleClientSite>	lpClientSite = NULL;
		CComPtr<IUnknown> pUnk = NULL;
		CComPtr<IOleObject> lpOleObject = NULL;

		//Get OLE Interface;		
		SendMessage(hRichEdit, EM_GETOLEINTERFACE, 0, (LPARAM)&pRichEditOle);
		if (pRichEditOle == NULL)
			return FALSE;

		// Initialize a Storage Object.
		hr = CreateILockBytesOnHGlobal(NULL, TRUE, &lpLockBytes);
		if (FAILED(hr))
			return FALSE;

		hr = StgCreateDocfileOnILockBytes(lpLockBytes, 
			STGM_SHARE_EXCLUSIVE | STGM_CREATE | STGM_READWRITE, //没有指定STGM_DELETEONRELEASE,RicheEdit能自动释放完全吗?jian.he questioned;
			0, &pStorage);
		if (FAILED(hr))
			return FALSE;

		FORMATETC formatEtc;
		formatEtc.cfFormat = 0;
		formatEtc.ptd = NULL;
		formatEtc.dwAspect = DVASPECT_CONTENT;
		formatEtc.lindex = -1;
		formatEtc.tymed = TYMED_NULL;

		hr = pRichEditOle->GetClientSite(&lpClientSite);
		if (FAILED(hr))
			return FALSE;

		CLSID clsid = CLSID_NULL;
		hr = OleCreateFromFile(clsid, szFileName, IID_IUnknown, OLERENDER_DRAW, 
			&formatEtc, lpClientSite, pStorage, (void**)&pUnk);
		if (FAILED(hr))
			return FALSE;

		hr = pUnk->QueryInterface(IID_IOleObject, (void**)&lpOleObject);
		if (FAILED(hr))
			return FALSE;

		OleSetContainedObject(lpOleObject, TRUE);
		REOBJECT reobject = { sizeof(REOBJECT)};
		hr = lpOleObject->GetUserClassID(&clsid);
		if (FAILED(hr))
			return FALSE;

		reobject.clsid = clsid;
		reobject.cp = REO_CP_SELECTION;
		reobject.dvaspect = DVASPECT_CONTENT;
		reobject.dwFlags = REO_RESIZABLE | REO_BELOWBASELINE;
		reobject.dwUser = 0;
		reobject.poleobj = lpOleObject;
		reobject.polesite = lpClientSite;
		reobject.pstg = pStorage;
		SIZEL sizel = { 0 };
		reobject.sizel = sizel;

		return (S_OK == pRichEditOle->InsertObject(&reobject));
	}

	BOOL CNativeRichEditUI::InsertBitmap(HBITMAP hBitmap, int iDesiredPixelX, int iDesiredPixelY)
	{
		HWND hRichEdit = NULL;
		if(m_pSkinScrollFrame)
			hRichEdit = ((CSkinScrollFrame*)m_pSkinScrollFrame)->GetNativeRichEditCtrl();
		if(!hRichEdit)
			return FALSE;

		//Variables that indicate the whole process;
		CComPtr<ILockBytes> lpLockBytes;
		CComPtr<IStorage> pStorage;
		CComPtr<IRichEditOle> pRichEditOle;
		CComPtr<IOleClientSite> lpClientSite;
		CComPtr<IOleObject> pOleObject;

		//Step 1: 为OLE Object分配内存;
		SCODE sc = ::CreateILockBytesOnHGlobal(NULL, TRUE, &lpLockBytes);
		if (sc != S_OK)
			return FALSE;

		sc = ::StgCreateDocfileOnILockBytes(lpLockBytes,
			STGM_SHARE_EXCLUSIVE|STGM_CREATE|STGM_READWRITE, 0, &pStorage);
		if (sc != S_OK)
			return FALSE;
		
		//Step 2: 获取RichEdit的OLEClientSite;
		SendMessage(hRichEdit, EM_GETOLEINTERFACE, 0, (LPARAM)&pRichEditOle);
		if(NULL == pRichEditOle)
			return FALSE;
		if(S_OK != pRichEditOle->GetClientSite(&lpClientSite))
			return FALSE;
		
		//Step 3: 创建OLE Object;
		STGMEDIUM stgm;
		stgm.tymed = TYMED_GDI;					// Storage medium = HBITMAP handle		
		stgm.hBitmap = hBitmap;
		stgm.pUnkForRelease = NULL;				// Use ReleaseStgMedium

		FORMATETC fm;
		fm.cfFormat = CF_BITMAP;				// Clipboard format = CF_BITMAP
		fm.ptd = NULL;							// Target Device = Screen
		fm.dwAspect = DVASPECT_CONTENT;			// Level of detail = Full content
		fm.lindex = -1;							// Index = Not applicaple
		fm.tymed = TYMED_GDI;					// Storage medium = HBITMAP handle

#ifdef _CREATEOLEOBJ_WITHSTATICDATA_MFC_WAY
		COleDataSource* pDataSource = new COleDataSource;
		pDataSource->CacheData(CF_BITMAP, &stgm);
		LPDATAOBJECT lpDataObject = (LPDATAOBJECT)pDataSource->GetInterface(&IID_IDataObject);
		sc = OleCreateStaticFromData(lpDataObject,IID_IOleObject,OLERENDER_FORMAT,
			&fm,lpClientSite,pStorage,(void **)&pOleObject);
		pDataSource->ExternalRelease();
		if(sc!=S_OK)
			return FALSE;
#else
		CImageDataObject* pDataObj = new CImageDataObject;
		pDataObj->SetData(&fm, &stgm, FALSE);
		pOleObject = pDataObj->GetOleObject(lpClientSite, pStorage);
		pDataObj->Release();
#endif
		//Step 4: Insert OLE Object;
		REOBJECT reobject;
		ZeroMemory(&reobject, sizeof(REOBJECT));
		reobject.cbStruct = sizeof(REOBJECT);

		CLSID clsid;
		sc = pOleObject->GetUserClassID(&clsid);
		if (sc != S_OK)
			return FALSE;

		reobject.clsid = clsid;
		reobject.cp = REO_CP_SELECTION;
		reobject.dwFlags = REO_RESIZABLE |REO_BELOWBASELINE;
		reobject.dwUser = 0;
		reobject.dvaspect = DVASPECT_CONTENT;
		reobject.poleobj = pOleObject;
		reobject.polesite = lpClientSite;
		reobject.pstg = pStorage;
		//The size of the object. The unit of measure is 0.01 millimeters, which is a HIMETRIC measurement.
		//Dots Per Millimeter;
		float fReoSizeW = 0.0f;
		float fReoSizeH = 0.0f;
		if((iDesiredPixelX > 0) && (iDesiredPixelY > 0))
		{
			HDC hDC = ::GetDC(hRichEdit);
			float fDevW = GetDeviceCaps(hDC, HORZSIZE);
			float fResW = GetDeviceCaps(hDC, HORZRES);
			float fDevH = GetDeviceCaps(hDC, VERTSIZE);
			float fResH = GetDeviceCaps(hDC, VERTRES);
			//const float kPhyMilliReoSizeRatio = 127.0f/9522.0f;
			float fDestResW = iDesiredPixelX;
			float fDestResH = iDesiredPixelY;
			fReoSizeW = fDevW / fResW * fDestResW / (127.0f/9522.0f);
			fReoSizeH = fDevH / fResH * fDestResH / (127.0f/9522.0f);
		}

		SIZEL sizel = { fReoSizeW, fReoSizeH};
		reobject.sizel = sizel;


		//jian.he test begin
		long count = pRichEditOle->GetObjectCount();
		for(int i = 0; i < count; ++i)
		{
			REOBJECT reobj = { 0 };
			reobj.cbStruct = sizeof(REOBJECT);
			pRichEditOle->GetObject(i, &reobj, REO_GETOBJ_POLEOBJ);
			
// 			GifSmiley::IGifSmileyCtrl* lpAnimator = 0;
// 			HRESULT hr = reobj.poleobj->QueryInterface(GifSmiley::IID_IGifSmileyCtrl, (void**)&lpAnimator);
// 			if (SUCCEEDED(hr) && lpAnimator)
// 			{
// 				BSTR*  fileName = nullptr;
// 				hr = lpAnimator->FileName(fileName);
// 			}

			if(reobj.poleobj)
				reobj.poleobj->Release();
		}
		//jian.he test end

		return (S_OK == pRichEditOle->InsertObject(&reobject));
	}

	BOOL CNativeRichEditUI::InsertImage(LPCTSTR szImagePath, int iDesiredPixelBox)
	{
		//::LoadImage() only support .bmp;
		//HBITMAP hBitmap = (HBITMAP)LoadImage(NULL, szImagePath, IMAGE_BITMAP, 100, 100, LR_LOADFROMFILE);//倒数2/3参数可以控制加载出来的图片的尺寸;
		
		//加载图片的几种方法优缺点对比:
		//Way 1: ::LoadImage() //释放::DeleteObject(hBitmap);
		//		优点: Windows API, 简单快速; 支持创建时设定期望尺寸;
		//		缺点: 只支持.bmp, 不支持常见的.jpg/.png; 不支持跨平台;
		//Way 2: DuiLib::GetImageEx()
		//		优点: 支持跨平台; 与duilib无缝结合;
		//		缺点: 不支持创建时设定期望尺寸;
		//Way 3: Gdiplus::Image
		//		优点: 功能强大;
		//		缺点: 修改尺寸也需要在两个Bitmap对象之间转换;
		//Way 4: IPicture接口
		//后来发现: OLE OBJECT的尺寸可以直接在REOBJECT.sizel成员设定! 只是需要注意单位!

		//Here, we take advantage of duilib;
		if(!m_pManager)
			return FALSE;
		const DuiLib::TImageInfo* pImgInfo = m_pManager->GetImageEx(szImagePath);
		if(!pImgInfo)
			return FALSE;

		//保持宽高比缩放;
		SIZE szLimit = {iDesiredPixelBox,iDesiredPixelBox};
		SIZE szOut = {0,0};
		SIZE szImg = {pImgInfo->nX, pImgInfo->nY};
		CalcScaledSize(szImg, szLimit, szOut);
		BOOL bSuc = InsertBitmap(pImgInfo->hBitmap, szOut.cx, szOut.cy);
		
		//Free;
		m_pManager->RemoveImage(szImagePath);

		return bSuc;
	}

	BOOL CNativeRichEditUI::CalcScaledSize(SIZE szIn, SIZE szLimit, SIZE& szOut)
	{
		if((szLimit.cx < 1) || (szLimit.cy < 1) || (szIn.cy < 1) || (szIn.cx < 1))
		{
			szOut.cx = 0;
			szOut.cy = 0;			
		}
		else
		{
			float fRatioLimit = szLimit.cx * 1.0f / szLimit.cy;
			float fRatioOri = szIn.cx * 1.0f / szIn.cy;
				
			if(fRatioOri >= fRatioLimit) //宽度铺满/原大小;
			{
				szOut.cx = (szIn.cx > szLimit.cx) ? szLimit.cx : szIn.cx;//取小值;
				szOut.cy = (int)(szOut.cx / fRatioOri);					
			}
			else //高度铺满/原大小;
			{
				szOut.cy = (szIn.cy > szLimit.cy) ? szLimit.cy : szIn.cy;//取小值;
				szOut.cx = (int)(szOut.cy * fRatioOri);
			}
		}
		return TRUE;
	}

	int CNativeRichEditUI::GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
	{
		/*
		image/bmp
		image/jpeg
		image/gif
		image/tiff
		image/png
		*/

		UINT  num = 0;          // number of image encoders
		UINT  size = 0;         // size of the image encoder array in bytes

		Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;
		Gdiplus::GetImageEncodersSize(&num, &size);
		if(size == 0)
			return -1;  // Failure

		pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
		if(pImageCodecInfo == NULL)
			return -1;  // Failure

		GetImageEncoders(num, size, pImageCodecInfo);

		for(UINT j = 0; j < num; ++j)
		{
			if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
			{
				*pClsid = pImageCodecInfo[j].Clsid;
				free(pImageCodecInfo);
				return j;  // Success
			}    
		}

		free(pImageCodecInfo);
		return -1;  // Failure
	}
	
#ifdef _DEBUG
	void CNativeRichEditUI::Test_FreeOleCallbackObject()
	{
		std::vector<IExRichEditOleCallback*> arOleCb;
		for(int i = 0; i < 1000; ++i)
		{
			IExRichEditOleCallback* pOleCb = new IExRichEditOleCallback;
			arOleCb.push_back(pOleCb);
		}

		::Sleep(1000);

		for(auto it1 = arOleCb.begin(); it1 != arOleCb.end();)
		{
			delete *it1;
			it1 = arOleCb.erase(it1);
		}
	}

	bool Utf8ToUnicode(std::string& aStr, std::wstring& wStr)
	{
		if (aStr.size() < 1)
			return false;

		int iLen = ::MultiByteToWideChar(CP_ACP, 0, aStr.c_str(), -1, NULL, 0);
		iLen += 1; //wcslen(szT) + 1;会出现末尾的乱码! 如果是UTF8可能3个字节表示一个字符呢!
		WCHAR* szW = new WCHAR[iLen];
		wmemset(szW, 0, iLen);
		::MultiByteToWideChar(CP_ACP, 0, aStr.c_str(), -1, szW, iLen);
		wStr = szW;
		delete szW;

		return true;
	}

	BOOL CNativeRichEditUI::Test_CopyAsRTF()
	{
		if(m_uRtfCF < 1)
			return FALSE;

		HWND hRichEdit = NULL;
		if(m_pSkinScrollFrame)
			hRichEdit = ((CSkinScrollFrame*)m_pSkinScrollFrame)->GetNativeRichEditCtrl();
		if(!hRichEdit)
			return FALSE;
		
		//prepare rtf string;
		std::ifstream in("1.rtf.txt");
		std::ostringstream tmp;
		tmp << in.rdbuf();
		std::string sRtf = tmp.str();

		//CF_RTF字符串使用UNICODE格式? 事实是不可以,必须是ANSI格式, 读取的字符串才能正确设置到Clipboard中去;
		
		//copy to clipboard;
		BOOL bRet = FALSE;
		if (OpenClipboard(hRichEdit)) 
		{
			//如果已经有字符串在剪切板中, 那么复制的数据不会覆盖原数据, 这个时候粘贴到RichEdit中,不能识别, 所以干脆先清空;
			::EmptyClipboard();

			// Allocate a global memory object for the text. 
			HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, sRtf.size()+100); 
			if(hglbCopy)
			{
				// Lock the handle and copy the text to the buffer.
				char* lptstrCopy = (char*)GlobalLock(hglbCopy);
				memcpy(lptstrCopy, sRtf.c_str(), sRtf.size()); 
				lptstrCopy[sRtf.size()] = (TCHAR) 0;    // null character 
						
				GlobalUnlock(hglbCopy);
				if(NULL == SetClipboardData(m_uRtfCF, hglbCopy))
				{
					DWORD dwErr = ::GetLastError();
					dwErr = dwErr + 1 - 1;
				}
				
				//GlobalFree(hglbCopy);
				//hglbCopy = NULL;
				CloseClipboard();
				bRet = TRUE;
			}
		}
		return bRet;
	}

	BOOL CNativeRichEditUI::Test_ConvertSelectionToPic()
	{
		HWND hRichEdit = NULL;
		if(m_pSkinScrollFrame)
			hRichEdit = ((CSkinScrollFrame*)m_pSkinScrollFrame)->GetNativeRichEditCtrl();
		if(!hRichEdit)
			return FALSE;

// 		CRichEditCtrl ctrlRE;
// 		ctrlRE.DoD2DPaint()

		//prepare..
		float fDevW = GetDeviceCaps(::GetDC(hRichEdit), HORZSIZE);//millimeter; 1 inch = 25.4mm;1 inch = 1440 twip;
		float fResW = GetDeviceCaps(::GetDC(hRichEdit), HORZRES);//pixel;
#define PIXEL_PER_INCH (fResW/fDevW*25.4f)
		RECT rcRE;
		::GetClientRect(hRichEdit, &rcRE);
		float fClientW = rcRE.right - rcRE.left;
		float fClientH = rcRE.bottom - rcRE.top;
		RECT rcFmtRange;
		rcFmtRange.left = rcFmtRange.top = 0;
		rcFmtRange.right = fClientW/PIXEL_PER_INCH*1440;
		rcFmtRange.bottom = fClientH/PIXEL_PER_INCH*1440;
		
		HDC hdcMem = ::CreateCompatibleDC(NULL);
		HBITMAP hBmpSnap = ::CreateCompatibleBitmap(hdcMem, fClientW, fClientH);
		::SelectObject(hdcMem, hBmpSnap);

		FORMATRANGE fmtRange;
		fmtRange.hdc = ::GetDC(hRichEdit);
		fmtRange.hdcTarget = hdcMem;
		fmtRange.rc = rcFmtRange;
		fmtRange.rcPage = fmtRange.rc;
		fmtRange.chrg.cpMin = 0;

		GETTEXTLENGTHEX lenInfoRE;
		lenInfoRE.flags = GTL_DEFAULT;//GTL_NUMCHARS;
		lenInfoRE.codepage = 1200;//CP_ACP for ANSI Code Page and 1200 for Unicode;
		int iCharsCount = ::SendMessage(hRichEdit, EM_GETTEXTLENGTHEX, (WPARAM)&lenInfoRE, 0);
		if(E_INVALIDARG != iCharsCount)
		{
			fmtRange.chrg.cpMax = iCharsCount;
			::SendMessage(hRichEdit, EM_FORMATRANGE, 1, (LPARAM)&fmtRange);
			::SendMessage(hRichEdit, EM_FORMATRANGE, 0, 0);//free cache;

			//save to disk file to see the result;
			Gdiplus::Bitmap bmpSnap(hBmpSnap, NULL);
			CLSID bmpClsid;
			GetEncoderClsid(L"image/bmp", &bmpClsid);
			bmpSnap.Save(L"RESnap.bmp", &bmpClsid, NULL);
		}

		if(hBmpSnap)
			::DeleteObject(hBmpSnap);
		if(hdcMem)
			::DeleteDC(hdcMem);

		return TRUE;
	}
#endif


}//namespace DuiLib END.
