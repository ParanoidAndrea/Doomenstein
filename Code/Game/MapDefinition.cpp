#include "Game/MapDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Render/Renderer.hpp"
std::vector<MapDefinition*> MapDefinition::s_mapDefs;

MapDefinition::MapDefinition(XmlElement const& mapDefElement)
{
	m_name = ParseXmlAttribute(mapDefElement, "name", m_name);

	std::string imagePath = ParseXmlAttribute(mapDefElement, "image", "UNKNOWN");
	m_image = new Image(imagePath.c_str());

	m_shaderPath = ParseXmlAttribute(mapDefElement, "shader", "UNKNOWN");
	std::string texturePath = ParseXmlAttribute(mapDefElement, "spriteSheetTexture", "UNKNOWN");

	m_spriteSheetCellCount = ParseXmlAttribute(mapDefElement, "spriteSheetCellCount", m_spriteSheetCellCount);

	m_spriteSheetTexture = g_theRenderer->CreateOrGetTextureFromFile(texturePath.c_str());

	std::string skyboxPath = ParseXmlAttribute(mapDefElement, "skybox", "UNKNOWN");
	m_skyboxTexture = g_theRenderer->CreateOrGetTextureFromFile(skyboxPath.c_str());

}

void MapDefinition::InitializeMapDefs(char const* filePath)
{
	XmlDocument mapDefsXml;
	XmlResult result = mapDefsXml.LoadFile(filePath);
	GUARANTEE_OR_DIE(result == XmlResult::XML_SUCCESS, Stringf("Failed to open the requried map definitions file."));
	XmlElement* rootElement = mapDefsXml.RootElement();
	GUARANTEE_OR_DIE(rootElement, Stringf("Failed to find the root element of the map definition."));
	XmlElement* mapDefElement = rootElement->FirstChildElement();
	while (mapDefElement)
	{
		std::string elementName = mapDefElement->Name();
		GUARANTEE_OR_DIE(elementName == "MapDefinition", Stringf("Failed to find the map definition's element."));
		MapDefinition* mapDefinition = new MapDefinition(*mapDefElement);

		XmlElement* spawnInfos = mapDefElement->FirstChildElement("SpawnInfos");
		if (spawnInfos)
		{
			XmlElement* spawnInfoElement = spawnInfos->FirstChildElement("SpawnInfo");
			mapDefinition->LoadSpawnInfos(spawnInfoElement);
		}

		s_mapDefs.push_back(mapDefinition);
		mapDefElement = mapDefElement->NextSiblingElement();
	}
}

bool MapDefinition::LoadSpawnInfos(XmlElement const* spawnInfoElement)
{
	while (spawnInfoElement)
	{
		std::string actorName = ParseXmlAttribute(*spawnInfoElement, "actor", "UNKNOWN");
		Vec3 position = ParseXmlAttribute(*spawnInfoElement, "position", Vec3());
		EulerAngles orientation = ParseXmlAttribute(*spawnInfoElement, "orientation", EulerAngles());
		SpawnInfo* info = new SpawnInfo(actorName.c_str(), position, orientation);
		m_infos.push_back(info);
		spawnInfoElement = spawnInfoElement->NextSiblingElement("SpawnInfo");
	}

	return true;

}

void MapDefinition::ClearDefinitions()
{
	s_mapDefs.clear();
}

const MapDefinition* MapDefinition::GetByName(const std::string& name)
{
	for (int i = 0; i < (int)MapDefinition::s_mapDefs.size(); i++)
	{
		if (MapDefinition::s_mapDefs[i]->m_name.compare(name) == 0)
		{
			return MapDefinition::s_mapDefs[i];
		}

	}
	return MapDefinition::s_mapDefs[0];
}
