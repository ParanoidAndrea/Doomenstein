#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Game/Actor.hpp"
#include <string>
struct SpawnInfo
{
public:
	SpawnInfo();
	SpawnInfo(char const* actorDefName, Vec3 const& position, EulerAngles const& orientation, Vec3 const& velocity = Vec3(0.f,0.f,0.f), Actor* owner = nullptr);

public:
	std::string m_actorDefName = "UNKNOWN";
	Vec3 m_position;
	EulerAngles m_orientation;
	Vec3 m_velocity;
	Actor* m_owner = nullptr;
};