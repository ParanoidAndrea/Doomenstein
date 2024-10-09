#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Game/Gamecommon.hpp"
class SpriteAnimDefinition;
class Texture;
struct WeaponDefinition
{
public:
	WeaponDefinition() {};
	WeaponDefinition(XmlElement const& weaponDefElement);
	std::string m_name = "UNKNOWN"; 
	std::string m_projectileActor = "UNKNOWN";
	float m_refireTime = 0.f;
	float m_rayCount = 0.f;
	float m_rayCone = 0.f;
	float m_rayRange = 0.f;
	FloatRange m_rayDamage;
	float m_rayImpulse = 0.f;
	float m_projectileCount = 0.f;
	float m_projectileCone = 0.f;//Maximum angle variation in degrees for each projectile launched cast. Each projectile launched should have its velocity randomly distributed in a cone of this angle relative to the forward direction of the firing actor.
	float m_projectileSpeed = 0.f;
	float m_meleeCount = 0.f;
	float m_meleeRange = 0.f;
	float m_meleeArc = 0.f;
	FloatRange m_meleeDamage;
	float m_meleeImpulse = 0.f;
	float m_attackArc = 0.f;
	float m_attackRange = 0.f;

	float m_recoilForce = 0.f;
	float m_recoilAmount = 0.f;
	float m_recoilDuration = 0.f;
	//HUD
	Texture* m_baseTexture = nullptr;
	Texture* m_reticleTexture = nullptr;
	Vec2 m_reticleSize;
	Vec2 m_spriteSize;
	Vec2 m_spritePivot;
	std::vector<SpriteAnimDefinition*> m_animations;

	//Sounds
	SoundID m_fireSoundID = 0;
public:
	static void InitializeWeaponDefs(char const* filePath);
	static void ClearDefinitions();
	static const WeaponDefinition* GetByName(const std::string& name);
	static std::vector<WeaponDefinition*> s_weaponDefs;
private:
	void LoadHUDElements(XmlElement* hudElement);
	void LoadSoundsElements(XmlElement* soundsElement);
};