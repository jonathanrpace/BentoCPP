#pragma once

#include <memory.h>

#include <bento/core/Process.h>
#include <bento/core/eRenderPhase.h>

namespace bento
{
	class RenderPass
		: public virtual Process
	{
	public:
		RenderPass(eRenderPhase _renderPhase);

		eRenderPhase GetRenderPhase() { return m_renderPhase;  }

	private:
		eRenderPhase m_renderPhase;
	};

	typedef std::shared_ptr<RenderPass> RenderPassPtr;
}