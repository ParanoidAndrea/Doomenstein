#include "Game/WeaponDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Render/SpriteAnimDefinition.hpp"
#include "Game/Gamecommon.hpp"

std::vector<WeaponDefinition*> WeaponDefinition::s_weaponDefs;


WeaponDefinition::WeaponDefinition(XmlElement const& weaponDefElement)
{
	m_name = ParseXmlAttribute(weaponDefElement, "name", m_name);
	m_refireTime = ParseXmlAttribute(weaponDefElement, "refireTime", m_refireTime);

	m_rayCount = ParseXmlAttribute(weaponDefElement, "rayCount", m_rayCount);
	m_rayCone = ParseXmlAttribute(weaponDefElement, "rayCone", m_rayCone);
	m_rayRange = ParseXmlAttribute(weaponDefElement, "rayRange", m_rayRange);
	m_rayDamage = ParseXmlAttribute(weaponDefElement, "rayDamage", m_rayDamage);
	m_rayImpulse = ParseXmlAttribute(weaponDefElement, "rayImpulse", m_rayImpulse);

	m_projectileActor = ParseXmlAttribute(weaponDefElement, "projectileActor", m_projectileActor);
	m_projectileCount = ParseXmlAttribute(weaponDefElement, "projectileCount", m_projectileCount);
	m_projectileCone = ParseXmlAttribute(weaponDefElement, "projectileCone", m_projectileCone);
	m_projectileSpeed = ParseXmlAttribute(weaponDefElement, "projectileSpeed", m_projectileSpeed);

	m_meleeCount = ParseXmlAttribute(weaponDefElement, "meleeCount", m_meleeCount);
	m_meleeArc = ParseXmlAttribute(weaponDefElement, "meleeArc", m_meleeArc);
	m_meleeRange = ParseXmlAttribute(weaponDefElement, "meleeRange", m_meleeRange);
	m_meleeDamage = ParseXmlAttribute(weaponDefElement, "meleeDamage", m_meleeDamage);
	m_meleeImpulse = ParseXmlAttribute(weaponDefElement, "meleeImpulse", m_meleeImpulse);

	m_attackArc = ParseXmlAttribute(weaponDefElement, "attackArc", m_attackArc);
	m_attackRange = ParseXmlAttribute(weaponDefElement, "attackRange", m_attackRange);

	m_recoilForce = ParseXmlAttribute(weaponDefElement, "recoilForce", m_recoilForce);
	m_recoilAmount = ParseXmlAttribute(weaponDefElement, "recoilAmount", m_recoilForce);
	m_recoilDuration = ParseXmlAttribute(weaponDefElement, "recoilDuration", m_recoilForce);
}

void WeaponDefinition::InitializeWeaponDefs(char const* filePath)
{
	XmlDocument mapDefsXml;
	XmlResult result = mapDefsXml.LoadFile(filePath);
	GUARANTEE_OR_DIE(result == XmlResult::XML_SUCCESS, Stringf("Failed to open the requried actor definitions file."));
	XmlElement* rootElement = mapDefsXml.RootElement();
	GUARANTEE_OR_DIE(rootElement, Stringf("Failed to find the root element of the actor definition."));
	XmlElement* weaponDefElement = rootElement->FirstChildElement();
	while (weaponDefElement)
	{
		std::string elementName = weaponDefElement->Name();
		GUARANTEE_OR_DIE(elementName == "WeaponDefinition", Stringf("Failed to find the weapon definition's element."));
		WeaponDefinition* weaponDefinition = new WeaponDefinition(*weaponDefElement);
		s_weaponDefs.push_back(weaponDefinition);
		XmlElement* hudELement = weaponDefElement->FirstChildElement("HUD");
		if (hudELement)
		{
			weaponDefinition->LoadHUDElements(hudELement);
		}
		XmlElement* soundsElement = weaponDefElement->FirstChildElement("Sounds");
		if (soundsElement)
		{
			weaponDefinition->LoadSoundsElements(soundsElement);
		}
		weaponDefElement = weaponDefElement->NextSiblingElement();
	}
}
void WeaponDefinition::LoadHUDElements(XmlElement* hudElement)
{
	std::string baseTexturePath = ParseXmlAttribute(*hudElement, "baseTexture", "UNKNOWN");
	std::string reticleTexturePath = ParseXmlAttribute(*hudElement, "reticleTexture", "UNKNOWN");
	m_baseTexture = g_theRenderer->CreateOrGetTextureFromFile(baseTexturePath.c_str());
	m_reticleTexture = g_theRenderer->CreateOrGetTextureFromFile(reticleTexturePath.c_str());
	m_reticleSize = ParseXmlAttribute(*hudElement, "reticleSize", m_reticleSize);
	m_spriteSize = ParseXmlAttribute(*hudElement, "spriteSize", m_spriteSize);
	m_spritePivot = ParseXmlAttribute(*hudElement, "spritePivot", m_spritePivot);
	XmlElement* animationElement = hudElement->FirstChildElement("Animation");
	while (animationElement)
	{
		std::string name = ParseXmlAttribute(*animationElement, "name", "UNKNOWN");
		std::string spriteSheetPath = ParseXmlAttribute(*animationElement, "spriteSheet", "UNKNOWN");
		IntVec2 cellCount = ParseXmlAttribute(*animationElement, "cellCount", IntVec2());
		float secondsPerFrame = ParseXmlAttribute(*animationElement, "secondsPerFrame", 0.f);
		int startFrame = ParseXmlAttribute(*animationElement, "startFrame", 0);
		int endFrame = ParseXmlAttribute(*animationElement, "endFrame", 0);
		float durationSeconds = (endFrame - startFrame + 1) * secondsPerFrame;
		SpriteSheet* spriteSheet = new SpriteSheet(*g_theRenderer->CreateOrGetTextureFromFile(spriteSheetPath.c_str()), cellCount);
		SpriteAnimDefinition* animation = new SpriteAnimDefinition(*spriteSheet, startFrame, endFrame, durationSeconds, SpriteAnimPlaybackType::ONCE);
		animation->SetAnimation(name);
		m_animations.push_back(animation);
		animationElement = animationElement->NextSiblingElement("Animation");
	}
}

void WeaponDefinition::LoadSoundsElements(XmlElement* soundsElement)
{
	XmlElement* soundElement = soundsElement->FirstChildElement("Sound");
	while (soundElement)
	{
		std::string soundName = ParseXmlAttribute(*soundElement, "sound", "UNKNOWN");
		if (soundName.compare("Fire") == 0)
		{
			std::string firePath = ParseXmlAttribute(*soundElement, "name", "UNKNOWN");
			m_fireSoundID = g_theAudio->CreateOrGetSound(firePath, true);
		}
		soundElement = soundElement->NextSiblingElement("Sound");
	}
}

void WeaponDefinition::ClearDefinitions()
{
	s_weaponDefs.clear();
}

const WeaponDefinition* WeaponDefinition::GetByName(const std::string& name)
{
	for (int i = 0; i < (int)WeaponDefinition::s_weaponDefs.size(); i++)
	{
		if (WeaponDefinition::s_weaponDefs[i]->m_name.compare(name) == 0)
		{
			return WeaponDefinition::s_weaponDefs[i];
		}

	}
	return WeaponDefinition::s_weaponDefs[0];
}

