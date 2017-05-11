#include "RenderPass.h"

namespace bento
{
	RenderPass::RenderPass(std::string _name, int _renderPhase)
		: Process(_name)
		, m_renderPhase(_renderPhase)
	{

	}
}

