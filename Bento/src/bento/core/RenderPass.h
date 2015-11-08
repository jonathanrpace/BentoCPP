#pragma once

#include <memory.h>

#include <bento/core/Process.h>
#include <bento/core/RenderPhase.h>

namespace bento
{
	class RenderPass
		: public virtual Process
	{
	public:
		RenderPass(RenderPhase _renderPhase);

		RenderPhase GetRenderPhase() { return m_renderPhase;  }

	private:
		RenderPhase m_renderPhase;
	};

	typedef std::shared_ptr<RenderPass> RenderPassPtr;
}