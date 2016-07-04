#pragma once

#include <memory>
#include <string.h>

#include <bento/core/SceneObject.h>

namespace bento
{
	// Forward declares
	class Scene;

	class Process
		: public SceneObject
	{
	public:
		Process(std::string _name = "Process", const std::type_info& _typeInfo = typeid(Process));

		virtual void BindToScene(Scene& _scene);
		virtual void UnbindFromScene(Scene& _scene);

		virtual void Advance(double _dt) = 0;

	protected:
		Scene* m_scene;
	};

	typedef std::shared_ptr<Process> ProcessPtr;
}