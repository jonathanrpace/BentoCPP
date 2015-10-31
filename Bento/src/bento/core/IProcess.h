#pragma once

#include <bento/core/ISceneObject.h>

namespace bento
{
	// Forward declares
	class Scene;

	class IProcess : public bento::ISceneObject
	{
	public:
		virtual void BindToScene(Scene * const _scene) = 0;
		virtual void UnbindFromScene(Scene * const _scene) = 0;
		virtual void Update(double dt) = 0;
	};
}