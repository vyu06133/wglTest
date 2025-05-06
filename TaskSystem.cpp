#include "pch.h"
#include "TaskSystem.h"
#include "App.h"

void TaskSystem::SetApp(App* app)
{
	m_app = app;
}

TaskSystem::TaskSystem()
{
}

TaskSystem::TaskSystem(App* app)
{
	m_app = app;
}

TaskSystem::~TaskSystem()
{
	DestroyAll();
}

void TaskSystem::Tick(float Delta)
{
	ASSERT(m_app);
	//if (!m_renderer->IsLoadingComplete())
	//{
	//	Sleep(0);
	//	return;
	//}
	for (auto e : ticks)
	{
		if (e->IsValid())
		{
			e->tickCount++;
			e->OnTick(Delta);
			e->elapsed += Delta;
		}
	}

	// それぞれのOnPostTickを呼ぶ
	for (auto e : ticks)
	{
		if (e->IsValid())
		{
			e->OnPostTick();
		}
	}

	// world_をクリアする
	for (auto e : ticks)
	{
		if (e->IsValid())
		{
			e->worldDirty = true;
			e->worldMatrix = mat4(1.0f);
		}
	}

	// それぞれの親子関係でworld_を構築する
	for (auto e : ticks)
	{
		if (e->IsValid())
		{
			e->EnsureWorld();
		}
	}
}

void TaskSystem::Draw() const
{
	for (auto e : draws)
	{
		if (e->IsValid())
		{
			e->EnsureWorld();
			e->OnDraw();
		}
	}
}


void TaskSystem::Destroy(TaskBase* task)
{
	ASSERT(task);
	task->OnDestroy();
	auto ticksItr = std::remove_if(ticks.begin(), ticks.end(), [task](TaskBase* e) { return e == task; });
	ticks.erase(ticksItr, ticks.end());
	auto drawsItr = std::remove_if(draws.begin(), draws.end(), [task](TaskBase* e) { return e == task; });
	draws.erase(drawsItr, draws.end());
	auto taskItr = std::remove_if(tasks.begin(), tasks.end(), [task](TaskBase* e) { return e == task; });
	draws.erase(taskItr, tasks.end());
	auto mapItr = taskMap.find(task->id);
	if (mapItr != taskMap.end())
	{
		taskMap.erase(mapItr);
	}
	delete task;
}

void TaskSystem::DestroyAll()
{
	for (auto& task : tasks)
	{
		if (task)
		{
			auto ticksItr = std::remove_if(ticks.begin(), ticks.end(), [task](TaskBase* e) { return e == task; });
			ticks.erase(ticksItr, ticks.end());
			auto drawsItr = std::remove_if(draws.begin(), draws.end(), [task](TaskBase* e) { return e == task; });
			draws.erase(drawsItr, draws.end());
			task->OnDestroy();
			delete task;
		}
	}
	taskMap.clear();
	tasks.clear();
}

