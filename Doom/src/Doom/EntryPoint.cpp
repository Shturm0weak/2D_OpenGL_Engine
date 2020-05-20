#include "EntryPoint.h"
#include "Core/Timer.h"
#include "Render/Line.h"

using namespace Doom;

EntryPoint::EntryPoint(Doom::Application* app) {
	Window::Init("Doom Engine", 800, 600, false);
	MainThread::Init();
	ThreadPool::Init();
	Batch::Init();
	this->app = app;
	EventSystem::Instance()->SendEvent("OnStart", nullptr);
	Window::GetCamera().frameBuffer = new FrameBuffer();
}
void EntryPoint::Run()
{
	bool isEditorEnable = false;
	double editortimer = 1;
	app->OnStart();
	while (!glfwWindowShouldClose(Window::GetWindow())) {

		Renderer::DrawCalls = 0;
		
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

		DeltaTime::calculateDeltaTime();
		Window::GetCamera().WindowResize();

		EventSystem::Instance()->ProcessEvents();
		Window::GetCamera().CameraMovement();

		if (editortimer > 0.2 && Input::IsKeyPressed(Keycode::KEY_E)) {
			if (isEditorEnable)
				isEditorEnable = false;
			else
				isEditorEnable = true;
			editortimer = 0;
		}
		editortimer += DeltaTime::deltatime;

		app->OnUpdate();
		app->OnImGuiRender();
		
		if (ImGui::IsAnyItemActive())
			Editor::Instance()->isItemActive = true;
		else
			Editor::Instance()->isItemActive = false;

		glBindFramebuffer(GL_FRAMEBUFFER, Window::GetCamera().frameBuffer->fbo);
		Renderer::Clear();
		Renderer::Render();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		Renderer::Clear();

		void* tex = reinterpret_cast<void*>(Window::GetCamera().frameBuffer->texture);

		ImGui::Begin("ViewPort", &ViewPort::Instance()->toolOpen, ImGuiWindowFlags_NoScrollbar);
		if (ImGui::IsWindowHovered()) {
			ViewPort::Instance()->IsHovered = true;
		}
		else {
			ViewPort::Instance()->IsHovered = false;
		}
		ViewPort::Instance()->SetViewPortPos(ImGui::GetWindowPos().x,ImGui::GetWindowPos().y);
		if (ViewPort::Instance()->GetSize().x != ImGui::GetWindowSize().x || ViewPort::Instance()->GetSize().y != ImGui::GetWindowSize().y) {
			ViewPort::Instance()->SetSize(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);
			ViewPort::Instance()->viewportResized = true;
		}
		else
			ViewPort::Instance()->viewportResized = false;

		ImGui::GetWindowDrawList()->AddImage(tex, ImVec2(ViewPort::Instance()->GetViewPortPos().x,
                            ViewPort::Instance()->GetViewPortPos().y), ImVec2(ViewPort::Instance()->GetViewPortPos().x + ViewPort::Instance()->GetSize().x, ViewPort::Instance()->GetViewPortPos().y + ViewPort::Instance()->GetSize().y), ImVec2(0, 1), ImVec2(1, 0));
		ImGui::End();

		if (isEditorEnable)
			Editor::Instance()->EditorUpdate();
		
		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (ViewPort::Instance()->viewportResized) {
			glBindTexture(GL_TEXTURE_2D, Window::GetCamera().frameBuffer->texture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ViewPort::Instance()->GetSize().x, ViewPort::Instance()->GetSize().y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		glfwSwapBuffers(Window::GetWindow());
		glfwPollEvents();
	}
	app->OnClose();
}

EntryPoint::~EntryPoint() {
	
	delete app;
	EventSystem::Instance()->Shutdown();
	ThreadPool::Instance()->shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
}