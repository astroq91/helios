#include "SandboxLayer.h"
#include <Helios/Core/EntryPoint.h>
#include <Helios/Core/Application.h>

using namespace Helios;

class Sandbox : public Helios::Application
{
public:
	Sandbox()
		: Application({})
	{
        push_layer(new SandboxLayer());
	}
};

Helios::Application* create_application()
{
	return new Sandbox();
}
