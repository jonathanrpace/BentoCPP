#pragma once

#include <typeinfo>

#include <ramen.h>
#include <kaiga.h>

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