#pragma once

namespace gl {
class IBuffer2D;
}

#include "FastDelegate.h"

class CDispView
	:
	public CWindowImpl<CDispView>,
	public CMessageFilter
{
public:
	DECLARE_WND_CLASS_EX(NULL, CS_NOCLOSE|CS_DBLCLKS|CS_OWNDC, -1)
	
	CDispView();
	BOOL PreTranslateMessage(MSG* pMsg);
	
	BEGIN_MSG_MAP(CGraphView)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_ERASEBKGND(OnEraseBkgnd)
		MSG_WM_SIZE(OnSize)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
	END_MSG_MAP()
	
// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
	
	LRESULT OnCreate(LPCREATESTRUCT lpCreateStruct);
	LRESULT OnDestroy(void);
	LRESULT OnEraseBkgnd(HDC hdc);
	LRESULT OnSize(UINT state, CSize Size);
	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnRequestDraw(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/);
	LRESULT OnDisplayChange(UINT bitsPerPixel, CSize Size);
	
	void Render();
private:
	
	HBITMAP m_hBMP;
	BITMAPINFO m_bmi;
	void* m_pBits;
	CDC m_memDC;
	
	void setupDIB();
};

