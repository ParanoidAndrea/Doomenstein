#include "Game.hpp"
#include "Game/App.hpp"
#include "Engine/Core/SimpleTriangleFont.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Game/Player.hpp"
#include "Game/ActorUID.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Render/DebugRender.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Render/IndexBuffer.hpp"
#include "Engine/Render/VertexBuffer.hpp"
#include "Engine/Render/BitmapFont.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/WeaponDefinition.hpp"
#include "Game/Map.hpp"

Clock* g_theGameClock = nullptr;
Clock* g_theUIClock = nullptr;
Game::Game(App* owner)
	:m_owner(owner)
{
	m_splitScreenCamera1.SetNormalizedViewport(AABB2(0.f, 0.5f, 1.f, 1.f));
	m_splitScreenCamera2.SetNormalizedViewport(AABB2(0.f, 0.0f, 1.f, 0.5f));
}

Game::~Game()
{
	delete m_currentMap;
	m_currentMap = nullptr;

}

void Game::Startup()
{
	g_theGameClock = new Clock(Clock::GetSystemClock());
	g_theUIClock = new Clock(Clock::GetSystemClock());
	
	EnterAttract();
	//EnterPlaying();
}

void Game::Update()
{

	if (m_state == PLAYING)
	{
		UpdatePlaying();
	}
	else if(m_state == ATTRACT)
	{
		UpdateAttract();
	}
	else if (m_state == LOBBY )
	{
		UpdateLobby();
	}
	else if (m_state == STORY)
	{
		UpdateStory();
	}

}


void Game::UpdatePlaying()
{
	//std::string gameScreenText = Stringf("[Game Clock] Time:%.2f FPS:%.1f Scale:%.1f ", g_theGameClock->GetTotalSeconds(), g_theGameClock->GetFrameCount() / g_theGameClock->GetDeltaSeconds(), g_theGameClock->GetTImeScale());
	//DebugAddScreenText(gameScreenText, AABB2(Vec2(SCREEN_SIZE_X - 600.f, SCREEN_SIZE_Y - 20.f), Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y)), 20.f, Vec2(), -1.f, Rgba8::WHITE);
	UpdatePlayingInputs();
	if (m_currentMap)
	{
		m_currentMap->Update(g_theGameClock->GetDeltaSeconds());
	}
	//DebugAddScreenText(Stringf("Camera position: %.1f, %.1f, %.1f", m_players[0]->m_position.x, m_players[0]->m_position.y, m_players[0]->m_position.z), AABB2(Vec2(0.f, SCREEN_SIZE_Y-20.f), Vec2(400.f, SCREEN_SIZE_Y)), 20.f, Vec2(), -1.f, Rgba8::BLACK);
}

void Game::UpdateStory()
{
	if (m_scene != StoryScene::NUM_SCENES)
	{
		if ((g_theInput->WasKeyJustPressed(KEYCODE_ENTER) || g_theInput->WasKeyJustPressed(' '))&& m_message->IsFullString())
		{
			m_scene = (StoryScene)((int)m_scene + 1);
			std::string attractString;
			switch (m_scene)
			{
			case StoryScene::SCENE1:
				attractString = "Once upon a town, there is a mystery tower located in a deserted place. \nNo one never ever, got away from that tower. They said, it is the tower of DOOM. ";
				break;
			case StoryScene::SCENE2:
				attractString = "You, called yourself the Mighty Doomer, out of deadly curiosity, \ndecided to find the secret of the tower of DOOM.";
				break;
			case StoryScene::SCENE3:
				attractString = "However, the moment you step into the tower, \nyou were surrounded by the deadly demons.\n Then, you lost consciousness. ";
				break;
			case StoryScene::SCENE4:
				attractString = "All you have left in your mind is the tower of DOOM";
				break;
			}
			m_message = new DialogueMessage(attractString, 5.f);
		}
		else if(g_theInput->WasKeyJustPressed(KEYCODE_ENTER) || g_theInput->WasKeyJustPressed(' '))
		{
			m_message->SetDialogFullString();
		}
	}
	else
	{
		ExitStory();
	}
	m_message->Update();
}

void Game::UpdateAttract()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC) || g_controller.WasButtonJustPressed(BACK))
	{
		PlayGameSound(g_soundID[ST_BUTTONCLICK]);
		FireEvent("Quit");
	}
	if (g_theInput->WasKeyJustPressed('N') || g_theInput->WasKeyJustPressed(' '))
	{
		PlayGameSound(g_soundID[ST_BUTTONCLICK]);
		ExitAttract(false);
		return;
	}

	if (g_controller.WasButtonJustPressed(XboxButtonID::A) || g_controller.WasButtonJustPressed(XboxButtonID::START))
	{
		PlayGameSound(g_soundID[ST_BUTTONCLICK]);
		ExitAttract(true);
		return;
	}
}


void Game::UpdateLobby()
{
	if (!m_isPlayer1Controller)
	{

		if (m_isSinglePlayer && g_theInput->WasKeyJustPressed(' ') && m_isPlayer2OnScreen)
		{
			m_isSinglePlayer = false;
			PlayGameSound(g_soundID[ST_BUTTONCLICK]);
			return;
		}
		if (m_isSinglePlayer && g_controller.WasButtonJustPressed(START) && !m_isPlayer2OnScreen)
		{
			m_isSinglePlayer = false;
			m_isPlayer2OnScreen = true;
			PlayGameSound(g_soundID[ST_BUTTONCLICK]);
			return;
		}
		if (!m_isSinglePlayer && g_controller.WasButtonJustPressed(BACK))
		{
			m_isSinglePlayer = true;
			m_isPlayer2OnScreen = false;
			PlayGameSound(g_soundID[ST_BUTTONCLICK]);
			return;
		}

		if (!m_isSinglePlayer && g_theInput->WasKeyJustPressed(KEYCODE_ESC))
		{
			m_isSinglePlayer = true;
			PlayGameSound(g_soundID[ST_BUTTONCLICK]);
			return;
		}
		if (m_isSinglePlayer && g_theInput->WasKeyJustPressed(' ') && !m_isPlayer2OnScreen)
		{
			PlayGameSound(g_soundID[ST_BUTTONCLICK]);
			ExitLobby();
		}
		if (m_isSinglePlayer && g_controller.WasButtonJustPressed(START) && m_isPlayer2OnScreen)
		{
			PlayGameSound(g_soundID[ST_BUTTONCLICK]);
			ExitLobby();
		}

		if (!m_isSinglePlayer && (g_controller.WasButtonJustPressed(START) || g_theInput->WasKeyJustPressed(' ')))
		{
			PlayGameSound(g_soundID[ST_BUTTONCLICK]);
			ExitLobby();
		}
		if (m_isSinglePlayer && g_theInput->WasKeyJustPressed(KEYCODE_ESC) && !m_isPlayer2OnScreen)
		{
			PlayGameSound(g_soundID[ST_BUTTONCLICK]);
			EnterAttract();
			return;
		}
		if (m_isSinglePlayer && g_controller.WasButtonJustPressed(BACK) && m_isPlayer2OnScreen)
		{
			PlayGameSound(g_soundID[ST_BUTTONCLICK]);
			EnterAttract();
			return;
		}
	}
	else
	{
		if (!m_isSinglePlayer && g_theInput->WasKeyJustPressed(KEYCODE_ESC))
		{
			m_isSinglePlayer = true;
			m_isPlayer2OnScreen = false;
			PlayGameSound(g_soundID[ST_BUTTONCLICK]);
			return;
		}
		if (m_isSinglePlayer && g_theInput->WasKeyJustPressed(KEYCODE_ESC) && m_isPlayer2OnScreen)
		{
			PlayGameSound(g_soundID[ST_BUTTONCLICK]);
			EnterAttract();
			return;
		}
		if (m_isSinglePlayer && g_controller.WasButtonJustPressed(BACK) && !m_isPlayer2OnScreen)
		{
			PlayGameSound(g_soundID[ST_BUTTONCLICK]);
			EnterAttract();
			return;
		}
		if (m_isSinglePlayer && g_controller.WasButtonJustPressed(START) && m_isPlayer2OnScreen)
		{
			PlayGameSound(g_soundID[ST_BUTTONCLICK]);
			m_isSinglePlayer = false;
			return;
		}


		if (m_isSinglePlayer && g_theInput->WasKeyJustPressed(' ') && !m_isPlayer2OnScreen)
		{
			PlayGameSound(g_soundID[ST_BUTTONCLICK]);
			m_isSinglePlayer = false;
			m_isPlayer2OnScreen = true;
			return;
		}

		if (m_isSinglePlayer && g_controller.WasButtonJustPressed(START) && !m_isPlayer2OnScreen)
		{
			PlayGameSound(g_soundID[ST_BUTTONCLICK]);
			ExitLobby();
			return;
		}
		if (m_isSinglePlayer && g_theInput->WasKeyJustPressed(' ') && m_isPlayer2OnScreen)
		{
			PlayGameSound(g_soundID[ST_BUTTONCLICK]);
			ExitLobby();
			return;
		}
		if (!m_isSinglePlayer && (g_controller.WasButtonJustPressed(START) || g_theInput->WasKeyJustPressed(' ')))
		{
			PlayGameSound(g_soundID[ST_BUTTONCLICK]);
			ExitLobby();
		}

		if (!m_isSinglePlayer && g_controller.WasButtonJustPressed(BACK))
		{
			PlayGameSound(g_soundID[ST_BUTTONCLICK]);
			m_isSinglePlayer = true;
			return;
		}

	}
}


void Game::UpdatePlayingInputs()
{

	if (g_theInput->WasKeyJustPressed('1'))
	{
		//DebugAddWorldLine(m_player->m_position, m_player->m_position + m_player->m_orientation.GetForwardDir_XFwd_YLeft_ZUp() * 20.f, 0.1f, 10.f, Rgba8::YELLOW, Rgba8::YELLOW, DebugRenderMode::X_RAY);
	}
	if (g_theInput->IsKeyDown('2'))
	{
		//DebugAddWorldPoint(m_player->m_position - Vec3(0.f, 0.f, 5.f), 0.5f, 60.f, Rgba8(150, 75, 0, 255));
	}
	if (g_theInput->WasKeyJustPressed('3'))
	{
		//DebugAddWorldWiredSphere(m_player->m_position + m_player->m_orientation.GetForwardDir_XFwd_YLeft_ZUp()*2.f,1.f, 5.f, Rgba8::GREEN,Rgba8::RED);
	}
	if (g_theInput->WasKeyJustPressed('4'))
	{
		//DebugAddWorldBasis(m_player->GetModelMatrix(), 20.f);
	}
	if (g_theInput->WasKeyJustPressed('5'))
	{
		//DebugAddWorldBillboardText(Stringf("Position: %.1f, %.1f, %.1f Orientation: %.1f, %.1f, %.1f", m_player->m_position.x, m_player->m_position.y, m_player->m_position.z, m_player->m_orientation.m_yawDegrees, m_player->m_orientation.m_pitchDegrees, m_player->m_orientation.m_rollDegrees), m_player->m_position + 2.f*m_player->m_orientation.GetForwardDir_XFwd_YLeft_ZUp(), 0.25f, Vec2(0.5f,0.5f),10.f, Rgba8::WHITE, Rgba8::RED);
	}
	if (g_theInput->WasKeyJustPressed('6'))
	{
		//DebugAddWorldWireCylinder(m_player->m_position, m_player->m_position + Vec3(0, 0.f, 2.5f), 1.f, 10.f, Rgba8::WHITE, Rgba8::RED);
	}
	if (g_theInput->WasKeyJustPressed('7'))
	{
		//DebugAddMessage(Stringf("Camera orientation: %.1f, %.1f, %.1f", m_player->m_orientation.m_yawDegrees, m_player->m_orientation.m_pitchDegrees, m_player->m_orientation.m_rollDegrees), 5.f);
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC) || g_controller.WasButtonJustPressed(BACK))
	{
		PlayGameSound(g_soundID[ST_BUTTONCLICK]);
		ExitPlaying();
	}

// 	if (g_theInput->WasKeyJustPressed('P'))
// 	{
// 		g_theGameClock->TogglePause();
// 	}
// 
// 	if (g_theInput->WasKeyJustPressed('T'))
// 	{
// 		if (g_theGameClock->GetTImeScale() == 0.1f)
// 		{
// 			g_theGameClock->SetTimeScale(1.f);
// 		}
// 		else
// 		{
// 			g_theGameClock->SetTimeScale(0.1f);
// 		}
// 	}
// 
// 	if (g_theInput->WasKeyJustPressed('O'))
// 	{
// 		g_theGameClock->StepSingleFrame();
// 	}


}

void Game::EnterAttract()
{

	m_isPlayer2OnScreen = false;
	if (m_state != LOBBY)
	{
		StopGameSound(m_gameSound);
		m_menuSound = PlayGameSound(g_soundID[ST_MENU], true, g_gameConfigBlackboard.GetValue("musicVolume", 1.f));
	}
	m_state = ATTRACT;
}

void Game::EnterLobby()
{
	m_state = LOBBY;
	m_isSinglePlayer = true;
}

void Game::EnterPlaying()
{
	if (m_firstPlayGame)
	{
		MapDefinition::InitializeMapDefs("Data/Definitions/MapDefinitions.xml");
		TileDefinition::InitializeTileDefs("Data/Definitions/TileDefinitions.xml");
		WeaponDefinition::InitializeWeaponDefs("Data/Definitions/WeaponDefinitions.xml");
		ActorDefinition::InitializeActorDefs("Data/Definitions/ActorDefinitions.xml");
		ActorDefinition::InitializeActorDefs("Data/Definitions/ProjectileActorDefinitions.xml");
		m_firstPlayGame = false;
	}
	
	
	if (m_isSinglePlayer)
	{
		m_currentMap = new Map(this, *MapDefinition::GetByName(g_gameConfigBlackboard.GetValue("defaultMap", "UNKNOWN")));
		if (!m_hasEnterDialogue)
		{
			m_currentMap->EnterDialogue();
			m_hasEnterDialogue = true;
		}
		else
		{
			m_currentMap->SetGameMode(GM_COMBAT);
			StopGameSound(m_menuSound);
			m_gameSound = PlayGameSound(g_soundID[ST_GAMEBGM], true, g_gameConfigBlackboard.GetValue("musicVolume", 1.f));
		}
		m_players.resize(1);
		if (!m_isPlayer1Controller)
		{
			m_players[0] = m_currentMap->SpawnPlayerToNewPosition(0, -1);
		}
		else
		{
			m_players[0] = m_currentMap->SpawnPlayerToNewPosition(0, 0);
		}
		m_players[0]->m_worldCamera.SetTransform(Vec3(m_players[0]->m_position.x, m_players[0]->m_position.y, m_players[0]->GetActor()->m_actorDef.m_eyeHeight), m_players[0]->m_orientation);
	}
	else
	{
		m_currentMap = new Map(this, *MapDefinition::GetByName("MPMap"));
		m_currentMap->SetGameMode(GM_COMBAT);
		StopGameSound(m_menuSound);
		m_gameSound = PlayGameSound(g_soundID[ST_GAMEBGM], true, g_gameConfigBlackboard.GetValue("musicVolume", 1.f));
		m_players.resize(2);
		if (!m_isPlayer1Controller)
		{
			m_players[0] = m_currentMap->SpawnPlayerToNewPosition(0, -1);
			m_players[1] = m_currentMap->SpawnPlayerToNewPosition(1, 0);
		}
		else
		{
			m_players[0] = m_currentMap->SpawnPlayerToNewPosition(0, 0);
			m_players[1] = m_currentMap->SpawnPlayerToNewPosition(1, -1);
		}
	}
	g_theInput->SetCursorMode(true, true);
	m_state = PLAYING;
}

void Game::EnterStory()
{
	StopGameSound(m_menuSound);
	m_menuSound = PlayGameSound(g_soundID[ST_STORYBGM], true, 1.5f*g_gameConfigBlackboard.GetValue("musicVolume", 1.f));
	m_state = STORY;
	std::string attractString = "Once upon a time, there is a mystery tower located in a deserted place. \nNo one never ever, got away from that tower. \nThey said, it is the tower of DOOM. ";
	m_message = new DialogueMessage(attractString, 5.f);
}

void Game::ExitStory()
{
	EnterPlaying();

}

void Game::ExitAttract(bool isController)
{
	if (!isController)
	{
		m_isPlayer1Controller = false;
	}
	else
	{
		m_isPlayer1Controller = true;
	}
	EnterLobby();
}

void Game::ExitLobby()
{
	if (m_firstPlayGame && m_isSinglePlayer)
	{
		EnterStory();
	}
	else 
	{
		EnterPlaying();
	}
	
}

void Game::ExitPlaying()
{
	delete m_currentMap;
	m_currentMap = nullptr;
	EnterAttract();
}



void Game::Render() const
{
	Rgba8 clearColor(25, 25, 25, 255);
	g_theRenderer->ClearScreen(clearColor);
	if (m_state == ATTRACT)
	{
		RenderAttract();

	}
	else if (m_state == LOBBY)
	{
		RenderLobby();
	}

	else if (m_state == PLAYING)
	{
		RenderPlaying();
	}
	else if (m_state == STORY)
	{
		RenderStory();
	}
	g_theRenderer->BindTexture(g_textures[TT_LOADING]);
}

void Game::RenderAttract() const
{   
	g_theRenderer->BeginCamera(m_screenCamera);
 	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->BindShader(nullptr);

	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
	g_theRenderer->SetModelConstants();
	std::vector<Vertex_PCU> imageVerts;
	g_theRenderer->BindTexture(g_textures[TT_ATTRACT]);
	AddVertsForAABB2D(imageVerts, m_screenCamera.GetCameraBounds(), Rgba8::WHITE);
	g_theRenderer->DrawVertexArray((int)imageVerts.size(), imageVerts.data());

	AABB2 attractBound = AABB2(Vec2(0.f,0.f ), Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y));

	std::vector<Vertex_PCU> verts;
	std::string attractString = "Press SPACE to join with mouse and keyboard\n\nPress START to join with controller\n\nPress ESCAPE or BACK to exit";

	g_bitmapFont->AddVertsForTextBox2D(verts, attractBound, 30.f, attractString, Rgba8(255,127,56,255), 0.8f, Vec2(0.5f, 0.5f), TextBoxMode::SHRINK_TO_FIT, 10000, Vec2(5.f, 5.f));
	g_bitmapFont->AddVertsForTextBox2D(verts, attractBound, 30.f, attractString, Rgba8(200,200,200,255), 0.8f, Vec2(0.5f,0.5f), TextBoxMode::SHRINK_TO_FIT);

	g_theRenderer->BindTexture(&g_bitmapFont->GetTexture());
	g_theRenderer->DrawVertexArray((int)verts.size(), verts.data());
	g_theRenderer->EndCamera(m_screenCamera);
}


void Game::RenderLobby() const
{
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(&g_bitmapFont->GetTexture());
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
	g_theRenderer->SetModelConstants();
	if (!m_isPlayer1Controller)
	{
		if (m_isSinglePlayer && !m_isPlayer2OnScreen)
		{
			g_theRenderer->BeginCamera(m_screenCamera);
			std::vector<Vertex_PCU> verts;
			g_bitmapFont->AddVertsForTextBox2D(verts, AABB2(Vec2(SCREEN_SIZE_X / 2.f - 300.f, SCREEN_SIZE_Y / 2.f), Vec2(SCREEN_SIZE_X / 2.f + 300.f, SCREEN_SIZE_Y / 2.f + 200.f)), 60.f, "Player 1 *-_-*", Rgba8::WHITE, 0.8f, Vec2(0.5f, 0.f));
			g_bitmapFont->AddVertsForTextBox2D(verts, AABB2(Vec2(SCREEN_SIZE_X / 2.f - 300.f, SCREEN_SIZE_Y / 2.f - 30.f), Vec2(SCREEN_SIZE_X / 2.f + 300.f, SCREEN_SIZE_Y / 2.f)), 40.f, "Mouse and Keyboard", Rgba8::WHITE, 0.8f, Vec2(0.5f, 0.5f));
			g_bitmapFont->AddVertsForTextBox2D(verts, AABB2(Vec2(SCREEN_SIZE_X / 2.f - 300.f, SCREEN_SIZE_Y / 2.f - 200.f), Vec2(SCREEN_SIZE_X / 2.f + 300.f, SCREEN_SIZE_Y / 2.f - 100.f)), 30.f, "Press SPACE to start game\nPress ESACPE to leave game\nPress START to join with controller", Rgba8::WHITE, 0.8f, Vec2(0.5f, 0.5f));
			g_theRenderer->DrawVertexArray((int)verts.size(), verts.data());
			g_theRenderer->EndCamera(m_screenCamera);
		}

		if (m_isSinglePlayer && m_isPlayer2OnScreen)
		{
			g_theRenderer->BeginCamera(m_screenCamera);
			std::vector<Vertex_PCU> verts;
			g_bitmapFont->AddVertsForTextBox2D(verts, AABB2(Vec2(SCREEN_SIZE_X / 2.f - 300.f, SCREEN_SIZE_Y / 2.f), Vec2(SCREEN_SIZE_X / 2.f + 300.f, SCREEN_SIZE_Y / 2.f + 200.f)), 60.f, "Player 2 >_<", Rgba8::WHITE, 0.8f, Vec2(0.5f, 0.f));
			g_bitmapFont->AddVertsForTextBox2D(verts, AABB2(Vec2(SCREEN_SIZE_X / 2.f - 300.f, SCREEN_SIZE_Y / 2.f - 30.f), Vec2(SCREEN_SIZE_X / 2.f + 300.f, SCREEN_SIZE_Y / 2.f)), 40.f, "Controller", Rgba8::WHITE, 0.8f, Vec2(0.5f, 0.5f));
			g_bitmapFont->AddVertsForTextBox2D(verts, AABB2(Vec2(SCREEN_SIZE_X / 2.f - 300.f, SCREEN_SIZE_Y / 2.f - 200.f), Vec2(SCREEN_SIZE_X / 2.f + 300.f, SCREEN_SIZE_Y / 2.f - 100.f)), 30.f, "Press START to start game\nPress BACK to leave game\nPress SPACE to join with Mouse And Keyboard", Rgba8::WHITE, 0.8f, Vec2(0.5f, 0.5f));
			g_theRenderer->DrawVertexArray((int)verts.size(), verts.data());
			g_theRenderer->EndCamera(m_screenCamera);
		}

		if (!m_isSinglePlayer)
		{
			g_theRenderer->BeginCamera(m_splitScreenCamera1);
			std::vector<Vertex_PCU> verts;
			g_bitmapFont->AddVertsForTextBox2D(verts, AABB2(Vec2(SCREEN_SIZE_X / 2.f - 300.f, SCREEN_SIZE_Y / 4.f + 20.f), Vec2(SCREEN_SIZE_X / 2.f + 300.f, SCREEN_SIZE_Y / 4.f + 70.f)), 50.f, "Player 1 *-_-*", Rgba8::WHITE, 0.8f, Vec2(0.5f, 0.f));

			g_bitmapFont->AddVertsForTextBox2D(verts, AABB2(Vec2(SCREEN_SIZE_X / 2.f - 300.f, SCREEN_SIZE_Y / 4.f - 10.f), Vec2(SCREEN_SIZE_X / 2.f + 300.f, SCREEN_SIZE_Y / 4.f + 20.f)), 30.f, "Mouse and Keyboard", Rgba8::WHITE, 0.8f, Vec2(0.5f, 0.5f));

			g_bitmapFont->AddVertsForTextBox2D(verts, AABB2(Vec2(SCREEN_SIZE_X / 2.f - 300.f, SCREEN_SIZE_Y / 4.f - 120.f), Vec2(SCREEN_SIZE_X / 2.f + 300.f, SCREEN_SIZE_Y / 4.f - 60.f)), 20.f, "Press SPACE to start game\nPress ESACPE to leave game", Rgba8::WHITE, 0.8f, Vec2(0.5f, 0.5f));

			g_theRenderer->DrawVertexArray((int)verts.size(), verts.data());
			g_theRenderer->EndCamera(m_splitScreenCamera1);

			g_theRenderer->BeginCamera(m_splitScreenCamera2);
			verts.clear();
			g_bitmapFont->AddVertsForTextBox2D(verts, AABB2(Vec2(SCREEN_SIZE_X / 2.f - 300.f, SCREEN_SIZE_Y / 4.f + 20.f), Vec2(SCREEN_SIZE_X / 2.f + 300.f, SCREEN_SIZE_Y / 4.f + 70.f)), 50.f, "Player 2 >_<", Rgba8::WHITE, 0.8f, Vec2(0.5f, 0.f));

			g_bitmapFont->AddVertsForTextBox2D(verts, AABB2(Vec2(SCREEN_SIZE_X / 2.f - 300.f, SCREEN_SIZE_Y / 4.f - 10.f), Vec2(SCREEN_SIZE_X / 2.f + 300.f, SCREEN_SIZE_Y / 4.f + 20.f)), 30.f, "Controller", Rgba8::WHITE, 0.8f, Vec2(0.5f, 0.5f));

			g_bitmapFont->AddVertsForTextBox2D(verts, AABB2(Vec2(SCREEN_SIZE_X / 2.f - 300.f, SCREEN_SIZE_Y / 4.f - 120.f), Vec2(SCREEN_SIZE_X / 2.f + 300.f, SCREEN_SIZE_Y / 4.f - 60.f)), 20.f, "Press START to start game\nPress BACK to leave game", Rgba8::WHITE, 0.8f, Vec2(0.5f, 0.5f));

			g_theRenderer->DrawVertexArray((int)verts.size(), verts.data());
			g_theRenderer->EndCamera(m_splitScreenCamera2);
		}
	}
	if (m_isPlayer1Controller)
	{
		if (m_isSinglePlayer && !m_isPlayer2OnScreen)
		{
			g_theRenderer->BeginCamera(m_screenCamera);
			std::vector<Vertex_PCU> verts;
			g_bitmapFont->AddVertsForTextBox2D(verts, AABB2(Vec2(SCREEN_SIZE_X / 2.f - 300.f, SCREEN_SIZE_Y / 2.f), Vec2(SCREEN_SIZE_X / 2.f + 300.f, SCREEN_SIZE_Y / 2.f + 200.f)), 60.f, "Player 1 ~_~", Rgba8::WHITE, 0.8f, Vec2(0.5f, 0.f));
			g_bitmapFont->AddVertsForTextBox2D(verts, AABB2(Vec2(SCREEN_SIZE_X / 2.f - 300.f, SCREEN_SIZE_Y / 2.f - 30.f), Vec2(SCREEN_SIZE_X / 2.f + 300.f, SCREEN_SIZE_Y / 2.f)), 40.f, "Controller", Rgba8::WHITE, 0.8f, Vec2(0.5f, 0.5f));
			g_bitmapFont->AddVertsForTextBox2D(verts, AABB2(Vec2(SCREEN_SIZE_X / 2.f - 300.f, SCREEN_SIZE_Y / 2.f - 200.f), Vec2(SCREEN_SIZE_X / 2.f + 300.f, SCREEN_SIZE_Y / 2.f - 100.f)), 30.f, "Press START to start game\nPress BACK to leave game\nPress SPACE to join with Mouse And Keyboard", Rgba8::WHITE, 0.8f, Vec2(0.5f, 0.5f));
			g_theRenderer->DrawVertexArray((int)verts.size(), verts.data());
			g_theRenderer->EndCamera(m_screenCamera);
		}

		if (m_isSinglePlayer && m_isPlayer2OnScreen)
		{
			g_theRenderer->BeginCamera(m_screenCamera);
			std::vector<Vertex_PCU> verts;
			g_bitmapFont->AddVertsForTextBox2D(verts, AABB2(Vec2(SCREEN_SIZE_X / 2.f - 300.f, SCREEN_SIZE_Y / 2.f), Vec2(SCREEN_SIZE_X / 2.f + 300.f, SCREEN_SIZE_Y / 2.f + 200.f)), 60.f, "Player 2 !_!", Rgba8::WHITE, 0.8f, Vec2(0.5f, 0.f));
			g_bitmapFont->AddVertsForTextBox2D(verts, AABB2(Vec2(SCREEN_SIZE_X / 2.f - 300.f, SCREEN_SIZE_Y / 2.f - 30.f), Vec2(SCREEN_SIZE_X / 2.f + 300.f, SCREEN_SIZE_Y / 2.f)), 40.f, "Mouse And Keyboard", Rgba8::WHITE, 0.8f, Vec2(0.5f, 0.5f));
			g_bitmapFont->AddVertsForTextBox2D(verts, AABB2(Vec2(SCREEN_SIZE_X / 2.f - 300.f, SCREEN_SIZE_Y / 2.f - 200.f), Vec2(SCREEN_SIZE_X / 2.f + 300.f, SCREEN_SIZE_Y / 2.f - 100.f)), 30.f, "Press SPACE to start game\nPress ESACPE to leave game\nPress START to join with controller", Rgba8::WHITE, 0.8f, Vec2(0.5f, 0.5f));
			g_theRenderer->DrawVertexArray((int)verts.size(), verts.data());
			g_theRenderer->EndCamera(m_screenCamera);
		}

		if (!m_isSinglePlayer)
		{
			g_theRenderer->BeginCamera(m_splitScreenCamera1);
			std::vector<Vertex_PCU> verts;
			g_bitmapFont->AddVertsForTextBox2D(verts, AABB2(Vec2(SCREEN_SIZE_X / 2.f - 300.f, SCREEN_SIZE_Y / 4.f + 20.f), Vec2(SCREEN_SIZE_X / 2.f + 300.f, SCREEN_SIZE_Y / 4.f + 70.f)), 50.f, "Player 1 ~_~", Rgba8::WHITE, 0.8f, Vec2(0.5f, 0.f));

			g_bitmapFont->AddVertsForTextBox2D(verts, AABB2(Vec2(SCREEN_SIZE_X / 2.f - 300.f, SCREEN_SIZE_Y / 4.f - 10.f), Vec2(SCREEN_SIZE_X / 2.f + 300.f, SCREEN_SIZE_Y / 4.f + 20.f)), 30.f, "Controller", Rgba8::WHITE, 0.8f, Vec2(0.5f, 0.5f));

			g_bitmapFont->AddVertsForTextBox2D(verts, AABB2(Vec2(SCREEN_SIZE_X / 2.f - 300.f, SCREEN_SIZE_Y / 4.f - 120.f), Vec2(SCREEN_SIZE_X / 2.f + 300.f, SCREEN_SIZE_Y / 4.f - 60.f)), 20.f, "Press START to start game\nPress BACK to leave game", Rgba8::WHITE, 0.8f, Vec2(0.5f, 0.5f));

			g_theRenderer->DrawVertexArray((int)verts.size(), verts.data());
			g_theRenderer->EndCamera(m_splitScreenCamera1);

			g_theRenderer->BeginCamera(m_splitScreenCamera2);
			verts.clear();
			g_bitmapFont->AddVertsForTextBox2D(verts, AABB2(Vec2(SCREEN_SIZE_X / 2.f - 300.f, SCREEN_SIZE_Y / 4.f + 20.f), Vec2(SCREEN_SIZE_X / 2.f + 300.f, SCREEN_SIZE_Y / 4.f + 70.f)), 50.f, "Player 2 >_<", Rgba8::WHITE, 0.8f, Vec2(0.5f, 0.f));

			g_bitmapFont->AddVertsForTextBox2D(verts, AABB2(Vec2(SCREEN_SIZE_X / 2.f - 300.f, SCREEN_SIZE_Y / 4.f - 10.f), Vec2(SCREEN_SIZE_X / 2.f + 300.f, SCREEN_SIZE_Y / 4.f + 20.f)), 30.f, "Mouse And Keyboard", Rgba8::WHITE, 0.8f, Vec2(0.5f, 0.5f));

			g_bitmapFont->AddVertsForTextBox2D(verts, AABB2(Vec2(SCREEN_SIZE_X / 2.f - 300.f, SCREEN_SIZE_Y / 4.f - 120.f), Vec2(SCREEN_SIZE_X / 2.f + 300.f, SCREEN_SIZE_Y / 4.f - 60.f)), 20.f, "Press SPACE to start game\nPress ESACPE to leave game", Rgba8::WHITE, 0.8f, Vec2(0.5f, 0.5f));

			g_theRenderer->DrawVertexArray((int)verts.size(), verts.data());
			g_theRenderer->EndCamera(m_splitScreenCamera2);
		}
	}
}

void Game::RenderStory() const
{
	g_theRenderer->BeginCamera(m_screenCamera);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->BindShader(nullptr);

	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
	g_theRenderer->SetModelConstants();
	std::vector<Vertex_PCU> imageVerts;
	std::vector<Vertex_PCU> verts;
	switch (m_scene)
	{
	case StoryScene::SCENE1:
		g_theRenderer->BindTexture(g_textures[TT_SCENE1]);
		break;

	case StoryScene::SCENE2:
		g_theRenderer->BindTexture(g_textures[TT_SCENE2]);
		break;
	case StoryScene::SCENE3:
		g_theRenderer->BindTexture(g_textures[TT_SCENE3]);
		break;
	case StoryScene::SCENE4:
		g_theRenderer->BindTexture(g_textures[TT_SCENE4]);
		break;
	}
	AddVertsForAABB2D(imageVerts, m_screenCamera.GetCameraBounds(), Rgba8::WHITE);
	g_theRenderer->DrawVertexArray((int)imageVerts.size(), imageVerts.data());

	AABB2 attractBound = AABB2(Vec2(SCREEN_SIZE_X / 2.f - 700.f, SCREEN_SIZE_Y / 2.f - 600.f), Vec2(SCREEN_SIZE_X / 2.f + 800.f, SCREEN_SIZE_Y / 2.f));


	std::string attractString = m_message->m_currentString;
	g_bitmapFont->AddVertsForTextBox2D(verts, attractBound, 30.f, attractString, Rgba8(255, 127, 56, 255), 0.8f, Vec2(0.5f, 0.5f), TextBoxMode::SHRINK_TO_FIT, 100000, Vec2(2.5f, 2.5f));
	g_bitmapFont->AddVertsForTextBox2D(verts, attractBound, 30.f, attractString, Rgba8(200, 200, 200, 255), 0.8f, Vec2(0.5f, 0.5f));

	g_theRenderer->BindTexture(&g_bitmapFont->GetTexture());
	g_theRenderer->DrawVertexArray((int)verts.size(), verts.data());

	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::RenderPlaying() const
{
	if (m_currentMap)
	{
		m_currentMap->Render();
	}


	g_theRenderer->BeginCamera(m_screenCamera);
	//RenderUIText();
	g_theRenderer->EndCamera(m_screenCamera);
	g_theRenderer->BindShader(nullptr);
	DebugRenderScreen(m_screenCamera);
}



bool Game::IsAttract() const
{
	return m_state == ATTRACT;
}

ActorUID const Game::GetActorUID(int playerIndex) const
{
	return m_players[playerIndex]->GetActor()->m_uid;
}

int Game::GetPlayerIndex(ActorUID const& uid) const
{
	for (int i = 0; i < (int)m_players.size(); ++i)
	{
		if (m_players[i]->GetActor()->m_uid == uid)
		{
			return i;
		}
	}
	return -1;
}

Player* Game::GetPlayer(ActorUID const& uid) const
{
	for (int i = 0; i < (int)m_players.size(); ++i)
	{
		if (m_players[i]->GetActor()->m_uid == uid)
		{
			return m_players[i];
		}
	}
	return nullptr;
}

Player* Game::GetPlayer(int index) const
{
	return m_players[index];
}

SoundPlaybackID Game::PlayGameSound(SoundID soundID, bool isLoop /*= false*/, float volume /*= 1.f*/  )
{
	return g_theAudio->StartSound(soundID, isLoop, volume);
}

void Game::StopGameSound(SoundPlaybackID soundPlaybackID)
{
	g_theAudio->StopSound(soundPlaybackID);
}

