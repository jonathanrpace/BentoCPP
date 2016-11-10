#pragma once

// std
#include <string>

// bento
#include <bento.h>
#include <bento/render/RendererBase.h>
#include <bento/core/Process.h>
#include <bento/core/Scene.h>
#include <bento/render/shaders/RectTextureToScreenShader.h>

// app
#include <render/RenderTarget.h>
#include <render/RenderParams.h>

namespace godBox
{
	class Renderer 
		: public bento::RendererBase
	{
	public:
		Renderer(std::string _name = "Renderer");
		~Renderer();

		// From Process
		virtual void Advance(double dt) override;

	private:
		RenderTarget m_renderTarget;
		bento::RectTextureToScreenShader m_rectTextureToScreenShader;
	};
}