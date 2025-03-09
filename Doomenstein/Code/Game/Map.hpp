#pragma once
#include <vector>
#include "Game/Tile.hpp"
#include "Game/Gamecommon.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/DialogueMessage.hpp"
class Game;
class IndexBuffer;
class VertexBuffer;
class SpriteSheet;
class Actor;
struct ActorUID;
class Player;
struct SpawnInfo;

class Map
{
public:
	Map(Game* game, MapDefinition mapDef);
	~Map();
	void Update(float deltaSeconds);
	void Render() const;
	void SetGameMode(GameMode gameMode);
	Actor* const GetProjectileActor() const;
	Vec3 GetCameraDirection(Vec3 const& referPos, int playerIndex) const;
	Mat44 const GetCameraModelMatrix(int playerIndex) const;
	Player* SpawnPlayerToNewPosition(int playerIndex, int controllerIndex, int kill = 0, int death = 0);
	Actor* SpawnActor(SpawnInfo const& spawnInfo);
	Actor* GetActorByUID(ActorUID const uid) const;
	RaycastResult3D GetRayWeaponResultAndDealDamage(Vec3 const& start, Vec3 const& direction, float distance, Actor* exceptionActor = nullptr);
	ActorUID GetTargetDemonActorUID()const;
	ActorUID GetTargetActorUID(float arcDegree, float arcRadius, Vec2 const& fwdXY, Vec2 const& tipPosXY, ActorFaction faction = ActorFaction::MARINE) const;
	Game* GetGame() const;
	Player* GetPlayer(int playerIndex) const;
	Vec3 const GetPlayerPosition(int playerIndex) const;
	void DebugPossessNext();
	void EnterDialogue();
	void EnterLose();
	void EnterVictory();

	bool m_isHUDHidden = false;
	bool m_freeFlyCameraMode = false;
	bool m_canFreeFly = true;
	bool m_canDebugPossession = true;
	Shader* m_shader = nullptr;
	GameMode m_mode = GM_DIALOGUE;
private:
	void SpawnCat();
	void SpawnNinja();
	void PopulateTiles();
	void AddVertsForTiles();
	void SpawnNewActors();
	void SetupListeners();
	void DebugUpdateSounds();
	void UpdateActors(float deltaSeconds);
	void CollideAllActorsWithEachOther();
	void PushActorsDiscs2DOutOfEachOther(Actor* actor1, Actor* actor2);
	void CollideAllActorsWithMap();
	void PushActorOutOfTile(Actor* actor, int x, int y);
	void DeleteDestoryedActors();
	void InputFunctionsForDialogue();
	void InitializeDialogSystems();
	void EnterCombat();
	void InputFunctionForRaycasting();
	void InputFunctionsForLighting();
	void AddTextForLighting();
	void CheckProjectileCollision(Actor* projectileActor);
	void RenderTiles() const;
	void RenderActors(int playerIndex) const;
	void RenderSkybox() const;
	bool IsPositionInBounds(Vec3 position, float const tolenrance = 0.f) const;
	bool AreCoordsInBounds(int x, int y) const;
	bool AreAllDemonsDestroyed() const;
	Tile const* GetTile(int x, int y) const;
	bool const IsTileSolid(int x, int y) const;


	RaycastResult3D RaycastAll(Vec3 const& start, Vec3 const& direction, float distance) const;
	RaycastResult3D RaycastWorldTiles(Vec3 const& start, Vec3 const& direction, float distance) const;
	RaycastResult3D RaycastWorldFloorAndCeiling(Vec3 const& start, Vec3 const& dirNormal, float distance) const;
	RaycastResult3D RaycastWorldActors(Vec3 const& start, Vec3 const& direction, float distance) const;

private:
	DialogueSystem* m_dialogSystem = nullptr;
	int m_rayTargetActorindex = -1;
	Actor* m_projectileActor = nullptr;
	std::vector<Actor*> m_actors;
	std::vector<Actor*> m_processedActors;
	static const unsigned int MAX_ACTOR_SALT = 0x0000ffffu;
	unsigned int m_actorSalt = MAX_ACTOR_SALT;
	std::vector<SpawnInfo> m_playerSpawnInfos;
	bool m_debugChangeCamera = false;
	std::vector<Tile> m_tiles;
	std::vector<Vertex_PCUTBN> m_vertexes;
	std::vector<unsigned int> m_indexes;
	Vec3 m_sunDirection = Vec3(2.f, 1.f, -1.f);
	float m_sunIntensity = 0.85f;
	float m_ambientIntensity = 0.35f;
	SpriteSheet* m_spriteSheet = nullptr;
	MapDefinition m_mapDef;
	IntVec2 m_dimensions;
	Game* m_game = nullptr;
	IndexBuffer* m_ibo = nullptr;
	VertexBuffer* m_vbo = nullptr;
	Actor* m_cat = nullptr;
	bool m_debugCollision = false;
	SoundPlaybackID m_debugSound;
	Vec3 m_playBackPosition = Vec3(26.5f, 10.5f, 0.f);
	DialogueMessage* m_victoryMessage = nullptr;
};