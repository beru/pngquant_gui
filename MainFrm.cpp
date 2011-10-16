// MainFrm.cpp : implmentation of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "aboutdlg.h"
#include "MainFrm.h"

#include "DispView.h"
#include "ParameterDialog.h"

#include "ImageIO/MemoryFile.h"
#include "ImageIO/ImageIO_png.h"

#include <vector>

#include "ExecProcess.h"

ImageInfo g_info;
std::vector<uint8_t> g_inData;
std::vector<uint8_t> g_inImage;
std::vector<uint8_t> g_outImage;
COLORREF g_color;

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	if (CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
		return TRUE;
	
	return m_pView->PreTranslateMessage(pMsg);
}

BOOL CMainFrame::OnIdle()
{
	UIUpdateToolBar();
	return FALSE;
}

LRESULT CMainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	m_pView = new CDispView();
	m_pParameterDialog = new CParameterDialog();
	m_pParameterDialog->Create(m_hWnd, rcDefault);
	m_pParameterDialog->m_onParameterChangeDelegate.bind(this, &CMainFrame::onParameterChange);
	
	// create command bar window
	HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
	// attach menu
	m_CmdBar.AttachMenu(GetMenu());
	// load command bar images
	m_CmdBar.LoadImages(IDR_MAINFRAME);
	// remove old menu
	SetMenu(NULL);

	HWND hWndToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_MAINFRAME, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);
	
	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	AddSimpleReBarBand(hWndCmdBar);
	AddSimpleReBarBand(hWndToolBar, NULL, TRUE);

	CreateSimpleStatusBar();

	m_hWndClient = m_pView->Create(m_hWnd);

	UIAddToolBar(hWndToolBar);
	UISetCheck(ID_VIEW_TOOLBAR, 1);
	UISetCheck(ID_VIEW_STATUS_BAR, 1);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	return 0;
}

LRESULT CMainFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	// unregister message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);

	bHandled = FALSE;
	return 1;
}

LRESULT CMainFrame::OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	PostMessage(WM_CLOSE);
	return 0;
}

LRESULT CMainFrame::OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: add code to initialize document

	return 0;
}

LRESULT CMainFrame::OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	static BOOL bVisible = TRUE;	// initially visible
	bVisible = !bVisible;
	CReBarCtrl rebar = m_hWndToolBar;
	int nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST + 1);	// toolbar is 2nd added band
	rebar.ShowBand(nBandIndex, bVisible);
	UISetCheck(ID_VIEW_TOOLBAR, bVisible);
	UpdateLayout();
	return 0;
}

LRESULT CMainFrame::OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	BOOL bVisible = !::IsWindowVisible(m_hWndStatusBar);
	::ShowWindow(m_hWndStatusBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
	UISetCheck(ID_VIEW_STATUS_BAR, bVisible);
	UpdateLayout();
	return 0;
}

LRESULT CMainFrame::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CAboutDlg dlg;
	dlg.DoModal();
	return 0;
}

LRESULT CMainFrame::OnFileOpen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CFileDialog dlg(TRUE, _T("png"), 0, 0, _T("PNGファイル(*.png)\0*.png\0すべてのファイル(*.*)\0*.*\0\0"));
	if (dlg.DoModal() == IDOK) {
		FILE* f = _tfopen(dlg.m_szFileName, _T("rb"));
		if (f) {
			size_t sz = GetFileSize(f);
			g_inData.resize(sz);
			fread(&g_inData[0], 1, sz, f);
			fclose(f);
			
			uint8_t* inp;
			MemoryFile f(&g_inData[0], g_inData.size());
			if (ReadImageInfo_png(f, g_info)) {
				g_inImage.resize(g_info.width * g_info.height * 4);
				g_outImage.resize(g_info.width * g_info.height * 4);
				inp = &g_inImage[0];
				ReadImage_png(f, g_info, inp, g_info.width*4, 0);
				Exec_pngquant(m_pParameterDialog->GetParameter());
				m_pView->Render();
			}
		}
	}

	return 0;
}

void CMainFrame::Exec_pngquant(Parameter p)
{
	TCHAR options[32] = {0};
	if (p.nofs) {
		_tcscat(options, _T(" -nofs"));
	}
	if (p.iebug) {
		_tcscat(options, _T(" -iebug"));
	}
	TCHAR cmdLine[64];
	_stprintf(cmdLine, _T("pngquant.exe %s -speed %d %d"), options, p.speed, p.color);
	uint8_t* outp = &g_outImage[0];
	std::vector<uint8_t> outData;
	if (ExecProcess(cmdLine, g_inData, outData)) {
		if (outData.size()) {
			MemoryFile f(&outData[0], outData.size());
			if (ReadImageInfo_png(f, g_info)) {
				if (ReadImage_png(f, g_info, outp, g_info.width*4, 0)) {
					int hoge = 1;
				}
			}
		}
	}
}

void CMainFrame::onParameterChange(Parameter p)
{
	if (g_outImage.size() == 0) {
		return;
	}
	Exec_pngquant(p);
	m_pView->Render();
}

LRESULT CMainFrame::OnViewBackgroundcolor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CColorDialog dlg(g_color);
	if (dlg.DoModal() == IDOK) {
		g_color = dlg.GetColor();
		m_pView->Render();
	}

	return 0;
}
