#include "../pch.h"
#include "SpriteRenderer.h"
#include "../Render/Batch.h"
#include "../Core/World.h"
#include "Core/Utils.h"

using namespace Doom;

void Doom::SpriteRenderer::Copy(const SpriteRenderer& rhs)
{
	m_Color = rhs.m_Color;
	m_RenderType = rhs.m_RenderType;
	m_Shader = rhs.m_Shader;
	m_Texture = rhs.m_Texture;
	m_TextureAtlas = rhs.m_TextureAtlas;
	memcpy(m_Mesh2D, rhs.m_Mesh2D, sizeof(rhs.m_Mesh2D));
}

Doom::SpriteRenderer::SpriteRenderer(const SpriteRenderer& rhs)
{
	Copy(rhs);
}

Doom::SpriteRenderer::SpriteRenderer()
{
	m_Shader = Shader::Get("Default2D");
	Renderer::s_Objects2d.push_back(this);
}

Doom::SpriteRenderer::~SpriteRenderer()
{
	auto iter = std::find(Renderer::s_Objects2d.begin(), Renderer::s_Objects2d.end(), this);
	if (iter != Renderer::s_Objects2d.end()) {
		Renderer::s_Objects2d.erase(iter);
	}
}

void Doom::SpriteRenderer::operator=(const SpriteRenderer& rhs)
{
	Copy(rhs);
}

void Doom::SpriteRenderer::GetTransformedVertices(float* buffer)
{
	Transform* tr = this->GetOwnerOfComponent()->GetComponentManager()->GetComponent<Transform>();
	float WorldVerPos[16];
	glm::mat4 scaleXview = tr->m_ViewMat4 * tr->m_ScaleMat4;
	float* pSource;
	pSource = (float*)glm::value_ptr(scaleXview);
	for (unsigned int i = 0; i < 4; i++) {
		for (unsigned int j = 0; j < 4; j++) {
			WorldVerPos[i * 4 + j] = 0;
			for (unsigned int k = 0; k < 4; k++) {
				WorldVerPos[i * 4 + j] += m_Mesh2D[i * 4 + k] * pSource[k * 4 + j];
			}
		}
	}
	buffer[0] = WorldVerPos[0];
	buffer[1] = WorldVerPos[1];
	buffer[2] = WorldVerPos[4];
	buffer[3] = WorldVerPos[5];
	buffer[4] = WorldVerPos[8];
	buffer[5] = WorldVerPos[9];
	buffer[6] = WorldVerPos[12];
	buffer[7] = WorldVerPos[13];
	pSource = nullptr;
}

void Doom::SpriteRenderer::Render()
{
	Batch::GetInstance().Submit(this);
}

Component* Doom::SpriteRenderer::Create()
{
	return new SpriteRenderer();
}

void SpriteRenderer::ReverseUVs()
{
	float v1 = m_Mesh2D[2];
	float v2 = m_Mesh2D[3];
	float v3 = m_Mesh2D[6];
	float v4 = m_Mesh2D[7];
	float v5 = m_Mesh2D[10];
	float v6 = m_Mesh2D[11];
	float v7 = m_Mesh2D[14];
	float v8 = m_Mesh2D[15];
	m_Mesh2D[2] = v3;
	m_Mesh2D[3] = v4;
	m_Mesh2D[6] = v1;
	m_Mesh2D[7] = v2;
	m_Mesh2D[10] = v7;
	m_Mesh2D[11] = v8;
	m_Mesh2D[14] = v5;
	m_Mesh2D[15] = v6;
}

void SpriteRenderer::ReversedUvs()
{
	m_Mesh2D[2] = 1.f;
	m_Mesh2D[3] = 0.f;
	m_Mesh2D[6] = 0.f;
	m_Mesh2D[7] = 0.f;
	m_Mesh2D[10] = 0.f;
	m_Mesh2D[11] = 1.f;
	m_Mesh2D[14] = 1.f;
	m_Mesh2D[15] = 1.f;
}

void SpriteRenderer::OriginalUvs()
{
	m_Mesh2D[2] = 0.f;
	m_Mesh2D[3] = 0.f;
	m_Mesh2D[6] = 1.f;
	m_Mesh2D[7] = 0.f;
	m_Mesh2D[10] = 1.f;
	m_Mesh2D[11] = 1.f;
	m_Mesh2D[14] = 0.f;
	m_Mesh2D[15] = 1.f;
}

void SpriteRenderer::SetUVs(float* uvs)
{
	m_Mesh2D[2] = uvs[0];
	m_Mesh2D[3] = uvs[1];
	m_Mesh2D[6] = uvs[2];
	m_Mesh2D[7] = uvs[3];
	m_Mesh2D[10] = uvs[4];
	m_Mesh2D[11] = uvs[5];
	m_Mesh2D[14] = uvs[6];
	m_Mesh2D[15] = uvs[7];
}

float Doom::SpriteRenderer::GetWidth() const
{
	return m_OwnerOfCom->GetScale()[0] * m_Mesh2D[4] * 2;
}

float Doom::SpriteRenderer::GetHeight() const
{
	return m_OwnerOfCom->GetScale()[1] * m_Mesh2D[9] * 2;
}

float * SpriteRenderer::GetUVs() const
{
	float uvs[8];
	uvs[0] = m_Mesh2D[2];
	uvs[1] = m_Mesh2D[3];
	uvs[2] = m_Mesh2D[6];
	uvs[3] = m_Mesh2D[7];
	uvs[4] = m_Mesh2D[10];
	uvs[5] = m_Mesh2D[11];
	uvs[6] = m_Mesh2D[14];
	uvs[7] = m_Mesh2D[15];
	return uvs;
}

void SpriteRenderer::Setlayer(int layer)
{
	World& world = World::GetInstance();
	unsigned int size = world.s_GameObjects.size();
#ifdef _DEBUG
	std::cout << m_OwnerOfCom->m_Name << " is set from layer " << m_OwnerOfCom->m_Id << " to " << layer << std::endl;
#endif
	world.s_GameObjects.erase(world.s_GameObjects.begin() + m_OwnerOfCom->m_Id);
	world.s_GameObjects.insert(world.s_GameObjects.begin() + layer, m_OwnerOfCom);
	for (unsigned int i = 0; i < world.s_GameObjects.size(); i++)
	{
		world.s_GameObjects[i]->m_Id = i;
		world.s_GameObjects[i]->m_Layer = i;
	}
	return;
}