#include "Game/Weapon.hpp"
#include "Game/Actor.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "Game/Map.hpp"
#include "Engine/Render/DebugRender.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/Game.hpp"
#include "Game/Player.hpp"
Weapon::Weapon(Actor* owner, std::string weaponName)
	:m_owner(owner)
{
	m_weaponDef = *WeaponDefinition::GetByName(weaponName);
	m_refireTimer = new Timer(m_weaponDef.m_refireTime, g_theGameClock);
	m_map = m_owner->m_map;
	if ((int)m_weaponDef.m_animations.size() == 2)
	{
		m_animationTimer = new Timer(m_weaponDef.m_animations[1]->GetDurationTime(), g_theGameClock);
	}
}

void Weapon::Fire(Actor* target)
{

	if (m_refireTimer->HasPeriodElapsed() || m_refireTimer->IsStopped())
	{
		m_refireTimer->Stop();



		if (m_weaponDef.m_name == "Pistol")
		{
			PistolFire();
		}
		else if (m_weaponDef.m_name == "PlasmaRifle")
		{
			PlasmaFire();
		}
		else if (m_weaponDef.m_name == "DemonMelee")
		{
			MeleeAttack();
		}
		else if (m_weaponDef.m_name == "NinjaMelee")
		{
			MeleeAttack();
		}
		else if (m_weaponDef.m_name == "Kama")
		{
			KamaFire(target);
		}
		else if (m_weaponDef.m_name == "CatGun")
		{
			CatFire(target);
		}
		else if (m_weaponDef.m_name == "SMG")
		{
			SMGFire();
		}
		m_refireTimer->Start();
		if (m_animationTimer)
		{
			m_animationTimer->Start();
		}
	}
}

Vec3 Weapon::GetRandomDirectionInCone()
{
	Vec3 defaultDirection = m_owner->m_orientation.GetForwardDir_XFwd_YLeft_ZUp();
	Vec3 startPosition = m_owner->m_position;
	float cone = 0.f;
	if (m_weaponDef.m_projectileCone != 0.f)
	{
		cone = m_weaponDef.m_projectileCone;
	}
	else if (m_weaponDef.m_rayCone != 0.f)
	{
		cone = m_weaponDef.m_rayCone;
	}
	if (cone == 0.f)
	{
		return defaultDirection;
	}
	EulerAngles orientation = m_owner->m_orientation;
	orientation.m_pitchDegrees = -m_owner->m_orientation.m_pitchDegrees;
	// Calculate random pitch and yaw within the cone angle
	float pitchConeRNG = g_rng->RollRandomFloatInRange(0.f, cone);
	float yawConeRNG = g_rng->RollRandomFloatInRange(0.f, cone);

	// Apply cone deviation to orientation
	orientation.m_pitchDegrees += pitchConeRNG;
	orientation.m_yawDegrees += yawConeRNG;

	// Calculate shortest angular displacement for pitch and yaw
	float shortestPitchDisp = GetShortestAngularDispDegrees(-m_owner->m_orientation.m_pitchDegrees, orientation.m_pitchDegrees);
	float shortestYawDisp = GetShortestAngularDispDegrees(m_owner->m_orientation.m_yawDegrees, orientation.m_yawDegrees);

	// Update orientation with shortest angular displacements
	orientation.m_pitchDegrees = orientation.m_pitchDegrees + shortestPitchDisp;
	orientation.m_yawDegrees = orientation.m_yawDegrees + shortestYawDisp;

	Vec3 direction = Vec3::MakeFromPolarDegrees(orientation.m_pitchDegrees, orientation.m_yawDegrees);
	return direction;
}

void Weapon::PlayAnimation()
{

}

void Weapon::PistolFire()
{
	m_owner->PlayActorSound(ST_FIRE);
	Vec3 direction = GetRandomDirectionInCone();
	Vec3 startPos = m_owner->GetEyePosition() - Vec3(0.f, 0.f, 0.05f);
	startPos = startPos + m_owner->m_actorDef.m_physicsRadius * direction;
	RaycastResult3D result = m_map->GetRayWeaponResultAndDealDamage(startPos, direction, m_weaponDef.m_rayRange, m_owner);
	if (result.m_didImpact)
	{
		//DebugAddWorldLine(startPos, result.m_impactPos, 0.01f, 1.f, RAYCOLOR, RAYCOLOR, DebugRenderMode::X_RAY);
		m_owner->PlayAnimation("Attack");
		Actor* targetActor = m_map->GetActorByUID(m_map->GetTargetDemonActorUID());
		if (targetActor)
		{

			targetActor->AddImpulse(direction * m_weaponDef.m_rayImpulse);
			float damage = g_rng->RollRandomFloatRange(m_weaponDef.m_rayDamage);
			SpawnInfo bloodInfo = SpawnInfo("BloodSplatter", result.m_impactPos, m_owner->m_orientation);

			m_map->SpawnActor(bloodInfo);
			// 					if (targetActor->m_actorDef.m_faction == ActorFaction::MARINE)
			// 					{
			// 						bloodInfoActor->m_notShownPlayerIndex = m_map->GetGame()->GetPlayerIndex(targetActor->m_uid);
			// 					}
			targetActor->Damage(damage, m_owner);
		}
		else
		{
			SpawnInfo bulletHitInfo = SpawnInfo("BulletHit", result.m_impactPos, m_owner->m_orientation);
			m_map->SpawnActor(bulletHitInfo);
		}
		// 				DebugAddWorldPoint(result.m_impactPos, 0.06f, 10.f);
		// 				DebugAddWorldArrow(result.m_impactPos, result.m_impactPos + 0.3f * result.m_impactNormal, 0.03f, 10.f, Rgba8::BLUE, Rgba8::BLUE);
	}
	else
	{
		Vec3 rayEndPos = startPos + direction * m_weaponDef.m_rayRange;
		DebugAddWorldLine(startPos, rayEndPos, 0.01f, 1.f, Rgba8(255, 0, 255, 255), Rgba8(255, 0, 255, 255), DebugRenderMode::X_RAY);
	}
}

void Weapon::PlasmaFire()
{
	m_owner->PlayActorSound(ST_FIRE);
	m_owner->PlayAnimation("Attack");
	Vec3 direction = GetRandomDirectionInCone();
	Vec3 startPos = m_owner->GetEyePosition() - Vec3(0.f, 0.f, 0.05f);
	startPos = startPos + direction * m_owner->m_actorDef.m_physicsRadius;
	SpawnInfo projectileActorInfo = SpawnInfo(m_weaponDef.m_projectileActor.c_str(), startPos, m_owner->m_orientation, direction * m_weaponDef.m_projectileSpeed, m_owner);
	m_map->SpawnActor(projectileActorInfo);
}

void Weapon::SMGFire()
{
	m_owner->PlayActorSound(ST_FIRE,1.5f);
	Vec3 direction = GetRandomDirectionInCone();
	Vec3 startPos = m_owner->GetEyePosition() - Vec3(0.f, 0.f, 0.05f);
	startPos = startPos + m_owner->m_actorDef.m_physicsRadius * direction;
	Vec3 recoilForce = -direction * m_weaponDef.m_recoilForce;
	m_owner->AddImpulse(recoilForce);
	if (m_owner->m_actorDef.m_isPlayer)
	{
		Player* player = m_map->GetGame()->GetPlayer(m_owner->m_uid);
		if (player)
		{
			player->ApplyRecoilEffect();
		}
	}
	RaycastResult3D result = m_map->GetRayWeaponResultAndDealDamage(startPos, direction, m_weaponDef.m_rayRange, m_owner);
	if (result.m_didImpact)
	{
		//DebugAddWorldLine(startPos, result.m_impactPos, 0.01f, 1.f, RAYCOLOR, RAYCOLOR, DebugRenderMode::X_RAY);
		m_owner->PlayAnimation("Attack");
		Actor* targetActor = m_map->GetActorByUID(m_map->GetTargetDemonActorUID());
		if (targetActor)
		{

			targetActor->AddImpulse(direction * m_weaponDef.m_rayImpulse);
			float damage = g_rng->RollRandomFloatRange(m_weaponDef.m_rayDamage);
			SpawnInfo bloodInfo = SpawnInfo("BloodSplatter", result.m_impactPos, m_owner->m_orientation);

			m_map->SpawnActor(bloodInfo);
			// 					if (targetActor->m_actorDef.m_faction == ActorFaction::MARINE)
			// 					{
			// 						bloodInfoActor->m_notShownPlayerIndex = m_map->GetGame()->GetPlayerIndex(targetActor->m_uid);
			// 					}
			targetActor->Damage(damage, m_owner);
		}
		else
		{
			SpawnInfo bulletHitInfo = SpawnInfo("BulletHit", result.m_impactPos, m_owner->m_orientation);
			m_map->SpawnActor(bulletHitInfo);
		}
		// 				DebugAddWorldPoint(result.m_impactPos, 0.06f, 10.f);
		// 				DebugAddWorldArrow(result.m_impactPos, result.m_impactPos + 0.3f * result.m_impactNormal, 0.03f, 10.f, Rgba8::BLUE, Rgba8::BLUE);
	}
	else
	{
		Vec3 rayEndPos = startPos + direction * m_weaponDef.m_rayRange;
		DebugAddWorldLine(startPos, rayEndPos, 0.01f, 1.f, Rgba8(255, 0, 255, 255), Rgba8(255, 0, 255, 255), DebugRenderMode::X_RAY);
	}
}

void Weapon::MeleeAttack()
{
	Vec2 fwd2D = m_owner->m_orientation.GetForwardXY();
	Actor* targetActor = m_map->GetActorByUID(m_map->GetTargetActorUID(m_weaponDef.m_meleeArc, m_weaponDef.m_meleeRange, fwd2D, m_owner->m_centerXY));
	if (targetActor)
	{
		m_owner->PlayActorSound(ST_FIRE);
		if (m_weaponDef.m_name=="NinjaMelee")
		{
			m_owner->PlayAnimation("Melee");
		}
		else
		{
			m_owner->PlayAnimation("Attack");
		}
		
		float damage = g_rng->RollRandomFloatRange(m_weaponDef.m_meleeDamage);
		targetActor->Damage(damage, m_owner);
		targetActor->AddImpulse(Vec3(fwd2D.x, fwd2D.y, 0.f).GetNormalized() * m_weaponDef.m_meleeImpulse);
	}
}

void Weapon::KamaFire(Actor* target)
{
	m_owner->PlayActorSound(ST_FIRE, 2.f);
	m_owner->PlayAnimation("Attack");
	Vec3 startPos = m_owner->GetEyePosition() - Vec3(0.f, 0.f, 0.05f);
	Vec3 direction = (target->GetEyePosition() - startPos).GetNormalized();
	m_owner->m_orientation = direction.GetEulerAngle();
	startPos = startPos + direction * m_owner->m_actorDef.m_physicsRadius;
	SpawnInfo projectileActorInfo = SpawnInfo(m_weaponDef.m_projectileActor.c_str(), startPos, m_owner->m_orientation, direction * m_weaponDef.m_projectileSpeed, m_owner);
	m_map->SpawnActor(projectileActorInfo);

}

void Weapon::CatFire(Actor* target)
{
	m_owner->PlayActorSound(ST_FIRE, 2.f);
	//m_owner->PlayAnimation("Attack");
	Vec3 startPos = m_owner->GetEyePosition();
	Vec3 direction = (target->GetEyePosition() - startPos).GetNormalized();
	//m_owner->m_orientation = direction.GetEulerAngle();
	startPos = startPos + direction * m_owner->m_actorDef.m_physicsRadius;
	SpawnInfo projectileActorInfo = SpawnInfo(m_weaponDef.m_projectileActor.c_str(), startPos, direction.GetEulerAngle(), direction * m_weaponDef.m_projectileSpeed, m_owner);
	m_map->SpawnActor(projectileActorInfo);
}
