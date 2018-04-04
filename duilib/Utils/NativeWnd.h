#ifndef __NATIVEWND_H__
#define __NATIVEWND_H__


#pragma once

#define WIN32_LEAN_AND_MEAN     // Exclude rarely-used stuff from Windows headers
#include <windows.h>			// Windows Header Files
#include <tchar.h>


/*
	@Purpose:	创建一个简单的Windows原生的窗口--不需要指定.xml文件(class WindowImplBase MUST).
	@Author:	jian.he;
	@Mail:		worksdata@163.com;
	@Date:		2018/03/01;
	@Company:	AnyMacro.com;
*/

namespace DuiLib
{
	class UILIB_API CNativeWnd
	{
	public:
		CNativeWnd(void);
		virtual ~CNativeWnd(void);
		virtual LRESULT HandleMessage(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

		HWND CreateWnd(DWORD dwStyle, RECT rc, HWND hParentOrOwner);
		static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


		const static DWORD m_kSimplestStyle;
		const static TCHAR* m_kClassName;

		HWND m_hWnd;
	};
}

#endif


