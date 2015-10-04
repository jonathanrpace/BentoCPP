#pragma once

#include <memory>
#include <Kaiga\Core\RenderPhase.h>

// Forward declares
namespace Ramen
{
	class Scene;
}

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