#include <windows.h>

#include <stdlib.h>
#include <crtdbg.h>
#include <memory>
#include <typeinfo>
#include <iostream>

#include <glew.h>
#include <GLFW/glfw3.h>
#include <glm.h>
#include "imgui/imgui_impl_glfw_gl3.cpp"

// Ramen
#include <Ramen/Core/InputManagerImpl.h>
#include <Ramen/Core/WindowImpl.h>

// Kaiga
#include <Kaiga/Renderers/DefaultRenderer.h>
#include <Kaiga/Components/Transform.h>
#include <Kaiga/Materials/TerrainMaterial.h>
#include <Kaiga/Geometry/ScreenQuadGeometry.h>
#include <Kaiga/Geometry/TerrainGeometry.h>
#include <Kaiga/Geometry/PlaneGeometry.h>
#include <Kaiga/RenderPasses/GPass.h>
#include <Kaiga/RenderPasses/TerrainGPass.h>
#include <Kaiga/Processes/OrbitCamera.h>

#include <Kaiga/Util/GLErrorUtil.h>

void mainLoop(GLFWwindow* window)
{
	auto inputManager = new Ramen::InputManagerImpl(window);
	auto ramenWindow = new Ramen::WindowImpl(window);
	Ramen::Scene scene(inputManager, ramenWindow);
	
	{
		int entity = scene.CreateEntity();
		auto geom = Kaiga::TerrainGeometry::Create();
		scene.AddComponentToEntity(geom, entity);
		auto transform = Kaiga::Transform::Create();
		scene.AddComponentToEntity(transform, entity);
		auto material = Kaiga::TerrainMaterial::Create();
		scene.AddComponentToEntity(material, entity);
	}
	
	auto renderer = Kaiga::DefaultRenderer::Create();
	scene.AddProcess(renderer);
	renderer->AddRenderPass(Kaiga::GPass::Create());
	renderer->AddRenderPass(Kaiga::TerrainGPass::Create());
	
	auto orbitCamera = Kaiga::OrbitCamera::Create();
	scene.AddProcess(orbitCamera);

	/* Loop until the user closes the window */
	bool show_test_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImColor(114, 144, 154);
	while (!glfwWindowShouldClose(window))
	{
		/* Poll for and process events */
		glfwPollEvents();

		scene.Update(1.0);
		
		ImGui_ImplGlfwGL3_NewFrame();
		// 1. Show a simple window
		// Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
		{
			static float f = 0.0f;
			ImGui::Text("Hello, world!");
			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
			ImGui::ColorEdit3("clear color", (float*)&clear_color);
			if (ImGui::Button("Test Window")) show_test_window ^= 1;
			if (ImGui::Button("Another Window")) show_another_window ^= 1;
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		}

		// UI Render
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		ImGui::Render();
		GL_CHECK(;);
		
		/* Swap front and back buffers */
		glfwSwapBuffers(window);
	}
}

#define REQUIRE_CAPABILITY( CAP )	\
{									\
	if (!CAP)						\
	{								\
		__debugbreak();				\
		glfwTerminate();			\
		return -1;					\
	}								\
}

int main(int argc, char **argv)
{
	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef GL_DEBUG_CONTEXT
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif
	GLFWwindow* window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
	if (!window)
	{
		__debugbreak();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Setup ImGui binding
	ImGui_ImplGlfwGL3_Init(window, true);

	glewExperimental = GL_TRUE;
	if ( glewInit() != GLEW_OK )
	{
		__debugbreak();
		glfwTerminate();
		return -1;
	}

	REQUIRE_CAPABILITY(GLEW_VERSION_4_5);
	REQUIRE_CAPABILITY(GLEW_ARB_separate_shader_objects);

	// Clear the GL error state. This is really horrible, but it seems 
	// glfw is generating some gl errors and not checking for them
	while (glGetError() != GL_NONE) {}

	Ramen::ResourcePath("../../../../Resources/");
	
	mainLoop(window);

	ImGui_ImplGlfwGL3_Shutdown();
	glfwTerminate();

	if (_CrtDumpMemoryLeaks() != 0)
	{
		__debugbreak();
	}

	return 0;
}