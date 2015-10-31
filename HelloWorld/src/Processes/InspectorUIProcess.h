#pragma once

#include <typeinfo>

#include <imgui.h>

#include <bento/core/Scene.h>
#include <bento/core/IProcess.h>
#include <bento/core/SharedObject.h>

class InspectorUIProcess
	: public bento::IProcess
	, public bento::SharedObject<InspectorUIProcess>
{
public:
	// Inherited via IProcess
	virtual const std::type_info & typeInfo() override;
	virtual void BindToScene(bento::Scene * const _scene) override;
	virtual void UnbindFromScene(bento::Scene * const _scene) override;
	virtual void Update(double dt) override;

private:
	bento::Scene* m_scene;
};