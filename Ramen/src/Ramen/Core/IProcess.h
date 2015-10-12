#pragma once

#include <Ramen/Core/ISceneObject.h>

namespace Ramen
{
	// Forward declares
	class Scene;

	class IProcess : public Ramen::ISceneObject
	{
	public:
		virtual void BindToScene(Scene * const _scene) = 0;
		virtual void UnbindFromScene(Scene * const _scene) = 0;
		virtual void Update(double dt) = 0;
	};
}