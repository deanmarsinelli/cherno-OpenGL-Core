#pragma once

#include <GLCore.h>
#include <GLCoreUtils.h>

struct ParticleProps
{
	glm::vec2 Position;
	glm::vec2 Velocity;
	glm::vec2 VelocityVariation;
	glm::vec4 ColorBegin;
	glm::vec4 ColorEnd;
	float SizeBegin;
	float SizeEnd;
	float SizeVariation;
	float LifeTime = 1.f;
};

class ParticleSystem
{
public:
	ParticleSystem();

	void OnUpdate(GLCore::Timestep ts);
	void OnRender(GLCore::Utils::OrthographicCamera& camera);

	void Emit(const ParticleProps& particleProps);

private:
	struct Particle
	{
		glm::vec2 Position;
		glm::vec2 Velocity;
		glm::vec4 ColorBegin;
		glm::vec4 ColorEnd;
		float Rotation = 0.f;
		float SizeBegin;
		float SizeEnd;
		float Lifetime = 1.f;
		float LifeRemaining = 1.f;

		bool Active = false;
	};

	std::vector<Particle> m_ParticlePool;
	int32_t m_PoolIndex = 999;

	GLuint m_QuadVA = 0;
	std::unique_ptr<GLCore::Utils::Shader> m_ParticleShader;

	// uniform locations
	GLint m_ParticleShaderViewProjection;
	GLint m_ParticleShaderTransform;
	GLint m_ParticleShaderColor;
};