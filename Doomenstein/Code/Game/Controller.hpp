#pragma once
#include "Game/Gamecommon.hpp"
#include "Game/ActorUID.hpp"
class Actor;
class Map;
class Timer;
class Controller
{
	friend class Actor;
public:
	Controller();
	virtual ~Controller();
	virtual void Update();
	virtual bool const IsFreeFlyCamera() const;
	void Possess(Actor* actor);
	Actor* GetActor() const;
	

protected:
	Timer* m_damageTimer = nullptr;
	ActorUID m_actorUID = ActorUID::INVALID;
	Map* m_map = nullptr;
};