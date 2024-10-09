#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB3.hpp"
#include <vector>
#include <string>
struct TileDefinition
{
public:
	TileDefinition(XmlElement const& tileDefElement);

	std::string m_name = "UNKNOWN";
	bool m_isSolid = false;
	Rgba8 m_mapImagePixelColor = Rgba8::WHITE;
	IntVec2 m_floorSpriteCoords = IntVec2(-1, -1);
	IntVec2 m_ceilingSpriteCoords = IntVec2(-1, -1);
	IntVec2 m_wallSpriteCoords = IntVec2(-1, -1);
public:
	static TileDefinition GetByColor(Rgba8 const& color);
	static std::vector<TileDefinition*> s_tileDefs;
	static void InitializeTileDefs(char const* path);
	static void ClearDefinitions();
	static const TileDefinition* GetByName(const std::string& name);
};
class Tile
{
public:
	Tile(TileDefinition tileDef );
	~Tile();
	Rgba8 const GetColor() const; 
	AABB3 m_bounds;
	AABB2 const GetBoundXY() const;
	TileDefinition const& GetTileDefinition() const;
private:
	TileDefinition m_tileDef;
};