#include "pch.h"
#include "TaskBase.h"

void TaskBase::OnConstruct()
{
}

void TaskBase::OnDestruct()
{
}

void TaskBase::OnTick(float deltaTime)
{
}

void TaskBase::OnPostTick()
{
}

void TaskBase::OnDraw()
{
}

void TaskBase::OnCreate()
{
}

void TaskBase::OnDestroy()
{
}

void TaskBase::OnKeyDown(int vk)
{
}

void TaskBase::OnKeyUp(int vk)
{
}

bool TaskBase::IsValid() const
{
	return true;
}

void TaskBase::EnsureWorld()
{
	if (worldDirty)
	{
		// world_が完成していないので完成させる
		if (parent && parent->IsValid())
		{
			if (parent->worldDirty)
			{
				// 親のworld_確定を念押しする
				parent->EnsureWorld();//再帰
			}
			worldMatrix = localMatrix * parent->worldMatrix;
		}
		else
		{
			// root	直下である
			worldMatrix= localMatrix;//* taskSys_->origin_;
		}
		worldDirty = false;
	}
}
