#pragma once
#include "Game/SpawnInfo.hpp"
#include "Engine/Core/XmlUtils.hpp"
class Image;
class Shader;
class Texture;
struct MapDefinition
{
public:
	MapDefinition(XmlElement const& mapDefElement);
	std::string m_name = "UNKNOWN";
	Image* m_image = nullptr;
	std::string m_shaderPath = "UNKNOWN";
	Texture* m_skyboxTexture = nullptr;
	Texture* m_spriteSheetTexture = nullptr;
	IntVec2 m_spriteSheetCellCount;
	std::vector<SpawnInfo*> m_infos;
public:
	static void InitializeMapDefs(char const* filePath);
	bool LoadSpawnInfos(XmlElement const* spawnInfoElement);
	static void ClearDefinitions();
	static const MapDefinition* GetByName(const std::string& name);
	static std::vector<MapDefinition*> s_mapDefs;
};