#include "Prop.hpp"
#include "Game/Gamecommon.hpp"
#include "Engine/Render/Renderer.hpp"

Prop::Prop(Game* owner)
	:Entity(owner)
{

}	

Prop::~Prop()
{
}

void Prop::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	m_orientation += m_angularVelocity * deltaSeconds;
}

void Prop::Render() const
{
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	Mat44 model = GetModelMatrix();
	g_theRenderer->SetModelConstants(model, m_color);
	g_theRenderer->BindTexture(m_texture);
	g_theRenderer->DrawVertexArray((int)m_vertexes.size(), m_vertexes.data());

}
