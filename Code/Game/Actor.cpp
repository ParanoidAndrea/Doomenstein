#include "Game/Actor.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Render/Renderer.hpp"
#include "Game/SpawnInfo.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/Controller.hpp"
#include "Game/Weapon.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Game/AI.hpp"
#include "Engine/Render/SpriteAnimationGroupDefinition.hpp"
#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include "Game/Player.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Render/DebugRender.hpp"
Actor::Actor(Map* owner, SpawnInfo const& spawnInfo)
	:m_map(owner)
{

	m_actorDef = *ActorDefinition::GetByName(spawnInfo.m_actorDefName);
	m_position = spawnInfo.m_position;
	m_orientation = spawnInfo.m_orientation;
	m_velocity = spawnInfo.m_velocity;
	m_owner = spawnInfo.m_owner;
	m_color = m_actorDef.m_solidColor;
	m_lighterColor = m_actorDef.m_wireframeColor;
	m_height = m_actorDef.m_physicsHeight;
	m_radius = m_actorDef.m_physicsRadius;
	m_health = m_actorDef.m_health;
	m_dieOnCollide = m_actorDef.m_dieOnCollide;
	m_corpsetimeTimer = new Timer(m_actorDef.m_corpseLifetime, g_theGameClock);
	m_footStepTimer = new Timer(0.5f, g_theGameClock);
// 	AddVertsForCylinder3D(m_vertexes, Vec3(), Vec3(0.f, 0.f, m_height), m_radius, Rgba8::WHITE, Vec2(), Vec2(1.f, 1.f), 16);
// 	if (m_actorDef.m_renderRounded)
// 	{
// 		m_fwd = m_orientation.GetForwardDir_XFwd_YLeft_ZUp();
// 		AddVertsForCone3D(m_vertexes, Vec3(0.f, 0.f, m_actorDef.m_eyeHeight) + Vec3(1.f, 0.f, 0.f) * (m_radius), Vec3(0.f, 0.f, m_actorDef.m_eyeHeight) + Vec3(1.f, 0.f, 0.f) * (m_radius + 0.15f), 0.08f, Rgba8::WHITE, Vec2(), Vec2(1.f, 1.f), 16);
// 	}
	for (int i = 0; i < (int)m_actorDef.m_weaponNames.size(); ++i)
	{
		Weapon* weapon = new Weapon(this, m_actorDef.m_weaponNames[i]);
		m_weapons.push_back(weapon);
	}
	m_minMaxZ = FloatRange(m_position.z, m_position.z + m_height);
	m_centerXY = Vec2(m_position.x, m_position.y);
	m_animationClock = new Clock(*g_theGameClock);
	PlayDefaultAnimation();
}

Actor::~Actor()
{
}

void Actor::Update(float deltaSeconds)
{
	if (m_actorDef.m_dieOnSpawn)
	{
		Die(nullptr);
	}

	if (IsAIControlling())
	{
		m_aiController->Update();
	}
	if (m_hasCollideWithWorld && !m_isDead)
	{
		Die(nullptr);
	}

	if (m_animationGroup->m_playBackMode == SpriteAnimPlaybackType::ONCE
		&& m_animationGroup->m_animations[0]->IsAnimationFinished(m_animationClock->GetTotalSeconds()))
	{
		if (m_animationGroup->m_name.compare("Death") == 0)
		{
			m_isVisble = false;
		}
		PlayDefaultAnimation();
	}
	if (m_animationGroup->m_scaleBySpeed)
	{
		m_animationClock->SetTimeScale(m_velocity.GetLength() / m_actorDef.m_runSpeed);
	}
	else
	{
		m_animationClock->SetTimeScale(1.f);
	}

	if (m_actorDef.m_simulated && !m_isDead && !m_isDestroyed)
	{
		UpdatePhysics(deltaSeconds);
	}


	m_minMaxZ = FloatRange(m_position.z, m_position.z + m_height);
	m_centerXY = Vec2(m_position.x, m_position.y);

	if (m_corpsetimeTimer->HasPeriodElapsed())
	{
		m_isDestroyed = true;
		m_corpsetimeTimer->Stop();
	}
	//DebugAddWorldArrow(m_position, m_position + m_orientation.GetForwardDir_XFwd_YLeft_ZUp(), 0.1f, deltaSeconds);
}

void Actor::UpdateSoundsPosition()
{
	g_theAudio->SetSoundPosition(m_hurtSound, m_position);
	g_theAudio->SetSoundPosition(m_deathSound, m_position);
	g_theAudio->SetSoundPosition(GetEquippedWeapon()->m_fireSound, m_position);
}

void Actor::UpdatePhysics(float deltaSeconds)
{
	Vec3 dragForce = m_actorDef.m_drag * (-m_velocity);
	AddForce(dragForce);
	m_velocity += m_acceleration * deltaSeconds;;
	m_position += m_velocity *deltaSeconds;
	if (m_footStepTimer->HasPeriodElapsed())
	{
		PlayActorSound(ST_WALK);
		m_footStepTimer->Stop();
	}
	if (m_velocity.GetLength() > 0.25f && m_footStepTimer->IsStopped())
	{
		m_footStepTimer->Start();
	}
	
	if (m_actorDef.m_isNinja && m_velocity.GetLength() > 0.5f && m_animationGroup->m_name != "Attack" && m_animationGroup->m_name!= "Hurt" && m_animationGroup->m_name != "Melee")
	{
		PlayAnimation("Walk");
	}
	else if (m_actorDef.m_isNinja && m_velocity.GetLength() <=0.5f && m_animationGroup->m_name != "Attack" && m_animationGroup->m_name != "Hurt" && m_animationGroup->m_name != "Melee" )
	{
		if (m_equippedWeaponIndex == 1)
		{
			PlayAnimation("MeleeStand");
		}
		else
		{
			PlayAnimation("Idle");
		}
	}
// 	else if (m_actorDef.m_isNinja && m_velocity.GetLength() <= 0.5f && m_animationGroup->m_name != "Attack" && m_animationGroup->m_name != "Hurt" && m_animationGroup->m_name != "Melee" && m_equippedWeaponIndex == 1)
// 	{
// 		PlayAnimation("MeleeStand");
// 	}
	if (!m_actorDef.m_flying)
	{
		m_position.z = 0.f;
	}
	m_acceleration = Vec3();
}

void Actor::Render(int playerIndex) const
{
	if (!m_isDestroyed && m_isVisble)
	{
		if ((m_controller == nullptr||IsAIControlling()|| m_controller->IsFreeFlyCamera()) || m_map->GetGame()->GetActorUID(playerIndex)!=m_uid)
	
		{
			Vec3 referDir = m_map->GetCameraDirection(m_position, playerIndex);
			referDir = GetModelMatrix().GetOrthonormalInverse().TransformVectorQuantity3D(referDir);
			SpriteAnimDefinition* anima = m_animationGroup->GetAnimationBasedOnDirection(referDir);
			SpriteDefinition spriteDefinition = anima->GetSpriteDefAtTime(m_animationClock->GetTotalSeconds());
			AABB2 uv = spriteDefinition.GetUVs();

			AABB2 bounds = AABB2(Vec2(), m_animationGroup->m_size);
			bounds.Translate(-m_actorDef.m_pivot * m_animationGroup->m_size);
			if (m_actorDef.m_renderLit)
			{
				std::vector<Vertex_PCUTBN> verts;
				if (m_actorDef.m_renderRounded)
				{
					AddVertsForRoundedQuad3D(verts, Vec3(0.f, bounds.m_mins.x, bounds.m_mins.y), Vec3(0.f, bounds.m_maxs.x, bounds.m_mins.y), Vec3(0.f, bounds.m_maxs.x, bounds.m_maxs.y), Vec3(0.f, bounds.m_mins.x, bounds.m_maxs.y), Rgba8::WHITE, uv);

				}
				else
				{
					AddVertsForQuad3D(verts, Vec3(0.f, bounds.m_mins.x, bounds.m_mins.y), Vec3(0.f, bounds.m_maxs.x, bounds.m_mins.y), Vec3(0.f, bounds.m_maxs.x, bounds.m_maxs.y), Vec3(0.f, bounds.m_mins.x, bounds.m_maxs.y), Rgba8::WHITE, uv);
				}
				g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
				g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
				g_theRenderer->SetDepthMode(DepthMode::ENABLED);
				g_theRenderer->BindTexture(&spriteDefinition.GetTexture());
				g_theRenderer->BindShader(m_map->m_shader);
				if (m_actorDef.m_billboardType != BillboardType::NONE)
				{
					Mat44 billboardMatrix = GetBillboardMatrix(m_actorDef.m_billboardType, m_map->GetCameraModelMatrix(playerIndex), m_position);
					g_theRenderer->SetModelConstants(billboardMatrix, Rgba8::WHITE);
				}
				else
				{
					g_theRenderer->SetModelConstants(GetRenderModelMatrix(), Rgba8::WHITE);
				}

				g_theRenderer->DrawVertexArray((int)verts.size(), verts.data());
			}
			else
			{
				std::vector<Vertex_PCU> verts;

				AddVertsForQuad3D(verts, Vec3(0.f, bounds.m_mins.x, bounds.m_mins.y), Vec3(0.f, bounds.m_maxs.x, bounds.m_mins.y), Vec3(0.f, bounds.m_maxs.x, bounds.m_maxs.y), Vec3(0.f, bounds.m_mins.x, bounds.m_maxs.y), Rgba8::WHITE, uv);

				g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
				g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
				g_theRenderer->SetDepthMode(DepthMode::ENABLED);
				g_theRenderer->BindTexture(&spriteDefinition.GetTexture());
				g_theRenderer->BindShader(nullptr);

				if (m_actorDef.m_billboardType != BillboardType::NONE)
				{
					Mat44 billboardMatrix = GetBillboardMatrix(m_actorDef.m_billboardType, m_map->GetCameraModelMatrix(playerIndex), m_position);
					g_theRenderer->SetModelConstants(billboardMatrix, Rgba8::WHITE);
				}
				else
				{
					g_theRenderer->SetModelConstants(GetRenderModelMatrix(), Rgba8::WHITE);
				}

				g_theRenderer->DrawVertexArray((int)verts.size(), verts.data());
			}

// 			g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
// 			g_theRenderer->SetRasterizerMode(RasterizerMode::WIREFRAME_CULL_BACK);
// 			g_theRenderer->BindTexture(nullptr);
// 			g_theRenderer->BindShader(nullptr);
// 
// 			g_theRenderer->SetModelConstants(GetRenderModelMatrix(), m_lighterColor);
// 			g_theRenderer->DrawVertexArray((int)m_vertexes.size(), m_vertexes.data());
			

		}
	}
}

void Actor::AddForce(const Vec3& force)
{
	m_acceleration += force;
}

void Actor::AddImpulse(Vec3 const& impulse)
{
	m_velocity += impulse;
}

void Actor::OnCollide(Actor* other)
{
	if ((other->m_actorDef.m_faction != m_actorDef.m_faction)||!m_map->GetGame()->m_isSinglePlayer)
	{
		float damage = g_rng->RollRandomFloatRange(m_actorDef.m_damageOnCollide);
		other->AddImpulse(m_velocity.GetNormalized() * m_actorDef.m_impulseOnCollide);
		other->Damage(damage, this->m_owner);
	}
}

void Actor::Damage(float damage, Actor* source)
{
	if (!m_isDead)
	{
		m_health -= damage;
		if (m_actorDef.m_isPlayer)
		{
			m_controller->m_damageTimer->Start();
		}
		if (m_actorDef.m_isCat)
		{
			PlayActorSound(ST_HURT,3.f);
		}
		else
		{
			PlayActorSound(ST_HURT, 1.f);
		}
		PlayAnimation("Hurt");

		if (m_health <= 0)
		{
			if (m_actorDef.m_isPlayer)
			{
				m_map->GetGame()->GetPlayer(m_uid)->m_deaths++;
				if (source->m_actorDef.m_isPlayer)
				{
					m_map->GetGame()->GetPlayer(source->m_uid)->m_kills++;
				}
			}

			Die(source);
		}
		if (IsAIControlling())
		{
			m_aiController->DamagedBy(source);

		}
	}
}

void Actor::Die(Actor* source)
{
	UNUSED(source);
	if (!m_isDead)
	{
		m_color -= Rgba8(150,150,150,100);
		m_isDead = true;
		PlayAnimation("Death");
		PlayActorSound(ST_DEATH);
		if (m_corpsetimeTimer->IsStopped())
		{
			m_corpsetimeTimer->Start();
		}
	}
}

void Actor::OnPossessed(Controller* controller)
{
	m_controller = controller;

}

void Actor::OnUnpossessed(Controller* controller)
{
	if (m_controller == controller)
	{
		m_controller = nullptr;

	}
	if (m_aiController && !m_controller)
	{
		m_controller = m_aiController;
	}
}

void Actor::MoveInDirection(Vec3 direction, float speed)
{

	direction = direction.GetNormalized();
	Vec3 force = direction * (speed * m_actorDef.m_drag);
	AddForce(force);
}

void Actor::TurnInDirection(float degrees, float maxAngle)
{
	m_orientation.m_yawDegrees = GetTurnedTowardDegrees(m_orientation.m_yawDegrees, degrees, maxAngle);
	//AddForce(m_orientation.GetForwardDir_XFwd_YLeft_ZUp()*maxAngle);
}

Weapon* Actor::GetEquippedWeapon()
{
	return m_weapons[m_equippedWeaponIndex];
}

void Actor::EquipWeapon(int weaponIndex)
{
	m_equippedWeaponIndex = weaponIndex;
}

void Actor::EquipNextWeapon()
{
	if (m_equippedWeaponIndex == (int)m_weapons.size() - 1)
	{
		m_equippedWeaponIndex = 0;
	}
	else
	{
		m_equippedWeaponIndex++;
	}
}

void Actor::EquipPreviousWeapon()
{
	if (m_equippedWeaponIndex == 0)
	{
		m_equippedWeaponIndex = (int)m_weapons.size() - 1;
	}
	else
	{
		m_equippedWeaponIndex--;
	}
}

void Actor::Attack(Actor* target)
{

	GetEquippedWeapon()->Fire(target);
	
}

void Actor::PlayDefaultAnimation()
{
	if (m_animationGroup != m_actorDef.m_animationGroups[0])
	{
		m_animationGroup = m_actorDef.m_animationGroups[0];
		m_animationClock->Reset();
	}

}

void Actor::PlayAnimation(std::string const& name)
{

	for (int i = 0; i < (int)m_actorDef.m_animationGroups.size(); ++i)
	{
		if (m_actorDef.m_animationGroups[i]->m_name.compare(name) == 0)
		{
			if (m_animationGroup != m_actorDef.m_animationGroups[i])
			{
				m_animationGroup = m_actorDef.m_animationGroups[i];
				m_animationClock->Reset();
			}
			break;
		}
	}
}

Mat44 const Actor::GetModelMatrix() const
{
	Mat44 model;
	model = m_orientation.GetAsMatrix_IFwd_JLeft_KUp();
	model.SetTranslation3D(m_position);
	return model;
}

Mat44 const Actor::GetRenderModelMatrix() const
{
	Mat44 model;
	EulerAngles renderOrientation;
	renderOrientation.m_yawDegrees = m_orientation.m_yawDegrees;
	model = renderOrientation.GetAsMatrix_IFwd_JLeft_KUp();
	model.SetTranslation3D(m_position);
	return model;
}

Vec3 const Actor::GetEyePosition() const
{
	return Vec3(m_position.x, m_position.y, m_actorDef.m_eyeHeight);
}

void Actor::PlayActorSound(SoundType type, float volume)
{
	switch (type)
	{
	case ST_HURT:
		if (m_actorDef.m_hurtSoundID != 0)
		{
			m_hurtSound = g_theAudio->StartSoundAt(m_actorDef.m_hurtSoundID, m_position, false, volume);
		}
		break;
	case ST_DEATH:
		g_theAudio->StopSound(m_hurtSound);
		if (m_actorDef.m_deathSoundID != 0)
		{
			m_deathSound = g_theAudio->StartSoundAt(m_actorDef.m_deathSoundID, m_position, false, volume);
		}
		break;
	case ST_FIRE:
		if (GetEquippedWeapon()->m_weaponDef.m_fireSoundID != 0)
		{
			GetEquippedWeapon()->m_fireSound = g_theAudio->StartSoundAt(GetEquippedWeapon()->m_weaponDef.m_fireSoundID, m_position, false, volume);
		}
		break;
	case ST_WALK:
		g_theAudio->StopSound(m_walkSound);
		if (m_actorDef.m_footstepID != 0)
		{
			m_walkSound = g_theAudio->StartSoundAt(m_actorDef.m_footstepID, m_position);
		}
		break;
	}
}




bool Actor::IsAIControlling() const
{

	return (m_actorDef.m_aiEnabled && m_controller == m_aiController);
}

