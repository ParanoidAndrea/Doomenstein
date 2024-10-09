#include "Player.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/Map.hpp"
#include "Game/Actor.hpp"
#include "Engine/Render/DebugRender.hpp"
#include "Game/Game.hpp"
#include "Engine/Core/Timer.hpp"
#include "Game/Weapon.hpp"
#include "Engine/Render/SpriteAnimDefinition.hpp"
#include "Engine/Render/BitmapFont.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
Player::Player(Game* game, int playerIndex, int controllerIndex, int kill, int dealth)
	:m_game(game),
	m_playerIndex(playerIndex),
	m_controllerIndex(controllerIndex),
	m_deaths(dealth),
	m_kills(kill)
{
	m_damageTimer = new Timer(0.5f, g_theGameClock);
	m_damageTimer->Stop();
	m_worldCamera.SetPerspectiveView(2.f, 60.f, 0.1f, 100.f);
	if (!m_game->m_isSinglePlayer)
	{
		m_worldCamera.SetPerspectiveView(4.f, 60.f, 0.1f, 100.f);
		m_screenCamera.SetOrthoView(Vec2(), Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y / 2.f));
		if (playerIndex == 0)
		{
			m_worldCamera.SetNormalizedViewport(AABB2(0.f, 0.5f, 1.f, 1.f));
			m_screenCamera.SetNormalizedViewport(AABB2(0.f, 0.5f, 1.f, 1.f));
		}
		else if (playerIndex == 1)
		{
			m_worldCamera.SetNormalizedViewport(AABB2(0.f, 0.f, 1.f, 0.5f));
			m_screenCamera.SetNormalizedViewport(AABB2(0.f, 0.f, 1.f, 0.5f));
		}
	}
}

Player::~Player()
{
}

void Player::Update(float deltaSeconds)
{
	deltaSeconds = Clock::GetSystemClock().GetDeltaSeconds();

	if (m_map->m_freeFlyCameraMode)
	{
		UpdateFreeFlyCamera(deltaSeconds);
	}
	else
	{
		if (m_controllerIndex == -1)
		{
			UpdateActorCameraMNK();
		}
		else if (m_controllerIndex == 0)
		{
			UpdateActorCameraController();
		}

	}
	if (m_recoilRecoveryTimer)
	{
		if (m_recoilRecoveryTimer->HasPeriodElapsed())
		{
			float recoveryFraction = m_recoilRecoveryTimer->GetElapsedFraction();
			EulerAngles recoveryOffset(GetActor()->GetEquippedWeapon()->m_weaponDef.m_recoilAmount * recoveryFraction,0.f, 0.f);
			m_orientation -= recoveryOffset;
			m_recoilRecoveryTimer=nullptr;
		}
	}
	InputActions();

	UpdateListener();
	if (GetActor()->m_isDead && !m_map->m_freeFlyCameraMode && GetActor()->m_actorDef.m_faction == ActorFaction::MARINE &&!GetActor()->m_corpsetimeTimer->HasPeriodElapsed())
	{
		m_position.z -= 0.1f * deltaSeconds;
	}
	if (GetActor()->m_corpsetimeTimer->HasPeriodElapsed())
	{
		GetActor()->m_isDestroyed = true;
		if (m_game->m_isSinglePlayer)
		{
			m_map->EnterLose();
		}
		m_map->SpawnPlayerToNewPosition(m_playerIndex, m_controllerIndex, m_kills, m_deaths);
	}
	m_worldCamera.SetTransform(m_position, m_orientation);

	if (m_speed == GetActor()->m_actorDef.m_runSpeed && GetActor()->m_acceleration.GetLengthSquared() >=0.25f)
	{
		m_screenShakeAmount = min(m_screenShakeAmount + SCREEN_SHAKE_INCREASE_PER_SEC * deltaSeconds, MAX_SCREEN_SHAKE);
		UpdateScreenShake();
	}
	else
	{
		// Reduce screen shake when not sprinting
		m_screenShakeAmount -= SCREEN_SHAKE_REDUCTION_PER_SEC * deltaSeconds;
		m_screenShakeAmount = max(m_screenShakeAmount, 0.f);
		if (m_screenShakeAmount > 0)
		{
			UpdateScreenShake();
		}
	}

}

void Player::RenderScreen() const
{
	Weapon* weapon = GetActor()->GetEquippedWeapon();
	if (m_damageTimer->GetElapsedFraction()>0.5f && m_damageTimer->GetElapsedFraction() < 1.f && !m_damageTimer->IsStopped())
	{
		std::vector<Vertex_PCU> damageVerts;
		AABB2 bounds= m_screenCamera.GetCameraBounds();
		g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
		g_theRenderer->SetDepthMode(DepthMode::DISABLED);
		g_theRenderer->BindShader(nullptr);
		g_theRenderer->SetModelConstants();
		AddVertsForAABB2D(damageVerts, bounds, Rgba8(255, 0, 0, 50));
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->DrawVertexArray((int)damageVerts.size(), damageVerts.data());
	}
	if (weapon && weapon->m_weaponDef.m_baseTexture && !m_map->m_freeFlyCameraMode && !m_map->m_isHUDHidden)
	{
		AABB2 cameraBounds = m_screenCamera.GetCameraBounds();
		Vec2 cameraDimensions = Vec2(cameraBounds.m_maxs.x - cameraBounds.m_mins.x, cameraBounds.m_maxs.y - cameraBounds.m_mins.y);

		g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
		g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
		g_theRenderer->SetDepthMode(DepthMode::DISABLED);
		g_theRenderer->BindShader(nullptr);
		g_theRenderer->SetModelConstants();
		//BaseHUD
		AABB2 baseBounds = AABB2(cameraBounds.m_mins, Vec2(cameraBounds.m_maxs.x, cameraBounds.m_mins.y + cameraDimensions.y* 0.15f));
		std::vector <Vertex_PCU> baseVerts;
		AddVertsForAABB2D(baseVerts, baseBounds, Rgba8::WHITE);
		g_theRenderer->BindTexture(weapon->m_weaponDef.m_baseTexture);
		g_theRenderer->DrawVertexArray((int)baseVerts.size(), baseVerts.data());

		//The profile
		AABB2 profileBounds = AABB2(cameraBounds.m_mins.x + cameraDimensions.x * 0.5f - baseBounds.GetDimensions().y * 0.5f, cameraBounds.m_mins.y, cameraBounds.m_mins.x + cameraDimensions.x * 0.5f + baseBounds.GetDimensions().y * 0.5f, baseBounds.GetDimensions().y);
		std::vector <Vertex_PCU> profileVerts;
		AddVertsForAABB2D(profileVerts, profileBounds, Rgba8::WHITE);
		g_theRenderer->BindTexture(g_textures[TT_MARINE]);
		g_theRenderer->DrawVertexArray((int)profileVerts.size(), profileVerts.data());

		if (!GetActor()->m_isDead)
		{
			//Weapon sprite
			std::vector<Vertex_PCU> spriteVerts;
			AABB2 spriteBounds = AABB2(Vec2(), weapon->m_weaponDef.m_spriteSize);
			if (m_game->m_isSinglePlayer)
			{
				spriteBounds.Translate(Vec2(cameraBounds.m_maxs.x * 0.5f, cameraBounds.m_mins.y + cameraDimensions.y * 0.15f));
				spriteBounds.Translate(-weapon->m_weaponDef.m_spritePivot * weapon->m_weaponDef.m_spriteSize);
			}
			else 
			{
				spriteBounds = AABB2(Vec2(), weapon->m_weaponDef.m_spriteSize * 0.75f);
				spriteBounds.Translate(Vec2(cameraBounds.m_maxs.x * 0.5f, cameraBounds.m_mins.y + cameraDimensions.y * 0.15f));
				spriteBounds.Translate(-weapon->m_weaponDef.m_spritePivot * weapon->m_weaponDef.m_spriteSize*0.75f);
			}
			if (weapon->m_animationTimer->IsStopped() || weapon->m_animationTimer->HasPeriodElapsed())
			{
				SpriteAnimDefinition* animation = weapon->m_weaponDef.m_animations[0];
				SpriteDefinition      spriteDefinition = animation->GetSpriteDefAtTime(0.f);
				AABB2 spriteUV = spriteDefinition.GetUVs();
				AddVertsForAABB2D(spriteVerts, spriteBounds, Rgba8::WHITE, spriteUV.m_mins, spriteUV.m_maxs);
				g_theRenderer->BindTexture(&spriteDefinition.GetTexture());
			}
			else
			{
				SpriteAnimDefinition* animation = weapon->m_weaponDef.m_animations[1];
				SpriteDefinition spriteDefinition = animation->GetSpriteDefAtTime(weapon->m_animationTimer->GetElapsedTime());
				AABB2 spriteUV = spriteDefinition.GetUVs();
				AddVertsForAABB2D(spriteVerts, spriteBounds, Rgba8::WHITE, spriteUV.m_mins, spriteUV.m_maxs);
				g_theRenderer->BindTexture(&spriteDefinition.GetTexture());
			}

			g_theRenderer->DrawVertexArray((int)spriteVerts.size(), spriteVerts.data());
			//Reticle
			std::vector<Vertex_PCU> reticleVerts;

			AABB2 reticleBounds = AABB2(-weapon->m_weaponDef.m_reticleSize * 0.5f, weapon->m_weaponDef.m_reticleSize * 0.5f);
			reticleBounds.Translate(m_screenCamera.GetCameraCenter());
			AddVertsForAABB2D(reticleVerts, reticleBounds, Rgba8::WHITE);
			g_theRenderer->BindTexture(weapon->m_weaponDef.m_reticleTexture);
			g_theRenderer->DrawVertexArray((int)reticleVerts.size(), reticleVerts.data());
		}
		//Kills
		std::vector<Vertex_PCU> textVerts;
		g_bitmapFont->AddVertsForTextBox2D(textVerts, AABB2(cameraBounds.m_maxs.x * 0.02f, cameraBounds.m_mins.y + cameraDimensions.y * 0.05f, cameraBounds.m_maxs.x * 0.12f, cameraBounds.m_mins.y + cameraDimensions.y * 0.1f), cameraDimensions.y * 0.05f, Stringf("%d", m_kills));
		//Health
		g_bitmapFont->AddVertsForTextBox2D(textVerts, AABB2(cameraBounds.m_maxs.x * 0.25f, cameraBounds.m_mins.y + cameraDimensions.y * 0.05f, cameraBounds.m_maxs.x * 0.35f, cameraBounds.m_mins.y + cameraDimensions.y * 0.1f), cameraDimensions.y * 0.05f, Stringf("%.0f", GetActor()->m_health));

		g_bitmapFont->AddVertsForTextBox2D(textVerts, AABB2(cameraBounds.m_maxs.x * 0.88f, cameraBounds.m_mins.y + cameraDimensions.y * 0.05f, cameraBounds.m_maxs.x * 0.98f, cameraBounds.m_mins.y + cameraDimensions.y * 0.1f), cameraDimensions.y * 0.05f, Stringf("%d", m_deaths));

		g_theRenderer->BindTexture(&g_bitmapFont->GetTexture());
		g_theRenderer->DrawVertexArray((int)textVerts.size(), textVerts.data());


	}
	if (GetActor() && GetActor()->m_isDead && !m_map->m_freeFlyCameraMode)
	{
		std::vector<Vertex_PCU> blurVerts;
		AddVertsForAABB2D(blurVerts, m_screenCamera.GetCameraBounds(), Rgba8(0, 0, 0, 125));
		g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->DrawVertexArray((int)blurVerts.size(), blurVerts.data());
	}
}

bool const Player::IsFreeFlyCamera() const
{
	return m_map->m_freeFlyCameraMode;
}

void Player::UpdateFreeFlyCamera(float deltaSeconds)
{

	Vec2 cursorDelta = g_theInput->GetCursorClientDelta();
	m_orientation.m_pitchDegrees += 0.075f* cursorDelta.y;
	m_orientation.m_yawDegrees -= 0.075f * cursorDelta.x;

	if (g_theInput->IsKeyDown(KEYCODE_SHIFT) || g_controller.IsButtonDown(A))
	{
		deltaSeconds = 15.f * deltaSeconds;
	}
	
	if (g_controller.IsConnected())
	{
		Vec2 rightStickInput = g_controller.GetRightStick().GetPosition();
		float maxTurnRate = 180.f; 
		float maxTurnThisFrame = maxTurnRate * deltaSeconds;
		m_orientation.m_pitchDegrees -= GetClamped(rightStickInput.y * maxTurnThisFrame, -maxTurnThisFrame, maxTurnThisFrame);
		m_orientation.m_yawDegrees -= GetClamped(rightStickInput.x * maxTurnThisFrame, -maxTurnThisFrame, maxTurnThisFrame);

	}

// 	if (g_theInput->IsKeyDown('Q')||g_controller.GetLeftTrigger()>0.1f)
// 	{
// 		m_orientation.m_rollDegrees -= PLAYER_ROTATESPEED * deltaSeconds;
// 	}
// 	else if (g_theInput->IsKeyDown('E') || g_controller.GetRightTrigger() > 0.1f)
// 	{
// 		m_orientation.m_rollDegrees += PLAYER_ROTATESPEED * deltaSeconds;
// 	}
	m_orientation.m_pitchDegrees = GetClamped(m_orientation.m_pitchDegrees, -85.f, 85.f);
	m_orientation.m_rollDegrees = GetClamped(m_orientation.m_rollDegrees, -45.f, 45.f);

	Vec3 iForwardBasis, jLeftBasis, kUpBasis;
	m_orientation.GetAsVectors_IFwd_JLeft_KUp(iForwardBasis, jLeftBasis, kUpBasis);

	if (g_controller.IsConnected())
	{

		m_position += 2.f * iForwardBasis * g_controller.GetLeftStick().GetPosition().y * deltaSeconds;
		m_position -= 2.f * jLeftBasis * g_controller.GetLeftStick().GetPosition().x * deltaSeconds;

	}
	if (g_theInput->IsKeyDown('W'))
	{
		m_position += iForwardBasis * PLAYER_MOVESPEED * deltaSeconds;
	}
	
	if (g_theInput->IsKeyDown('S'))
	{
		m_position -= iForwardBasis * PLAYER_MOVESPEED * deltaSeconds;
	}
	
	if (g_theInput->IsKeyDown('A'))
	{
		m_position += jLeftBasis * PLAYER_MOVESPEED * deltaSeconds;
	}
	
	if (g_theInput->IsKeyDown('D'))
	{
		m_position -= jLeftBasis * PLAYER_MOVESPEED * deltaSeconds;
	}
	
	if (g_theInput->IsKeyDown('Z') || g_controller.GetButton(SHOUDLER_LEFT).m_isPressed)
	{
		m_position -= Vec3(0.f,0.f,1.f) * PLAYER_MOVESPEED * deltaSeconds;
	}
	
	if (g_theInput->IsKeyDown('C') || g_controller.GetButton(SHOUDLER_RIGHT).m_isPressed)
	{
		m_position += Vec3(0.f, 0.f, 1.f) * PLAYER_MOVESPEED * deltaSeconds;
	}
	DebugAddScreenText(Stringf("[F] Camera mode: Free-Fly"), AABB2(10.f, 760.f, 600.f, 780.f), 20.f, Vec2(), -1.f);
	

	
}

void Player::InputActions()
{
	if (GetActor()->m_actorDef.m_faction == ActorFaction::MARINE && m_controllerIndex == 0) //Marine player 
	{
		if (g_controller.GetRightTrigger() > 0.1f)
		{
			GetActor()->Attack();
		}
		if (g_controller.WasButtonJustPressed(DPAD_UP))
		{
			GetActor()->EquipPreviousWeapon();
		}
		if ( g_controller.WasButtonJustPressed(DPAD_DOWN))
		{
			GetActor()->EquipNextWeapon();
		}
		if (g_controller.WasButtonJustPressed(X))
		{
			GetActor()->EquipWeapon(0);
		}
		if (g_controller.WasButtonJustPressed(Y))
		{
			GetActor()->EquipWeapon(1);
		}
	}
	if (GetActor()->m_actorDef.m_faction == ActorFaction::MARINE && m_controllerIndex == -1) //Marine player 
	{
		if (g_theInput->IsKeyDown(KEYCODE_LEFT_MOUSE) )
		{
			GetActor()->Attack();
		}
		if (g_theInput->WasKeyJustPressed(KEYCODE_LEFTARROW) )
		{
			GetActor()->EquipPreviousWeapon();
		}
		if (g_theInput->WasKeyJustPressed(KEYCODE_RIGHTARROW))
		{
			GetActor()->EquipNextWeapon();
		}
		if (g_theInput->WasKeyJustPressed('1'))
		{
			GetActor()->EquipWeapon(0);
		}
		if (g_theInput->WasKeyJustPressed('2'))
		{
			GetActor()->EquipWeapon(1);
		}
		if (g_theInput->WasKeyJustPressed('3'))
		{
			GetActor()->EquipWeapon(2);
		}
	}
	if (GetActor()->m_actorDef.m_faction == ActorFaction::DEMON) //Demon player 
	{
		if (g_theInput->IsKeyDown(KEYCODE_LEFT_MOUSE) || g_controller.GetRightTrigger() > 0.1f)
		{
			GetActor()->Attack();
		}
	}

// 	if (g_theInput->WasKeyJustPressed('N') && m_map->m_canDebugPossession)
// 	{
// 		m_map->DebugPossessNext();
// 	}
}

void Player::UpdateListener()
{
	Vec3 forward, up, left;
	m_orientation.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);
	g_theAudio->UpdateListener(m_playerIndex, m_position, forward, up);
}

void Player::UpdateActorCameraMNK()
{

	Actor* actor = m_map->GetActorByUID(m_actorUID);
	if (!actor->m_isDead)
	{
		m_orientation = actor->m_orientation;
		m_position = actor->GetEyePosition();
		Vec2 cursorDelta = g_theInput->GetCursorClientDelta();
		m_orientation.m_pitchDegrees += 0.075f * cursorDelta.y;
		m_orientation.m_yawDegrees -= 0.075f * cursorDelta.x;

		m_orientation.m_pitchDegrees = GetClamped(m_orientation.m_pitchDegrees, -85.f, 85.f);

		Vec3 iForwardBasis, jLeftBasis, kUpBasis;
		m_orientation.GetAsVectors_IFwd_JLeft_KUp(iForwardBasis, jLeftBasis, kUpBasis);

		if (g_theInput->IsKeyDown(KEYCODE_SHIFT) )
		{
			m_speed = GetActor()->m_actorDef.m_runSpeed;
		}
		else
		{
			m_speed = GetActor()->m_actorDef.m_walkSpeed;
		}


		if (g_theInput->IsKeyDown('W'))
		{
			//m_position += iForwardBasis * PLAYER_MOVESPEED * deltaSeconds;
			actor->MoveInDirection(iForwardBasis, m_speed);
		}

		if (g_theInput->IsKeyDown('S'))
		{
			//m_position -= iForwardBasis * PLAYER_MOVESPEED * deltaSeconds;
			actor->MoveInDirection(-iForwardBasis, m_speed);
		}

		if (g_theInput->IsKeyDown('A'))
		{
			//m_position += jLeftBasis * PLAYER_MOVESPEED * deltaSeconds;
			actor->MoveInDirection(jLeftBasis, m_speed);
		}

		if (g_theInput->IsKeyDown('D'))
		{
			//m_position -= jLeftBasis * PLAYER_MOVESPEED * deltaSeconds;
			actor->MoveInDirection(-jLeftBasis, m_speed);
		}
		actor->m_orientation = m_orientation;
		//actor->m_position = Vec3(m_position.x, m_position.y, 0.f);
	}
}

void Player::UpdateActorCameraController()
{
	Actor* actor = m_map->GetActorByUID(m_actorUID);
	if (!actor->m_isDead)
	{
		m_orientation = actor->m_orientation;
		m_position = actor->GetEyePosition();
		m_orientation.m_pitchDegrees -= g_controller.GetRightStick().GetPosition().y * actor->m_actorDef.m_turnSpeed * g_theGameClock->GetDeltaSeconds();
		m_orientation.m_yawDegrees -= g_controller.GetRightStick().GetPosition().x * actor->m_actorDef.m_turnSpeed * g_theGameClock->GetDeltaSeconds();
		m_orientation.m_pitchDegrees = GetClamped(m_orientation.m_pitchDegrees, -85.f, 85.f);

		Vec3 iForwardBasis, jLeftBasis, kUpBasis;
		m_orientation.GetAsVectors_IFwd_JLeft_KUp(iForwardBasis, jLeftBasis, kUpBasis);
		if (g_controller.IsButtonDown(A))
		{
			m_speed = GetActor()->m_actorDef.m_runSpeed;
			
		}
		else
		{
			m_speed = GetActor()->m_actorDef.m_walkSpeed;
		}
		Vec2 leftStickInput = g_controller.GetLeftStick().GetPosition();
		Vec3 movementDirection = iForwardBasis * leftStickInput.y + jLeftBasis * -leftStickInput.x;
		actor->MoveInDirection(movementDirection.GetNormalized(), m_speed);
		actor->m_orientation = m_orientation;
	}
}

void Player::UpdateScreenShake()
{
	//float shakeX = g_rng->RollRandomFloatInRange(-m_screenShakeAmount, m_screenShakeAmount);
	//float shakeY = g_rng->RollRandomFloatInRange(-m_screenShakeAmount, m_screenShakeAmount);
	//Vec3 cameraPosition = m_position;
	//Vec3 iBasis,jLeftBasis, kUpBasis;
	//m_orientation.GetAsVectors_IFwd_JLeft_KUp(iBasis, jLeftBasis, kUpBasis);
	//cameraPosition += shakeX* jLeftBasis;
	//cameraPosition += shakeY * kUpBasis;
	//
	//m_worldCamera.SetTransform(cameraPosition, m_orientation);
	//m_screenShakeAmount -= SCREEN_SHAKE_REDUCTION_PER_SEC * g_theGameClock->GetDeltaSeconds();
	//m_screenShakeAmount = GetClamped(m_screenShakeAmount, 0.f, MAX_SCREEN_SHAKE);
	//
	//m_worldCamera.SetFOV(60.f - m_screenShakeAmount * 1000.f);
	float shakeX = g_rng->RollRandomFloatInRange(-m_screenShakeAmount, m_screenShakeAmount);
	float shakeY = g_rng->RollRandomFloatInRange(-m_screenShakeAmount, m_screenShakeAmount);
	Vec3 cameraPosition = m_position;
	Vec3 iBasis, jLeftBasis, kUpBasis;
	m_orientation.GetAsVectors_IFwd_JLeft_KUp(iBasis, jLeftBasis, kUpBasis);
	cameraPosition += 0.025f * shakeX * jLeftBasis;
	cameraPosition += 0.025f * shakeY * kUpBasis;

	m_worldCamera.SetTransform(cameraPosition, m_orientation);
	// Adjust the reduction logic as needed
	m_screenShakeAmount -= SCREEN_SHAKE_REDUCTION_PER_SEC * g_theGameClock->GetDeltaSeconds();
	m_screenShakeAmount = max(m_screenShakeAmount, 0.f);

	m_worldCamera.SetFOV(60.f - m_screenShakeAmount * 20.f);
}

Mat44 const Player::GetModelMatrix() const
{
	Mat44 model;
	if (m_map->m_mode == GM_COMBAT)
	{
		model = m_orientation.GetAsMatrix_IFwd_JLeft_KUp();
		model.SetTranslation3D(m_position);
	}
	else if(m_map->m_mode == GM_DIALOGUE)
	{
		model =m_worldCamera.GetCameraOrientation().GetAsMatrix_IFwd_JLeft_KUp();
		model.SetTranslation3D(m_worldCamera.GetCameraPosition());
	}
	return model;
}

int Player::GetActorIndexInMap() const
{
	return m_actorUID.GetIndex();
}

void Player::SetCameraPosition(Actor* targetActor)
{
	if (targetActor)
	{
		Vec3 targetPosition = targetActor->m_position + targetActor->m_orientation.GetForwardDir_XFwd_YLeft_ZUp() * 1.f;
		EulerAngles targetOrientation = (targetActor->m_orientation.GetForwardDir_XFwd_YLeft_ZUp() * -1.f).GetEulerAngle();
		m_worldCamera.SetTransform(Vec3(targetPosition.x, targetPosition.y,GetActor()->m_actorDef.m_eyeHeight), targetOrientation);
	}
	else
	{
		m_worldCamera.SetTransform(Vec3(m_position.x, m_position.y, GetActor()->m_actorDef.m_eyeHeight), m_orientation);
	}
}

void Player::ApplyRecoilEffect()
{

	EulerAngles recoilOffset(GetActor()->GetEquippedWeapon()->m_weaponDef.m_recoilAmount,0.f, 0.f);
	m_orientation += recoilOffset;
	m_recoilRecoveryTimer = new Timer(GetActor()->GetEquippedWeapon()->m_weaponDef.m_recoilDuration);
	m_recoilRecoveryTimer->Start();
}


