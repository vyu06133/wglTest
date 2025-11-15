
#pragma once
#include "framework.h"

struct MyUtil
{
	static inline std::string assetDir_;

	static bool Init(const char* locale);
	static std::wstring SjisToWstring(const char* sjis);
	static std::wstring Utf8ToWstring(const char* utf8);
	static std::string U16ToString(const wchar_t* u16str);
	static std::vector<int32_t> StringToIntVector(const char* str, const char sep = ',');
	static std::vector<float> StringToFloatVector(const char* str, const char sep = ',');
	static std::vector<std::string> StringToStringVector(const char* str, const char sep = ',');
	/*
	* [0]="drive:"(ex A: B: C:)
	* [1..n-2]="dir"
	* [n-1]="file.ext"
	*/
	static std::vector<std::wstring> WStringToWstringVector(const wchar_t* str, const wchar_t sep = L',');
	static std::vector<std::string> CommandLineToArgvA(const std::string& cmdLine);
	static std::vector<std::wstring> CommandLineToArgvW(const std::wstring& cmdLine);
	static std::string RemoveDoubleQuoteEncloserA(const std::string& str);
	static std::wstring RemoveDoubleQuoteEncloserW(const std::wstring& str);
	static std::string ModulePath();
	static std::string SlashToBackSlash(const std::string& str)
	{
		std::string result(str);
		std::replace(result.begin(), result.end(), '/', '\\');
		return result;
	}
	class PathA
	{
	public:
		inline PathA(const std::string& path) : path_(path) {}
		inline PathA(const char* str) : path_(str) {}
		inline PathA() : path_() {}
		// パス文字列を取得
		inline const std::string& string() const { return path_; }

		// パス要素を取得 (vectorで返す)
		inline std::vector<std::string> Tokenize() const
		{
			std::vector<std::string> components;
			std::size_t start = 0, end;
			while ((end = path_.find(SEPARATOR, start)) != std::string::npos)
			{
				components.push_back(path_.substr(start, end - start));
				start = end + 1;
			}
			if (start != path_.length())
			{
				components.push_back(path_.substr(start));
			}
			return components;
		}

		// パス結合
		inline PathA Append(const PathA& other)
		{
			if (other.string().empty()) return *this;
			if (path_.empty())
			{
				path_ = other.string();
				return other;
			}
			if (path_.back() != SEPARATOR && other.string().front() != SEPARATOR)
			{
				path_ += SEPARATOR;
				path_ += other.string();
				return *this;
			}
			path_ += other.string();
			return *this;
		}

		inline PathA StripPath() const
		{
			// パスを最後のパス区切り文字で分割
			auto pos = path_.find_last_of(SEPARATOR);
			if (pos == std::string::npos)
			{
				// パス区切り文字が見つからなかった場合、パス全体をファイル名とする
				return path_;
			}
			else
			{
				return path_.substr(pos + 1);
			}
		}
		
		inline PathA FindExtension() const
		{
			// パス区切り文字で分割し、最後の要素（ファイル名）を取得
			auto pos = path_.find_last_of(SEPARATOR);
			std::string filename((pos == std::string::npos) ? path_ : path_.substr(pos + 1));

			// 拡張子の位置を探す
			auto ext_pos = filename.find_last_of('.');
			if (ext_pos != std::string::npos && ext_pos != 0)
			{
				// 拡張子が見つかった場合、拡張子部分を返す
				return PathA(filename.substr(ext_pos));
			}
			else
			{
				// 拡張子が見つからなかった場合、空文字列を返す
				return PathA("");
			}
		}

		inline PathA RenameExtension(const std::string& ext)
		{
			// パス区切り文字で分割し、最後の要素（ファイル名）を取得
			auto pos = path_.find_last_of(SEPARATOR);
			std::string filename(pos == std::string::npos ? path_ : path_.substr(pos + 1));

			// 拡張子の位置を探す
			auto ext_pos = filename.find_last_of('.');
			if (ext_pos != std::string::npos)
			{
				// 拡張子が見つかった場合、拡張子部分を置き換える
				return PathA(path_.substr(0, ext_pos) + ext);
			}
			else
			{
				// 拡張子が見つからなかった場合、ファイル名の後に拡張子を付加する
				return PathA(path_ + ext);
			}
		}

		PathA RemoveFileSpec() const
		{
			// パス区切り文字で分割し、最後の要素（ファイル名）を除く
			auto pos = path_.find_last_of(SEPARATOR);
			if (pos == std::string::npos)
			{
				// パス区切り文字が見つからなかった場合、ルートディレクトリとみなす
				return PathA(path_);
			}
			else
			{
				return PathA(path_.substr(0, pos));
			}
		}

	private:
		std::string path_;

		// プラットフォーム依存のパス区切り文字を取得
		static const char SEPARATOR = '\\';
	};

	class PathW
	{
		public:
			inline PathW(const std::wstring& path) : path_(path) {}
			inline PathW(const wchar_t* path) : path_(path) {}
			inline PathW() {}

			// パス文字列を取得
			inline const std::wstring& wstring() const { return path_; }

			// パス要素を取得 (vectorで返す)
			inline std::vector<std::wstring> Tokenize() const
			{
				std::vector<std::wstring> components;
				std::size_t start = 0, end;
				while ((end = path_.find(SEPARATOR, start)) != std::wstring::npos)
				{
					components.push_back(path_.substr(start, end - start));
					start = end + 1;
				}
				if (start != path_.length())
				{
					components.push_back(path_.substr(start));
				}
				return components;
			}

			// パス結合
			inline PathW Append(const PathW& other) 
			{
				if (other.wstring().empty()) return *this;
				if (path_.empty())
				{
					path_ = other.wstring();
					return *this;
				}
				if (path_.back() != SEPARATOR && other.wstring().front() != SEPARATOR)
				{
					path_ += SEPARATOR;
					path_ += other.wstring();
					return *this;
				}
				path_ += other.wstring();
				return *this;
			}

			inline PathW StripPath() const
			{
				// パスを最後のパス区切り文字で分割
				auto pos = path_.find_last_of(SEPARATOR);
				if (pos == std::string::npos)
				{
					// パス区切り文字が見つからなかった場合、パス全体をファイル名とする
					return path_;
				}
				else
				{
					return path_.substr(pos + 1);
				}
			}

			inline PathW FindExtension() const
			{
				// パス区切り文字で分割し、最後の要素（ファイル名）を取得
				auto pos = path_.find_last_of(SEPARATOR);
				std::wstring filename((pos == std::string::npos) ? path_ : path_.substr(pos + 1));

				// 拡張子の位置を探す
				auto ext_pos = filename.find_last_of(L'.');
				if (ext_pos != std::string::npos && ext_pos != 0)
				{
					// 拡張子が見つかった場合、拡張子部分を返す
					return PathW(filename.substr(ext_pos));
				}
				else
				{
					// 拡張子が見つからなかった場合、空文字列を返す
					return PathW(L"");
				}
			}

			inline PathW RenameExtension(const std::wstring& ext)
			{
				// パス区切り文字で分割し、最後の要素（ファイル名）を取得
				auto pos = path_.find_last_of(SEPARATOR);
				std::wstring filename(pos == std::string::npos ? path_ : path_.substr(pos + 1));

				// 拡張子の位置を探す
				auto ext_pos = filename.find_last_of('.');
				if (ext_pos != std::string::npos)
				{
					// 拡張子が見つかった場合、拡張子部分を置き換える
					return PathW(path_.substr(0, ext_pos) + ext);
				}
				else
				{
					// 拡張子が見つからなかった場合、ファイル名の後に拡張子を付加する
					return PathW(path_ + ext);
				}
			}

			PathW RemoveFileSpec() const
			{
				// パス区切り文字で分割し、最後の要素（ファイル名）を除く
				auto pos = path_.find_last_of(SEPARATOR);
				if (pos == std::string::npos)
				{
					// パス区切り文字が見つからなかった場合、ルートディレクトリとみなす
					return PathW(path_);
				}
				else
				{
					return PathW(path_.substr(0, pos));
				}
			}

		private:
			std::wstring path_;

			// プラットフォーム依存のパス区切り文字を取得
			static const wchar_t SEPARATOR = L'\\';
	};
	using Path = PathA;
	
	static std::string RemoveFileSpec(const std::string& name);
	static std::string StripPath(const std::string& name);
	static std::string RenameExtension(const std::string& name, const std::string& ext);
	static std::string FindExtension(const std::string& name);
	struct ASyncStat
	{
		std::promise<void> promise;
		std::future<void> future;
		enum Stat
		{
			READY = std::future_status::ready,
			TIMEOUT = std::future_status::timeout,
			DEFERRED = std::future_status::deferred
		};
		inline ASyncStat() { Init(); }
		inline ~ASyncStat() { Term(); }
		inline void Init()
		{
			future = promise.get_future();
		}
		inline void Term()
		{
			promise.set_value();
			future.get();
		}
		inline Stat GetStat(const long long timeout = 100)
		{
			auto status = future.wait_for(std::chrono::milliseconds(timeout));
			return (Stat)status;

		}
	};
	static std::future<std::vector<uint8_t>> ReadBlobAsync(const std::string& fileName);
	static void ReadFileAsync(const std::string& filePath, std::vector<uint8_t>* outBuf, ASyncStat* stat);
	static HRESULT ReadFromFile(const std::string& filename, std::vector<uint8_t>* buffer);
	static HINSTANCE GetInstanceHandle();
	static void SetAssetDir(const std::string& dir) { assetDir_ = dir; }
	
	/*
	* std::unordered_mapをファイル出力
	* KEY	固定長キー
	* VALUE	固定長データ
	*/
	template <typename KEY, typename VALUE>
	inline static void SerializeMap(const std::unordered_map<KEY, VALUE>& map, const std::string& filename)
	{
		std::ofstream outFile(filename, std::ios::binary);

		// サイズを書き込み
		size_t size = map.size();
		outFile.write(reinterpret_cast<const char*>(&size), sizeof(size));

		// 各ペアを書き込み
		for (const auto& [key, value] : map)
		{
			outFile.write(reinterpret_cast<const char*>(&key), sizeof(key));
			outFile.write(reinterpret_cast<const char*>(&value), sizeof(value));
		}

		outFile.close();
	}

	template <typename KEY, typename VALUE>
	inline static std::unordered_map<KEY, VALUE> DeserializeMap(const std::string& filename)
	{
		try
		{
			std::unordered_map<KEY, VALUE> map;
			auto fbuf = ReadBlobAsync(filename).get();
			size_t fp = 0;

			// record size
			size_t size = *(size_t*)(&fbuf[fp]);
			fp += sizeof(size_t);

			for (size_t i = 0; i < size; ++i)
			{
				KEY key = *(KEY*)(&fbuf[fp]);
				fp += sizeof(KEY);

				VALUE value = *(VALUE*)(&fbuf[fp]);
				fp += sizeof(VALUE);

				map[key] = value;
			}
			return map;
		}
		catch (const std::exception& ex)
		{
			TRACE("Error: %s\n", ex.what());
		}
		return std::unordered_map<KEY, VALUE>();
	}

	template<class T>class Aarray2d
	{
	protected:
		std::vector<T> data_;
		int32_t width_;
		int32_t height_;

	public:
		Aarray2d(int32_t width, int32_t height)
			: width_(width)
			, height_(height)
		{
			data_.resize(width * height);
		}

		Aarray2d()
			: width_(0)
			, height_(0)
		{}

		void Init(int32_t width, int32_t height, const T& InInitialValue)
		{
			width_ = width;
			height_ = height;
			data_.resize(width * height);
			data_.assign(width * height, InInitialValue);
		}

		void Fill(const T& InValue)
		{
			Init(width_, height_, InValue);
		}

		int32_t GetWidth() const
		{
			return width_;
		}

		int32_t GetHeight() const
		{
			return height_;
		}

		bool IsValidIndex(int32_t InX, int32_t InY) const
		{
			return (InX >= 0 && InY >= 0 && InX < width_ && InY < height_);
		}

		void Set(int32_t InX, int32_t InY, const float& InValue)
		{
			data_[InY * width_ + InX] = InValue;
		}

		T& Get(int32_t InX, int32_t InY)
		{
			return data_[InY * width_ + InX];
		}

		T& operator()(int32_t InX, int32_t InY)
		{
			return data_[InY * width_ + InX];
		}

		const T& Get(int32_t InX, int32_t InY) const
		{
			return data_[InY * width_ + InX];
		}

		const T& operator()(int32_t InX, int32_t InY) const
		{
			return data_[InY * width_ + InX];
		}
	};

#if true
	template<typename TYPE>
	class Tree
	{
	public:
		//!	ノード定義
		struct Item
		{
			Item(const TYPE& t = TYPE()) : data(t), parent(nullptr), child(nullptr), sibling(nullptr), next(nullptr) {}
			TYPE data;			//!<	ノードデータ
			Item* parent;		//!<	親ノード
			Item* child;		//!<	子ノード
			Item* sibling;		//!<	兄弟ノード
			Item* next;		//!<	リストチェイン
			uint32_t depth;
		};
		Item mRoot;
		Item* mBegin = nullptr;
		Item* mEnd = nullptr;
		uint32_t mCount = 0;
		inline Item* GetRoot()
		{
			return &mRoot;
		}
		inline Item* GetFirstChild()
		{
			return mRoot.child;
		}
		inline auto Begin()
		{
			return mBegin;
		}
		inline auto End()
		{
			return nullptr;
		}
		inline uint32_t GetCount() const
		{
			return mCount;
		}
		inline void Clear()
		{
			auto i = mBegin;
			while(i)
			{
				auto next = i->next;
				delete i;
				i= next;
			}
			mBegin = mEnd = nullptr;
			mCount = 0;
			mRoot.parent = nullptr;
			mRoot.child = nullptr;
			mRoot.sibling = nullptr;
			mRoot.next = nullptr;
			mRoot.depth = 0;
		}
		inline Tree(void)
		{
			mBegin = mEnd = nullptr;
			mCount = 0;
			mRoot.parent = nullptr;
			mRoot.child = nullptr;
			mRoot.sibling = nullptr;
			mRoot.next = nullptr;
			mRoot.depth = 0;
		}
		inline ~Tree(void)
		{
			Clear();
		}
		inline Item* Insert(const TYPE& data, Item* parent)
		{
			Item* item = _NEW Item(data);
			if (!item)
			{
				return nullptr;
			}

			if(!mBegin)
			{
				mBegin = item;
				mEnd = item;
				mCount++;
			}
			else
			{
				mEnd->next = item;
				mEnd = item;
				mCount++;
			}

			// 対象→親へリンク
			item->parent = parent;
			item->depth = parent ? (parent->depth + 1) : 0;

			//兄弟チェイン
			Item** s = nullptr;
			Item* c = nullptr;
			if (parent)
			{
				c = parent->child;
			}
			else
			{
				//親がルートの場合
				c = mRoot.child;
			}
			//兄弟の末端を探す
			while (c && c->sibling)
			{
				c = c->sibling;
			}
			if (c)
			{
				c->sibling = item;
			}

			// 親→子（対象）へリンク
			if (parent)
			{
				if(!parent->child)
				{
					parent->child = item;
				}
			}
			else
			{
				//親がルートの場合
				if (!mRoot.child)
				{
					mRoot.child = item;
				}
			}

			return item;
		}
		inline void Remove(Item* item,bool withChildren=true)
		{
			ASSERT(item);
			auto parent = item->parent;
			if (parent)
			{
				Item* c=parent->child;
				if (c == item)
				{
					parent->child = item->sibling;
				}
				else
				{
					while (c && c->sibling)
					{
						if (c->sibling == item)
						{
							c->sibling = item->sibling;
							break;
						}
						c = c->sibling;
					}
				}
			}
			else
			{
				//親がルートの場合
				Item* c = mRoot.child;
				if (c == item)
				{
					mRoot.child = item->sibling;

				}
				else
				{
					while (c && c->sibling)
					{
						if (c->sibling == item)
						{
							c->sibling = item->sibling;
							break;
						}
						c = c->sibling;
					}
				}
			}
			if (withChildren)
			{
				// 子供を再帰的に削除
				auto c = item->child;
				while (c)
				{
					auto next = c->sibling;
					Remove(c);
					c = next;
				}
			}
			// リストから削除
			for (auto c = mBegin; c != mEnd; c = c->next)
			{
				auto& next = c->next;
				if (next == item)
				{
					next = next->next;
					break;
				}
			}

			// メモリ解放
			delete item;
			mCount--;
		}
	};
#endif
#if true
//#include <iostream>
//#include <vector>
//#include <memory>

	template <typename T>
	class TreeNode
	{
	public:
		T data;
		std::vector<std::unique_ptr<TreeNode<T>>> children;
		std::weak_ptr<TreeNode<T>> parent;
		TreeNode<T>* next;
		inline static TreeNode<T>* lastNode = nullptr;

		TreeNode(const T& data) : data(data), next(nullptr) {}

		// 子ノードを追加する
		void addChild(std::unique_ptr<TreeNode<T>> child)
		{
			child->parent = this;
			children.push_back(std::move(child));

			// 新しい子ノードをチェインの最後に追加
			if (lastNode) {
				lastNode->next = child.get();
			}
			lastNode = child.get();
		}

		// ノードを削除する (自分自身を削除)
		void removeSelf()
		{
			// 親ノードのchildrenから自分自身を削除
			if (auto parentPtr = parent.lock())
			{
				auto& children = parentPtr->children;
				children.erase(std::remove_if(children.begin(), children.end(),
					[this](const std::unique_ptr<TreeNode<T>>& child)
					{
						return child.get() == this;
					}
				),
					children.end()
				);

				// 兄弟ノードのnextを更新
				if (next)
				{
					// 自分が最後のノードでない場合
					TreeNode<T>* prev = this;
					while (prev->next != this)
					{
						prev = prev->next;
					}
					prev->next = next;
				}
				else
				{
					// 自分が最後のノードの場合
					// lastNodeを更新する
					TreeNode<T>* temp = this;
					while (temp->parent)
					{
						temp = temp->parent.lock().get();
					}
					// ルートノードの最後の子ノードを探す
					while (temp->children.size() > 0 && temp->children.back()->next)
					{
						temp = temp->children.back().get();
					}
					if (temp->children.size() > 0)
					{
						temp->children.back()->next = nullptr;
						lastNode = temp->children.back().get();
					}
					else
					{
						lastNode = nullptr;
					}
				}
			}
		}
	};



#endif
#if false
	//
	// 
	// 
	//
	template<typename TYPE>
	class Tree
	{
	public:
		//!	ノード定義
		struct Node
		{
			Node() : data(), pParent(nullptr), pSiblings(nullptr), pChildren(nullptr), pNext(nullptr), pPrev(nullptr) {}
			TYPE data;			//!<	ノードデータ
			Node* pParent;		//!<	親ノード
			Node* pSiblings;	//!<	兄弟ノード
			Node* pChildren;	//!<	子ノード
			Node* pNext;		//!<	リストノード
			Node* pPrev;		//!<	リストノード
			uint32_t depth;
		};

	protected:
		//!	ノード数
		unsigned int			m_nCount;

		//!	ルートノード
		Node	m_root;

	public:
		Tree(void)
		{
			m_root.pParent = nullptr;
			m_root.pSiblings = nullptr;
			m_root.pChildren = nullptr;
			m_root.pNext = nullptr;
			m_root.pPrev = nullptr;
			m_root.depth = 0;
			m_nCount = 0;
		}
		Tree(const Tree& src)
		{
			m_root.pParent = nullptr;
			m_root.pSiblings = nullptr;
			m_root.pChildren = nullptr;
			m_root.pNext = nullptr;
			m_root.pPrev = nullptr;
			m_root.depth = 0;
			m_nCount = 0;
			copyNode(src.GetRoot(), nullptr);
		}
		Node* copyNode(Node* node, Node* parent)
		{
			if (!node)
			{
				return;
			}
			auto ins = Insert(node->data, parent);
			for (auto sib = node->pSiblings; sib; sib = sib->pSiblings)
			{
				copyNode(sib, parent);
			}
			if (node->pChildren)
			{
				return copyNode(node->pChildren, ins);
			}
		}
		~Tree()
		{
			Clear();
		}

		inline Node* Search(bool (*pred)(const TYPE& data)) const
		{
			for (auto node = GetRoot(); node != nullptr; node = GetNext(node))
			{
				if (pred(node->data))
				{
					return node;
				}
			}
		}

		inline unsigned int GetCount() const
		{
			return m_nCount;
		}

		inline Node* Insert(const TYPE& data, Node* hParent, bool bAtLast = true)
		{
			Node* pNode = _NEW Node;

			if (pNode == nullptr)
			{
				return nullptr;
			}

			pNode->data = data;

			pNode->pChildren = nullptr;
			pNode->pSiblings = nullptr;

			pNode->pParent = hParent;
			pNode->depth = hParent ? (hParent->depth + 1) : 0;
			//if (hParent == nullptr)
			//{
			//	pNode->pParent = &m_root;	// Insert At Root
			//}
			//else
			//{
			//	pNode->pParent = hParent;
			//}

			// Link Chain
			if (bAtLast)
			{
				Node** s;
				if (pNode->pParent)
					s = &(pNode->pParent->pChildren);
				else
					s = &m_root.pChildren;

				while (*s != nullptr)
					s = &((*s)->pSiblings);
				*s = pNode;
			}
			else
			{
				// tree
				pNode->pSiblings = pNode->pParent->pChildren;
				pNode->pParent->pChildren = pNode;
			}

			// 探索用双方向リスト
			if (!m_root.pNext)
			{
				// リスト要素がない＝リスト最初もリスト終端も存在しない
				ASSERT(m_nCount < 1);
				m_root.pNext = pNode;
				pNode->pNext = nullptr;
				pNode->pPrev = &m_root;
			}
			else
			{
				if (bAtLast)
				{
					// リスト終端を探す
					Node* itr = m_root.pNext;
					ASSERT(itr);
					for (; itr->pNext; itr = itr->pNext) {}
					itr->pNext = pNode;
					pNode->pPrev = itr;
				}
				else
				{
					// リスト最初に挿入
					Node* pNext = m_root.pNext;
					ASSERT(pNext);
					ASSERT(pNext->pPrev == &m_root);
					pNext->pPrev = pNode;
					pNode->pNext = pNext;
					pNode->pPrev = &m_root;
					m_root.pNext = pNode;
				}
			}
			m_nCount++;

			return pNode;
		}

		inline void Remove(Node* hNode)
		{
			Node* pNode = hNode;
			Node** ppChain;

			if (pNode->pPrev)
			{
				ASSERT(pNode->pPrev->pNext == pNode);
				pNode->pPrev->pNext = pNode->pNext;
			}

			if (pNode->pNext)
			{
				ASSERT(pNode->pNext->pPrev == pNode);
				pNode->pNext->pPrev = pNode->pPrev;
			}

			// Remove Children
			while (pNode->pChildren)
			{
				Remove((pNode->pChildren));
				pNode->pChildren = nullptr;
			}
			// Remove this
			if (pNode->pParent)
			{
				ppChain = &(pNode->pParent->pChildren);
				while (ppChain && *ppChain)
				{
					if (*ppChain == pNode)
					{
						*ppChain = pNode->pSiblings;
						break;
					}
//					ppChain = &((*ppChain)->pSiblings);
				}
			}

			delete pNode;

			m_nCount--;
		}

		inline void Clear()
		{
			auto p = GetBegin();
			while (p != nullptr)
			{
				auto n = p->pNext;
				//Remove(p);
				//TRACE("Remove %p\n", p);
				delete p;
				p = n;
			}
			m_root.pNext = nullptr;
		}

		inline Node* GetRoot() const
		{
			return (m_root.pChildren);
		}

		/// <summary>
		/// シーケンシャルアクセス（GenNextで手繰る）用
		/// </summary>
		/// <returns></returns>
		inline Node* GetBegin() const
		{
			return (m_root.pNext);
		}

		inline Node* Find(const TYPE& data) const
		{
			Node* ptr = GetBegin();
			while (ptr)
			{
				if (ptr->data == data)
				{
					return ptr;
				}
				ptr = ptr->pNext;
			}
			return nullptr;
		}

		inline Node* Find(const TYPE& data, bool (*comp)(const TYPE& a, const TYPE& b)) const
		{
			Node* ptr = GetBegin();
			while (ptr)
			{
				if (comp(ptr->data, data))
				{
					return ptr;
				}
				ptr = ptr->pNext;
			}
			return nullptr;
		}

		inline Node* GetParent(Node* hNode) const
		{
			if ((hNode->pParent) == &m_root)
			{
				return nullptr;
			}
			else
			{
				return hNode->pParent;
			}
		}

		inline static Node* GetSiblings(Node* hNode)
		{
			return hNode->pSiblings;
		}

		inline static Node* GetChildren(Node* hNode)
		{
			return hNode->pChildren;
		}

		inline static Node* GetNext(Node* hNode)
		{
			return hNode->pNext;
		}

		inline static Node* GetPrev(Node* hNode)
		{
			return hNode->pPrev;
		}

		inline void SetAt(Node* hNode, const TYPE& data)
		{
			hNode->data = data;
		}

		inline const TYPE& GetAt(Node* hNode) const
		{
			return hNode->data;
		}

		inline const TYPE& operator[](Node* hNode) const
		{
			return hNode->data;
		}

		inline TYPE& operator[](Node* hNode)
		{
			return hNode->data;
		}
	};
#endif
};

