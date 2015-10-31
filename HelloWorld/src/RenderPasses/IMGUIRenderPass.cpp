#include "IMGUIRenderPass.h"

#include <glew.h>
#include <imgui.h>

#include <bento/util/GLErrorUtil.h>

void IMGUIRenderPass::BindToScene(bento::Scene & _scene)
{
	m_scene = &_scene;
}

void IMGUIRenderPass::UnbindFromScene(bento::Scene & _scene)
{
	m_scene = nullptr;
}

void IMGUIRenderPass::Render()
{
	bool show_test_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImColor(114, 144, 154);
	/*
	// 1. Show a simple window
	// Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
	{
		ImGui::Begin("Some window");
		static float f = 0.0f;
		ImGui::Text("Hello, world!");
		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
		ImGui::ColorEdit3("clear color", (float*)&clear_color);
		if (ImGui::Button("Test Window")) show_test_window ^= 1;
		if (ImGui::Button("Another Window")) show_another_window ^= 1;
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}
	*/
	ivec2 windowSize = m_scene->GetWindow()->GetWindowSize();
	glViewport(0, 0, windowSize.x, windowSize.y);
	ImGui::Render();
	GL_CHECK(;);
}

bento::RenderPhase IMGUIRenderPass::GetRenderPhase()
{
	return bento::RenderPhase::eRenderPhase_UI;
}
