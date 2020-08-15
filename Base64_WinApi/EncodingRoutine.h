#pragma once

#include <string>
#include <Windows.h>

// Convert wstring to UTF-8 string
std::string utf8_encode(const std::wstring& wstr) {
	if (wstr.empty()) return std::string();

	const int size_needed =
		WideCharToMultiByte(CP_UTF8,
							0,
							wstr.data(),
							(int)wstr.size(),
							NULL,
							0,
							NULL,
							NULL);

	//const int wstrLength = static_cast<int>(wstr.size());

	std::string str_enc(size_needed, ' ');
	WideCharToMultiByte(CP_UTF8,
						0,
						wstr.data(),
						(int)wstr.size(),
						&str_enc[0],
						size_needed,
						NULL,
						NULL);
	
	return str_enc;
}

// Convert UTF-8 string to wstring
std::wstring utf8_decode(const std::string& str) {
	if (str.empty()) return std::wstring();

	const int size_needed =
		MultiByteToWideChar(CP_UTF8,
							0,
							str.data(),
							(int)str.size(),
							NULL,
							0);

	std::wstring wstr_dec(size_needed, ' ');
	MultiByteToWideChar(CP_UTF8,
						0,
						str.data(),
						(int)str.size(),
						&wstr_dec[0],
						size_needed);

	return wstr_dec;
}