#include "AI.hpp"
#include "Game/Actor.hpp"
#include "Game/Map.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Render/DebugRender.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Game/Weapon.hpp"
#include "Game/Player.hpp"
AI::AI()
{

}

AI::~AI()
{
}

void AI::DamagedBy(Actor* actor)
{
	m_targetActorUID = actor->m_uid;
}

void AI::Update()
{
// 	if (GetActor()->m_actorDef.m_isNinja)
// 	{
// 		if (m_moveTimer->HasPeriodElapsed() || m_moveTimer->IsStopped())
// 		{
// 			m_moveTimer->Stop();
// 			GetActor()->m_orientation.m_yawDegrees += g_rng->RollRandomFloatInRange(-10.f, 10.f);
// 			Vec3 fwd = GetActor()->m_orientation.GetForwardDir_XFwd_YLeft_ZUp();
// 			GetActor()->MoveInDirection(fwd, GetActor()->m_actorDef.m_walkSpeed);
// 			m_moveTimer->Start();
// 		}
// 	}
	if (GetActor()->m_actorDef.m_isCat)
	{
		Player* player = m_map->GetPlayer(0);
		Vec3 targetPosition = player->m_position - player->m_orientation.GetJLeftNormal() * 1.f;
		Vec3 fwd = (targetPosition - GetActor()->m_position).GetNormalized();
		if ((targetPosition - GetActor()->m_position).GetLength() > 1.f)
		{
			GetActor()->m_orientation = fwd.GetEulerAngle();
			GetActor()->MoveInDirection(GetActor()->m_orientation.GetForwardDir_XFwd_YLeft_ZUp(), GetActor()->m_actorDef.m_walkSpeed);
		}
		if (m_targetActorUID.IsValid())
			{
				Actor* targetActor = m_map->GetActorByUID(m_targetActorUID);
				if (targetActor && !targetActor->m_isDead)
				{
					float targetDegree = (targetActor->m_centerXY - GetActor()->m_centerXY).GetOrientationDegrees();
					GetActor()->TurnInDirection(targetDegree, GetActor()->m_actorDef.m_turnSpeed * g_theGameClock->GetDeltaSeconds());
	
					Vec2 fwd2D = GetActor()->m_orientation.GetForwardXY();
					Actor* target = m_map->GetActorByUID(m_map->GetTargetActorUID(GetActor()->m_actorDef.m_sightAngle, GetActor()->m_actorDef.m_sightRadius, fwd2D, GetActor()->m_centerXY, ActorFaction::DEMON));
					if ((targetActor->m_position - GetActor()->m_position).GetLength() <= GetActor()->m_actorDef.m_sightRadius && target )
					{
						GetActor()->Attack(target);
					}
				}
	
			}
			else
			{
				Vec2 fwdXY = GetActor()->m_orientation.GetForwardXY();
				m_targetActorUID = m_map->GetTargetActorUID(GetActor()->m_actorDef.m_sightAngle, GetActor()->m_actorDef.m_sightRadius, fwdXY, GetActor()->m_centerXY);
			}
		}
	else if (!GetActor()->m_isDead && !GetActor()->m_isDestroyed)
	{
		if (m_targetActorUID.IsValid())
		{
			Actor* targetActor = m_map->GetActorByUID(m_targetActorUID);
			if (targetActor && !targetActor->m_isDead)
			{
				if (!GetActor()->m_actorDef.m_isNinja || GetActor()->GetEquippedWeapon()->m_weaponDef.m_name == "NinjaMelee")
				{
					float targetDegree = (targetActor->m_centerXY - GetActor()->m_centerXY).GetOrientationDegrees();
					GetActor()->TurnInDirection(targetDegree, GetActor()->m_actorDef.m_turnSpeed * g_theGameClock->GetDeltaSeconds());

					if ((targetActor->m_position - GetActor()->m_position).GetLength() > 1.f * (targetActor->m_radius + GetActor()->m_radius))
					{
						GetActor()->MoveInDirection(GetActor()->m_orientation.GetForwardDir_XFwd_YLeft_ZUp(), GetActor()->m_actorDef.m_runSpeed);
					}
					// 				if ((targetActor->m_position - GetActor()->m_position).GetLength() > 1.2f * (targetActor->m_radius + GetActor()->m_radius))
					// 				{
					// 					GetActor()->MoveInDirection(dir, GetActor()->m_actorDef.m_walkSpeed);
					// 				}
					GetActor()->Attack();
				}
				else
				{
					float targetDegree = (targetActor->m_centerXY - GetActor()->m_centerXY).GetOrientationDegrees();
					GetActor()->TurnInDirection(targetDegree, GetActor()->m_actorDef.m_turnSpeed * g_theGameClock->GetDeltaSeconds());
				
					Vec2 fwd2D = GetActor()->m_orientation.GetForwardXY();
					Actor* target = m_map->GetActorByUID(m_map->GetTargetActorUID(GetActor()->m_actorDef.m_sightAngle, GetActor()->m_actorDef.m_sightRadius, fwd2D, GetActor()->m_centerXY));
					if ((targetActor->m_position - GetActor()->m_position).GetLength() > GetActor()->m_actorDef.m_sightRadius || target==nullptr)
					{
						GetActor()->MoveInDirection(GetActor()->m_orientation.GetForwardDir_XFwd_YLeft_ZUp(), GetActor()->m_actorDef.m_walkSpeed);
					}
					else if (target)
					{
						GetActor()->Attack(target);
					}
				}

			}
			else if (targetActor &&targetActor->m_isDead)
			{
				m_targetActorUID = ActorUID::INVALID;
			}
			else
			{
				m_targetActorUID = ActorUID::INVALID;
			}
		}
		else
		{
			Vec2 fwdXY = GetActor()->m_orientation.GetForwardXY();

			m_targetActorUID = m_map->GetTargetActorUID(GetActor()->m_actorDef.m_sightAngle, GetActor()->m_actorDef.m_sightRadius, fwdXY, GetActor()->m_centerXY);
// 			if (m_targetActorUID.IsValid())
// 			{
// 				DebugAddWorldLine(GetActor()->m_position, m_map->GetActorByUID(m_targetActorUID)->m_position, 0.1f, 5.f);
// 				DebugAddMessage(Stringf("x:%.1f, y:%.1f", GetActor()->m_centerXY.x, GetActor()->m_centerXY.y), 5.f);
// 				DebugAddMessage(Stringf("x:%.1f, y:%.1f", GetActor()->m_position.x, GetActor()->m_position.y), 5.f);
// 			}
		}
	}
}

void AI::CatUpdate()
{

}
