#pragma once

#include <bento/core/IProcess.h>
#include <bento/core/IRenderPass.h>

namespace bento
{
	struct IRenderer : IProcess
	{
		IRenderer(std::string _name = "IRenderer");

		virtual void AddRenderPass(RenderPassPtr _renderPass) = 0;
		virtual void RemoveRenderPass(RenderPassPtr _renderPass) = 0;
	};
}
