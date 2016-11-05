#pragma once

#include <string>

#include <bento/core/Process.h>
#include <bento/core/SharedObject.h>
#include <bento/core/RenderPass.h>
#include <bento/core/Scene.h.>

namespace bento
{
	class IMGUIRenderPass
		: public RenderPass
		, public SharedObject<IMGUIRenderPass>
	{
	public:
		IMGUIRenderPass(std::string _name = "IMGUIRenderPass");

		// Inherited via IProcess
		virtual void Advance(double _dt) override;
	};
}