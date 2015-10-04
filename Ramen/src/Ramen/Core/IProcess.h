#pragma once

namespace Ramen
{
	// Forward declares
	class Scene;

	class IProcess
	{
	public:
		virtual void BindToScene(Scene * const _scene) = 0;
		virtual void UnbindFromScene(Scene * const _scene) = 0;
		virtual void Update(double dt) = 0;
	};
}