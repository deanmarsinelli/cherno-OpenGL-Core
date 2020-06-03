#include "BatchRenderingLayer.h"

using namespace GLCore;
using namespace GLCore::Utils;

BatchRenderingLayer::BatchRenderingLayer()
	: m_CameraController(16.0f / 9.0f)
{
}

BatchRenderingLayer::~BatchRenderingLayer()
{
}

void BatchRenderingLayer::OnAttach()
{
	EnableGLDebugging();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_Shader = Shader::FromGLSLTextFiles(
		"assets/shaders/test.vert.glsl",
		"assets/shaders/test.frag.glsl"
	);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	// position, color
	float vertices[] = {
		// quad 1
		-1.5f, -0.5f, 0.0f, 0.18f, 0.6f, 0.96f, 1.0f,
		-0.5f, -0.5f, 0.0f, 0.18f, 0.6f, 0.96f, 1.0f,
		-0.5f,  0.5f, 0.0f, 0.18f, 0.6f, 0.96f, 1.0f,
		-1.5f,  0.5f, 0.0f, 0.18f, 0.6f, 0.96f, 1.0f,

		// quad 2
		 0.5f, -0.5f, 0.0f, 1.0f, 0.93f, 0.24f, 1.0f,
		 1.5f, -0.5f, 0.0f, 1.0f, 0.93f, 0.24f, 1.0f,
		 1.5f,  0.5f, 0.0f, 1.0f, 0.93f, 0.24f, 1.0f,
		 0.5f,  0.5f, 0.0f, 1.0f, 0.93f, 0.24f, 1.0f
	};

	glCreateVertexArrays(1, &m_QuadVA);
	glBindVertexArray(m_QuadVA);

	glCreateBuffers(1, &m_QuadVB);
	glBindBuffer(GL_ARRAY_BUFFER, m_QuadVB);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glEnableVertexArrayAttrib(m_QuadVB, 0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 7, 0);

	glEnableVertexArrayAttrib(m_QuadVB, 1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (const void*)12 /*12 bytes to get to color*/);

	uint32_t indices[] = {
		0, 1, 2, 2, 3, 0,
		4, 5, 6, 6, 7, 4
	};

	glCreateBuffers(1, &m_QuadIB);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_QuadIB);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
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

void BatchRenderingLayer::OnUpdate(Timestep ts)
{
	m_CameraController.OnUpdate(ts);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(m_Shader->GetRendererID());

	auto vp = m_CameraController.GetCamera().GetViewProjectionMatrix();
	SetUniformMat4(m_Shader->GetRendererID(), "u_ViewProjection", vp);
	SetUniformMat4(m_Shader->GetRendererID(), "u_Transform", glm::mat4(1.f));

	glBindVertexArray(m_QuadVA);
	glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, nullptr);
}

void BatchRenderingLayer::OnImGuiRender()
{
	// ImGui here
}
