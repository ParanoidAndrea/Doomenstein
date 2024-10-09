#pragma once
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/ActorUID.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include <vector>
class Map;
struct SpawnInfo;
class Controller;
class Weapon;
class Timer;
class AI;
class Clock;
class Actor
{
public:
	Actor(Map* owner, SpawnInfo const& spawnInfo);
	virtual ~Actor();

	void Update(float deltaSeconds); // subclass must implement the class
	void UpdateSoundsPosition();
	void UpdatePhysics(float deltaSeconds);
	void Render(int playerIndex) const;
	void AddForce( Vec3 const& force);
	void AddImpulse(Vec3 const& impulse);
	void OnCollide(Actor* other);

	void Damage(float damage, Actor* source);
	void Die(Actor* source);
	
	void OnPossessed(Controller* controller);
	void OnUnpossessed(Controller* controller);
	void MoveInDirection(Vec3 direction, float speed);
	void TurnInDirection(float degrees, float maxAngle);

	Weapon* GetEquippedWeapon();
	void EquipWeapon(int weaponIndex);
	void EquipNextWeapon();
	void EquipPreviousWeapon();
	void Attack(Actor* target = nullptr);


	void PlayDefaultAnimation();
	void PlayAnimation(std::string const& name);

	Mat44 const GetModelMatrix() const;
	Mat44 const GetRenderModelMatrix() const;
	Vec3 const GetEyePosition() const;

	void PlayActorSound(SoundType type, float volume = 1.f);

public:
	bool IsAIControlling() const;
	std::vector<Weapon*> m_weapons;
	int m_equippedWeaponIndex = 0;

	Actor* m_owner = nullptr;
	FloatRange m_damage;
	Timer* m_corpsetimeTimer = nullptr;
	Timer* m_footStepTimer = nullptr;
	Controller* m_controller = nullptr;
	AI* m_aiController = nullptr;

	SoundPlaybackID m_hurtSound;
	SoundPlaybackID m_deathSound; 
	SoundPlaybackID m_walkSound;
	ActorUID m_uid = ActorUID::INVALID;
	ActorDefinition m_actorDef;
	Map* m_map = nullptr;
	Vec3  m_position;
	Vec3 m_velocity;
	Vec3 m_acceleration;
	Rgba8 m_color = Rgba8::RED;
	Rgba8 m_lighterColor = Rgba8::WHITE;
	EulerAngles m_orientation;
	EulerAngles m_angularVelocity;
	FloatRange m_minMaxZ;
	Vec3 m_fwd;
	Vec2 m_centerXY;
	Clock* m_animationClock = nullptr;
	SpriteAnimationGroupDefinition* m_animationGroup = nullptr;
	float m_health = 200.f;
	float m_height = 0.75f;
	float m_radius = 0.35f;
	bool m_isVisble = true;
	bool m_isStatic = false;
	bool m_isDead = false;
	bool m_isDestroyed = false;
	bool m_dieOnCollide = false;
	bool m_hasCollideWithWorld = false;
};