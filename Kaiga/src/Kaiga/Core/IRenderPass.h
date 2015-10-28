#pragma once

#include <memory>

#include <Ramen\Core\Scene.h>

#include <Kaiga\Core\RenderPhase.h>

namespace Kaiga
{
	class IRenderPass
	{
	public:
		virtual void BindToScene(Ramen::Scene & scene) = 0;
		virtual void UnbindFromScene(Ramen::Scene & scene) = 0;
		virtual void Render() = 0;
		virtual RenderPhase GetRenderPhase() = 0;
	};

	typedef std::shared_ptr<IRenderPass> RenderPassPtr;
}