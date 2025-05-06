#pragma once
#include "framework.h"
#include "MyMath.h"
#include "Texture2D.h"

class BinpackingSolver
{
public:
	class Node
	{
		bool contain(const FT_Bitmap* bmp, const Rect& rect)
		{
			return (int)bmp->width <= rect.Width() && (int)bmp->rows <= rect.Height();
		}

		bool fitPerfect(const FT_Bitmap* bmp, const Rect& rect)
		{
			return (int)bmp->width == rect.Width() && (int)bmp->rows == rect.Height();
		}
	public:
		Node(wchar_t id = 0) :
			id_(id),
			rect_(0, 0, 0, 0),
			bitmap_(nullptr),
			used_(false),
			parent_(nullptr)
		{
			child_[0] = child_[1] = nullptr;
		}
		~Node()
		{
			if (child_[0])
			{
				delete child_[0];
				DecNumNodes();
				child_[0] = nullptr;
			}
			if (child_[1])
			{
				delete child_[1];
				DecNumNodes();
				child_[1] = nullptr;
			}
		}
		void _cdecl remove()
		{
			auto& c0 = child_[0];
			auto& c1 = child_[1];
			if (c0)
			{
				c0->remove();
				delete c0;
				c0 = nullptr;
				DecNumNodes();
			}
			if (c1)
			{
				c1->remove();
				delete c1;
				c1 = nullptr;
				DecNumNodes();
			}
		}
		Node* _cdecl insert(const FT_Bitmap* bmp, wchar_t id)
		{
			if (!IsLeaf())
			{
				Node* newNode = nullptr;
				if (child_[0] != nullptr)
				{
					newNode = child_[0]->insert(bmp, id);

					if (newNode != nullptr)
						return newNode;
				}

				return child_[1]->insert(bmp, id);
			}
			else
			{
				if (IsUsed())
					return nullptr;

				if (!contain(bmp, rect_))
					return nullptr;

				if (fitPerfect(bmp, rect_))
				{
					SetUsed(bmp);
					SetId(id);
					bitmap_ = (FT_Bitmap*)bmp;
					return this;
				}

				child_[0] = _NEW Node;
				child_[1] = _NEW Node;

				IncNumNodes();
				IncNumNodes();

				child_[0]->parent_ = this;
				child_[1]->parent_ = this;

				const auto dw = rect_.Width() - bmp->width;
				const auto dh = rect_.Height() - bmp->rows;
				const auto rw = rect_.Width();
				const auto rh = rect_.Height();

				if (dw > dh)
				{
					child_[0]->rect_ = Rect(rect_.left, rect_.top, rect_.left + bmp->width - 1, rect_.bottom);
					child_[1]->rect_ = Rect(rect_.left + bmp->width, rect_.top, rect_.right, rect_.bottom);
				}
				else
				{
					child_[0]->rect_ = Rect(rect_.left, rect_.top, rect_.right, rect_.top + bmp->rows - 1);
					child_[1]->rect_ = Rect(rect_.left, rect_.top + bmp->rows, rect_.right, rect_.bottom);
				}

				return child_[0]->insert(bmp, id);
			}

			return nullptr;
		}
		const Rect& GetRect() const
		{
			return rect_;
		}
		void SetRect(const Rect& rc)
		{
			rect_ = rc;
		}
		FT_Bitmap* GetBitmap() const
		{
			return bitmap_;
		}
		bool IsUsed() const
		{
			return used_;
		}
		void SetUsed(bool flag)
		{
			used_ = flag;
		}
		Node* GetParent() const
		{
			return parent_;
		}
		Node* GetChild0() const
		{
			return child_[0];
		}
		Node* GetChild1()
		{
			return child_[1];
		}
		bool IsLeaf() const
		{
			return child_[0] == nullptr && child_[1] == nullptr;
		}

		bool IsChildrenEmpty()
		{
			return IsLeaf();
		}

		void SetId(wchar_t id)
		{
			id_ = id;
		}

		wchar_t GetId() const
		{
			return id_;
		}

		inline static void ResetNumNodes()
		{
			numNodes_ = 0;
		}

		inline static void IncNumNodes()
		{
			numNodes_++;
		}

		inline static void DecNumNodes()
		{
			numNodes_--;
		}

		inline static int GetNumNodes()
		{
			return numNodes_;
		}
	private:
		inline static int numNodes_ = 0;
		wchar_t id_;
		Node* parent_;
		Node* child_[2];
		Rect rect_;			// ノードの担当する領域
		FT_Bitmap* bitmap_;	// FreeTypeのビットマップ
		bool used_;
	};

public:
	BinpackingSolver(Texture2D* atlas)
	{
		Node::ResetNumNodes();
		root_ = _NEW Node();
		Node::IncNumNodes();
		Atlas_ = atlas;
		root_->SetRect(MyMath::XYWH(0, 0, static_cast<int>(atlas->width_), static_cast<int>(atlas->height_)));
	}
	~BinpackingSolver()
	{
		if (root_)
		{
			delete root_;
			Node::DecNumNodes();
		}
		root_ = nullptr;
	}
	Rect insert(const FT_Bitmap* bmp, wchar_t id)
	{
		auto* node = root_->insert(bmp, id);
		if (node != nullptr)
		{
			node->SetId(id);
			return node->GetRect();
		}

		//		TRACE("Cannot insert.\n");
		return Rect();
	}

public:
	Node* root_ = nullptr;
	Texture2D* Atlas_ = nullptr;
};

