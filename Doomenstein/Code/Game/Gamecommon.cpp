#include "GameCommon.hpp"
#include "Engine/Render/Renderer.hpp"
#include "Engine/Math/MathUtils.hpp"
extern Renderer* g_theRenderer;
void DebugDrawRing(Vec2 const& center, float radius, float thickness, Rgba8 const& color)
{

	float halfThickness = 0.5f * thickness;
	float innerRadius = radius - halfThickness;
	float outerRadius = radius + halfThickness;
	constexpr int NUM_SIDES = 32;
	constexpr int NUM_TRIS = 2 * NUM_SIDES;  // each side is a trapezoid
	constexpr int NUM_VERTS = 3 * NUM_TRIS;  // 3 vertexes per triangle
	Vertex_PCU verts[NUM_VERTS];
	constexpr float DEGREES_PER_SIDE = 360.f / static_cast<float>(NUM_SIDES);
	for (int sideNum = 0; sideNum < NUM_SIDES; ++sideNum)
	{
		//Compute angle-related terms
		float startDegrees = DEGREES_PER_SIDE * static_cast<float>(sideNum);
		float endDegrees = DEGREES_PER_SIDE * static_cast<float>(sideNum + 1);

		//Compute inner & outer positions
		Vec3 innerStartPos = Vec3(center.x + innerRadius * CosDegrees(startDegrees), center.y + innerRadius * SinDegrees(startDegrees), 0.f);
		Vec3 innerEndPos   = Vec3(center.x + innerRadius * CosDegrees(endDegrees), center.y + innerRadius * SinDegrees(endDegrees), 0.f);
		Vec3 outerStartPos = Vec3(center.x + outerRadius * CosDegrees(startDegrees), center.y + outerRadius * SinDegrees(startDegrees), 0.f);
		Vec3 outerEndPos   = Vec3(center.x + outerRadius * CosDegrees(endDegrees), center.y + outerRadius * SinDegrees(endDegrees), 0.f);
	
		//Trapezoid is made of two triangles ABC and DEF
		//A is inner end; B is inner start; C is outer start
		//D is inner end; E is outer start; F is outer end;
		int vertIndexA = (6 * sideNum) + 0;
		int vertIndexB = (6 * sideNum) + 1;
		int vertIndexC = (6 * sideNum) + 2;
		int vertIndexD = (6 * sideNum) + 3;
		int vertIndexE = (6 * sideNum) + 4;
		int vertIndexF = (6 * sideNum) + 5;

		verts[vertIndexA].m_position = innerEndPos;
		verts[vertIndexB].m_position = innerStartPos;
		verts[vertIndexC].m_position = outerStartPos;
		verts[vertIndexA].m_color = color;
		verts[vertIndexB].m_color = color;
		verts[vertIndexC].m_color = color;

		verts[vertIndexD].m_position = innerEndPos;
		verts[vertIndexE].m_position = outerStartPos;
		verts[vertIndexF].m_position = outerEndPos;
		verts[vertIndexD].m_color = color;
		verts[vertIndexE].m_color = color;
		verts[vertIndexF].m_color = color;
	}
	g_theRenderer->DrawVertexArray(NUM_VERTS, verts);
}

void DebugDrawLine(Vec2 const& startPos, Vec2 const& endPos, float thickness, Rgba8 const& color)
{
	Vec2 Distance = endPos - startPos;
	Vec2 D = Distance.GetNormalized();
	float radius = thickness * 0.5f;
	Vec2 fwd = radius * D;
	Vec2 fwdleft = fwd.GetRotated90Degrees();
	Vertex_PCU verts[6];
	Vec2 APos = endPos + fwd + fwdleft;
	Vec2 BPos = startPos - fwd + fwdleft;
	Vec2 CPos = startPos - fwd - fwdleft;
	Vec2 DPos = endPos + fwd - fwdleft;

	//Triangle ABC
	verts[0].m_position = Vec3(APos.x, APos.y, 0.f);
	verts[1].m_position = Vec3(BPos.x, BPos.y, 0.f);
	verts[2].m_position = Vec3(CPos.x, CPos.y, 0.f);
	verts[0].m_color = color;
	verts[1].m_color = color;
	verts[2].m_color = color;

	//Triangle CDA
	verts[3].m_position = Vec3(CPos.x, CPos.y, 0.f);
	verts[4].m_position = Vec3(DPos.x, DPos.y, 0.f);
	verts[5].m_position = Vec3(APos.x, APos.y, 0.f);
	verts[3].m_color = color;
	verts[4].m_color = color;
	verts[5].m_color = color;
	g_theRenderer->DrawVertexArray(6, verts);
}

void WarpVertexes(int numVerts, Vertex_PCU* verts, Vec2 const& warpCenter, float warpAmount)
{
	for (int i = 0; i < numVerts; i++)
	{
		Vec3& pos = verts[i].m_position;
		Vec2 localPos(pos.x - warpCenter.x, pos.y - warpCenter.y);
		float r = localPos.GetLength();
		float thetaDegrees = localPos.GetOrientationDegrees();
		//float warpScale = r*.01f;
		//float warpScale = RangeMap()
		//float warpAmound=RangeMap(sin(GetCurrentTimeSeconds()),-1.f,1.f,0.f,10.f)
		//float warpScale
		//thetaDegrees += warpScale* warpAmount;
		thetaDegrees += warpAmount;
		pos.x = warpCenter.x + r * CosDegrees(warpAmount);
		pos.y = warpCenter.y + r * SinDegrees(thetaDegrees);
		//pos.y -= warpAmount * 100.f;
	}
}

//DebugDrawGlow(m_position, 10.f, Rgba8(255,0,0,100), Rgba8(255,0,0,0));
void DebugDrawGlow(Vec2 const& center, float radius, Rgba8 const& centerColor, Rgba8 const& edgeColor)
{
	constexpr int NUM_SIDES = 32;
	constexpr int NUM_TRIS = 2 * NUM_SIDES;
	constexpr int NUM_VERTS = 3 * NUM_TRIS;
	Vertex_PCU verts[NUM_VERTS];

	constexpr float DEGREES_PER_SIDE = 360.f / static_cast<float>(NUM_SIDES);

	for (int sideNum = 0; sideNum < NUM_SIDES; ++sideNum)
	{
		float startDegrees = DEGREES_PER_SIDE * static_cast<float>(sideNum);
		float endDegrees = DEGREES_PER_SIDE * static_cast<float>(sideNum + 1);

		float innerRadius = radius * 0.9f; 
		float outerRadius = radius * 1.1f; 

		Vec3 innerStartPos = Vec3(center.x , center.y , 0.f);
		Vec3 innerEndPos   = Vec3(center.x , center.y , 0.f);
		Vec3 outerStartPos = Vec3(center.x + outerRadius * CosDegrees(startDegrees), center.y + outerRadius * SinDegrees(startDegrees), 0.f);
		Vec3 outerEndPos   = Vec3(center.x + outerRadius * CosDegrees(endDegrees)  , center.y + outerRadius * SinDegrees(endDegrees)  , 0.f);

		// Trapezoid is made of two triangles ABC and DEF
		// A is inner end; B is inner start; C is outer start
		// D is inner end; E is outer start; F is outer end;
		int vertIndexA = (6 * sideNum) + 0;
		int vertIndexB = (6 * sideNum) + 1;
		int vertIndexC = (6 * sideNum) + 2;
		int vertIndexD = (6 * sideNum) + 3;
		int vertIndexE = (6 * sideNum) + 4;
		int vertIndexF = (6 * sideNum) + 5;

		float distanceFromCenter = GetDistance2D(center, Vec2(innerStartPos.x+ innerEndPos.x , innerStartPos.y + innerEndPos.y)*0.5f);
		float alpha = RangeMap(distanceFromCenter, innerRadius, outerRadius, static_cast<float>(centerColor.a), static_cast<float>(edgeColor.a));

		verts[vertIndexA].m_position = innerEndPos;
		verts[vertIndexB].m_position = innerStartPos;
		verts[vertIndexC].m_position = outerStartPos;
		verts[vertIndexA].m_color = Rgba8(centerColor.r, centerColor.g, centerColor.b, static_cast<unsigned char>(alpha));
		verts[vertIndexB].m_color = Rgba8(centerColor.r, centerColor.g, centerColor.b, static_cast<unsigned char>(alpha));
		verts[vertIndexC].m_color = Rgba8(centerColor.r, centerColor.g, centerColor.b, static_cast<unsigned char>(alpha));

		verts[vertIndexD].m_position = innerEndPos;
		verts[vertIndexE].m_position = outerStartPos;
		verts[vertIndexF].m_position = outerEndPos;
		verts[vertIndexD].m_color = Rgba8(centerColor.r, centerColor.g, centerColor.b, static_cast<unsigned char>(alpha));
		verts[vertIndexE].m_color = Rgba8(centerColor.r, centerColor.g, centerColor.b, static_cast<unsigned char>(alpha));
		verts[vertIndexF].m_color = Rgba8(edgeColor.r, edgeColor.g, edgeColor.b, static_cast<unsigned char>(alpha));
	}

	g_theRenderer->DrawVertexArray(NUM_VERTS, verts);
}