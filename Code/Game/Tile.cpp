#include "Game/Tile.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"


std::vector<TileDefinition*> TileDefinition::s_tileDefs;
TileDefinition::TileDefinition(XmlElement const& tileDefElement)
{
	m_name = ParseXmlAttribute(tileDefElement, "name", m_name);
	m_isSolid = ParseXmlAttribute(tileDefElement, "isSolid", m_isSolid);
	m_mapImagePixelColor = ParseXmlAttribute(tileDefElement, "mapImagePixelColor", m_mapImagePixelColor);
	m_floorSpriteCoords = ParseXmlAttribute(tileDefElement, "floorSpriteCoords", m_floorSpriteCoords);
	m_ceilingSpriteCoords = ParseXmlAttribute(tileDefElement, "ceilingSpriteCoords", m_ceilingSpriteCoords);
	m_wallSpriteCoords = ParseXmlAttribute(tileDefElement, "wallSpriteCoords", m_wallSpriteCoords);
}

void TileDefinition::InitializeTileDefs(char const* filePath)
{
	XmlDocument tileDefsXml;
	XmlResult result = tileDefsXml.LoadFile(filePath);
	GUARANTEE_OR_DIE(result == XmlResult::XML_SUCCESS, Stringf("Failed to open the requried tile definitions file."));

	XmlElement* rootElement = tileDefsXml.RootElement();
	GUARANTEE_OR_DIE(rootElement, Stringf("Failed to find the root element of the tile definition."));

	XmlElement* tileDefElement = rootElement->FirstChildElement();
	while (tileDefElement)
	{
		std::string elementName = tileDefElement->Name();
		GUARANTEE_OR_DIE(elementName == "TileDefinition", Stringf("Failed to find the tile definition's element."));
		TileDefinition* tileDefinition = new TileDefinition(*tileDefElement);
		s_tileDefs.push_back(tileDefinition);
		tileDefElement = tileDefElement->NextSiblingElement();
	}
}

void TileDefinition::ClearDefinitions()
{
	s_tileDefs.clear();
}

TileDefinition const* TileDefinition::GetByName(const std::string& name)
{
	for (int i = 0; i < (int)TileDefinition::s_tileDefs.size(); i++)
	{
		if (TileDefinition::s_tileDefs[i]->m_name.compare(name) == 0)
		{
			return TileDefinition::s_tileDefs[i];
		}

	}
	return TileDefinition::s_tileDefs[0];
}
TileDefinition TileDefinition::GetByColor(Rgba8 const& color)
{
	for (int i = 0; i < (int)TileDefinition::s_tileDefs.size(); i++)
	{
		if (TileDefinition::s_tileDefs[i]->m_mapImagePixelColor == color)
		{
			return *TileDefinition::s_tileDefs[i];
		}
	}
	return *TileDefinition::s_tileDefs[0];
}

Tile::Tile(TileDefinition tileDef)
  :m_tileDef(tileDef)
{
}

Tile::~Tile()
{
}

Rgba8 const Tile::GetColor() const
{
	return m_tileDef.m_mapImagePixelColor;
}

AABB2 const Tile::GetBoundXY() const
{
	return AABB2(m_bounds.m_mins.x, m_bounds.m_mins.y, m_bounds.m_maxs.x, m_bounds.m_maxs.y);
}

TileDefinition const& Tile::GetTileDefinition() const
{
	return m_tileDef;
}

