#pragma once

#include <Ramen/Core/SharedObject.h>

#include <Kaiga/Core/IRenderPass.h>
#include <Kaiga/Core/RenderPhase.h.>

#include <glm.h>

class IMGUIRenderPass
	: public Kaiga::IRenderPass
	, public Ramen::SharedObject<IMGUIRenderPass>
{
public:
	// Inherited via IRenderPass
	virtual void BindToScene(Ramen::Scene & _scene) override;
	virtual void UnbindFromScene(Ramen::Scene & _scene) override;
	virtual void Render() override;
	virtual Kaiga::RenderPhase GetRenderPhase() override;
private:
	Ramen::Scene* m_scene;
};