#include <stdlib.h>
#include <crtdbg.h>
#include <memory>
#include <typeinfo>
#include <iostream>

#include <ramen.h>
#include <kaiga.h>

#include <glew.h>
#include <glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Ramen/Components/TestComponent.h>
#include <Kaiga/Renderers/DefaultRenderer.h>
#include <Kaiga/Components/Transform.h>
#include <Kaiga/Components/Geom/ScreenQuadGeometry.h>
#include <Kaiga/RenderPasses/TestRenderPass.h>

void mainLoop(GLFWwindow* window)
{
	Ramen::Scene scene;

	int entity = scene.CreateEntity();
	auto geom = Kaiga::ScreenQuadGeometry::Create();
	auto transform = Kaiga::Transform::Create();
	scene.AddComponentToEntity(geom, entity);
	scene.AddComponentToEntity(transform, entity);

	auto renderer = Kaiga::DefaultRenderer::Create();
	scene.AddProcess(renderer);

	auto renderPass = Kaiga::TestRenderPass::Create();
	renderer->AddRenderPass(renderPass);

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

	if ( !GLEW_ARB_separate_shader_objects )
	{
		__debugbreak();
		glfwTerminate();
		return -1;
	}

	if (!GLEW_VERSION_4_5)
	{
		__debugbreak();
		glfwTerminate();
		return -1;
	}

	mainLoop(window);

	glfwTerminate();

	if (_CrtDumpMemoryLeaks() != 0)
	{
		__debugbreak();
	}

	return 0;
}