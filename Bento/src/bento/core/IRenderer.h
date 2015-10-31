#pragma once

#include <bento/core/IProcess.h>
#include <bento/core/IRenderPass.h>

namespace bento
{
	class IRenderer : public bento::IProcess
	{
	public:
		virtual void AddRenderPass(RenderPassPtr _renderPass) = 0;
		virtual void RemoveRenderPass(RenderPassPtr _renderPass) = 0;
	};
}
