#pragma once
#include <iostream>
#include <istream>
#include <streambuf>
#include <algorithm> // std::min を使用

class MemStreamBuf : public std::streambuf
{
public:
	MemStreamBuf(const uint8_t* data, size_t size) : mClosed(false)
	{
		char* begin = reinterpret_cast<char*>(const_cast<uint8_t*>(data));
		setg(begin, begin, begin + size);
	}

	void close()
	{
		mClosed = true;
		// 必要であれば setg(nullptr, nullptr, nullptr); でバッファ解放的挙動も可能
		// ただし data は外部所有なのでポインタ解放は行わない
		setg(eback(), egptr(), egptr()); // 読み取り位置を終端に移動
	}

	bool is_closed() const { return mClosed; }

private:
	bool mClosed;

protected:
	pos_type seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which) override
	{
		if (!(which & std::ios_base::in))
		{
			return pos_type(off_type(-1)); // 読み込みモードのみ対応
		}

		char* newpos = nullptr;

		if (dir == std::ios_base::beg)
		{
			newpos = eback() + off;
		}
		else if (dir == std::ios_base::cur)
		{
			newpos = gptr() + off;
		}
		else if (dir == std::ios_base::end)
		{
			newpos = egptr() + off;
		}
		else
		{
			return pos_type(off_type(-1));
		}

		if (newpos < eback() || newpos > egptr())
		{
			setg(eback(), newpos, egptr());
		}

		return pos_type(newpos - eback());
	}

	pos_type seekpos(pos_type pos, std::ios_base::openmode which) override
	{
		return seekoff(pos - pos_type(0), std::ios_base::beg, which);
	}
};

// メモリバッファを直接利用する istream クラス
class IMemStream : public MemStreamBuf, public std::istream
{
public:
	// コンストラクタで streambuf と istream の両方を初期化
	IMemStream(const uint8_t* data, size_t size)
		// MemStreamBuf を初期化
		: MemStreamBuf(data, size),
		// istream を、自身（MemStreamBuf）を streambuf として関連付けて初期化
		std::istream(static_cast<std::streambuf*>(this)) {
	}
	IMemStream(const std::vector<uint8_t>& data)
		// MemStreamBuf を初期化
		: MemStreamBuf(data.data(), data.size()),
		// istream を、自身（MemStreamBuf）を streambuf として関連付けて初期化
		std::istream(static_cast<std::streambuf*>(this)) {
	}
};
