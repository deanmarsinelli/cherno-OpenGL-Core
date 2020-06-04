#include "ParticleSystem.h"

#include "Random.h"

#include <glm/gtc/constants.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp>

ParticleSystem::ParticleSystem()
{
	m_ParticlePool.resize(1000);
}

void ParticleSystem::OnUpdate(GLCore::Timestep ts)
{
	for (Particle& particle : m_ParticlePool)
	{
		if (particle.Active == false)
		{
			continue;
		}

		if (particle.LifeRemaining <= 0.f)
		{
			particle.Active = false;
			continue;
		}

		particle.LifeRemaining -= ts;
		particle.Position += particle.Velocity * (float)ts;
		particle.Rotation += 0.01f * ts;
	}
}

void ParticleSystem::OnRender(GLCore::Utils::OrthographicCamera& camera)
{
	if (m_QuadVA == 0)
	{
		// create render state
		float vertices[] = {
			-0.5f, -0.5f, 0.0f,
			 0.5f, -0.5f, 0.0f,
			 0.5f,  0.5f, 0.0f,
			-0.5f,  0.5f, 0.0f
		};

		glCreateVertexArrays(1, &m_QuadVA);
		glBindVertexArray(m_QuadVA);

		GLuint quadVB;
		glCreateBuffers(1, &quadVB);
		glBindBuffer(GL_ARRAY_BUFFER, quadVB);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glEnableVertexArrayAttrib(quadVB, 0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);

		uint32_t indices[] = {
			0, 1, 2, 2, 3, 0
		};

		GLuint quadIB;
		glCreateBuffers(1, &quadIB);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadIB);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		m_ParticleShader = std::unique_ptr<GLCore::Utils::Shader>(GLCore::Utils::Shader::FromGLSLTextFiles("assets/shaders/particle.vert.glsl", "assets/shaders/particle.frag.glsl"));
		m_ParticleShaderViewProjection = glGetUniformLocation(m_ParticleShader->GetRendererID(), "u_ViewProjection");
		m_ParticleShaderTransform = glGetUniformLocation(m_ParticleShader->GetRendererID(), "u_Transform");
		m_ParticleShaderColor = glGetUniformLocation(m_ParticleShader->GetRendererID(), "u_Color");
	}

	glUseProgram(m_ParticleShader->GetRendererID());
	glUniformMatrix4fv(m_ParticleShaderViewProjection, 1, GL_FALSE, glm::value_ptr(camera.GetViewProjectionMatrix()));

	for (Particle& particle : m_ParticlePool)
	{
		if (particle.Active == false)
		{
			continue;
		}

		// fade particles
		float life = particle.LifeRemaining / particle.Lifetime;
		glm::vec4 color = glm::lerp(particle.ColorEnd, particle.ColorBegin, life);
		color.a = color.a * life;

		float size = glm::lerp(particle.SizeEnd, particle.SizeBegin, life);

		// render
		glm::mat4 transform = glm::translate(glm::mat4(1.f), { particle.Position.x, particle.Position.y, 0.f });
		transform = glm::rotate(transform, particle.Rotation, { 0.f, 0.f, 1.f });
		transform = glm::scale(transform, { size, size, 1.f });

		glUniformMatrix4fv(m_ParticleShaderTransform, 1, GL_FALSE, glm::value_ptr(transform));
		glUniform4fv(m_ParticleShaderColor, 1, glm::value_ptr(color));
		glBindVertexArray(m_QuadVA);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	}
}

void ParticleSystem::Emit(const ParticleProps& particleProps)
{
	Particle& particle = m_ParticlePool[m_PoolIndex];
	particle.Active = true;
	particle.Position = particleProps.Position;
	particle.Rotation = Random::Float() * 2.f * glm::pi<float>(); // random 0 - 2pi

	// velocity
	particle.Velocity = particleProps.Velocity;
	particle.Velocity.x += particleProps.VelocityVariation.x * (Random::Float() - 0.5f); // variation +- 1/2
	particle.Velocity.y += particleProps.VelocityVariation.y * (Random::Float() - 0.5f);

	// color
	particle.ColorBegin = particleProps.ColorBegin;
	particle.ColorEnd = particleProps.ColorEnd;

	particle.Lifetime = particleProps.LifeTime;
	particle.LifeRemaining = particleProps.LifeTime;

	particle.SizeBegin = particleProps.SizeBegin + particleProps.SizeVariation * (Random::Float() - 0.5f); // SizeVariation +- 1/2
	particle.SizeEnd = particleProps.SizeEnd;

	m_PoolIndex = --m_PoolIndex;
	m_PoolIndex = (m_PoolIndex < 0) ? (m_ParticlePool.size() - 1) : m_PoolIndex;
}

