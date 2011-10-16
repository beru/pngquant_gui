#include "StdAfx.h"
#include "DispView.h"

#include "winutil.h"
#include "ImageIO/ImageIO_png.h"
#include <vector>

#include "gl/buffer2d.h"
#include "gl/colorConverter.h"
#include "gl/bitBlockTransfer.h"

extern ImageInfo g_info;
extern std::vector<uint8_t> g_inImage;
extern std::vector<uint8_t> g_outImage;
extern COLORREF g_color;

CDispView::CDispView()
{
}

BOOL CDispView::PreTranslateMessage(MSG* pMsg)
{
	return FALSE;
}

LRESULT CDispView::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CPaintDC dc(m_hWnd);
	CRect rect = dc.m_ps.rcPaint;
	
	dc.BitBlt(
		rect.left,
		rect.top,
		rect.Width(),
		rect.Height(),
		m_memDC,
		rect.left,
		rect.top,
		SRCCOPY
	);
	return 0;
}

void CDispView::setupDIB()
{
	CWindow wnd(GetDesktopWindow());
	HDC hDC = wnd.GetDC();
	int width = GetDeviceCaps(hDC, HORZRES) + 10;
	int height = GetDeviceCaps(hDC, VERTRES) + 10;
	wnd.ReleaseDC(hDC);
	
	// TopDownŒ`Ž®
	if (m_hBMP) {
		DeleteObject(m_hBMP);
		m_hBMP = 0;
	}
	m_hBMP = CreateDIB32(width, -height, m_bmi, m_pBits);
	
	if (m_memDC.m_hDC) {
		m_memDC.DeleteDC();
	}
	HDC dc = GetDC();
	m_memDC.CreateCompatibleDC(dc);
	m_memDC.SetMapMode(GetMapMode(dc));
	ReleaseDC(dc);
	m_memDC.SelectBitmap(m_hBMP);

}

LRESULT CDispView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	m_hBMP = 0;
	setupDIB();
	
	
	return 0;
}

LRESULT CDispView::OnDestroy(void)
{
	if (m_hBMP) {
		DeleteObject(m_hBMP);
		m_hBMP = 0;
	}
	m_memDC.DeleteDC();
	
	//You should call SetMsgHandled(FALSE) or set bHandled = FALSE for the main window of your application
	SetMsgHandled(FALSE);
	return 0;
}

LRESULT CDispView::OnEraseBkgnd(HDC hdc)
{
	SetMsgHandled(TRUE);
	return TRUE; // background is erased
}

LRESULT CDispView::OnSize(UINT state, CSize Size)
{
	if (IsWindow() && Size.cx != 0 && Size.cy != 0 && m_hBMP) {

	}
	return 0;
}

// http://www.virtualdub.org/blog/pivot/entry.php?id=117
__forceinline
void blendColor_Phaeron(uint32_t fc, uint8_t fca, uint32_t& bc)
{
	uint32_t bc2 = bc;
	uint32_t srb = fc & 0xff00ff;
	uint32_t sg = fc & 0x00ff00;
	uint32_t drb = bc2 & 0xff00ff;
	uint32_t dg = bc2 & 0x00ff00;
#if 1
	if ((srb | sg) == (drb | dg)) {
		return;
	}
#endif
	
	uint32_t alpha = fca;
	alpha += (alpha > 0);
	uint32_t orb = (drb + (((srb - drb) * alpha + 0x800080) >> 8)) & 0xff00ff;
	uint32_t og = (dg + (((sg - dg ) * alpha + 0x008000) >> 8)) & 0x00ff00;
	
	bc = orb+og;
}

typedef gl::ColorBGRA<uint8_t> Color;

void bitBlockTransfer(const gl::Buffer2D<Color>& src, gl::Buffer2D<Color>& dst, size_t x, size_t y)
{
	size_t w = src.GetWidth();
	size_t h = src.GetHeight();
	size_t srcLineOffset = src.GetLineOffset();
	const Color* pSrc = src.GetPixelPtr(0, 0);

	size_t dstLineOffset = dst.GetLineOffset();
	Color* pDst = dst.GetPixelPtr(x, y);
	for (size_t y=0; y<h; ++y) {
		for (size_t x=0; x<w; ++x) {
			Color c = pSrc[x];
			blendColor_Phaeron(*(uint32_t*)&c, c.a, *(uint32_t*)(pDst+x));
		}
		OffsetPtr(pSrc, srcLineOffset);
		OffsetPtr(pDst, dstLineOffset);
	}
}

void CDispView::Render()
{
	const BITMAPINFOHEADER& hdr = m_bmi.bmiHeader;
	gl::Buffer2D<Color> dst(hdr.biWidth, abs(hdr.biHeight), hdr.biWidth*4, m_pBits);
	
	Color bg;
	bg.r = GetRValue(g_color);
	bg.g = GetGValue(g_color);
	bg.b = GetBValue(g_color);
	bg.a = 0xff;
	gl::Buffer2D_Fill(dst, bg);

	if (g_inImage.size()) {
		gl::Buffer2D<Color> src(g_info.width, g_info.height, g_info.width*4, &g_inImage[0]);
		bitBlockTransfer(src, dst, 20, 20);
	
		if (g_outImage.size()) {
			gl::Buffer2D<Color> src2(g_info.width, g_info.height, g_info.width*4, &g_outImage[0]);
			bitBlockTransfer(src2, dst, 20+g_info.width+20, 20);
		}
	}
	
	Invalidate();
	UpdateWindow();
}
