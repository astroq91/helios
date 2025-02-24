#include "hlpch.h"
#include "Application.h"

extern Helios::Application* create_application();

int main()
{
    Helios::Log::init();

	auto app = create_application();
    app->run();
	delete app;
}
