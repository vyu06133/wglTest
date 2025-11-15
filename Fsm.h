#pragma once
#include "framework.h"
class FSM
{
public:
	std::unordered_map<std::string, std::function<void()>> stateActions;	// ステートとアクションのペアを保持するマップ
	std::string currentState;	// 現在のステート
	std::string nextState;
	bool finish = false;
	bool valid = false;
	bool IsFinished() const { return finish; }
	void Reset()
	{
		currentState = "";
		nextState = "";
		stateActions.clear();
		finish = false;
		valid = false;
	}
	FSM(const std::unordered_map<std::string, std::function<void()>>& states) : stateActions(states)
	{
		auto itr = states.begin();
		if (itr != states.end())
		{
			currentState = itr->first;
		}
	}
	FSM() { Reset(); }
	void SetStateActions(const std::unordered_map<std::string, std::function<void()>>& states)
	{
		if (!valid)
		{
			stateActions = states;
			auto itr = states.begin();
			if (itr != states.end())
			{
				currentState = itr->first;
			}
			nextState = currentState;
			valid = true;
		}
	}
	// 現在のステートのアクションを実行するメソッド
	void Run()
	{
		auto itr = stateActions.find(currentState);
		ASSERT(itr != stateActions.end());
		itr->second();
		currentState = nextState;
	}
	// ステート変更を予約するメソッド
	void changeState(const std::string& newState)
	{
		nextState = newState;
	}
	void Finish()
	{
		finish = true;
	}
};

