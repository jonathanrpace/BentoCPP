#pragma once

#include "Ramen/Core/IProcess.h"
#include "IRenderPass.h"

namespace Kaiga
{
	class IRenderer : public Ramen::IProcess
	{
	public:
		virtual void AddRenderPass(RenderPassPtr _renderPass) = 0;
		virtual void RemoveRenderPass(RenderPassPtr _renderPass) = 0;
	};
}
