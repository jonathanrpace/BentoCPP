#include <windows.h>

// std
#include <stdlib.h>
#include <crtdbg.h>
#include <memory>
#include <typeinfo>
#include <iostream>

// libs
#include <gl/glew.h>
#include <glfw3.h>
#include <imgui/imgui_impl_glfw_gl3.cpp>

// bento
#include <bento.h>
#include <bento/Core/Entity.h>
#include <bento/core/InputManagerImpl.h>
#include <bento/core/WindowImpl.h>
#include <bento/components/Transform.h>
#include <bento/components/materials/CubeMapSkyMaterial.h>
#include <bento/components/geom/ScreenQuadGeometry.h>
#include <bento/components/geom/PlaneGeometry.h>
#include <bento/components/geom/SkyboxGeometry.h>
#include <bento/processes/OrbitCamera.h>
#include <bento/util/GLErrorUtil.h>
#include <bento/render/passes/CubeMapSkyPass.h>

// App 
#include <render/Renderer.h>

// Render passes
#include <render/passes/TerrainPass.h>
#include <render/passes/TerrainSidesPass.h>
#include <render/passes/WaterPass.h>
#include <render/passes/FoamPass.h>
#include <render/passes/SteamPass.h>
#include <render/passes/CloudPass.h>
#include <render/passes/IMGUIRenderPass.h>

// Processes
#include <processes/TerrainSimulationProcess.h>
#include <processes/InspectorUIProcess.h>

// Geometry
#include <components/geom/TerrainGeometry.h>
#include <components/geom/TerrainSidesGeometry.h>
#include <components/geom/FoamParticleGeom.h>
#include <components/geom/SteamParticleGeom.h>

// Materials
#include <components/materials/TerrainMaterial.h>
#include <components/materials/WaterMaterial.h>
#include <components/materials/SmokeParticleMaterial.h>
#include <components/materials/CloudMaterial.h>

using namespace bento;
using namespace godBox;

void mainLoop(GLFWwindow* window)
{
	auto inputManager = new InputManagerImpl(window);
	auto bentoWindow = new WindowImpl(window);
	
	// Terrain
	bento::Scene scene(inputManager, bentoWindow);
	{
		auto entity = bento::Entity::Create();
		entity->Name("Terrain");
		scene.AddEntity(entity);

		auto geom = TerrainGeometry::Create();
		scene.AddComponentToEntity(geom, entity);

		auto sidesGeom = TerrainSidesGeometry::Create();
		scene.AddComponentToEntity(sidesGeom, entity);

		auto transform = Transform::Create();
		scene.AddComponentToEntity(transform, entity);
		auto terrainMaterial = TerrainMaterial::Create();
		scene.AddComponentToEntity(terrainMaterial, entity);
		auto waterMaterial = WaterMaterial::Create();
		scene.AddComponentToEntity(waterMaterial, entity);
		auto steamMaterial = SmokeParticleMaterial::Create();
		scene.AddComponentToEntity(steamMaterial, entity);

		//auto foamParticleGeom = FoamParticleGeom::Create();
		//scene.AddComponentToEntity(foamParticleGeom, entity);
		auto steamParticleGeom = SteamParticleGeom::Create();
		scene.AddComponentToEntity(steamParticleGeom, entity);
		auto moltenParticleGeom = MoltenParticleGeom::Create();
		scene.AddComponentToEntity(moltenParticleGeom, entity);
	}

	// Sky
	{
		auto skyBoxGeom = bento::SkyboxGeometry::Create();	
		auto skyBoxMaterial = bento::CubeMapSkyMaterial::Create();
		auto entity = bento::Entity::Create();
		entity->Name("Sky");
		scene.AddEntity(entity);
		scene.AddComponentToEntity(skyBoxGeom, entity);
		scene.AddComponentToEntity(skyBoxMaterial, entity);
	}

	// Clouds
	{
		auto entity = bento::Entity::Create();
		entity->Name("Clouds");
		scene.AddEntity(entity);

		auto geom = bento::ScreenQuadGeometry::Create();
		scene.AddComponentToEntity(geom, entity);

		auto cloudsMaterial = godBox::CloudMaterial::Create();
		scene.AddComponentToEntity(cloudsMaterial, entity);
		
		auto transform = Transform::Create();
		transform->matrix = glm::translate(mat4(), vec3(0.0,0.1f,0.0));
		scene.AddComponentToEntity(transform, entity);
	}

	// Processes
	scene.AddProcess(OrbitCamera::Create());
	scene.AddProcess(TerrainSimulationProcess::Create());
	
	//#ifndef B_RELEASE
		scene.AddProcess(bento::InspectorUIProcess::Create());
	//#endif
		
	auto renderer = godBox::Renderer::Create();
	// Render passes
	{
		renderer->AddRenderPass(CubeMapSkyPass::Create()); // TODO - How do I specify renderPhase through shared object construction?
		//renderer->AddRenderPass(CloudPass::Create());
		renderer->AddRenderPass(TerrainPass::Create());
		renderer->AddRenderPass(TerrainSidesPass::Create());
		
		renderer->AddRenderPass(WaterPass::Create());
		
		//renderer->AddRenderPass(SteamPass::Create());
		//renderer->AddRenderPass(TerrainFoamPass::Create());
		
		renderer->AddRenderPass(IMGUIRenderPass::Create());
	}
	scene.AddProcess(renderer);
	

	// Loop until the user closes the window 
	while (!glfwWindowShouldClose(window))
	{
		/* Poll for and process events */
		glfwPollEvents();
		ImGui_ImplGlfwGL3_NewFrame();
		scene.Advance(1.0f / 60.0f);
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
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef B_DEBUG
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

	// Load OpenGL extensions via GLEW
	{
		glewExperimental = GL_TRUE;
		if ( glewInit() != GLEW_OK )
		{
			__debugbreak();
			glfwTerminate();
			return -1;
		}
		REQUIRE_CAPABILITY(GLEW_VERSION_4_3);
		REQUIRE_CAPABILITY(GLEW_ARB_separate_shader_objects);
	}

	// Clear the GL error state. This is really horrible, but it seems 
	// glfw is generating some gl errors and not checking for them
	while (glGetError() != GL_NONE) {}

	Config::Init("../../../", "../../../");
	DefaultsManager::Init("../../../defaults.json");

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);     

	mainLoop(window);

	Config::Shutdown();
	DefaultsManager::Flush();
	DefaultsManager::Shutdown();

	ImGui_ImplGlfwGL3_Shutdown();
	glfwTerminate();


	if (_CrtDumpMemoryLeaks() != 0)
	{
		__debugbreak();
	}

	return 0;
}