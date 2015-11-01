#pragma once

#include <bento/core/Scene.h>
#include <bento/core/SceneObject.h>
#include <bento/core/RenderPhase.h>

namespace bento
{
	struct IRenderPass : SceneObject
	{
		virtual void BindToScene(bento::Scene & _scene) = 0;
		virtual void UnbindFromScene(bento::Scene & _scene) = 0;
		virtual void Render() = 0;
		virtual RenderPhase GetRenderPhase() = 0;
	};

	typedef std::shared_ptr<IRenderPass> RenderPassPtr;
}