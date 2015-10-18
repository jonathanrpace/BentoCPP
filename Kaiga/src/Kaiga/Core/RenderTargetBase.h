#pragma once

#include <glew.h>
#include <Ramen/Core/AbstractValidatable.h>
#include <Kaiga\Core\IRenderTarget.h>

namespace Kaiga
{
	class RenderTargetBase : Ramen::AbstractValidatable, IRenderTarget
	{
	public:
		RenderTargetBase(
			int _width,
			int _height,
			bool _isRectangular,
			bool _hasDepthStencil,
			GLenum _internalFormat,
			GLenum _depthStencilFormat
			);
		~RenderTargetBase();

		void SetSize(int _width, int _height);
		virtual void Bind() override;

	private:
		bool m_isRectangular;
		bool m_hasDepthStencil;
		GLenum m_internalFormat;
		GLenum m_depthStencilFormat;
		GLuint m_frameBuffer;
		int m_width;
		int m_height;

		virtual void Validate() override;
		virtual void OnInvalidate() override;
	};
}