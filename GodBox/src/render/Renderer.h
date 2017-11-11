#pragma once

// std
#include <string>

// bento
#include <bento.h>
#include <bento/render/RendererBase.h>
#include <bento/core/Process.h>
#include <bento/core/Scene.h>
#include <bento/core/IInspectable.h>
#include <bento/core/SharedObject.h>
#include <bento/core/SerializableBase.h>
#include <bento/render/shaders/RectTextureToScreenShader.h>
#include <bento/render/shaders/RectBlurTap5Shader.h>
#include <bento/components/textures/TextureResource.h>

// godbox
#include <render/shaders/ResolveShader.h>

// app
#include <render/RenderTarget.h>
#include <render/RenderParams.h>

using namespace bento;

namespace godBox
{
	class Renderer 
		: public RendererBase
		, public SharedObject<Renderer>
		, public IInspectable
		, public SerializableBase
	{
	public:
		Renderer(std::string _name = "Renderer");
		~Renderer();
		
		// From Process
		virtual void Advance(double dt) override;

		// From IInspectable
		virtual void AddUIElements() override;

	private:
		RenderTarget m_renderTarget;
		RenderTargetBase m_cloudRenderTarget;
		RectTextureToScreenShader m_rectTextureToScreenShader;
		RectBlurTap5Shader m_blurShader;
		ResolveShader m_resolveShader;
		TextureResource m_dirtyLensTexture;

		RenderTargetBase m_blurredRenderTargetA;
		RenderTargetBase m_blurredRenderTargetB;

		float m_bloomPower;
		float m_bloomStrength;
	};
}