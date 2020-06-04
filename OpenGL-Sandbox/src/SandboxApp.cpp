#include "GLCore.h"
#include "BatchRenderingLayer.h"
#include "ParticleSystemLayer.h"
#include "SandboxLayer.h"

using namespace GLCore;

class Sandbox : public Application
{
public:
	Sandbox()
	{
		//PushLayer(new BatchRenderingLayer());
		PushLayer(new ParticleSystemLayer());
	}
};

int main()
{
	std::unique_ptr<Sandbox> app = std::make_unique<Sandbox>();
	app->Run();
}