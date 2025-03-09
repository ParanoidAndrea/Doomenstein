#pragma once

#include "GameCommon.hpp"
#include "Engine/Render/Camera.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Game/DialogueMessage.hpp"


class App;
class Clock;
class Player;
class Timer;
class IndexBuffer;
class VertexBuffer;
class Map;
struct ActorUID;

enum GameState
{
	ATTRACT,
	LOBBY,
	STORY,
	PLAYING
};
enum class StoryScene
{
	SCENE1,
	SCENE2,
	SCENE3,
	SCENE4,
	NUM_SCENES
};
class Game
{
	
public:
	Game(App* owner);
	~Game();
	App* m_owner;
	void Startup();
	void Update();
	void Render() const;
	bool IsAttract() const;
	Camera m_screenCamera = Camera(Vec2(0.f, 0.f), Vec2(SCREEN_SIZE_X,SCREEN_SIZE_Y));
	Camera m_splitScreenCamera1 = Camera(Vec2(0.f, 0.f), Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y/2.f));
	Camera m_splitScreenCamera2 = Camera(Vec2(0.f, 0.f), Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y / 2.f));
	ActorUID const GetActorUID(int playerIndex) const;
	int GetPlayerIndex(ActorUID const& uid) const;
	Player* GetPlayer(ActorUID const& uid) const;
	Player* GetPlayer(int index) const;
	std::vector<Player*> m_players;
	bool m_isSinglePlayer = true;
	SoundPlaybackID PlayGameSound(SoundID soundID, bool isLoop = false, float volume = 1.f);
	SoundPlaybackID m_gameSound;
	SoundPlaybackID m_menuSound;
	void ExitPlaying();
	void EnterPlaying();
	void StopGameSound(SoundPlaybackID soundPlaybackID);
	bool m_hasEnterDialogue = false;
private:

	void UpdateStory();
	void UpdatePlaying();
	void UpdateAttract();
	void UpdateLobby();
	void RenderAttract() const;
	void RenderLobby() const;
	void RenderStory() const;
	void RenderPlaying() const;
	void UpdatePlayingInputs();
	void EnterAttract();
	void EnterLobby();

	void EnterStory();
	void ExitStory();
	void ExitAttract(bool isController);
	void ExitLobby();
	
private:

	bool m_isPlayer1Controller = false;
	bool m_isPlayer2OnScreen = false;
	StoryScene m_scene = StoryScene::SCENE1;
	GameState m_state = GameState::ATTRACT;
	Map* m_currentMap = nullptr;
	DialogueMessage* m_message = nullptr;
	bool m_firstPlayGame = true;

    //Vec3 m_startPosition = Vec3(0.f, 0.f, -2.f);
};
