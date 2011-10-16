#pragma once

#include <string>

bool SetClipboardText(const wchar_t* str, size_t len);
bool GetClipboardText(std::wstring& str);

HBITMAP CreateDIB32(int width, int height, BITMAPINFO& bmi, void*& pBits);

CString FormatMessage(LONG errorCode);

int GetCheckedRadioButton(HWND hWnd, int nIDFirstButton, int nIDLastButton);

CString GetModuleFileFolderPath();

int GetMenuItemPos(HMENU hMenu, UINT id);

void SetEnableMenuItems(HMENU hMenu, bool bEnable);

template <size_t NBufferMax>
const TCHAR* ResourceLoadString(UINT uID, TCHAR (&buff)[NBufferMax])
{
	static const TCHAR* EMPTY_STR = _T("");
	static HINSTANCE s_hInstance;
	if (!s_hInstance) {
		s_hInstance = ::GetModuleHandle(NULL);
	}
	int r = ::LoadString(s_hInstance, uID, buff, NBufferMax);
	return r ? buff : EMPTY_STR;
}

bool ComboBox_SelectItemByString(HWND hWnd, LPCTSTR value);
bool ComboBox_SelectItemByData(HWND hWnd, LPARAM data);
bool ComboBox_GetSelectedItemString(HWND hWnd, CString& value);
LRESULT ComboBox_GetSelectedItemData(HWND hWnd);

