#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#define UNUSED(x) (void)(x)

enum GameMode
{
	GM_COMBAT,
	GM_LOSE,
	GM_DIALOGUE,
	GM_WIN,
	GM_CREDIT
};
enum SoundType
{
	ST_WALK,
	ST_VICTORY,
	ST_LOSE,
	ST_DIALOGUEBLIP,
	ST_GAMEBGM,
	ST_DIALOGUEBGM,
	ST_STORYBGM,
	ST_MENU,
	ST_BUTTONCLICK,
	ST_HURT,
	ST_DEATH,
	ST_FIRE,
	NUM_SOUNDS
};
enum TextureType
{
	TT_VICTORY,
	TT_CREDIT,
	TT_LOSE,
	TT_DIALOGUEBOX,
	TT_ATTRACT,
	TT_MARINE,
	TT_SCENE1,
	TT_SCENE2,
	TT_SCENE3,
	TT_SCENE4,
	TT_LOADING,
	NUM_TEXTURES
};
class App;
class Renderer;
class InputSystem;
class Window;
class AudioSystem;
class RandomNumberGenerator;
class XboxController;
class Texture;
class BitmapFont;
class Clock;

constexpr float MAX_SCREEN_SHAKE = 0.01f;
constexpr float SCREEN_SHAKE_INCREASE_PER_SEC = 0.001f;
constexpr float SCREEN_SHAKE_REDUCTION_PER_SEC = 0.001f;
constexpr int ENTITIES_HEALTH = 3;
constexpr float SCREEN_SIZE_X = 1600.f;
constexpr float SCREEN_SIZE_Y = 800.f;
constexpr float SCREEN_CENTER_X = SCREEN_SIZE_X / 2.f;
constexpr float SCREEN_CENTER_Y = SCREEN_SIZE_Y / 2.f;
constexpr float PLAYER_MOVESPEED = 1.f;
constexpr float PLAYER_ROTATESPEED = 90.f;
static Vec2 SCREEN_SIZE = Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y);
static EulerAngles PLAYER_START_ANGLE = EulerAngles(45.f, 30.f, 0.f);
static Rgba8 MEDIUM_DARKGREY = Rgba8(100, 100, 100, 255);
static Rgba8 DEEP_GREEN = Rgba8(92, 159, 63, 255);
static Rgba8 YELLOW = Rgba8(255, 255, 6, 255);
static Rgba8 BLACK_GREY = Rgba8(102, 153, 204, 255);
static Rgba8 WHITE = Rgba8(255, 255, 255, 255);
static Rgba8 BLACK = Rgba8(0, 0, 0, 255);
static Rgba8 DARK_RED = Rgba8(158, 50, 47, 255);
static Rgba8 RAYCOLOR = Rgba8(200, 99, 255, 255);
extern App* g_theApp;
extern RandomNumberGenerator* g_rng;
extern InputSystem* g_theInput;
extern AudioSystem* g_theAudio;
extern Renderer* g_theRenderer;
extern XboxController g_controller;
extern Texture* g_testUVTexture;
extern BitmapFont* g_bitmapFont;
extern Clock* g_theGameClock;
extern Clock* g_theUIClock;
extern SoundPlaybackID g_soundID[NUM_SOUNDS];
extern Texture* g_textures[NUM_TEXTURES];
void DebugDrawRing(Vec2 const& center, float radius, float thickness, Rgba8 const& color);
void DebugDrawGlow(Vec2 const& center, float radius, Rgba8 const& centerColor, Rgba8 const& edgeColor);
void DebugDrawLine(Vec2 const& startPos, Vec2 const& endPos, float thickness, Rgba8 const& color);
void DebugDrawGlow(Vec2 const& center, float radius, Rgba8 const& centerColor, Rgba8 const& edgeColor);
void WarpVertexes(int numVerts, Vertex_PCU* verts, Vec2 const& warpCenter, float warpAmount);
