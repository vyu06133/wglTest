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
		// world_‚ªŠ®¬‚µ‚Ä‚¢‚È‚¢‚Ì‚ÅŠ®¬‚³‚¹‚é
		if (parent && parent->IsValid())
		{
			if (parent->worldDirty)
			{
				// e‚Ìworld_Šm’è‚ð”O‰Ÿ‚µ‚·‚é
				parent->EnsureWorld();//Ä‹A
			}
			worldMatrix = localMatrix * parent->worldMatrix;
		}
		else
		{
			// root	’¼‰º‚Å‚ ‚é
			worldMatrix= localMatrix;//* taskSys_->origin_;
		}
		worldDirty = false;
	}
}
