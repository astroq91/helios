#include <Helios/Core/EntryPoint.h>
#include <Helios/Core/Application.h>

#include "EditorLayer.h"

class Editor : public Helios::Application
{
public:
	Editor()
		: Application({.max_frames_in_flight = 2})
	{
        push_layer(new EditorLayer());
	}
};

Helios::Application* create_application()
{
	return new Editor();
}
