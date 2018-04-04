#include "StdAfx.h"
#include "NativeWnd.h"

namespace DuiLib
{

#ifndef HINST_THISCOMPONENT
	EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif

	const TCHAR* CNativeWnd::m_kClassName = _T("CNativeWnd");
	//Specify WS_VISIBLE/WS_CHILD/WS_POPUP .. window styles OUTSIDE!
	const DWORD CNativeWnd::m_kSimplestStyle = (WS_CLIPCHILDREN | WS_CLIPSIBLINGS) & ~WS_CAPTION & ~WS_BORDER &~WS_SYSMENU &~WS_SIZEBOX;

	CNativeWnd::CNativeWnd(void):m_hWnd(NULL)
	{
	}


	CNativeWnd::~CNativeWnd(void)
	{
	}

	HWND CNativeWnd::CreateWnd(DWORD dwStyle, RECT rc, HWND hParentOrOwner)
	{
		//Register the window class;
		WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
		wcex.style      =  NULL;
		wcex.lpfnWndProc = CNativeWnd::WndProc;
		wcex.cbClsExtra    = 0;
		wcex.cbWndExtra    = sizeof(LONG_PTR);
		wcex.hInstance     = HINST_THISCOMPONENT;
		wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		wcex.lpszMenuName  = NULL;
		wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
		wcex.lpszClassName = m_kClassName;

		RegisterClassEx(&wcex);

		m_hWnd = CreateWindowEx(NULL, m_kClassName, _T(""), dwStyle, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hParentOrOwner, NULL, HINST_THISCOMPONENT, this);	

		return m_hWnd;
	}

	LRESULT CALLBACK CNativeWnd::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		LRESULT result = 0;//返回非零值表示不需要系统再进行处理;An application should return nonzero if it erases the background; otherwise, it should return zero.

		if(!message)//exception handler;
			return 1;

		if (WM_CREATE == message)
		{
			LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
			CNativeWnd *pAppInst = (CNativeWnd *)pcs->lpCreateParams;
			::SetWindowLongPtrW(hwnd, GWLP_USERDATA, PtrToUlong(pAppInst));

			pAppInst->m_hWnd = hwnd;
			result = 1;
		}
		else
		{
			CNativeWnd *pAppInst = reinterpret_cast<CNativeWnd *>(static_cast<LONG_PTR>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA)));
			BOOL wasHandled = FALSE;
			if (pAppInst)
			{
				switch (message)
				{
// 				case WM_ERASEBKGND:
// 					result = 1;
// 					wasHandled = true;
// 					break;
// 
// 				case WM_PAINT:
// 				case WM_DISPLAYCHANGE:
// 					{
// 						PAINTSTRUCT ps;
// 						BeginPaint(hwnd, &ps);
// 						//pAppInst->OnRender();
// 						EndPaint(hwnd, &ps);					
// 					}
// 					result = 1;
// 					wasHandled = true;
// 					break;
// 
// 				case WM_DESTROY:
// 					{
// 						PostQuitMessage(0);
// 						result = 1;
// 						wasHandled = true;
// 						break;
// 					}

				default:
					result = pAppInst->HandleMessage(message, wParam, lParam, wasHandled);
					break;
				}
			}

			if (!wasHandled)
				result = DefWindowProc(hwnd, message, wParam, lParam);
		}

		return result;
	}

	LRESULT CNativeWnd::HandleMessage(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return bHandled ? 1 : 0;
	}
}

