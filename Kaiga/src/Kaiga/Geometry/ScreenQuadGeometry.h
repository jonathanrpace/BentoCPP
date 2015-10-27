#pragma once

#include <typeinfo>

#include <Ramen/Core/SharedObject.h>

#include <Kaiga/Core/GeometryBase.h>

namespace Kaiga
{
	class ScreenQuadGeometry : 
		public GeometryBase,
		public Ramen::SharedObject<ScreenQuadGeometry>
	{
	protected:
		// From AbstractValidatable
		virtual void Validate() override;

		// From ISceneObject
		virtual const std::type_info & typeInfo() override;
	};
}