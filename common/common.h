#pragma once

#include <string>

#include <windows.h>

bool ReadFileText(LPCTSTR filePath, std::string& text);

inline std::string ToMultiByte(const char* val) { return val; }
std::string ToMultiByte(const wchar_t* val);
inline std::string ToMultiByte(const std::string& val) { return val; }
inline std::string ToMultiByte(const std::wstring& val) { return ToMultiByte(val.c_str()); }

std::wstring ToWideChar(const char* val);
inline std::wstring ToWideChar(const wchar_t* val) { return val; }
inline std::wstring ToWideChar(const std::string& val) { return ToWideChar(val.c_str()); }
inline std::wstring ToWideChar(const std::wstring& val) { return val; }
std::basic_string<TCHAR> ToT(const char* str);
std::basic_string<TCHAR> ToT(const wchar_t* str);

std::basic_string<TCHAR> ToT(const std::string& str);
std::basic_string<TCHAR> ToT(const std::wstring& str);

bool IsFileExist(const TCHAR* path);

size_t GetFileSize(FILE* f);

struct StrProxy
{
	StrProxy(const char* s)
		:
		s_(s)
	{
	}
	
	const char* s_;
};

inline bool operator == (const StrProxy& lhs, const StrProxy& rhs)
{
	return strcmp(lhs.s_, rhs.s_) == 0;
}
