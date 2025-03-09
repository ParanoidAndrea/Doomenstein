#include "Game/DialogueMessage.hpp"
#include "Game/Gamecommon.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Render/Camera.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Render/Renderer.hpp"
#include "Engine/Render/BitmapFont.hpp"
DialogueMessage::DialogueMessage(std::string dialogString, float overallDuration,std::string owner)
	:m_dialogString(dialogString),
	m_owner(owner)
{
	UNUSED(overallDuration);
	//float stringLength = (float)dialogString.length();
	//m_dialogTimer = new Timer(overallDuration/stringLength, g_theGameClock);
	m_dialogTimer = new Timer(0.05f, g_theUIClock);
	m_dialogTimer->Start();
	m_currentString = m_dialogString[m_currentIndex];
}

void DialogueMessage::Update()
{
	if (m_dialogTimer->HasPeriodElapsed()&& m_currentIndex< (int)m_dialogString.length()-1 )
	{
		m_currentIndex++;
		if (m_currentIndex % 2 == 0)
		{
			g_theAudio->StartSound(g_soundID[ST_DIALOGUEBLIP],false,0.5f);
		}
		m_currentString.push_back(m_dialogString[m_currentIndex]);
		m_dialogTimer->Start();
	}
}

void DialogueMessage::SetDialogFullString()
{
	m_currentString = m_dialogString;
	m_currentIndex = (int)m_dialogString.length() - 1;
	m_dialogTimer->Stop();
}

bool DialogueMessage::IsFullString() const
{
	return m_currentIndex == (int)m_dialogString.length() - 1;
}

DialogueSystem::DialogueSystem(std::vector<DialogueMessage> messages)
	:m_messages(messages)
{
	m_currentMessage = m_messages[0];
}

void DialogueSystem::Update()
{
	m_currentMessage.Update();
	if (m_currentMessage.IsFullString() && (g_theInput->WasKeyJustPressed(' ')|| g_theInput->WasKeyJustPressed(KEYCODE_ENTER)||g_controller.WasButtonJustPressed(A)))
	{
		m_currentIndex++;
		if (m_currentIndex > (int)m_messages.size() - 1)
		{
			return;
		}
		m_currentMessage = m_messages[m_currentIndex];
	}
	else if (g_theInput->WasKeyJustPressed(' ') || g_theInput->WasKeyJustPressed(KEYCODE_ENTER) || g_controller.WasButtonJustPressed(A))
	{
		m_currentMessage.SetDialogFullString();
	}
}

void DialogueSystem::RenderDialogueBox(Camera screenCamera) const
{
	UNUSED(screenCamera);
	std::vector<Vertex_PCU> dialogBoxVerts;
	AddVertsForAABB2D(dialogBoxVerts, AABB2(100.f, 50.f, SCREEN_SIZE_X - 100.f, SCREEN_SIZE_Y/2.f),Rgba8(125,125,125,200));
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetDepthMode(DepthMode::DISABLED);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture(g_textures[TT_DIALOGUEBOX]);
	g_theRenderer->DrawVertexArray((int)dialogBoxVerts.size(), dialogBoxVerts.data());

	AABB2 nameBound = AABB2(120.f, SCREEN_SIZE_Y / 2.f+10.f, SCREEN_SIZE_X - 100.f, SCREEN_SIZE_Y / 2.f + 50.f);
	AABB2 dialogueBound = AABB2(150.f, 0.f, SCREEN_SIZE_X - 120.f, SCREEN_SIZE_Y / 2.f-50.f);
	std::vector<Vertex_PCU> verts;
	std::string dialogueString = m_currentMessage.m_currentString;
	g_bitmapFont->AddVertsForTextBox2D(verts, nameBound, 30.f, m_currentMessage.m_owner, Rgba8(255, 255, 255, 255), 0.8f, Vec2(0.f, 0.f));
	g_bitmapFont->AddVertsForTextBox2D(verts, dialogueBound, 25.f, dialogueString, Rgba8(0, 0, 0, 200), 0.8f, Vec2(0.f, 1.f), TextBoxMode::SHRINK_TO_FIT, 100000, Vec2(2.5f, 2.5f));
	g_bitmapFont->AddVertsForTextBox2D(verts, dialogueBound, 25.f, dialogueString, Rgba8(255, 255, 255, 255), 0.8f, Vec2(0.f, 1.f));

	g_theRenderer->BindTexture(&g_bitmapFont->GetTexture());
	g_theRenderer->DrawVertexArray((int)verts.size(), verts.data());
}
