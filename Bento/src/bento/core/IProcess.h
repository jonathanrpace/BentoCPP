#pragma once

#include <bento/core/SceneObject.h>

namespace bento
{
	// Forward declares
	class Scene;

	struct IProcess : SceneObject
	{
		IProcess(std::string _name = "IProcess");

		virtual void BindToScene(Scene * const _scene) = 0;
		virtual void UnbindFromScene(Scene * const _scene) = 0;
		virtual void Update(double dt) = 0;
	};

	typedef std::shared_ptr<IProcess> ProcessPtr;
}