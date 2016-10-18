#include <windows.h>

#include <stdlib.h>
#include <crtdbg.h>
#include <memory>
#include <typeinfo>
#include <iostream>

#include <gl/glew.h>
#include <GLFW/glfw3.h>

// Bento
#include <bento.h>
#include <bento/Core/Entity.h>
#include <bento/core/InputManagerImpl.h>
#include <bento/core/WindowImpl.h>
#include <bento/render/DefaultRenderer.h>
#include <bento/Components/Transform.h>
#include <bento/components/materials/TerrainMaterial.h>
#include <bento/components/geom/ScreenQuadGeometry.h>
#include <bento/components/geom/TerrainGeometry.h>
#include <bento/components/geom/PlaneGeometry.h>
#include <bento/components/geom/FoamParticleGeom.h>
#include <bento/components/geom/SteamParticleGeom.h>
#include <bento/render/passes/GPass.h>
#include <bento/render/passes/TerrainGPass.h>
#include <bento/render/passes/TerrainWaterPass.h>
#include <bento/render/passes/TerrainFoamPass.h>
#include <bento/render/passes/TerrainSteamPass.h>
#include <bento/processes/TerrainSimulationProcess.h>
#include <bento/processes/OrbitCamera.h>
#include <bento/util/GLErrorUtil.h>

// Local 
#include "RenderPasses/IMGUIRenderPass.h"
#include "Processes/InspectorUIProcess.h"
#include "imgui/imgui_impl_glfw_gl3.cpp"

void mainLoop(GLFWwindow* window)
{
	auto inputManager = new bento::InputManagerImpl(window);
	auto bentoWindow = new bento::WindowImpl(window);
	bento::Scene scene(inputManager, bentoWindow);
	{
		auto entity = bento::Entity::Create();
		entity->Name("Terrain");
		scene.AddEntity(entity);
		auto geom = bento::TerrainGeometry::Create();
		scene.AddComponentToEntity(geom, entity);
		auto transform = bento::Transform::Create();
		scene.AddComponentToEntity(transform, entity);
		auto material = bento::TerrainMaterial::Create();
		scene.AddComponentToEntity(material, entity);
		//auto foamParticleGeom = bento::FoamParticleGeom::Create();
		//scene.AddComponentToEntity(foamParticleGeom, entity);
		auto steamParticleGeom = bento::SteamParticleGeom::Create();
		scene.AddComponentToEntity(steamParticleGeom, entity);
		auto moltenParticleGeom = bento::MoltenParticleGeom::Create();
		scene.AddComponentToEntity(moltenParticleGeom, entity);
	}

	// Processes
	scene.AddProcess(bento::OrbitCamera::Create());
	scene.AddProcess(bento::InspectorUIProcess::Create());
	scene.AddProcess(bento::TerrainSimulationProcess::Create());
	
	auto renderer = bento::DefaultRenderer::Create();
	// Render passes
	{
		renderer->AddRenderPass(bento::GPass::Create());
		renderer->AddRenderPass(bento::TerrainGPass::Create());
		renderer->AddRenderPass(bento::TerrainWaterPass::Create());
		//renderer->AddRenderPass(bento::TerrainFoamPass::Create());
		//renderer->AddRenderPass(bento::TerrainSteamPass::Create());
		renderer->AddRenderPass(bento::IMGUIRenderPass::Create());
	}
	scene.AddProcess(renderer);

	/* Loop until the user closes the window */
	
	while (!glfwWindowShouldClose(window))
	{
		/* Poll for and process events */
		glfwPollEvents();
		ImGui_ImplGlfwGL3_NewFrame();
		scene.Advance(1.0);
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
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef GL_DEBUG_CONTEXT
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif
	GLFWwindow* window = glfwCreateWindow(1280, 720, "Hello World", NULL, NULL);
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

	REQUIRE_CAPABILITY(GLEW_VERSION_4_3);
	REQUIRE_CAPABILITY(GLEW_ARB_separate_shader_objects);

	// Clear the GL error state. This is really horrible, but it seems 
	// glfw is generating some gl errors and not checking for them
	while (glGetError() != GL_NONE) {}

	bento::Config::Init("../../../../Resources/", "../../../../Defaults/");
	bento::DefaultsManager::Init("../../../../Defaults/defaults.json");

	mainLoop(window);

	bento::Config::Shutdown();
	bento::DefaultsManager::Flush();
	bento::DefaultsManager::Shutdown();

	ImGui_ImplGlfwGL3_Shutdown();
	glfwTerminate();


	if (_CrtDumpMemoryLeaks() != 0)
	{
		__debugbreak();
	}

	return 0;
}