#pragma once
#include "Engine/Render/Renderer.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Window/Window.hpp"
#include "Game.hpp"
//#include "Engine/Render/Renderer.hpp"
class App
{
public:
	App();
	~App();
	void Startup();
	void Shutdown();
	void Run();
	bool IsAttract() const;
	bool IsQuitting() const;
	static bool Quit(EventArgs& args);
private:
	void BeginFrame();
	void Update();
	void UpdateFromInputSystem();
	void Render() const;
	void EndFrame();
	void StartupGameConfig();
	void StartupTextures();
	void StartupBGMs();
	void AddDebugConsoleLines();
private:
	
	bool m_isAttractMode = true;
	bool m_isQuitting = false;
	bool m_isPaused = false;
	bool m_isSlowMo = false;
	bool m_isSpeedUp = false;
	Vec2 m_shipPos;
	float m_orientDegrees=30.f;

	Game* m_theGame;
};
