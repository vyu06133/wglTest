#pragma once
#include "TaskBase.h"

class App;

class TaskSystem
{
public:
	static const int32_t defaultTickPrio = 0;
	static const int32_t defaultDrawPrio = 0;
	App* m_app = nullptr;
	App* GetApp() { return m_app; }
	void SetApp(App* app);
	TaskSystem();
	TaskSystem(App* app);
	~TaskSystem();
	void Destroy(TaskBase* task);
	void DestroyAll();
	void SortPrio()
	{
		std::sort(
			ticks.begin(),
			ticks.end(),
			[](TaskBase* a, TaskBase* b)->bool {
				return(a->tickPriority > b->tickPriority);
			}
		);
		std::sort(
			draws.begin(),
			draws.end(),
			[](TaskBase* a, TaskBase* b)->bool {
				return(a->drawPriority > b->drawPriority);
			}
		);
	}
	void Tick(float Delta);
	void Draw() const;

	void BroadcastKeyDown(int key)
	{
		for (auto& t : ticks)
		{
			if (t && t->IsValid())
			{
				t->OnKeyDown(key);
			}
		}
	}
	template<class T> T* CreateTask(TaskBase* const parent, const char* name = nullptr, int32_t tickPrio = defaultTickPrio, int32_t drawPrio = defaultDrawPrio)
	{
		do {
			taskId++;
			//todo: エラーチェック
		} while (taskMap.count(taskId));
		T* newtask = _NEW T();
		if (newtask)
		{
			newtask->id = taskId;
			taskMap[taskId] = newtask;
			newtask->ts = this;
			newtask->name = name ? name : "no name";
			newtask->parent = parent;
			newtask->tickPriority = tickPrio;
			newtask->drawPriority = drawPrio;

			newtask->OnConstruct();
			tasks.push_back(newtask);
			ticks.push_back(newtask);
			draws.push_back(newtask);
			SortPrio();
			taskMap[taskId] = newtask;
			newtask->OnCreate();
		}
		return newtask;
	}

	std::vector<TaskBase*> FindTaskByName(const std::string& name)
	{
		std::vector<TaskBase*> result;
		for (auto& t : tasks)
		{
			if (t->name == name)
			{
				result.push_back(t);
			}
		}
		return result;
	}

public:
	int32_t taskId = 0;
	std::vector<TaskBase*> tasks;//作成順
	std::unordered_map<int32_t, TaskBase*> taskMap;
	std::vector<TaskBase*> ticks;//tick順
	std::vector<TaskBase*> draws;//draw順
};

