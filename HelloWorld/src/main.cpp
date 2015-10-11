#include <stdlib.h>
#include <crtdbg.h>
#include <memory>
#include <typeinfo>
#include <iostream>

#include <glew.h>
#include <glfw3.h>

#include <glm.h>
#include <ramen.h>
#include <kaiga.h>

#include <Ramen/Core/InputManagerImpl.h>

#include <Kaiga/Renderers/DefaultRenderer.h>
#include <Kaiga/Components/Transform.h>
#include <Kaiga/Components/Geom/ScreenQuadGeometry.h>
#include <Kaiga/RenderPasses/TestRenderPass.h>
#include <Kaiga/Processes/OrbitCamera.h>

void mainLoop(GLFWwindow* window)
{
	auto inputManager = new Ramen::InputManagerImpl(window);
	Ramen::Scene scene(inputManager);

	int entity = scene.CreateEntity();
	auto geom = Kaiga::ScreenQuadGeometry::Create();

	auto transform = Kaiga::Transform::Create();
	scene.AddComponentToEntity(geom, entity);
	scene.AddComponentToEntity(transform, entity);

	auto renderer = Kaiga::DefaultRenderer::Create();
	scene.AddProcess(renderer);

	auto renderPass = Kaiga::TestRenderPass::Create();
	renderer->AddRenderPass(renderPass);

	auto orbitCamera = Kaiga::OrbitCamera::Create();
	scene.AddProcess(orbitCamera);

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		scene.Update(1.0);

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
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
	/* Make the window's context current */
	glfwMakeContextCurrent(window);
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
	
	mainLoop(window);

	glfwTerminate();

	if (_CrtDumpMemoryLeaks() != 0)
	{
		__debugbreak();
	}

	return 0;
}