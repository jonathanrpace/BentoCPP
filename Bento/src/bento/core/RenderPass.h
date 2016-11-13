#pragma once

#include <memory.h>

#include <bento/core/Process.h>

namespace bento
{
	class RenderPass
		: public virtual Process
	{
	public:
		RenderPass(int _renderPhase);

		int GetRenderPhase() { return m_renderPhase;  }

	private:
		int m_renderPhase;
	};

	typedef std::shared_ptr<RenderPass> RenderPassPtr;
}