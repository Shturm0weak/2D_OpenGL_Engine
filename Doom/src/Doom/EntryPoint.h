#pragma once

#include "Render/Renderer.h"
#include "Render/Batch.h"
#include "Text/Gui.h"
#include <ImGui/imgui.h>
#include <ImGui/examples/imgui_impl_glfw.h>
#include <ImGui/examples/imgui_impl_opengl3.h>
#include "Core/ViewPort.h"
#include "Application.h"

namespace Doom {

	class DOOM_API EntryPoint {
	public:

		EntryPoint(Doom::Application* app);
		~EntryPoint();

		void Run();
	};

}