#include "pch.h"
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Storage.Streams.h>
#include <fstream>
#include <string>
#include <future>
#include "MyUtil.h"

static_assert(sizeof(wchar_t) == 2, "this function is windows only");

bool MyUtil::Init(const char* locale)
{
	setlocale(LC_CTYPE, locale);
	return true;
}

std::wstring MyUtil::SjisToWstring(const char* sjis)
{
	std::wstring result;
	wchar_t unicode_char[2] = {};  // Unicode文字は2バイト（ワイド文字）必要
	char sjis_char[4] = {};
	char* s = (char*)sjis;
	while (*s)
	{
		if (IsDBCSLeadByte(*s))
		{
			sjis_char[0] = *s++;
			sjis_char[1] = *s++;
		}
		else
		{
			sjis_char[0] = *s++;
			sjis_char[1] = 0;
		}

		if (MultiByteToWideChar(CP_ACP, 0, sjis_char, -1, unicode_char, 2) > 0)
		{
			result += unicode_char;
		}
	}

	return result;
}

std::wstring MyUtil::Utf8ToWstring(const char* utf8)
{
	std::wstring result;
	std::wstring err;
	auto GetU8ByteCount = [](char ch)
		{
			if (0 <= uint8_t(ch) && uint8_t(ch) < 0x80)
			{
				return 1;
			}
			if (0xc2 <= uint8_t(ch) && uint8_t(ch) < 0xe0)
			{
				return 2;
			}
			if (0xe0 <= uint8_t(ch) && uint8_t(ch) < 0xf0)
			{
				return 3;
			}
			if (0xf0 <= uint8_t(ch) && uint8_t(ch) < 0xf8)
			{
				return 4;
			}
			return 0;
		};

	auto IsU8FollowByte = [](char ch)
		{
			return 0x80 <= uint8_t(ch) && uint8_t(ch) < 0xc0;
		};

	uint32_t u32c = 0;
	char* s = (char*)utf8;
	while (*s)
	{
		//utf8→utf32
		switch (GetU8ByteCount(*s))
		{
		case 1:
			u32c = uint32_t(uint8_t(s[0]));
			s += 1;
			break;
		case 2:
			if (!IsU8FollowByte(s[1]))
			{
				return err;
			}
			if ((uint8_t(s[0]) & 0x1e) == 0)
			{
				return err;
			}

			u32c = uint32_t(s[0] & 0x1f) << 6;
			u32c |= uint32_t(s[1] & 0x3f);
			s += 2;
			break;
		case 3:
			if (!IsU8FollowByte(s[1]) || !IsU8FollowByte(s[2]))
			{
				return err;
			}
			if ((uint8_t(s[0]) & 0x0f) == 0 && (uint8_t(s[1]) & 0x20) == 0)
			{
				return err;
			}

			u32c = uint32_t(s[0] & 0x0f) << 12;
			u32c |= uint32_t(s[1] & 0x3f) << 6;
			u32c |= uint32_t(s[2] & 0x3f);
			s += 3;
			break;
		case 4:
			if (!IsU8FollowByte(s[1]) || !IsU8FollowByte(s[2]) || !IsU8FollowByte(s[3]))
			{
				return err;
			}
			if ((uint8_t(s[0]) & 0x07) == 0 && (uint8_t(s[1]) & 0x30) == 0)
			{
				return err;
			}

			u32c = uint32_t(s[0] & 0x07) << 18;
			u32c |= uint32_t(s[1] & 0x3F) << 12;
			u32c |= uint32_t(s[2] & 0x3F) << 6;
			u32c |= uint32_t(s[3] & 0x3F);
			s += 4;
			break;
		}
		//utf32→utf16
		if (u32c < 0 || u32c > 0x10ffff)
		{
			return err;
		}

		if (u32c < 0x10000)
		{
			result += wchar_t(u32c);
		}
		else
		{
			//u16c[0] = char16_t((u32Ch - 0x10000) / 0x400 + 0xD800);
			//u16c[1] = char16_t((u32Ch - 0x10000) % 0x400 + 0xDC00);
			return err;
		}
	}


	return result;
}

std::string MyUtil::U16ToString(const wchar_t* u16str)
{
	auto len = ::WideCharToMultiByte(932/*CP_ACP*/, 0, u16str, -1, nullptr, 0, nullptr, nullptr);
	std::string result(len * 2, '\0');
	if (!::WideCharToMultiByte(CP_ACP, 0, u16str, -1, &result[0], len, nullptr, nullptr))
	{
		return std::string();
	}
	auto real_len = std::strlen(result.c_str());
	result.resize(real_len);
	result.shrink_to_fit();
	return result;
}

std::vector<int32_t> MyUtil::StringToIntVector(const char* str, const char sep)
{
	std::vector<int32_t> result;
	std::string t;
	char* s = const_cast<char*>(str);
	while (*s)
	{
		if (*s == sep)
		{
			if (!t.empty())
			{
				result.push_back(static_cast<int32_t>(atoi(t.c_str())));
			}
			t = "";
		}
		else
		{
			t += *s;
		}
		s++;
	}
	if (!t.empty())
	{
		result.push_back(static_cast<int32_t>(atoi(t.c_str())));
	}
	return result;
}

std::vector<float> MyUtil::StringToFloatVector(const char* str, const char sep)
{
	std::vector<float> result;
	std::string t;
	char* s = const_cast<char*>(str);
	while (*s)
	{
		if (*s == sep)
		{
			if (!t.empty())
			{
				result.push_back(static_cast<float>(atof(t.c_str())));
			}
			t = "";
		}
		else
		{
			t += *s;
		}
		s++;
	}
	if (!t.empty())
	{
		result.push_back(static_cast<float>(atof(t.c_str())));
	}
	return result;
}

std::vector<std::string> MyUtil::StringToStringVector(const char* str, const char sep)
{
	std::vector<std::string> result;
	std::string t;
	char* s = const_cast<char*>(str);
	while (*s)
	{
		if (*s == sep)
		{
			if (!t.empty())
			{
				result.push_back(t);
			}
			t = "";
		}
		else
		{
			t += *s;
		}
		if (IsDBCSLeadByte(*s))
		{
			s++;
		}
		s++;
	}
	if (!t.empty())
	{
		result.push_back(t);
	}

	return result;
}

std::vector<std::wstring> MyUtil::WStringToWstringVector(const wchar_t* str, const wchar_t sep)
{
	std::vector<std::wstring> result;
	std::wstring txt;
	const wchar_t* ch = str;
	while (*ch)
	{
		if (*ch == sep)
		{
			if (!txt.empty())
			{
				result.push_back(txt);
			}
			txt.clear();
		}
		else
		{
			txt += *ch;
		}
		ch++;
	}
	if (!txt.empty())
	{
		result.push_back(txt);
	}

	return result;
}


#if 1

std::future<std::vector<uint8_t>> MyUtil::ReadBlobAsync(const std::string& fileName)
{
	//TRACEW(L"MyUtil::ReadBlobAsync(%s)\n", fileName.c_str());
	return std::async(std::launch::async, [fileName]()
		{
			//TRACE("std::async\n");
			std::vector<uint8_t> buf;
			HRESULT hr = ReadFromFile(fileName, &buf);
			if (FAILED(hr))
			{
				// エラー処理
				DBGBREAK();
				throw hr;// return nullptr;
			}
			return buf;
		}
	);
}
/*
* ブロッキングしてもよいならfutureをいきなりgetしてしまえば良い。
* そうでないならwait_for(ms)==std::future_status::readyになるまで待ってからget。
*/

void MyUtil::ReadFileAsync(const std::string& filePath, std::vector<uint8_t>* outBuf, ASyncStat* stat)
{
	assert(outBuf != nullptr);
	assert(stat != nullptr);

	try {
		std::cout << "ReadFileAsync(" << filePath << ")\n";

		// 非同期でファイルを読み込む
		auto future = std::async(std::launch::async, [filePath, outBuf, stat]() {
			try {
				std::ifstream file(filePath, std::ios::binary | std::ios::ate);
				if (!file.is_open()) {
					throw std::runtime_error("Failed to open file: " + filePath);
				}

				// ファイルサイズを取得
				std::streamsize size = file.tellg();
				file.seekg(0, std::ios::beg);

				// バッファをリサイズしてデータを読み込む
				outBuf->resize(static_cast<size_t>(size));
				if (!file.read(reinterpret_cast<char*>(outBuf->data()), size)) {
					throw std::runtime_error("Failed to read file: " + filePath);
				}

				// 成功を通知
				stat->promise.set_value();
			}
			catch (...) {
				stat->promise.set_exception(std::current_exception());
			}
			});

	}
	catch (const std::exception& ex) {
		std::cerr << "Error: " << ex.what() << std::endl;
		if (stat) {
			stat->promise.set_exception(std::current_exception());
		}
	}
}

#else 
#endif


HRESULT MyUtil::ReadFromFile(const std::string& filename, std::vector<uint8_t>* buf)
{
	std::ifstream fs(filename, std::ios::binary);
	if (!fs || !buf)
	{
		TRACE("File Not Found %s\n", filename);
		return ERROR_FILE_NOT_FOUND;
	}
	fs.seekg(0, std::ios::end);
	size_t size = static_cast<size_t>(fs.tellg());
	fs.seekg(0);

	buf->clear();
	buf->resize(size + 1);
	ZeroMemory(buf->data(), size + 1);
	fs.read(reinterpret_cast<char*>(buf->data()), size);
	fs.close();
	return S_OK;
}

std::vector<std::wstring> MyUtil::CommandLineToArgvW(const std::wstring& cmdLine)
{
	std::vector<std::wstring> args;
	std::wstring arg;
	auto len = cmdLine.length();
	bool inQuote = false;
	for (auto i = 0u; i < len; i++)
	{
		if (cmdLine[i] == L'"')
		{
			inQuote = !inQuote;
		}
		else if (cmdLine[i] == L' ' && !inQuote)
		{
			if (!arg.empty())
			{
				arg = RemoveDoubleQuoteEncloserW(arg);
				args.push_back(arg);
				arg.clear();
			}
		}
		else
		{
			arg += cmdLine[i];
		}
	}

	if (!arg.empty())
	{
		arg = RemoveDoubleQuoteEncloserW(arg);
		args.push_back(arg);
	}

	return args;
}

std::vector<std::string> MyUtil::CommandLineToArgvA(const std::string& cmdLine)
{
	std::vector<std::string> args;
	std::string arg;
	auto len = cmdLine.length();
	bool inQuote = false;
	for (auto i = 0u; i < len; i++)
	{
		if (cmdLine[i] == '\"')
		{
			inQuote = !inQuote;
		}
		else if (cmdLine[i] == ' ' && !inQuote)
		{
			if (!arg.empty())
			{
				arg = RemoveDoubleQuoteEncloserA(arg);
				args.push_back(arg);
				arg.clear();
			}
		}
		else
		{
			arg += cmdLine[i];
		}
	}

	if (!arg.empty())
	{
		arg = RemoveDoubleQuoteEncloserA(arg);
		args.push_back(arg);
	}

	return args;
}

std::string MyUtil::RemoveDoubleQuoteEncloserA(const std::string& str)
{
	if (str.empty())
	{
		return str; // 空文字列の場合はそのまま返す
	}

	if (str.front() == '\"' && str.back() == '\"')
	{
		return str.substr(1, str.length() - 2); // 先頭と末尾のダブルクォーテーションを除去
	}
	else
	{
		return str; // ダブルクォーテーションで囲まれていない場合はそのまま返す
	}
}

std::wstring MyUtil::RemoveDoubleQuoteEncloserW(const std::wstring& str)
{
	if (str.empty())
	{
		return str; // 空文字列の場合はそのまま返す
	}

	if (str.front() == L'\"' && str.back() == L'\"')
	{
		return str.substr(1, str.length() - 2); // 先頭と末尾のダブルクォーテーションを除去
	}
	else
	{
		return str; // ダブルクォーテーションで囲まれていない場合はそのまま返す
	}
}

HINSTANCE MyUtil::GetInstanceHandle()
{
	return HINSTANCE(::GetModuleHandle(nullptr));
}

std::string MyUtil::ModulePath()
{
	//auto package = winrt::Windows::ApplicationModel::Package::Current();
	//auto installedLocation = package.InstalledLocation();
	//auto path = installedLocation.Path();
	//return path.c_str();
	char path[MAX_PATH];
	GetModuleFileNameA(nullptr, path, MAX_PATH);
	return path;
}

std::string MyUtil::RemoveFileSpec(const std::string& name)
{
	return PathA(name).RemoveFileSpec().string();
}

std::string MyUtil::StripPath(const std::string& name)
{
	return PathA(name).StripPath().string();
}

std::string MyUtil::RenameExtension(const std::string& name, const std::string& ext)
{
	return PathA(name).RenameExtension(ext).string();
}

std::string MyUtil::FindExtension(const std::string& name)
{
	return PathA(name).FindExtension().string();
}

