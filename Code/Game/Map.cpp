#include "Game/Map.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Render/Shader.hpp"
#include "Engine/Render/Renderer.hpp"
#include "Engine/Render/SpriteSheet.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Render/IndexBuffer.hpp"
#include "Engine/Render/VertexBuffer.hpp"
#include "Engine/Render/DebugRender.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/Actor.hpp"
#include "Game/Game.hpp"
#include "Game/Player.hpp"
#include "Game/ActorUID.hpp"
#include "Game/SpawnInfo.hpp"
#include "Game/AI.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Render/BitmapFont.hpp"
Map::Map(Game* game, MapDefinition mapDef)
	:m_game(game),
	m_mapDef(mapDef)
{	
	if (!m_game->m_isSinglePlayer)
	{
		m_canDebugPossession = false;
		m_canFreeFly = false;
	}

	m_dimensions = m_mapDef.m_image->GetDimensions();
	m_spriteSheet = new SpriteSheet(*m_mapDef.m_spriteSheetTexture, m_mapDef.m_spriteSheetCellCount);
	m_shader = g_theRenderer->CreateOrGetShaderFromFile(m_mapDef.m_shaderPath.c_str(), VertexType::Vertex_PCUTBN);
	PopulateTiles();
	AddVertsForTiles();
	SetupListeners();
	if (m_game->m_isSinglePlayer)
	{
		SpawnNewActors();
		SpawnCat();
	}
	else
	{
		for (int i = 0; i < (int)m_mapDef.m_infos.size(); ++i)
		{
			if (m_mapDef.m_infos[i]->m_actorDefName.compare("SpawnPoint") == 0)
			{
				m_playerSpawnInfos.push_back(*m_mapDef.m_infos[i]);
			}
		}
	}
	//SpawnNinja();
}

Map::~Map()
{
	delete m_ibo;
	m_ibo = nullptr;
	delete m_vbo;
	m_vbo = nullptr;
}

void Map::InitializeDialogSystems()
{
	std::vector<DialogueMessage> messages;
	DialogueMessage m = DialogueMessage("....");
	messages.push_back(m);
	 m = DialogueMessage("....");
	 messages.push_back(m);
	 m = DialogueMessage("Ah...My head...It hurts...");
	 messages.push_back(m);
	 m = DialogueMessage("You are awake! Seems like their master like you a lot\n and they kept you alive for a reason...",0.f,"Cat");
	 messages.push_back(m);
	 m = DialogueMessage("What? For what reason?");
	 messages.push_back(m);
	 m = DialogueMessage("Wait...You are a CAT and you speak like a human!");
	 messages.push_back(m);
	 m = DialogueMessage("There's no time to chat.I found some weapons at the floor.\nHelp me get out of here so I can tell you why.", 0.f, "Cat");
	 messages.push_back(m);
	 m = DialogueMessage("They're MY weapons!");
	 messages.push_back(m);
	 m = DialogueMessage("OK. But it's not my fault to get caught by ugly Demons", 0.f, "Cat");
	 messages.push_back(m);
	 m = DialogueMessage("...");
	 messages.push_back(m);
	 m = DialogueMessage("Pro tips: Press 1 to Use Pistol, Press 2 to use Plasma Riffle\nPress 3 to use SMG",0.f,"");
	 messages.push_back(m);
	 m = DialogueMessage("Kill all your enemies to win the game.\nIf you or the cat die, you lose the game.", 0.f, "");
	 messages.push_back(m);
	 m = DialogueMessage("You can press ESC to exit the game during battle", 0.f, "");
	 messages.push_back(m);
	 m = DialogueMessage("Alright, since I don't have any other choices...");
	 messages.push_back(m);
	 m_dialogSystem = new DialogueSystem(messages);
}



void Map::SpawnCat()
{
	SpawnInfo spawnInfo;
	spawnInfo = *m_mapDef.m_infos[1];
	spawnInfo.m_actorDefName = "Cat";
	m_cat = SpawnActor(spawnInfo);
}

void Map::SpawnNinja()
{
	SpawnInfo spawnInfo1 = *m_mapDef.m_infos[0];
	spawnInfo1.m_actorDefName = "NinjaTurtle";
	SpawnActor(spawnInfo1);
	SpawnInfo spawnInfo2 = *m_mapDef.m_infos[0];
	spawnInfo2.m_actorDefName = "NinjaTurtle";
	Actor* ninja= SpawnActor(spawnInfo2);
	ninja->m_equippedWeaponIndex = 1;
}

void Map::PopulateTiles()
{
	//Populate Tiles
	int tilesNumber = m_dimensions.x * m_dimensions.y;
	m_tiles.reserve(tilesNumber);
	for (int j = 0; j < m_dimensions.y; ++j)
	{
		for (int i = 0; i < m_dimensions.x; ++i)
		{
			Rgba8 texelColor = m_mapDef.m_image->GetTexelColor(IntVec2(i, j));
			TileDefinition tileDef = TileDefinition::GetByColor(texelColor);
			Tile tile = Tile(tileDef);
			tile.m_bounds = AABB3(Vec3((float)i, (float)j, 0.f), Vec3((float)i + 1.f, (float)j + 1.f, 1.f));
			m_tiles.push_back(tile);
		}
	}
}

void Map::AddVertsForTiles()
{
	m_vertexes.reserve(m_tiles.size() * 4 * 6);
	m_indexes.reserve(m_tiles.size() * 6 * 6);
	//Add Verts for Tiles
	for (int i = 0; i < (int)m_tiles.size(); ++i)
	{
		AABB2 uvs;
		IntVec2 floorSpriteCoords = m_tiles[i].GetTileDefinition().m_floorSpriteCoords;
		IntVec2 ceilingSpriteCoords = m_tiles[i].GetTileDefinition().m_ceilingSpriteCoords;
		IntVec2 wallingSpriteCoords = m_tiles[i].GetTileDefinition().m_wallSpriteCoords;
		if (floorSpriteCoords != IntVec2(-1, -1))
		{
			uvs = m_spriteSheet->GetSpriteUVsFromCoords(floorSpriteCoords);
			AddVertsForQuad3D(m_vertexes, m_indexes, Vec3(m_tiles[i].m_bounds.m_mins.x, m_tiles[i].m_bounds.m_mins.y, 0.f), Vec3(m_tiles[i].m_bounds.m_maxs.x, m_tiles[i].m_bounds.m_mins.y, 0.f), Vec3(m_tiles[i].m_bounds.m_maxs.x, m_tiles[i].m_bounds.m_maxs.y, 0.f), Vec3(m_tiles[i].m_bounds.m_mins.x, m_tiles[i].m_bounds.m_maxs.y, 0.f), Rgba8::WHITE, uvs);
		}
		if (ceilingSpriteCoords != IntVec2(-1, -1))
		{
			uvs = m_spriteSheet->GetSpriteUVsFromCoords(ceilingSpriteCoords);
			AddVertsForQuad3D(m_vertexes, m_indexes, Vec3(m_tiles[i].m_bounds.m_mins.x, m_tiles[i].m_bounds.m_maxs.y, 1.f), Vec3(m_tiles[i].m_bounds.m_maxs.x, m_tiles[i].m_bounds.m_maxs.y, 1.f), Vec3(m_tiles[i].m_bounds.m_maxs.x, m_tiles[i].m_bounds.m_mins.y, 1.f), Vec3(m_tiles[i].m_bounds.m_mins.x, m_tiles[i].m_bounds.m_mins.y, 1.f), Rgba8::WHITE, uvs);
		}
		if (wallingSpriteCoords != IntVec2(-1, -1))
		{
			uvs = m_spriteSheet->GetSpriteUVsFromCoords(wallingSpriteCoords);
			AddVertsForQuad3D(m_vertexes, m_indexes, Vec3(m_tiles[i].m_bounds.m_mins.x, m_tiles[i].m_bounds.m_mins.y, 0.f), Vec3(m_tiles[i].m_bounds.m_maxs.x, m_tiles[i].m_bounds.m_mins.y, 0.f), Vec3(m_tiles[i].m_bounds.m_maxs.x, m_tiles[i].m_bounds.m_mins.y, 1.f), Vec3(m_tiles[i].m_bounds.m_mins.x, m_tiles[i].m_bounds.m_mins.y, 1.f), Rgba8::WHITE, uvs);
			AddVertsForQuad3D(m_vertexes, m_indexes, Vec3(m_tiles[i].m_bounds.m_maxs.x, m_tiles[i].m_bounds.m_mins.y, 0.f), Vec3(m_tiles[i].m_bounds.m_maxs.x, m_tiles[i].m_bounds.m_maxs.y, 0.f), Vec3(m_tiles[i].m_bounds.m_maxs.x, m_tiles[i].m_bounds.m_maxs.y, 1.f), Vec3(m_tiles[i].m_bounds.m_maxs.x, m_tiles[i].m_bounds.m_mins.y, 1.f), Rgba8::WHITE, uvs);
			AddVertsForQuad3D(m_vertexes, m_indexes, Vec3(m_tiles[i].m_bounds.m_maxs.x, m_tiles[i].m_bounds.m_maxs.y, 0.f), Vec3(m_tiles[i].m_bounds.m_mins.x, m_tiles[i].m_bounds.m_maxs.y, 0.f), Vec3(m_tiles[i].m_bounds.m_mins.x, m_tiles[i].m_bounds.m_maxs.y, 1.f), Vec3(m_tiles[i].m_bounds.m_maxs.x, m_tiles[i].m_bounds.m_maxs.y, 1.f), Rgba8::WHITE, uvs);
			AddVertsForQuad3D(m_vertexes, m_indexes, Vec3(m_tiles[i].m_bounds.m_mins.x, m_tiles[i].m_bounds.m_maxs.y, 0.f), Vec3(m_tiles[i].m_bounds.m_mins.x, m_tiles[i].m_bounds.m_mins.y, 0.f), Vec3(m_tiles[i].m_bounds.m_mins.x, m_tiles[i].m_bounds.m_mins.y, 1.f), Vec3(m_tiles[i].m_bounds.m_mins.x, m_tiles[i].m_bounds.m_maxs.y, 1.f), Rgba8::WHITE, uvs);
		}
	}

	size_t indexBuffersize = sizeof(unsigned int) * (int)m_indexes.size();
	size_t vertexBuffersize = sizeof(Vertex_PCUTBN) * (int)m_vertexes.size();
	m_ibo = g_theRenderer->CreateIndexBuffer(indexBuffersize);
	m_vbo = g_theRenderer->CreateVertexBuffer(vertexBuffersize);

	g_theRenderer->CopyCPUToGPU(m_vertexes.data(), vertexBuffersize, m_vbo);
	g_theRenderer->CopyCPUToGPU(m_indexes.data(), indexBuffersize, m_ibo);
}

void Map::Update(float deltaSeconds)
{

	if (g_theInput->WasKeyJustPressed('F') && m_canFreeFly)
	{
		m_freeFlyCameraMode = !m_freeFlyCameraMode;
	}
	if (m_mode == GM_COMBAT)
	{
		if (m_game->m_isSinglePlayer)
		{
			DebugAddScreenText(Stringf("Cat health:%.1f",m_cat->m_health), AABB2(Vec2(SCREEN_SIZE_X - 600.f, SCREEN_SIZE_Y - 40.f), Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y-20.f)), 20.f, Vec2(), -1.f, Rgba8::WHITE);
			if (AreAllDemonsDestroyed())
			{
				EnterVictory();
			}
			if (g_theInput->WasKeyJustPressed('K'))
			{
				for (int i = 0; i < (int)m_actors.size(); ++i)
				{
					if (m_actors[i] && m_actors[i]->m_actorDef.m_faction == ActorFaction::DEMON && !m_actors[i]->m_isDestroyed)
					{
						m_actors[i]->Damage(m_actors[i]->m_health, m_cat);
					}
				}
			}
			else if (m_cat->m_isDead)
			{
				EnterLose();
			}
		}

		for (int i = 0; i < (int)m_game->m_players.size(); ++i)
		{
			m_game->m_players[i]->Update(deltaSeconds);
		}

		UpdateActors(deltaSeconds);
		InputFunctionsForLighting();
		//AddTextForLighting();
		//InputFunctionForRaycasting();
		if (g_theInput->WasKeyJustPressed('Q'))
		{
			m_debugCollision = !m_debugCollision;
		}
		if (!m_debugCollision)
		{

			CollideAllActorsWithEachOther();
			CollideAllActorsWithMap();
		}
		DeleteDestoryedActors();
	}
	else if (m_mode == GM_DIALOGUE)
	{
		//m_game->m_players[0]->Update(deltaSeconds);
		m_dialogSystem->Update();
		InputFunctionsForDialogue();
	}
	else if (m_mode == GM_LOSE)
	{
		if (g_theInput->WasKeyJustPressed(' ') || g_controller.WasButtonJustPressed(A))
		{
			m_game->EnterPlaying();
		}
		if (g_theInput->WasKeyJustPressed(KEYCODE_ESC) || g_controller.WasButtonJustPressed(START))
		{
			m_game->PlayGameSound(g_soundID[ST_BUTTONCLICK]);
			m_game->ExitPlaying();
		}
	}
	else if (m_mode == GM_WIN)
	{
		m_victoryMessage->Update();
		if ((g_theInput->WasKeyJustPressed(' ') || g_theInput->WasKeyJustPressed(KEYCODE_ENTER) || g_controller.WasButtonJustPressed(A)) && !m_victoryMessage->IsFullString())
		{
			m_victoryMessage->SetDialogFullString();
		}
		else if( (g_theInput->WasKeyJustPressed(' ')|| g_theInput->WasKeyJustPressed(KEYCODE_ENTER) || g_controller.WasButtonJustPressed(A))&&m_victoryMessage->IsFullString())
		{
			m_game->PlayGameSound(g_soundID[ST_BUTTONCLICK]);
			m_mode = GM_CREDIT;
		}
		if (g_theInput->WasKeyJustPressed(KEYCODE_ESC) || g_controller.WasButtonJustPressed(START))
		{
			m_game->PlayGameSound(g_soundID[ST_BUTTONCLICK]);
			m_mode = GM_CREDIT;
		}
	}
	else if (m_mode == GM_CREDIT)
	{
		if (g_theInput->WasKeyJustPressed(' ') || g_controller.WasButtonJustPressed(A))
		{
			m_game->PlayGameSound(g_soundID[ST_BUTTONCLICK]);
			m_game->ExitPlaying();
		}
	}
}
void Map::UpdateActors(float deltaSeconds)
{
	for (int i = 0; i < (int)m_actors.size(); i++)
	{
		if (m_actors[i])
		{
			m_actors[i]->Update(deltaSeconds);
		}
	}
}


void Map::CollideAllActorsWithEachOther()
{
	for (int i = 0; i < (int)m_actors.size(); i++)
	{
		for (int j = i + 1; j < (int)m_actors.size(); j++)
		{
			if (m_actors[i] && m_actors[j] && m_actors[i]->m_actorDef.m_collidesWithActors && m_actors[j]->m_actorDef.m_collidesWithActors 
				&&!m_actors[i]->m_isDead && !m_actors[j]->m_isDead
				&& m_actors[i]->m_minMaxZ.IsOverlappingWith(m_actors[j]->m_minMaxZ) && DoDiscsOverLap(m_actors[i]->m_centerXY, m_actors[i]->m_radius, m_actors[j]->m_centerXY, m_actors[j]->m_radius))
			{
				if (m_actors[i]->m_dieOnCollide && m_actors[j]->m_dieOnCollide)
				{
					continue;
				}
				if (m_actors[i]->m_owner == m_actors[j])
				{
					continue;
				}
				if (m_actors[j]->m_owner == m_actors[i])
				{
					continue;
				}
				if (m_actors[i]->m_dieOnCollide)
				{

					m_actors[i]->Die(m_actors[j]);
					m_actors[i]->OnCollide(m_actors[j]);
					continue;
				}

				if (m_actors[j]->m_dieOnCollide)
				{
					m_actors[j]->Die(m_actors[i]);
					m_actors[j]->OnCollide(m_actors[i]);
					continue;
				}
				PushActorsDiscs2DOutOfEachOther(m_actors[i], m_actors[j]);
			}
		}
	}
}


void Map::PushActorsDiscs2DOutOfEachOther(Actor* actor1, Actor* actor2)
{
	if (actor1->m_isStatic && actor2->m_isStatic)
	{
		return;
	}
	if (!actor1->m_isStatic && !actor2->m_isStatic)
	{
		PushDiscsOutOfEachOther2D(actor1->m_centerXY, actor1->m_radius, actor2->m_centerXY, actor2->m_radius);
	}
	else if (actor1->m_isStatic && !actor2->m_isStatic)
	{
		PushDiscOutOfFixedDisc2D(actor2->m_centerXY, actor2->m_radius, actor1->m_centerXY, actor1->m_radius);
	}
	else if (!actor1->m_isStatic && actor2->m_isStatic)
	{
		PushDiscOutOfFixedDisc2D(actor1->m_centerXY, actor1->m_radius, actor2->m_centerXY, actor2->m_radius);
	}
	actor1->m_position = Vec3(actor1->m_centerXY.x, actor1->m_centerXY.y, actor1->m_position.z);
	actor2->m_position = Vec3(actor2->m_centerXY.x, actor2->m_centerXY.y, actor2->m_position.z);
}

void Map::CollideAllActorsWithMap()
{
	for (int i = 0; i < (int)m_actors.size(); i++)
	{
		if (m_actors[i]&& !m_actors[i]->m_isDead  && m_actors[i]->m_actorDef.m_collidesWithWorld && !m_actors[i]->m_dieOnCollide)
		{
			int x = (int)m_actors[i]->m_position.x;
			int y = (int)m_actors[i]->m_position.y;
			PushActorOutOfTile(m_actors[i], x, y + 1);//North tile
			PushActorOutOfTile(m_actors[i], x, y - 1);//South tile
			PushActorOutOfTile(m_actors[i], x + 1, y);//East tile
			PushActorOutOfTile(m_actors[i], x - 1, y);//West tile
			PushActorOutOfTile(m_actors[i], x + 1, y + 1);//Northeast tile
			PushActorOutOfTile(m_actors[i], x - 1, y + 1);//Northwest tile
			PushActorOutOfTile(m_actors[i], x + 1, y - 1);//Southeast tile
			PushActorOutOfTile(m_actors[i], x - 1, y - 1);//Southwest tile

			//Check collision with the floor and the ceiling
			m_actors[i]->m_position.z = GetClamped(m_actors[i]->m_position.z, 0.f, 1.f - m_actors[i]->m_height);
			

		}
		else if (m_actors[i] && m_actors[i]->m_actorDef.m_collidesWithWorld &&!m_actors[i]->m_isDead && m_actors[i]->m_dieOnCollide)
		{
			CheckProjectileCollision(m_actors[i]);
		}
	}
}


void Map::PushActorOutOfTile(Actor* actor, int x, int y)
{
	
	Tile const* tile = GetTile(x, y);
// 	if (x == 0 || x == m_dimensions.x - 1 || y == 0 || y == m_dimensions.y - 1)
// 	{
// 		AABB2 bounds = tile->GetBoundXY();
// 		PushDiscOutOfFixedAABB2D(actor->m_centerXY, actor->m_radius, bounds);
// 		actor->m_position = Vec3(actor->m_centerXY.x, actor->m_centerXY.y, actor->m_position.z);
// 		//return;
// 	}
	if (tile->GetTileDefinition().m_isSolid)
	{
		AABB2 bounds = tile->GetBoundXY();
		PushDiscOutOfFixedAABB2D(actor->m_centerXY, actor->m_radius, bounds);
		actor->m_position = Vec3(actor->m_centerXY.x, actor->m_centerXY.y, actor->m_position.z);
	}

}
void Map::CheckProjectileCollision(Actor* projectileActor)
{
	int x = (int)(projectileActor->m_position.x + projectileActor->m_radius);
	int y = (int)(projectileActor->m_position.y + projectileActor->m_radius);
	Tile const* tile1 = GetTile(x, y);
	if (IsTileSolid(x, y))
	{
		AABB2 bounds = tile1->GetBoundXY();
		PushDiscOutOfFixedAABB2D(projectileActor->m_centerXY, projectileActor->m_radius, bounds);
		projectileActor->m_position = Vec3(projectileActor->m_centerXY.x, projectileActor->m_centerXY.y, projectileActor->m_position.z);
		projectileActor->m_hasCollideWithWorld = true;
		return;
	}

	x = (int)(projectileActor->m_position.x + projectileActor->m_radius);
	y = (int)(projectileActor->m_position.y - projectileActor->m_radius);
	Tile const* tile2 = GetTile(x, y);
	if (IsTileSolid(x, y))
	{
		AABB2 bounds = tile2->GetBoundXY();
		PushDiscOutOfFixedAABB2D(projectileActor->m_centerXY, projectileActor->m_radius, bounds);
		projectileActor->m_position = Vec3(projectileActor->m_centerXY.x, projectileActor->m_centerXY.y, projectileActor->m_position.z);
		projectileActor->m_hasCollideWithWorld = true;
		return;
	}

	x = (int)(projectileActor->m_position.x - projectileActor->m_radius);
	y = (int)(projectileActor->m_position.y - projectileActor->m_radius);
	Tile const* tile3 = GetTile(x, y);
	if (IsTileSolid(x, y))
	{
		AABB2 bounds = tile3->GetBoundXY();
		PushDiscOutOfFixedAABB2D(projectileActor->m_centerXY, projectileActor->m_radius, bounds);
		projectileActor->m_position = Vec3(projectileActor->m_centerXY.x, projectileActor->m_centerXY.y, projectileActor->m_position.z);
		projectileActor->m_hasCollideWithWorld = true;
		return;
	}

	x = (int)(projectileActor->m_position.x - projectileActor->m_radius);
	y = (int)(projectileActor->m_position.y + projectileActor->m_radius);
	Tile const* tile4 = GetTile(x, y);
	if (IsTileSolid(x, y))
	{
		AABB2 bounds = tile4->GetBoundXY();
		PushDiscOutOfFixedAABB2D(projectileActor->m_centerXY, projectileActor->m_radius, bounds);
		projectileActor->m_position = Vec3(projectileActor->m_centerXY.x, projectileActor->m_centerXY.y, projectileActor->m_position.z);
		projectileActor->m_hasCollideWithWorld = true;
		return;
	}

	if (projectileActor->m_minMaxZ.IsOnRange(1.f) || projectileActor->m_minMaxZ.IsOnRange(0.f))
	{
		projectileActor->m_hasCollideWithWorld = true;
	}
}


void Map::DeleteDestoryedActors()
{
	for (int i = 0; i < (int)m_actors.size(); ++i)
	{
		if (m_actors[i] && m_actors[i]->m_isDestroyed)
		{
			m_actors[i] = nullptr;
		}
	}
}

void Map::InputFunctionsForDialogue()
{
	if (m_dialogSystem->m_currentIndex > 2)
	{
		m_game->m_players[0]->SetCameraPosition(m_cat);
	}
	
	if ((g_theInput->WasKeyJustPressed(' ')||g_theInput->WasKeyJustPressed(KEYCODE_ENTER) || g_controller.WasButtonJustPressed(A))&&m_dialogSystem->m_currentMessage.IsFullString())
	{
// 		m_debugChangeCamera = true;
// 		if (m_debugChangeCamera)
// 		{
// 			m_game->m_players[0]->SetCameraPosition(m_cat);
// 		}
// 		else
// 		{
// 			m_game->m_players[0]->SetCameraPosition();
// 		}
		if (m_dialogSystem->m_currentIndex > (int)m_dialogSystem->m_messages.size() - 1)
		{
			EnterCombat();
		}
	}
}


void Map::InputFunctionForRaycasting()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE))
	{
		Vec3 rayStartPos = m_game->m_players[0]->m_position;
		Vec3 rayDirection= m_game->m_players[0]->m_orientation.GetForwardDir_XFwd_YLeft_ZUp().GetNormalized();
		Vec3 rayEndPos = rayStartPos + rayDirection * 10.f;
		DebugAddWorldLine(rayStartPos, rayEndPos, 0.01f, 10.f, Rgba8::WHITE, Rgba8::WHITE, DebugRenderMode::X_RAY);
		RaycastResult3D result = RaycastAll(rayStartPos, rayDirection, 10.f);
		if (result.m_didImpact)
		{
			DebugAddWorldPoint(result.m_impactPos, 0.06f, 10.f);
			DebugAddWorldArrow(result.m_impactPos, result.m_impactPos + 0.3f * result.m_impactNormal, 0.03f, 10.f, Rgba8::BLUE, Rgba8::BLUE);
		}
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_RIGHT_MOUSE))
	{
		Vec3 rayStartPos  = m_game->m_players[0]->m_position;
		Vec3 rayDirection = m_game->m_players[0]->m_orientation.GetForwardDir_XFwd_YLeft_ZUp().GetNormalized();
		Vec3 rayEndPos = rayStartPos + rayDirection * 0.25f;
		DebugAddWorldLine(rayStartPos, rayEndPos, 0.01f, 10.f, Rgba8::WHITE, Rgba8::WHITE, DebugRenderMode::X_RAY);
		RaycastResult3D result = RaycastAll(rayStartPos, rayDirection, 0.25f);
		if (result.m_didImpact)
		{
			DebugAddWorldPoint(result.m_impactPos, 0.06f, 10.f);
			DebugAddWorldArrow(result.m_impactPos, result.m_impactPos + 0.3f * result.m_impactNormal, 0.03f, 10.f, Rgba8::BLUE, Rgba8::BLUE);
		}
	}
}
void Map::InputFunctionsForLighting()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_F2))
	{
		m_sunDirection.x--;
		DebugAddMessage(Stringf("Sun Direction X: %5.2f", m_sunDirection.x), 5.f);
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F3))
	{
		m_sunDirection.x++;
		DebugAddMessage(Stringf("Sun Direction X: %5.2f", m_sunDirection.x), 5.f);
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F4))
	{
		m_sunDirection.y--;
		DebugAddMessage(Stringf("Sun Direction Y: %5.2f", m_sunDirection.y), 5.f);
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F5))
	{
		m_sunDirection.y++;
		DebugAddMessage(Stringf("Sun Direction Y: %5.2f", m_sunDirection.y), 5.f);
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F6))
	{
		m_sunIntensity-=0.05f;
		DebugAddMessage(Stringf("Sun Intensity: %5.2f", m_sunIntensity), 5.f);
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F7))
	{
		m_sunIntensity += 0.05f;
		DebugAddMessage(Stringf("Sun Intensity: %5.2f", m_sunIntensity), 5.f);
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F8))
	{
		m_ambientIntensity -= 0.05f;
		DebugAddMessage(Stringf("Ambient Intensity: %5.2f", m_ambientIntensity), 5.f);
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F9))
	{
		m_ambientIntensity += 0.05f;
		DebugAddMessage(Stringf("Ambient Intensity: %5.2f", m_ambientIntensity), 5.f);
	}
}

void Map::AddTextForLighting()
{
	DebugAddScreenText(
		Stringf("Sun Direction X:"),
		AABB2(Vec2(SCREEN_SIZE_X - 800.f, SCREEN_SIZE_Y - 55.f), Vec2(SCREEN_SIZE_X - 400.f, SCREEN_SIZE_Y - 40.f)), 15.f, Vec2(1.f, 0.5f), -1.f, Rgba8::WHITE);
	DebugAddScreenText(
		Stringf("%5.2f [F2 / F3 to change]", m_sunDirection.x),
		AABB2(Vec2(SCREEN_SIZE_X - 400.f, SCREEN_SIZE_Y - 55.f), Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y - 40.f)), 15.f, Vec2(0.5f, 0.5f), -1.f, Rgba8::WHITE);
	DebugAddScreenText(
		Stringf("Sun Direction Y:"),
		AABB2(Vec2(SCREEN_SIZE_X - 800.f, SCREEN_SIZE_Y - 70.f), Vec2(SCREEN_SIZE_X - 400.f, SCREEN_SIZE_Y - 55.f)), 15.f, Vec2(1.f, 0.5f), -1.f, Rgba8::WHITE);
	DebugAddScreenText(
		Stringf("%5.2f [F4 / F5 to change]", m_sunDirection.y),
		AABB2(Vec2(SCREEN_SIZE_X - 400.f, SCREEN_SIZE_Y - 70.f), Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y - 55.f)), 15.f, Vec2(0.5f, 0.5f), -1.f, Rgba8::WHITE);
	DebugAddScreenText(
		Stringf("Sun Intensity:"),
		AABB2(Vec2(SCREEN_SIZE_X - 800.f, SCREEN_SIZE_Y - 85.f), Vec2(SCREEN_SIZE_X - 400.f, SCREEN_SIZE_Y - 70.f)), 15.f, Vec2(1.f, 0.5f), -1.f, Rgba8::WHITE);
	DebugAddScreenText(
		Stringf("%5.2f [F6 / F7 to change]", m_sunIntensity),
		AABB2(Vec2(SCREEN_SIZE_X - 400.f, SCREEN_SIZE_Y - 85.f), Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y - 70.f)), 15.f, Vec2(0.5f, 0.5f), -1.f, Rgba8::WHITE);
	DebugAddScreenText(
		Stringf("Ambient Intensity:"),
		AABB2(Vec2(SCREEN_SIZE_X - 800.f, SCREEN_SIZE_Y - 100.f), Vec2(SCREEN_SIZE_X - 400.f, SCREEN_SIZE_Y - 85.f)), 15.f, Vec2(1.f, 0.5f), -1.f, Rgba8::WHITE);
	DebugAddScreenText(
		Stringf("%5.2f [F8 / F9 to change]", m_ambientIntensity),
		AABB2(Vec2(SCREEN_SIZE_X - 400.f, SCREEN_SIZE_Y - 100.f), Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y - 85.f)), 15.f, Vec2(0.5f, 0.5f), -1.f, Rgba8::WHITE);
}



void Map::Render() const
{
	if (m_mode == GM_COMBAT)
	{
		for (int i = 0; i < (int)m_game->m_players.size(); ++i)
		{
			g_theRenderer->BeginCamera(m_game->m_players[i]->m_worldCamera);
			RenderSkybox();
			RenderTiles();
			RenderActors(i);

			g_theRenderer->EndCamera(m_game->m_players[i]->m_worldCamera);
			DebugRenderWorld(m_game->m_players[i]->m_worldCamera);
			g_theRenderer->BeginCamera(m_game->m_players[i]->m_screenCamera);
			m_game->m_players[i]->RenderScreen();
			g_theRenderer->EndCamera(m_game->m_players[i]->m_screenCamera);
		}
	}
	else if (m_mode == GM_DIALOGUE)
	{
		g_theRenderer->BeginCamera(m_game->m_players[0]->m_worldCamera);
		RenderSkybox();
		RenderTiles();
		RenderActors(0);

		g_theRenderer->EndCamera(m_game->m_players[0]->m_worldCamera);
		DebugRenderWorld(m_game->m_players[0]->m_worldCamera);
		g_theRenderer->BeginCamera(m_game->m_players[0]->m_screenCamera);
		m_dialogSystem->RenderDialogueBox(m_game->m_players[0]->m_screenCamera);
		g_theRenderer->EndCamera(m_game->m_players[0]->m_screenCamera);
	}
	else if (m_mode == GM_LOSE)
	{ 
		g_theRenderer->BeginCamera(m_game->m_players[0]->m_screenCamera);
		std::vector<Vertex_PCU> verts;
		AddVertsForAABB2D(verts, m_game->m_players[0]->m_screenCamera.GetCameraBounds(), Rgba8::WHITE);
		g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
		g_theRenderer->SetDepthMode(DepthMode::DISABLED);
		g_theRenderer->BindShader(nullptr);
		g_theRenderer->SetModelConstants();
		g_theRenderer->BindTexture(g_textures[TT_LOSE]);
		g_theRenderer->DrawVertexArray((int)verts.size(), verts.data());
		g_theRenderer->EndCamera(m_game->m_players[0]->m_screenCamera);
	}
	else if (m_mode == GM_WIN)
	{
		g_theRenderer->BeginCamera(m_game->m_players[0]->m_screenCamera);
		std::vector<Vertex_PCU> verts;
		AddVertsForAABB2D(verts, m_game->m_players[0]->m_screenCamera.GetCameraBounds(), Rgba8::WHITE);
		g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
		g_theRenderer->SetDepthMode(DepthMode::DISABLED);
		g_theRenderer->BindShader(nullptr);
		g_theRenderer->SetModelConstants();
		g_theRenderer->BindTexture(g_textures[TT_VICTORY]);
		g_theRenderer->DrawVertexArray((int)verts.size(), verts.data());
		std::vector<Vertex_PCU> stringVerts;
		AABB2 bounds = AABB2(SCREEN_SIZE_X / 2 - 700.f, SCREEN_SIZE_Y / 2 - 200.f, SCREEN_SIZE_X / 2 + 700.f, SCREEN_SIZE_Y / 2 + 200.f);
		g_bitmapFont->AddVertsForTextBox2D(stringVerts, bounds, 25.f, m_victoryMessage->m_currentString, Rgba8::WHITE, 0.8f, Vec2(0.f, 1.f));
		g_theRenderer->BindTexture(&g_bitmapFont->GetTexture());
		g_theRenderer->DrawVertexArray((int)stringVerts.size(), stringVerts.data());
		g_theRenderer->EndCamera(m_game->m_players[0]->m_screenCamera);
	}
	else if (m_mode == GM_CREDIT)
	{
		g_theRenderer->BeginCamera(m_game->m_players[0]->m_screenCamera);
		std::vector<Vertex_PCU> verts;
		AddVertsForAABB2D(verts, m_game->m_players[0]->m_screenCamera.GetCameraBounds(), Rgba8::WHITE);
		g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
		g_theRenderer->SetDepthMode(DepthMode::DISABLED);
		g_theRenderer->BindShader(nullptr);
		g_theRenderer->SetModelConstants();
		g_theRenderer->BindTexture(g_textures[TT_CREDIT]);
		g_theRenderer->DrawVertexArray((int)verts.size(), verts.data());
		g_theRenderer->EndCamera(m_game->m_players[0]->m_screenCamera);
	}
}

void Map::SetGameMode(GameMode gameMode)
{
	m_mode = gameMode;
}

Actor* const Map::GetProjectileActor() const
{
	return m_projectileActor;
}
void Map::SpawnNewActors()
{
	for (int i = 0; i < (int)m_mapDef.m_infos.size(); ++i)
	{
		if (m_mapDef.m_infos[i]->m_actorDefName.compare("Demon") == 0)
		{
			SpawnActor(*m_mapDef.m_infos[i]);
		}
		else if (m_mapDef.m_infos[i]->m_actorDefName.compare("NinjaTurtle") == 0)
		{
			Actor* actor = SpawnActor(*m_mapDef.m_infos[i]);
 			if (i % 2 == 0)
 			{
				actor->m_equippedWeaponIndex = 1;
         	}
		}
		else if (m_mapDef.m_infos[i]->m_actorDefName.compare("SpawnPoint") == 0)
		{
			m_playerSpawnInfos.push_back(*m_mapDef.m_infos[i]);
		}
	}


}

void Map::SetupListeners()
{
	SoundID debug = g_theAudio->CreateOrGetSound("Data/Audio/Music/HowToDisappearCompletely.wav", true);
	m_debugSound = g_theAudio->StartSoundAt(debug, m_playBackPosition,true);
	if (m_game->m_isSinglePlayer)
	{
		g_theAudio->SetNumListeners(1);
	}
	else
	{
		g_theAudio->SetNumListeners(2);
	}
}

void Map::DebugUpdateSounds()
{

}

void Map::DebugPossessNext()
{

	int actorIndex = m_game->m_players[0]->GetActorIndexInMap();
	if (actorIndex == m_actors.size()-1)
	{
		actorIndex = 0;
	}
	else
	{
		actorIndex++;
	}

	for (int i = actorIndex; i < (int)m_actors.size(); ++i)
	{
		if (m_actors[i] && m_actors[i]->m_actorDef.m_canBePossessed)
		{
			m_game->m_players[0]->m_worldCamera.SetPerspectiveView(2.f, m_actors[i]->m_actorDef.m_cameraFOVDegrees, 0.1f, 100.f);
			m_game->m_players[0]->Possess(m_actors[i]);
			return;
		}
	}
	for (int i = 0; i < actorIndex; ++i)
	{
		if (m_actors[i] && m_actors[i]->m_actorDef.m_canBePossessed)
		{
			m_game->m_players[0]->m_worldCamera.SetPerspectiveView(2.f, m_actors[i]->m_actorDef.m_cameraFOVDegrees, 0.1f, 100.f);
			m_game->m_players[0]->Possess(m_actors[i]);
			return;
		}
	}

}

void Map::EnterDialogue()
{
	m_game->StopGameSound(m_game->m_menuSound);
	m_game->m_gameSound = m_game->PlayGameSound(g_soundID[ST_DIALOGUEBGM], true, g_gameConfigBlackboard.GetValue("musicVolume", 1.f));
	InitializeDialogSystems();
	g_theGameClock->Pause();
	SetGameMode(GM_DIALOGUE);

}
void Map::EnterLose()
{
	m_game->StopGameSound(m_game->m_gameSound);
	m_game->m_gameSound = m_game->PlayGameSound(g_soundID[ST_LOSE], false, g_gameConfigBlackboard.GetValue("musicVolume", 1.f));
	SetGameMode(GM_LOSE);
}
void Map::EnterVictory()
{
	m_victoryMessage = new DialogueMessage("After beating the Ninjas, they decided to work together \nand find the mystery of this tower. At the end, they found the evil master\n - THE SCEIENTIST Mr.Squirrel.\n With a tough battle, they conquer Mr.Squirrel and then conquer the tower of DOOM.\n \nOBTAIN 'FAKE ENDING' ACHIEVEMENT\n\nActually it'S TO BE CONTINUTED");
	m_game->StopGameSound(m_game->m_gameSound);
	m_game->m_gameSound = m_game->PlayGameSound(g_soundID[ST_VICTORY], true, g_gameConfigBlackboard.GetValue("musicVolume", 1.f));
	SetGameMode(GM_WIN);
}
void Map::EnterCombat()
{
	SetGameMode(GM_COMBAT);
	g_theGameClock->Unpause();
	m_game->StopGameSound(m_game->m_gameSound);
	m_game->StopGameSound(m_game->m_menuSound);
	m_game->m_gameSound = m_game->PlayGameSound(g_soundID[ST_GAMEBGM], true, g_gameConfigBlackboard.GetValue("musicVolume", 1.f));
}
Vec3 Map::GetCameraDirection(Vec3 const& referPos, int playerIndex) const
{
	return (referPos - m_game->m_players[playerIndex]->m_position).GetNormalized();
}

Mat44 const Map::GetCameraModelMatrix(int playerIndex) const
{
    return m_game->m_players[playerIndex]->GetModelMatrix();
	

}

Player* Map::SpawnPlayerToNewPosition(int playerIndex, int controllerIndex, int kill, int death)
{
	Player* player = new Player(m_game, playerIndex, controllerIndex, kill, death);
	if (!m_freeFlyCameraMode|| m_game->m_players[playerIndex] == nullptr)
	{
		if (m_game->m_isSinglePlayer)
		{
			player->m_worldCamera.SetPerspectiveView(2.f, 60.f, 0.1f, 10000.f);
		}
		else
		{
			player->m_worldCamera.SetPerspectiveView(4.f, 60.f, 0.1f, 10000.f);
		}
		player->m_worldCamera.SetRenderBasis(Vec3(0.f, 0.f, 1.f), Vec3(-1.f, 0.f, 0.f), Vec3(0.f, 1.f, 0.f));
		m_game->m_players[playerIndex] = player;
	}

	Actor* marineActor = SpawnActor(SpawnInfo::SpawnInfo("Marine", Vec3(), EulerAngles()));
	player->Possess(marineActor);
// 	int playerInfoSize = (int)m_playerSpawnInfos.size();
// 	int spawnIndex = g_rng->RollRandomIntInRange(1, playerInfoSize - 1);
	//int spawnIndex = g_rng->RollRandomIntInRange(0, 0);
	SpawnInfo spawnInfo;
	if (m_game->m_isSinglePlayer)
	{
		spawnInfo = m_playerSpawnInfos[0];
	}
	else
	{
		int playerInfoSize = (int)m_playerSpawnInfos.size();
 	    int spawnIndex = g_rng->RollRandomIntInRange(0, playerInfoSize - 1);
		spawnInfo = m_playerSpawnInfos[spawnIndex];
	}
	player ->GetActor()->m_position = spawnInfo.m_position;
	player ->GetActor()->m_orientation = spawnInfo.m_orientation;
	player ->GetActor()->m_centerXY = Vec2(player->GetActor()->m_position.x, player->GetActor()->m_position.y);
	player->m_position = spawnInfo.m_position;
	player->m_orientation = spawnInfo.m_orientation;
	return player;
// 	m_player->m_position.z  = ActorDefinition::GetByName("Marine")->m_eyeHeight;
// 	m_player->m_orientation = spawnInfo.m_orientation;

}

Actor* Map::SpawnActor(SpawnInfo const& spawnInfo)
{

	if (m_actors.size() < 0x0000fffeu)
	{
		for (int i = 0; i < (int)m_actors.size(); ++i)
		{
			if (!m_actors[i] || m_actors[i]->m_isDestroyed)
			{
				m_actorSalt++;
				ActorUID uid = ActorUID(m_actorSalt, i);
				Actor* actor = new Actor(this, spawnInfo);
				actor->m_uid = uid;
				if (actor->m_actorDef.m_aiEnabled)
				{
					actor->m_aiController = new AI();
// 					if (actor->m_actorDef.m_isNinja)
// 					{
// 						actor->m_aiController->m_moveTimer = new Timer(actor->m_actorDef.m_moveCooldown, g_theGameClock);
// 						actor->m_aiController->m_moveTimer->Start();
// 					}
					actor->m_controller = actor->m_aiController;
					actor->m_controller->Possess(actor);
				}
				m_actors[i] = actor;

				return actor;
			}
		}
		m_actorSalt++;
		ActorUID uid = ActorUID(m_actorSalt, (unsigned int)m_actors.size());
		Actor* actor = new Actor(this, spawnInfo);
		actor->m_uid = uid;
		if (actor->m_actorDef.m_aiEnabled)
		{
			actor->m_aiController = new AI();
			actor->m_aiController->Possess(actor);
// 			if (actor->m_actorDef.m_isNinja)
// 			{
// 				actor->m_aiController->m_moveTimer = new Timer(actor->m_actorDef.m_moveCooldown, g_theGameClock);
// 				actor->m_aiController->m_moveTimer->Start();
// 			}
			actor->m_controller = actor->m_aiController;
			
		}
		m_actors.push_back(actor);
		return actor;
	}
	else
	{
		return nullptr;
	}

}

Actor* Map::GetActorByUID(ActorUID const uid) const
{
	if (uid.IsValid())
	{
		int index = uid.GetIndex();
		if (m_actors[index])
		{
			if (m_actors[index]->m_uid != uid)
			{
				return nullptr;
			}
			else
			{
				return m_actors[index];
			}
		}
		else
		{
			return nullptr;
		}
	}
	return nullptr;
}

void Map::RenderTiles() const
{
	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->BindShader(m_shader);
	g_theRenderer->SetLightingConstants(m_sunDirection.GetNormalized(), m_sunIntensity, m_ambientIntensity);
	g_theRenderer->BindTexture(&m_spriteSheet->GetTexture());
	g_theRenderer->DrawVertexTBN((int)m_indexes.size(), m_vbo, m_ibo);
}

void Map::RenderActors(int playerIndex) const
{
	for (int i = 0; i < (int)m_actors.size(); ++i)
	{
		if (m_actors[i])
		{
			m_actors[i]->Render(playerIndex);
		}
	}
}

void Map::RenderSkybox() const
{
	std::vector<Vertex_PCU> verts;
	//AddVertsForSphere3D(verts, Vec3(), 1000.f);
	Vec3 points[NUM_CUBEPOINTS];
	AABB3 bounds(-1000.f,-1000.f,-1000.f, 1000.f, 1000.f, 1000.f);
	points[BLN] = Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z);
	points[BRN] = Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z);
	points[BRF] = Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z);
	points[BLF] = Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z);
	points[TLN] = Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z);
	points[TRN] = Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z);
	points[TRF] = Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z);
	points[TLF] = Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z);
	float t = 0.33333f;
	float t2 = 0.66f;
	AddVertsForQuad3D(verts, points[BRN], points[BLN], points[TLN], points[TRN], Rgba8::WHITE, AABB2(Vec2(0.f, t), Vec2(0.25f, t2)));
	AddVertsForQuad3D(verts, points[BRF], points[BRN], points[TRN], points[TRF], Rgba8::WHITE, AABB2(Vec2(0.75f, t), Vec2(1.f, t2)));
	AddVertsForQuad3D(verts, points[BLF], points[BRF], points[TRF], points[TLF], Rgba8::WHITE, AABB2(Vec2(0.5f, t), Vec2(0.75f, t2)));
	AddVertsForQuad3D(verts, points[BLN], points[BLF], points[TLF], points[TLN], Rgba8::WHITE, AABB2(Vec2(0.25f, t), Vec2(0.5f, t2)));
	AddVertsForQuad3D(verts, points[TLF], points[TRF], points[TRN], points[TLN], Rgba8::WHITE, AABB2(Vec2(0.5f,0.67f), Vec2(0.75f, 1.f)));
	AddVertsForQuad3D(verts, points[BLF], points[BRF], points[BRN], points[BLN], Rgba8::WHITE, AABB2(Vec2(0.5f, 0.f), Vec2(0.75f, t)));
	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->SetDepthMode(DepthMode::DISABLED);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(m_mapDef.m_skyboxTexture);
	g_theRenderer->DrawVertexArray((int)verts.size(), verts.data());
}

bool Map::IsPositionInBounds(Vec3 position, float const tolenrance) const
{
	AABB3 bounds = AABB3(-tolenrance,-tolenrance, -tolenrance, m_dimensions.x + tolenrance, m_dimensions.y+ tolenrance, 1.f+ tolenrance);
	return IsPointInsideAABB3D(position, bounds);
}

bool Map::AreCoordsInBounds(int x, int y) const
{
	if (x < 0 || y < 0 || x > m_dimensions.x - 1 || y > m_dimensions.y - 1)
	{
		return false;
	}
	return true;
}

bool Map::AreAllDemonsDestroyed() const
{
	for (int i = 0; i < (int)m_actors.size(); ++i)
	{
		if (m_actors[i] &&m_actors[i]->m_actorDef.m_faction==ActorFaction::DEMON &&!m_actors[i]->m_isDestroyed )
		{
			return false;
		}
	}
	return true;
}

Tile const* Map::GetTile(int x, int y) const
{
	if (!AreCoordsInBounds(x, y))
	{
		ERROR_AND_DIE("The tile is out of bounds");
	}
	int tileIndex = x + y * m_dimensions.x;
	return &m_tiles[tileIndex];
}

bool const Map::IsTileSolid(int x, int y) const
{
	return GetTile(x, y)->GetTileDefinition().m_isSolid;
}

RaycastResult3D Map::GetRayWeaponResultAndDealDamage(Vec3 const& start, Vec3 const& direction, float distance, Actor* exceptionActor)
{
	RaycastResult3D result;
	float closestImpactDistance = 99999.f;
	int rayCastIndex = -1;
	for (int i = 0; i < (int)m_actors.size(); ++i)
	{
		if (m_actors[i] && m_actors[i] != exceptionActor && !m_actors[i]->m_isDead)
		{
			RaycastResult3D t_result = RaycastVsCylinderZ3D(start, direction, distance, m_actors[i]->m_centerXY, m_actors[i]->m_minMaxZ, m_actors[i]->m_radius);
			if (t_result.m_didImpact && t_result.m_impactDist < closestImpactDistance)
			{
				result = t_result;
				closestImpactDistance = t_result.m_impactDist;
				rayCastIndex = i;
			}
		}
	}
	RaycastResult3D t_result = RaycastWorldFloorAndCeiling(start, direction, distance);
	if (t_result.m_didImpact && t_result.m_impactDist < closestImpactDistance)
	{
		result = t_result;
		closestImpactDistance = t_result.m_impactDist;
		rayCastIndex = -1;
	}
	t_result = RaycastWorldTiles(start, direction, distance);
	if (t_result.m_didImpact && t_result.m_impactDist < closestImpactDistance)
	{
		result = t_result;
		closestImpactDistance = t_result.m_impactDist;
		rayCastIndex = -1;
	}

	m_rayTargetActorindex = rayCastIndex;
	return result;
}

ActorUID Map::GetTargetDemonActorUID() const
{
	if (m_rayTargetActorindex != -1 )
	{
		return m_actors[m_rayTargetActorindex]->m_uid;
	}
	return ActorUID::INVALID;
}

ActorUID Map::GetTargetActorUID(float arcDegree, float arcRadius, Vec2 const& fwdXY, Vec2 const& tipPosXY, ActorFaction faction) const
{
	for (int i = 0; i < (int)m_actors.size(); ++i)
	{
		if (m_actors[i]&& !m_actors[i]->m_isDead && m_actors[i]->m_actorDef.m_faction == faction)
		{
			if (IsPointInsideDirectedSector2D(m_actors[i]->m_centerXY, tipPosXY, fwdXY, arcDegree, arcRadius))
			{

				Vec3 starPos = Vec3(tipPosXY.x, tipPosXY.y, 0.75f);
				Vec3 dir = (m_actors[i]->GetEyePosition() - starPos).GetNormalized();

				RaycastResult3D result = RaycastWorldTiles(starPos, dir , arcRadius);
				if (result.m_didImpact)
				{
					if (result.m_impactDist >= (m_actors[i]->m_position - starPos).GetLength())
					{
						return m_actors[i]->m_uid;
					}
				}
				else
				{
					return m_actors[i]->m_uid;
				}
				
			}
		}
	}
	
	return ActorUID::INVALID;
}

Game* Map::GetGame() const
{
	return m_game;
}

Player* Map::GetPlayer(int playerIndex) const
{
	return m_game->GetPlayer(playerIndex);
}

Vec3 const Map::GetPlayerPosition(int playerIndex) const
{
	return m_game->GetPlayer(playerIndex)->m_position;
}

RaycastResult3D Map::RaycastAll(Vec3 const& start, Vec3 const& direction, float distance) const
{
	RaycastResult3D result;
	float closestImpactDistance = 99999.f;
	RaycastResult3D t_result = RaycastWorldActors(start, direction, distance);
	if (t_result.m_didImpact && t_result.m_impactDist < closestImpactDistance)
	{
		closestImpactDistance = t_result.m_impactDist;
		result = t_result;
	}
	t_result = RaycastWorldFloorAndCeiling(start, direction, distance);
	if (t_result.m_didImpact && t_result.m_impactDist < closestImpactDistance)
	{
		result = t_result;
		closestImpactDistance = t_result.m_impactDist;
	}

	t_result = RaycastWorldTiles(start, direction, distance);
	if (t_result.m_didImpact && t_result.m_impactDist < closestImpactDistance)
	{
		result = t_result;
		closestImpactDistance = t_result.m_impactDist;
	}
	return result;
}

RaycastResult3D Map::RaycastWorldTiles(Vec3 const& start, Vec3 const& direction, float distance) const
{
	RaycastResult3D result;
	int tileX = (int)start.x;
	int tileY = (int)start.y;
	if (!AreCoordsInBounds(tileX, tileY))
	{
		return result;
	}
	if (IsTileSolid(tileX, tileY))
	{
		if (IsPositionInBounds(start))
		{
			result.m_didImpact = true;
			result.m_impactPos = start;
			result.m_impactDist = 0.f;
			result.m_impactNormal = -1.f * direction;
			return result;
		}
	}

	float fwdDistPerXCrossing = 1.f / AbsFloat(direction.x); // How far forward between each X crossing
	int tileStepDirectionX = 0; // The one tile step forward horizontally on x
	if (direction.x < 0.f)
	{
		tileStepDirectionX = -1;
	}
	else
	{
		tileStepDirectionX = 1;
	}
	float xAtFirstXCrossing = tileX + (tileStepDirectionX + 1.f) / 2.f; //This is the x coordinate of the first x-crossing.  Essentially rounding up (east) or down (west).
	float xDistToFirstXCrossing = xAtFirstXCrossing - start.x; // How far on x we must go until the first x-crossing.
	float fwdDistAtNextXCrossing = AbsFloat(xDistToFirstXCrossing) * fwdDistPerXCrossing; //How far forward we must go until the first x - crossing.

	float fwdDistPerYCrossing = 1.f / AbsFloat(direction.y); // How far forward between each Y crossing
	int tileStepDirectionY = 0; // The one tile step forward horizontally on y
	if (direction.y < 0.f)
	{
		tileStepDirectionY = -1;
	}
	else
	{
		tileStepDirectionY = 1;
	}
	float yAtFirstYCrossing = tileY + (tileStepDirectionY + 1.f) / 2.f; //This is the y coordinate of the first y-crossing. 
	float yDistToFirstYCrossing = yAtFirstYCrossing - start.y; // How far on y we must go until the first y-crossing.
	float fwdDistAtNextYCrossing = AbsFloat(yDistToFirstYCrossing) * fwdDistPerYCrossing; //How far forward we must go until the first y - crossing.
	while (1)
	{
		if (fwdDistAtNextXCrossing < fwdDistAtNextYCrossing) //Determines whether we'll cross X or Y line next, going forward from the last crossing point.
		{
			if (fwdDistAtNextXCrossing > distance) //This means next crossing would be beyond the end of the ray!
			{
				return result;
			}
			tileX += tileStepDirectionX; //Step horizontally forward (tileX +1 or -1); we’ve officially crossed over from one tile to the next
			Vec3 pos = start + direction * fwdDistAtNextXCrossing;
			if (IsPositionInBounds(pos) && AreCoordsInBounds(tileX, tileY))
			{
				if (IsTileSolid(tileX, tileY))
				{
					result.m_didImpact = true;
					result.m_impactDist = fwdDistAtNextXCrossing;
					result.m_impactPos = pos;
					AABB2 boundXY = GetTile(tileX, tileY)->GetBoundXY();
					if (start.x < boundXY.m_mins.x)
					{
						result.m_impactNormal = Vec3(-1.f, 0.f, 0.f);  // Impact on the west face
					}
					else
					{
						result.m_impactNormal = Vec3(1.f, 0.f, 0.f);   // Impact on the east face
					}
					return result;
				}
			}
		    
			fwdDistAtNextXCrossing += fwdDistPerXCrossing;
		}
		else //going forward cross Y line
		{
			if (fwdDistAtNextYCrossing > distance) //This means next crossing would be beyond the end of the ray!
			{
				return result;
			}
			tileY += tileStepDirectionY; //Step horizontally forward (tileY +1 or -1); we’ve officially crossed over from one tile to the next
			Vec3 pos = start + direction * fwdDistAtNextYCrossing;
			if (IsPositionInBounds(pos) && AreCoordsInBounds(tileX, tileY))
			{
			     if (IsTileSolid(tileX, tileY))
			     {

					result.m_didImpact = true;
					result.m_impactDist = fwdDistAtNextYCrossing;
					result.m_impactPos = pos;
					AABB2 boundXY = GetTile(tileX, tileY)->GetBoundXY();
					if (start.y < boundXY.m_mins.y)
					{
						result.m_impactNormal = Vec3(0.f, -1.f, 0.f);  // Impact on the south face
					}
					else
					{
						result.m_impactNormal = Vec3(0.f, 1.f, 0.f);   // Impact on the north face
					}
					return result;
				}
			}
			fwdDistAtNextYCrossing += fwdDistPerYCrossing;
		}
	}
	
	return result;
}

RaycastResult3D Map::RaycastWorldFloorAndCeiling(Vec3 const& start, Vec3 const& rayNormal, float distance) const
{
	RaycastResult3D result;
	Vec3 direction = rayNormal * distance;
	if (direction.z > 0.f)
	{

		float t = (1.f - start.z) / direction.z;
		if (t >= 0.f && t <= 1.f)
		{

			result.m_didImpact = true;
			result.m_impactPos = start + direction * t;
			result.m_impactDist = (result.m_impactPos - start).GetLength();
			result.m_impactNormal = Vec3(0.f, 0.f, -1.f);
			return result;
		}
	}
	else if (direction.z < 0.f)
	{
		float t = (0.f- start.z)/ direction.z;

		if (t >= 0.f && t <= 1.f)
		{
			result.m_didImpact = true;
			result.m_impactPos = start + direction * t;
			result.m_impactDist = (result.m_impactPos - start).GetLength();
			result.m_impactNormal = Vec3(0.f, 0.f, 1.f);
			return result;
		}
	}
	return result;
}

RaycastResult3D Map::RaycastWorldActors(Vec3 const& start, Vec3 const& direction, float distance) const
{
	RaycastResult3D result;
	float closestImpactDistance = 99999.f;
	for (int i = 0; i < (int)m_actors.size(); ++i)
	{
		RaycastResult3D t_result = RaycastVsCylinderZ3D(start, direction, distance, m_actors[i]->m_centerXY, m_actors[i]->m_minMaxZ, m_actors[i]->m_radius);
		if (t_result.m_didImpact && t_result.m_impactDist < closestImpactDistance)
		{
			result = t_result;
			closestImpactDistance = t_result.m_impactDist;
		}
	}
	return result;
}


