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
#include <bento/render/shaders/RectBlurTap5Shader.h>

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
		RectBlurTap5Shader m_blurShader;

		RectangleTexture m_blurredBufferA;
		RenderTargetBase m_blurredRenderTargetA;

		RectangleTexture m_blurredBufferB;
		RenderTargetBase m_blurredRenderTargetB;
	};
}