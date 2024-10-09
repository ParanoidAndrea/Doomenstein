#pragma once
#include "Entity.hpp"
#include <vector>
#include "Engine/Core/Rgba8.hpp"
class Texture;
class Prop : public Entity
{
public:
	Prop(Game* owner);
	~Prop();
	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;

public:
	std::vector<Vertex_PCU> m_vertexes;
	Rgba8 m_color = Rgba8::WHITE;
	Texture* m_texture = nullptr;
};