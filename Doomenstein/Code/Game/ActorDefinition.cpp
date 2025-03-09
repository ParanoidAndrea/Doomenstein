#include "Game/ActorDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

std::vector<ActorDefinition*> ActorDefinition::s_actorDefs;

ActorDefinition::ActorDefinition(XmlElement const& actorDefElement)
{
	m_name = ParseXmlAttribute(actorDefElement, "name", m_name);

	std::string factionName = ParseXmlAttribute(actorDefElement, "faction", "UNKNOWN");

	if (factionName.compare("Marine") == 0)
	{
		m_faction = ActorFaction::MARINE;
		m_solidColor = Rgba8::GREEN;
	}
	else if (factionName.compare("Demon") == 0)
	{
		m_faction = ActorFaction::DEMON;
		m_solidColor = Rgba8::RED;
	}
	else
	{
		m_faction = ActorFaction::NETURAL;
		m_solidColor = Rgba8(32, 32, 32, 255);

	}

	if (m_name.compare("PlasmaProjectile") == 0)
	{
		m_solidColor = RAYCOLOR;
	}

	m_wireframeColor = Rgba8(192, 192, 192, 255);
	m_visible = ParseXmlAttribute(actorDefElement, "visible", m_visible);
	m_health = ParseXmlAttribute(actorDefElement, "health", m_health);
	m_corpseLifetime = ParseXmlAttribute(actorDefElement, "corpseLifetime", m_corpseLifetime);
	m_dieOnSpawn = ParseXmlAttribute(actorDefElement, "dieOnSpawn", m_dieOnSpawn);

	m_canBePossessed = ParseXmlAttribute(actorDefElement, "canBePossessed", m_canBePossessed);
	m_isPlayer = ParseXmlAttribute(actorDefElement, "isPlayer", m_isPlayer);
	m_isNinja = ParseXmlAttribute(actorDefElement, "isNinja", m_isNinja);
	m_isCat = ParseXmlAttribute(actorDefElement, "isCat", m_isCat);
	m_moveCooldown = ParseXmlAttribute(actorDefElement, "moveCooldown", m_moveCooldown);
}

ActorDefinition::ActorDefinition()
{

}

void ActorDefinition::LoadCollisionElements(XmlElement const& collisionElement)
{

	m_physicsRadius = ParseXmlAttribute(collisionElement, "radius", m_physicsRadius);
	m_physicsHeight = ParseXmlAttribute(collisionElement, "height", m_physicsHeight);
	m_collidesWithWorld = ParseXmlAttribute(collisionElement, "collidesWithWorld", m_collidesWithWorld);
	m_collidesWithActors = ParseXmlAttribute(collisionElement, "collidesWithActors", m_collidesWithActors);
	m_damageOnCollide = ParseXmlAttribute(collisionElement, "damageOnCollide", m_damageOnCollide);
	m_impulseOnCollide = ParseXmlAttribute(collisionElement, "impulseOnCollide", m_impulseOnCollide);
	m_dieOnCollide = ParseXmlAttribute(collisionElement, "dieOnCollide", m_dieOnCollide);
}

void ActorDefinition::LoadPhysicsElements(XmlElement const& physicsElement)
{

	m_simulated = ParseXmlAttribute(physicsElement, "simulated", m_simulated);
	m_walkSpeed = ParseXmlAttribute(physicsElement, "walkSpeed", m_walkSpeed);
	m_runSpeed = ParseXmlAttribute(physicsElement, "runSpeed", m_runSpeed);
	m_turnSpeed = ParseXmlAttribute(physicsElement, "turnSpeed", m_turnSpeed);
	m_drag = ParseXmlAttribute(physicsElement, "drag", m_drag);
	m_flying = ParseXmlAttribute(physicsElement, "flying", m_flying);
}

void ActorDefinition::LoadCameraElements(XmlElement const& cameraElement)
{

	m_eyeHeight = ParseXmlAttribute(cameraElement, "eyeHeight", m_eyeHeight);
	m_cameraFOVDegrees = ParseXmlAttribute(cameraElement, "cameraFOV", m_cameraFOVDegrees);
}



void ActorDefinition::LoadAIElements(XmlElement const& aiElement)
{

	m_aiEnabled = ParseXmlAttribute(aiElement, "aiEnabled", m_aiEnabled);
	m_sightRadius = ParseXmlAttribute(aiElement, "sightRadius", m_sightRadius);
	m_sightAngle = ParseXmlAttribute(aiElement, "sightAngle", m_sightAngle);
}


void ActorDefinition::LoadVisualElements(XmlElement* visualElement)
{
	m_size = ParseXmlAttribute(*visualElement, "size", m_size);
	m_pivot = ParseXmlAttribute(*visualElement, "pivot", m_pivot);
	m_billboardType = ParseXmlAttribute(*visualElement, "billboardType", m_billboardType);
	m_renderLit = ParseXmlAttribute(*visualElement, "renderLit", m_renderLit);
	m_renderRounded = ParseXmlAttribute(*visualElement, "renderRounded", m_renderRounded);
	std::string spriteSheetPath = ParseXmlAttribute(*visualElement, "spriteSheet", "UNKNOWN");
 	IntVec2 cellCount = ParseXmlAttribute(*visualElement, "cellCount", IntVec2()); 
	XmlElement* animaGroupElement = visualElement->FirstChildElement("AnimationGroup");
	SpriteSheet* spriteSheet = new SpriteSheet(*g_theRenderer->CreateOrGetTextureFromFile(spriteSheetPath.c_str()), cellCount);
	while (animaGroupElement)
	{
		std::string name = ParseXmlAttribute(*animaGroupElement, "name", "UNKNOWN");
		bool scaleBySpeed = ParseXmlAttribute(*animaGroupElement, "scaleBySpeed", false);
		float secondsPerFrame = ParseXmlAttribute(*animaGroupElement, "secondsPerFrame", 0.f);
		SpriteAnimPlaybackType playbackMode = ParseXmlAttribute(*animaGroupElement, "playbackMode", SpriteAnimPlaybackType::LOOP);
		std::vector<Vec3> directions;
		std::vector<SpriteAnimDefinition*> animations;
		std::string newSpriteSheetPath = ParseXmlAttribute(*animaGroupElement, "newSpriteSheet", "UNKNOWN");

		if (newSpriteSheetPath.compare("UNKNOWN")!=0)
		{
			cellCount = ParseXmlAttribute(*animaGroupElement, "newCellCount", cellCount);
			spriteSheet = new SpriteSheet(*g_theRenderer->CreateOrGetTextureFromFile(newSpriteSheetPath.c_str()), cellCount);
		}
		XmlElement* directionElement = animaGroupElement->FirstChildElement("Direction");
		while (directionElement)
		{
			Vec3 direction = ParseXmlAttribute(*directionElement, "vector", Vec3());
			directions.push_back(direction);
			XmlElement* animationElement = directionElement->FirstChildElement("Animation");
			int startFrame = ParseXmlAttribute(*animationElement, "startFrame", 0);
			int endFrame = ParseXmlAttribute(*animationElement, "endFrame", 0);
		
			SpriteAnimDefinition* animation = new SpriteAnimDefinition(*spriteSheet, startFrame, endFrame, (endFrame-startFrame +1)*secondsPerFrame, playbackMode);
			animations.push_back(animation);
			
			directionElement = directionElement->NextSiblingElement("Direction");
		}
		
		SpriteAnimationGroupDefinition* animationGroup = new SpriteAnimationGroupDefinition(name,scaleBySpeed,secondsPerFrame,playbackMode, directions, animations);
		animationGroup->m_scaleBySpeed = scaleBySpeed;
		animationGroup->m_size = ParseXmlAttribute(*animaGroupElement, "newSize",m_size);
		m_animationGroups.push_back(animationGroup);
		animaGroupElement = animaGroupElement->NextSiblingElement("AnimationGroup");
	}

}

void ActorDefinition::LoadSoundsElements(XmlElement* soundsElement)
{
	XmlElement* soundElement = soundsElement->FirstChildElement("Sound");
	while (soundElement)
	{
		std::string soundName = ParseXmlAttribute(*soundElement, "sound", "UNKNOWN");
		if (soundName.compare("Hurt") == 0)
		{
			std::string hurtPath = ParseXmlAttribute(*soundElement, "name", "UNKNOWN");
			m_hurtSoundID = g_theAudio->CreateOrGetSound(hurtPath, true);
		}
		else if(soundName.compare("Death") == 0)
		{
			std::string deathPath = ParseXmlAttribute(*soundElement, "name", "UNKNOWN");
			m_deathSoundID = g_theAudio->CreateOrGetSound(deathPath, true);
		}
		else if (soundName.compare("Walk") == 0)
		{
			std::string walkPath = ParseXmlAttribute(*soundElement, "name", "UNKNOWN");
			m_footstepID = g_theAudio->CreateOrGetSound(walkPath, true);
		}
		soundElement = soundElement->NextSiblingElement("Sound");
	}
}

void ActorDefinition::LoadInventoryElements(XmlElement* inventoryElement)
{
	XmlElement* weaponElement = inventoryElement->FirstChildElement("Weapon");
	while (weaponElement)
	{
		std::string weaponName = ParseXmlAttribute(*weaponElement, "name", "UNKNOWN");
		m_weaponNames.push_back(weaponName);
		weaponElement = weaponElement->NextSiblingElement("Weapon");
	}
}

void ActorDefinition::InitializeActorDefs(char const* filePath)
{
	XmlDocument mapDefsXml;
	XmlResult result = mapDefsXml.LoadFile(filePath);
	GUARANTEE_OR_DIE(result == XmlResult::XML_SUCCESS, Stringf("Failed to open the requried actor definitions file."));
	XmlElement* rootElement = mapDefsXml.RootElement();
	GUARANTEE_OR_DIE(rootElement, Stringf("Failed to find the root element of the actor definition."));
	XmlElement* actorDefElement = rootElement->FirstChildElement();
	while (actorDefElement)
	{
		std::string elementName = actorDefElement->Name();
		GUARANTEE_OR_DIE(elementName == "ActorDefinition", Stringf("Failed to find the actor definition's element."));
		ActorDefinition* actorDefinition = new ActorDefinition(*actorDefElement);
		XmlElement* collisionElement = actorDefElement->FirstChildElement("Collision");
		if (collisionElement)
		{
			actorDefinition->LoadCollisionElements(*collisionElement);
		}

		XmlElement* physicsElement = actorDefElement->FirstChildElement("Physics");
		if (physicsElement)
		{
			actorDefinition->LoadPhysicsElements(*physicsElement);
		}

		XmlElement* cameraElement = actorDefElement->FirstChildElement("Camera");
		if (cameraElement)
		{
			actorDefinition->LoadCameraElements(*cameraElement);
		}
		XmlElement* visualElement = actorDefElement->FirstChildElement("Visuals");
		if (visualElement)
		{
			actorDefinition->LoadVisualElements(visualElement);
		}
		XmlElement* aiElement = actorDefElement->FirstChildElement("AI");
		if (aiElement)
		{
			actorDefinition->LoadAIElements(*aiElement);
		}
		XmlElement* soundsElement = actorDefElement->FirstChildElement("Sounds");
		if (soundsElement)
		{
			actorDefinition->LoadSoundsElements(soundsElement);
		}

		XmlElement* inventoryElement = actorDefElement->FirstChildElement("Inventory");
		if (inventoryElement)
		{
			actorDefinition->LoadInventoryElements(inventoryElement);
		}
		s_actorDefs.push_back(actorDefinition);

		actorDefElement = actorDefElement->NextSiblingElement();
	}
}


void ActorDefinition::ClearDefinitions()
{
	s_actorDefs.clear();
}

ActorDefinition const* ActorDefinition::GetByName(const std::string& name)
{
	for (int i = 0; i < (int)ActorDefinition::s_actorDefs.size(); i++)
	{
		if (ActorDefinition::s_actorDefs[i]->m_name.compare(name) == 0)
		{
			return ActorDefinition::s_actorDefs[i];
		}

	}
	return ActorDefinition::s_actorDefs[0];
}
