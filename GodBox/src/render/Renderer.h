#pragma once

// std
#include <string>

// bento
#include <bento.h>
#include <bento/render/RendererBase.h>
#include <bento/core/Process.h>
#include <bento/core/Scene.h>
#include <bento/core/SharedObject.h>
#include <bento/render/shaders/RectTextureToScreenShader.h>

// app
#include <render/RenderTarget.h>
#include <render/RenderParams.h>

using namespace bento;

namespace godBox
{
	class Renderer 
		: public RendererBase
		, public SharedObject<Renderer>
	{
	public:
		Renderer(std::string _name = "Renderer");
		~Renderer();

		// From Process
		virtual void Advance(double dt) override;

	private:
		RenderTarget m_renderTarget;
		RectTextureToScreenShader m_rectTextureToScreenShader;
	};
}