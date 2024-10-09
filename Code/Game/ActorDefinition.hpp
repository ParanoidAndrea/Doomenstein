#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Render/SpriteAnimationGroupDefinition.hpp"
#include "Game/Gamecommon.hpp"
enum class ActorFaction
{
	MARINE,
	DEMON,
	NETURAL
};
struct ActorDefinition
{
public:
	ActorDefinition();
	ActorDefinition(XmlElement const& actorDefElement);
	std::string m_name = "UNKNOWN"; 
	ActorFaction m_faction = ActorFaction::NETURAL;
	bool m_visible = false;
	float m_health = 1;
	float m_corpseLifetime = 0.f; // Time that the actor should linger after death
	Rgba8 m_solidColor = Rgba8(32,32,32,255);
	Rgba8 m_wireframeColor = Rgba8(192, 192, 192, 255);
	bool m_canBePossessed = false;
	bool m_dieOnSpawn = false;
	bool m_isPlayer = false;
	bool m_isNinja = false;
	bool m_isCat = false;
	float m_moveCooldown = 0.f;
	//Collision
	float m_physicsRadius = 0.f;
	float m_physicsHeight = 0.f;
	bool m_collidesWithWorld = false;
	bool m_collidesWithActors = false;
	bool m_dieOnCollide = false;
	FloatRange m_damageOnCollide; //Damage should be a random number in this range.
	float m_impulseOnCollide = 0.f;

	//Physics
	bool m_simulated = false; //Specifies whether this actor should use physics to control position and orientation.
	bool m_flying = false; //Specifies whether the actor can come off the ground or not. Non-flying actors should have their z-coordinate set to zero each frame.
	float m_walkSpeed = 0.f; //Normally
	float m_runSpeed = 0.f; //Sprinting
	float m_drag = 0.f;
	float m_turnSpeed = 0.f;

	//Camera
	float m_eyeHeight = 0.f; //Height off the ground at which to place the camera when this actor is possessed.
	float m_cameraFOVDegrees = 60.f;

	//AI
	bool m_aiEnabled = false;
	float m_sightRadius = 0.f;
	float m_sightAngle = 0.f;

	//Visuals
	Vec2 m_size;
	Vec2 m_pivot;
	BillboardType m_billboardType = BillboardType::NONE;
	bool m_renderLit = false;

	bool m_renderRounded = false; // Render the beak-like cone or not;
	std::vector<SpriteAnimationGroupDefinition*> m_animationGroups;


	//Inventory - Weapon
	std::vector<std::string> m_weaponNames; //Definition name of the weapon to add to this actor when it is spawned.

	//Audio
	SoundID m_hurtSoundID;
	SoundID m_deathSoundID;
	SoundID m_footstepID;
public:
	void LoadCollisionElements(XmlElement const& collisionElement);
	void LoadPhysicsElements(XmlElement const& physicsElement);
	void LoadCameraElements(XmlElement const& cameraElement);
	void LoadAIElements(XmlElement const& aiElement);
	void LoadVisualElements(XmlElement* visualElement);
	void LoadSoundsElements(XmlElement* soundElement);
	void LoadInventoryElements(XmlElement* inventoryElement);

	static void InitializeActorDefs(char const* filePath);
	static void ClearDefinitions();
	static const ActorDefinition* GetByName(const std::string& name);
	static std::vector<ActorDefinition*> s_actorDefs;
};