#include "stdafx.h"
#include "common.h"

bool ReadFileText(LPCTSTR filePath, std::string& text)
{
	FILE* file = _tfopen(filePath, _T("rb"));
	if (!file)
		return false;
	fseek(file, 0, SEEK_END);
	int length = ftell(file);
	fseek(file, 0, SEEK_SET);
	text = std::string(length, ' ');
	fread(&text[0], 1, length, file);
	fclose(file);
	return true;
}

std::string ToMultiByte(const wchar_t* val)
{
	int length( ::WideCharToMultiByte( CP_ACP, 0, val, -1, NULL, 0, NULL, NULL ) );
	std::string result( length - 1, 0 );
	::WideCharToMultiByte( CP_ACP, 0, &val[ 0 ], -1, &result[ 0 ], length, NULL, NULL );
	return result;
}

std::wstring ToWideChar(const char* val)
{
	int length( ::MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, &val[ 0 ], -1, NULL, 0 ) );
	std::wstring result( length - 1, 0 );
	::MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, &val[ 0 ], -1, &result[ 0 ], length );
	return result;
}

std::basic_string<TCHAR> ToT(const char* str)
{
#ifdef UNICODE
	return ToWideChar(str);
#else
	return str;
#endif
}

std::basic_string<TCHAR> ToT(const std::string& str)
{
	return ToT(str.c_str());
}

std::basic_string<TCHAR> ToT(const wchar_t* str)
{
#ifdef UNICODE
	return str;
#else
	return ToMultiByte(str);
#endif
}

std::basic_string<TCHAR> ToT(const std::wstring& str)
{
	return ToT(str.c_str());
}

bool IsFileExist(const TCHAR* path)
{
	WIN32_FIND_DATA findData;
	HANDLE hResult = ::FindFirstFile(path, &findData);
	bool ret = hResult != INVALID_HANDLE_VALUE;
	::FindClose(hResult);
	return ret;
}

size_t GetFileSize(FILE* file)
{
	fseek(file, 0, SEEK_END);
	int length = ftell(file);
	fseek(file, 0, SEEK_SET);
	return length;
}
