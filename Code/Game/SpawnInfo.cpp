#include "Game/SpawnInfo.hpp"


SpawnInfo::SpawnInfo()
{

}

SpawnInfo::SpawnInfo(char const* actorDefName, Vec3 const& position, EulerAngles const& orientation, Vec3 const& velocity, Actor* owner)
	:m_actorDefName(actorDefName),
	m_position(position),
	m_orientation(orientation),
	m_velocity(velocity),
	m_owner(owner)
{
	
}

