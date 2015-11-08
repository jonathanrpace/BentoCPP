#include "IMGUIRenderPass.h"

#include <glew.h>
#include <imgui.h>

#include <bento/core/Scene.h>

#include <bento/util/GLErrorUtil.h>

namespace bento
{
	IMGUIRenderPass::IMGUIRenderPass(std::string _name)
		: Process(_name, typeid(IMGUIRenderPass))
		, RenderPass(eRenderPhase_UI)
	{
	}

	void IMGUIRenderPass::Advance(double _dt)
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
}