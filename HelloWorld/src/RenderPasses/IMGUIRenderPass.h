#pragma once

#include <bento/core/SharedObject.h>

#include <bento/core/IRenderPass.h>
#include <bento/core/RenderPhase.h.>

#include <glm.h>

class IMGUIRenderPass
	: public bento::IRenderPass
	, public bento::SharedObject<IMGUIRenderPass>
{
public:
	// Inherited via IRenderPass
	virtual void BindToScene(bento::Scene & _scene) override;
	virtual void UnbindFromScene(bento::Scene & _scene) override;
	virtual void Render() override;
	virtual bento::RenderPhase GetRenderPhase() override;
private:
	bento::Scene* m_scene;
};