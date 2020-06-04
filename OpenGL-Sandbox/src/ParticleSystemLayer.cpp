#include "ParticleSystemLayer.h"

#include <GLCore/Core/Input.h>
#include <GLCore/Core/MouseButtonCodes.h>

using namespace GLCore;
using namespace GLCore::Utils;

ParticleSystemLayer::ParticleSystemLayer()
	: m_CameraController(16.f / 9.f)
{
}

ParticleSystemLayer::~ParticleSystemLayer()
{
}

void ParticleSystemLayer::OnAttach()
{
	EnableGLDebugging();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_Particle.ColorBegin = { 254 / 255.f, 212 / 255.f, 123 / 255.f, 1.f };
	m_Particle.ColorEnd = { 254 / 255.f, 109 / 255.f, 41 / 255.f, 1.f };
	m_Particle.SizeBegin = 0.5f;
	m_Particle.SizeEnd = 0.f;
	m_Particle.SizeVariation = 0.3f;
	m_Particle.Velocity = { 0.f, 0.f };
	m_Particle.VelocityVariation = { 3.f, 1.f };
	m_Particle.Position = { 0.f, 0.f };
}

void ParticleSystemLayer::OnDetach()
{
	// Shutdown here
}

void ParticleSystemLayer::OnEvent(Event& event)
{
	// Events here
	m_CameraController.OnEvent(event);

	if (event.GetEventType() == EventType::WindowResize)
	{
		WindowResizeEvent& e = (WindowResizeEvent&)event;
		glViewport(0, 0, e.GetWidth(), e.GetHeight());
	}
}

void ParticleSystemLayer::OnUpdate(Timestep ts)
{
	// Render here
	glClearColor(0.1f, 0.1f, 0.1f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);

	if (GLCore::Input::IsMouseButtonPressed(HZ_MOUSE_BUTTON_LEFT))
	{
		auto[x, y] = Input::GetMousePosition();

		float width = (float)GLCore::Application::Get().GetWindow().GetWidth();
		float height = (float)GLCore::Application::Get().GetWindow().GetHeight();

		float xNDC = (x / width) - ((width - x) / width);
		float yNDC = (y / height) - ((height - y) / height);

		const glm::vec4 MousePosNDC = { xNDC, -yNDC, 0.f, 1.f };
		const glm::mat4 InverseViewProj = glm::inverse(m_CameraController.GetCamera().GetViewProjectionMatrix());

		const glm::vec4 MousePosWorldSpace = InverseViewProj * MousePosNDC;
		m_Particle.Position = { MousePosWorldSpace.x, MousePosWorldSpace.y };

		for (int i = 0; i < 5; i++)
		{
			m_ParticleSystem.Emit(m_Particle);
		}
	}

	m_ParticleSystem.OnUpdate(ts);
	m_ParticleSystem.OnRender(m_CameraController.GetCamera());
}

void ParticleSystemLayer::OnImGuiRender()
{
	// ImGui here

	ImGui::Begin("Settings");
	ImGui::ColorEdit4("Birth Color", glm::value_ptr(m_Particle.ColorBegin));
	ImGui::ColorEdit4("Death Color", glm::value_ptr(m_Particle.ColorEnd));
	ImGui::DragFloat("Life Time", &m_Particle.LifeTime, 0.1, 0.f, 1000.f);
	ImGui::End();
}
