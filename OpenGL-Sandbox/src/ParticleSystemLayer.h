#pragma once

#include <GLCore.h>
#include <GLCoreUtils.h>

#include "ParticleSystem.h"

class ParticleSystemLayer : public GLCore::Layer
{
public:
	ParticleSystemLayer();
	virtual ~ParticleSystemLayer();

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnEvent(GLCore::Event& event) override;
	virtual void OnUpdate(GLCore::Timestep ts) override;
	virtual void OnImGuiRender() override;
private:
	GLCore::Utils::OrthographicCameraController m_CameraController;
	ParticleProps m_Particle;
	ParticleSystem m_ParticleSystem;
};