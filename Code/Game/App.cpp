#include "App.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Input/XboxController.hpp"
#include "Engine/Render/DebugRender.hpp"
#include "Engine/Render/BitmapFont.hpp"
// Created and owned by the App
Renderer* g_theRenderer = nullptr;
InputSystem* g_theInput = nullptr;
Window* g_theWindow = nullptr;
RandomNumberGenerator* g_rng = nullptr;
DevConsole* g_theConsole = nullptr;
XboxController g_controller;
Texture* g_testUVTexture = nullptr;
BitmapFont* g_bitmapFont = nullptr;
SoundPlaybackID g_soundID[NUM_SOUNDS] = {};
AudioSystem* g_theAudio = nullptr;
Texture* g_textures[NUM_TEXTURES] = {};
App::App()
{
	m_theGame = new Game(this);
}
App::~App()
{
}
//-----------------------------------------------------------------------------------------------
void App::Startup( )
{

	g_rng = new RandomNumberGenerator();
	InputConfig inputConfig;
	g_theInput = new InputSystem(inputConfig);

	WindowConfig windowConfig;
	windowConfig.m_inputSystem = g_theInput;
	windowConfig.m_windowTitle = "Meow-sion Impossible: Doom";
	windowConfig.m_aspectRatio = 2.f;
	g_theWindow = new Window(windowConfig);

	RenderConfig renderConfig;
	renderConfig.m_window = g_theWindow;
	g_theRenderer = new Renderer(renderConfig);

	AudioConfig audioConfig;
	g_theAudio = new AudioSystem(audioConfig);

	EventSystemConfig eventSystemConfig;
	g_theEventSystem = new EventSystem(eventSystemConfig);

	m_theGame = new Game(this);

	DevConsoleConfig devConsoleConfig;
	devConsoleConfig.m_camera = &m_theGame->m_screenCamera;
	devConsoleConfig.m_defalutRenderer = g_theRenderer;
	g_theConsole = new DevConsole(devConsoleConfig);

	g_theEventSystem->Startup();
	g_theConsole->Startup();

	g_theWindow->StartUp();
	g_theInput->Startup();
	g_theRenderer->Startup();
	g_theAudio->Startup();

	g_testUVTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/TestUV.png");
	g_bitmapFont = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
	DebugRenderConfig debugRenderConfig;
	debugRenderConfig.m_renderer = g_theRenderer;
	DebugRenderSystemStartup(debugRenderConfig);



	g_theEventSystem->SubscribeEventCallbackFunction("Quit", App::Quit);

	StartupGameConfig();
	StartupTextures();
	StartupBGMs();
	AddDebugConsoleLines();
	m_theGame->Startup();

}


//-----------------------------------------------------------------------------------------------
void App::Shutdown()
{
	delete m_theGame;
	m_theGame = nullptr;
	g_theEventSystem->Shutdown();
	g_theInput->Shutdown();
	g_theWindow->ShutDown();
	g_theRenderer->Shutdown();
	g_theConsole->Shutdown();
	g_theAudio->Shutdown();
	delete g_theEventSystem;
	g_theEventSystem = nullptr;
	delete g_theInput;
	g_theInput = nullptr;
	delete g_theWindow;
	g_theWindow = nullptr;
	delete g_theRenderer;
	g_theRenderer = nullptr;
	delete g_theConsole;
	g_theConsole = nullptr;
	delete g_theAudio;
	g_theAudio = nullptr;
	DebugRenderSystemShutdown();
}


bool App::Quit(EventArgs& args)
{
	UNUSED(args);
	g_theApp->m_isQuitting = true; // sets m_isQuitting=true
	return true;
}

void App::BeginFrame()
{   

	g_theEventSystem->BeginFrame();
	g_theInput->BeginFrame();
	g_theWindow->BeginFrame();
	g_theRenderer->BeginFrame();
	g_theAudio->BeginFrame();
	g_controller = g_theInput->GetController(0);
	
	g_theConsole->BeginFrame();
	DebugRenderBeginFrame();
}


void App::Update()
{
	UpdateFromInputSystem();
	m_theGame->Update();
	
}




void App::UpdateFromInputSystem()
{
	//Exit
// 	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
// 	{
// 
// 		m_isQuitting = true;
// 	}
// 	if (g_theInput->WasKeyJustPressed(KEYCODE_F8))
// 	{
// 		m_theGame = new Game(this);
// 		m_theGame->Startup();
// 	}
	if (!m_theGame->IsAttract() && g_theWindow->GetHwnd() == GetActiveWindow())
	{
		g_theInput->SetCursorMode(true, true);
	}

	if (g_theConsole->IsOpen() || m_theGame->IsAttract() || g_theWindow->GetHwnd() != GetActiveWindow())
	{
		g_theInput->SetCursorMode(false, false);
	}
	
}


void App::Render() const
{

// 	g_theRenderer->BeginCamera(Camera(Vec2(), Vec2(1600.f, 800.f)));
// 	std::vector<Vertex_PCU> verts;
// 	g_theRenderer->BindTexture(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Loading.png"));
// 	AddVertsForAABB2D(verts, AABB2(0.f, 0.f, 1600.f, 800.f), Rgba8::WHITE);
// 	g_theRenderer->DrawVertexArray((int)verts.size(), verts.data());
// 	g_theRenderer->EndCamera(Camera(Vec2(), Vec2(1600.f, 800.f)));
	if (m_theGame)
	{
		m_theGame->Render();
	}

	g_theRenderer->BeginCamera(*g_theConsole->GetCamera());
	g_theConsole->Render();
	g_theRenderer->EndCamera(*g_theConsole->GetCamera());

}

void App::EndFrame()
{
	g_theEventSystem->EndFrame();
	g_theInput->EndFrame();
	g_theWindow->EndFrame();
	g_theRenderer->EndFrame();
	g_theAudio->EndFrame();
	g_theConsole->EndFrame();
	DebugRenderEndFrame();
}


void App::StartupGameConfig()
{
	XmlDocument document;
	char const* filename = "Data/GameConfig.xml";
	XmlResult result = document.LoadFile(filename);
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, "invaild filename");
	XmlElement* rootElement = document.RootElement();
	g_gameConfigBlackboard.PopulateFromXmlElementAttributes(*rootElement);
}

void App::StartupTextures()
{
	g_textures[TT_VICTORY] = g_theRenderer->CreateOrGetTextureFromFile(g_gameConfigBlackboard.GetValue("victoryImage", "UNKNOWN").c_str());
	g_textures[TT_CREDIT] = g_theRenderer->CreateOrGetTextureFromFile(g_gameConfigBlackboard.GetValue("creditImage", "UNKNOWN").c_str());
	g_textures[TT_LOSE] = g_theRenderer->CreateOrGetTextureFromFile(g_gameConfigBlackboard.GetValue("loseImage", "UNKNOWN").c_str());
	g_textures[TT_DIALOGUEBOX] = g_theRenderer->CreateOrGetTextureFromFile(g_gameConfigBlackboard.GetValue("dialogueBoxImage", "UNKNOWN").c_str());
	g_textures[TT_ATTRACT] = g_theRenderer->CreateOrGetTextureFromFile(g_gameConfigBlackboard.GetValue("attractScreen", "UNKNOWN").c_str());
	g_textures[TT_MARINE] = g_theRenderer->CreateOrGetTextureFromFile(g_gameConfigBlackboard.GetValue("marinePlayerImage", "UNKNOWN").c_str());
	g_textures[TT_SCENE1] = g_theRenderer->CreateOrGetTextureFromFile(g_gameConfigBlackboard.GetValue("storyScreen1", "UNKNOWN").c_str());
	g_textures[TT_SCENE2] = g_theRenderer->CreateOrGetTextureFromFile(g_gameConfigBlackboard.GetValue("storyScreen2", "UNKNOWN").c_str());
	g_textures[TT_SCENE3] = g_theRenderer->CreateOrGetTextureFromFile(g_gameConfigBlackboard.GetValue("storyScreen3", "UNKNOWN").c_str());
	g_textures[TT_SCENE4] = g_theRenderer->CreateOrGetTextureFromFile(g_gameConfigBlackboard.GetValue("storyScreen4", "UNKNOWN").c_str());
	g_textures[TT_LOADING] = g_theRenderer->CreateOrGetTextureFromFile(g_gameConfigBlackboard.GetValue("loadingImage", "UNKNOWN").c_str());
}

void App::StartupBGMs()
{
	g_soundID[ST_VICTORY] = g_theAudio->CreateOrGetSound(g_gameConfigBlackboard.GetValue("victoryBGM", "UNKNOWN"));
	g_soundID[ST_LOSE] = g_theAudio->CreateOrGetSound(g_gameConfigBlackboard.GetValue("loseSound", "UNKNOWN"));
	g_soundID[ST_STORYBGM] = g_theAudio->CreateOrGetSound(g_gameConfigBlackboard.GetValue("storyMusic", "UNKNOWN"));
	g_soundID[ST_DIALOGUEBLIP]= g_theAudio->CreateOrGetSound(g_gameConfigBlackboard.GetValue("dialogueBlipSound", "UNKNOWN"));	
	g_soundID[ST_DIALOGUEBGM] = g_theAudio->CreateOrGetSound(g_gameConfigBlackboard.GetValue("dialogueMusic", "UNKNOWN"));
	g_soundID[ST_MENU] = g_theAudio->CreateOrGetSound(g_gameConfigBlackboard.GetValue("mainMenuMusic", "UNKNOWN"));
	g_soundID[ST_GAMEBGM] = g_theAudio->CreateOrGetSound(g_gameConfigBlackboard.GetValue("gameMusic", "UNKNOWN"));
	g_soundID[ST_BUTTONCLICK] = g_theAudio->CreateOrGetSound(g_gameConfigBlackboard.GetValue("buttonClickSound", "UNKNOWN"));
}

void App::AddDebugConsoleLines()
{
	g_theConsole->AddLine(DevConsole::INFO_MINOR, "Type help for the lists of the commands");
	g_theConsole->AddLine(DevConsole::INFO_MAJOR, "Mouse  - Aim");
	g_theConsole->AddLine(DevConsole::INFO_MAJOR, "W / A  - Move");
	g_theConsole->AddLine(DevConsole::INFO_MAJOR, "S / D  - Strafe");
	g_theConsole->AddLine(DevConsole::INFO_MAJOR, "Z / C  - Elevate");
	g_theConsole->AddLine(DevConsole::INFO_MAJOR, "Shift  - Sprint");
	g_theConsole->AddLine(DevConsole::INFO_MAJOR, "1      - Pistol");
	g_theConsole->AddLine(DevConsole::INFO_MAJOR, "2      - Plasma Riffle");
	g_theConsole->AddLine(DevConsole::INFO_MAJOR, "3      - SMG");
	g_theConsole->AddLine(DevConsole::INFO_MAJOR, "SPACE  - Skip Current Dialogue Line");
	g_theConsole->AddLine(DevConsole::INFO_MAJOR, "~      - Open Dev Console");
	g_theConsole->AddLine(DevConsole::INFO_MAJOR, "Escape - Return to Menu or Exit the Game");

}

void App::Run()
{

	Clock::TickSystemClock();
	
	BeginFrame();	
	Update();
	Render();		
	EndFrame();		
}


bool App::IsAttract() const
{
	return m_theGame->IsAttract();
}

bool App::IsQuitting() const
{
	return m_isQuitting;
}





