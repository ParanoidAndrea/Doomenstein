#include "Game/Controller.hpp"
#include "Game/Map.hpp"
Controller::Controller()
{
}

Controller::~Controller()
{
}

void Controller::Update()
{
}

bool const Controller::IsFreeFlyCamera() const
{
	return false;
}

void Controller::Possess(Actor* actor)
{
	Actor* currentActor = GetActor();
	if (currentActor != nullptr)
	{
		currentActor->OnUnpossessed(this);
		m_actorUID = ActorUID::INVALID;
		m_map = nullptr;
	}
	if (actor == nullptr)
	{
		return;
	}
	if (!actor->m_actorDef.m_canBePossessed)
	{
		return;
	}

	m_actorUID = actor->m_uid;
	m_map = actor->m_map;
	actor->OnPossessed(this);
}

Actor* Controller::GetActor() const
{
	return m_map->GetActorByUID(m_actorUID);
}
