#pragma once
#include "Engine/Render/Camera.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Game/Controller.hpp"
#include "Engine/Math/AABB2.hpp"
class Map;
class Game;
class Player : public Controller
{
public:
	Player(Game* game, int playerIndex = 0, int controllerIndex = -1, int kill = 0, int dealth = 0);
	virtual ~Player();
	virtual void Update(float deltaSeconds);
	void RenderScreen() const;
	virtual bool const IsFreeFlyCamera() const override;
	Mat44 const GetModelMatrix() const;
	int GetActorIndexInMap() const;
	void SetCameraPosition(Actor* targetActor = nullptr);
	void ApplyRecoilEffect();
	Camera m_worldCamera = Camera(Vec2(-1.f,-1.f),Vec2(1.f,1.f));
	Camera m_screenCamera = Camera(Vec2(0.f, 0.f), Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y));
	Vec3  m_position;
	EulerAngles m_orientation;
	Game* m_game;
	int m_kills = 0;
	int m_deaths = 0;
private:
	void UpdateFreeFlyCamera(float deltaSeconds);
	void InputActions();
	void UpdateListener();
	void UpdateActorCameraMNK();
	void UpdateActorCameraController();
	void UpdateScreenShake();
	float m_speed = PLAYER_MOVESPEED;
	Vec3  m_velocity;
	Rgba8 m_color = Rgba8::WHITE;
	AABB2 m_normalizedViewport = AABB2::ZERO_TO_ONE;
	EulerAngles m_angularVelocity;
	Timer* m_recoilRecoveryTimer = nullptr;
	float m_screenShakeAmount = 0.f;
	int m_playerIndex = 0;
	int m_controllerIndex = -1;
	bool isFireing = false;
};