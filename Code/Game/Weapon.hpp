#pragma once
#include "Game/WeaponDefinition.hpp"
class Actor;
class Timer;
class Map;
class Weapon
{
public:
	Weapon(Actor* owner, std::string weaponName);
	void Fire(Actor* target = nullptr);
	Vec3 GetRandomDirectionInCone();
public:
	SoundPlaybackID m_fireSound;
	Map* m_map = nullptr;
	Actor* m_owner;
	WeaponDefinition m_weaponDef;
	Timer* m_refireTimer = nullptr;
	void PlayAnimation();
	Timer* m_animationTimer = nullptr;
private:
	void PistolFire();
	void PlasmaFire();
	void SMGFire();
	void MeleeAttack();
	void KamaFire(Actor* target);
	void CatFire(Actor* target);
};