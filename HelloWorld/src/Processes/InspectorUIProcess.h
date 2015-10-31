#pragma once

#include <typeinfo>

#include <imgui.h>

#include <Ramen/Core/Scene.h>
#include <Ramen/Core/IProcess.h>
#include <Ramen/Core/SharedObject.h>

class InspectorUIProcess
	: public Ramen::IProcess
	, public Ramen::SharedObject<InspectorUIProcess>
{
public:
	// Inherited via IProcess
	virtual const std::type_info & typeInfo() override;
	virtual void BindToScene(Ramen::Scene * const _scene) override;
	virtual void UnbindFromScene(Ramen::Scene * const _scene) override;
	virtual void Update(double dt) override;

private:
	Ramen::Scene* m_scene;
};