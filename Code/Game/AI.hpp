#pragma once
#include "Game/Controller.hpp"
class Timer;
class AI : public Controller
{
public:
	AI();
	virtual ~AI();

	virtual void DamagedBy(Actor* actor);
	virtual void Update() override;

	ActorUID m_targetActorUID = ActorUID::INVALID;
private:
	void CatUpdate();
};
