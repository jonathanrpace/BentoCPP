#pragma once

#include <bento.h>
#include <bento/core/ILens.h>

#include <render/RenderTarget.h>

namespace godBox
{
	class RenderParams
	{
	public:
		static RenderTarget& GetRenderTarget() { return *s_renderTarget; }
		static void SetRenderTarget(RenderTarget& _renderTarget) { s_renderTarget = &_renderTarget; }

	private:
		static RenderTarget* s_renderTarget;
	};
}