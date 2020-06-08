#include "BatchRenderingLayer.h"
#include <stb_image/stb_image.h>

using namespace GLCore;
using namespace GLCore::Utils;

struct Vec2
{
	float x, y;
};

struct Vec3
{
	float x, y, z;
};

struct Vec4
{
	float x, y, z, w;
};

struct Vertex
{
	Vec3 Position;
	Vec4 Color;
	Vec2 TexCoords;
	float TexIndex;
};

BatchRenderingLayer::BatchRenderingLayer()
	: m_CameraController(16.0f / 9.0f)
{
}

BatchRenderingLayer::~BatchRenderingLayer()
{
}

static GLuint LoadTexture(const std::string& path)
{
	int w, h, bits;

	stbi_set_flip_vertically_on_load(1);
	auto* pixels = stbi_load(path.c_str(), &w, &h, &bits, STBI_rgb);

	GLuint textureID;
	glCreateTextures(GL_TEXTURE_2D, 1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	stbi_image_free(pixels);
	return textureID;
}

void BatchRenderingLayer::OnAttach()
{
	EnableGLDebugging();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_Shader = Shader::FromGLSLTextFiles(
		"assets/shaders/batch.vert.glsl",
		"assets/shaders/batch.frag.glsl"
	);

	glUseProgram(m_Shader->GetRendererID());
	auto loc = glGetUniformLocation(m_Shader->GetRendererID(), "u_Textures");
	int samplers[2] = { 0, 1 };
	glUniform1iv(loc, 2, samplers);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	const size_t MaxQuadCount = 1000;
	const size_t MaxVertexCount = MaxQuadCount * 4;
	const size_t MaxIndexCount = MaxQuadCount * 6;

	glCreateVertexArrays(1, &m_QuadVA);
	glBindVertexArray(m_QuadVA);

	glCreateBuffers(1, &m_QuadVB);
	glBindBuffer(GL_ARRAY_BUFFER, m_QuadVB);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * MaxVertexCount, nullptr, GL_DYNAMIC_DRAW);

	// position
	glEnableVertexArrayAttrib(m_QuadVB, 0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, Position));

	// color
	glEnableVertexArrayAttrib(m_QuadVB, 1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, Color));

	// texcoord
	glEnableVertexArrayAttrib(m_QuadVB, 2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, TexCoords));

	// texindex
	glEnableVertexArrayAttrib(m_QuadVB, 3);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, TexIndex));

	//uint32_t indices[] = {
	//	0, 1, 2, 2, 3, 0,
	//	4, 5, 6, 6, 7, 4
	//};

	uint32_t indices[MaxIndexCount];
	uint32_t offset = 0; // offset = num iterations * 4
	for (size_t i = 0; i < MaxIndexCount; i += 6)
	{
		indices[i + 0] = 0 + offset;
		indices[i + 1] = 1 + offset;
		indices[i + 2] = 2 + offset;

		indices[i + 3] = 2 + offset;
		indices[i + 4] = 3 + offset;
		indices[i + 5] = 0 + offset;

		offset += 4;
	}

	glCreateBuffers(1, &m_QuadIB);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_QuadIB);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	m_ChernoTex = LoadTexture("assets/textures/Cherno.png");
	m_HazelTex = LoadTexture("assets/textures/Hazel.png");
}

void BatchRenderingLayer::OnDetach()
{
	glDeleteVertexArrays(1, &m_QuadVA);
	glDeleteBuffers(1, &m_QuadVB);
	glDeleteBuffers(1, &m_QuadIB);
}

void BatchRenderingLayer::OnEvent(Event& event)
{
	m_CameraController.OnEvent(event);
}

static void SetUniformMat4(uint32_t shader, const char* name, const glm::mat4& matrix)
{
	int loc = glGetUniformLocation(shader, name);
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(matrix));
}

static Vertex* CreateQuad(Vertex* target, float x, float y, float texIndex)
{
	float size = 1.f;

	target->Position = { x, y, 0.0f };
	target->Color = { 0.18f, 0.6f, 0.96f, 1.0f };
	target->TexCoords = { 0.0f, 0.0f };
	target->TexIndex = texIndex;
	target++;

	target->Position = { x + size, y, 0.0f };
	target->Color = { 0.18f, 0.6f, 0.96f, 1.0f };
	target->TexCoords = { 1.0f, 0.0f };
	target->TexIndex = texIndex;
	target++;

	target->Position = { x + size, y + size, 0.0f };
	target->Color = { 0.18f, 0.6f, 0.96f, 1.0f };
	target->TexCoords = { 1.0f, 1.0f };
	target->TexIndex = texIndex;
	target++;

	target->Position = { x, y + size, 0.0f };
	target->Color = { 0.18f, 0.6f, 0.96f, 1.0f };
	target->TexCoords = { 0.0f, 1.0f };
	target->TexIndex = texIndex;
	target++;

	return target;
}

void BatchRenderingLayer::OnUpdate(Timestep ts)
{
	m_CameraController.OnUpdate(ts);

	uint32_t indexCount = 0;

	std::array<Vertex, 1000> vertices;
	Vertex* buffer = vertices.data();
	for (int y = 0; y < 5; y++)
	{
		for (int x = 0; x < 5; x++)
		{
			buffer = CreateQuad(buffer, x, y, (x + y) % 2);
			indexCount += 6;
		}
	}
	buffer = CreateQuad(buffer, m_QuadPosition[0], m_QuadPosition[1], 0.f);
	indexCount += 6;

	glBindBuffer(GL_ARRAY_BUFFER, m_QuadVB);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), vertices.data());

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(m_Shader->GetRendererID());
	glBindTextureUnit(0, m_ChernoTex);
	glBindTextureUnit(1, m_HazelTex);

	auto vp = m_CameraController.GetCamera().GetViewProjectionMatrix();
	SetUniformMat4(m_Shader->GetRendererID(), "u_ViewProjection", vp);
	SetUniformMat4(m_Shader->GetRendererID(), "u_Transform", glm::mat4(1.f));

	glBindVertexArray(m_QuadVA);
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
}

void BatchRenderingLayer::OnImGuiRender()
{
	// ImGui here
	ImGui::Begin("Controls");
	ImGui::DragFloat2("Quad Position", m_QuadPosition, 0.1f);
	ImGui::End();
}
