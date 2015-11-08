#pragma once

#include <typeinfo>
#include <string>

#include <bento/core/SharedObject.h>
#include <bento/geom/Geometry.h>

namespace bento
{
	class ScreenQuadGeometry : 
		public Geometry,
		public SharedObject<ScreenQuadGeometry>
	{

	public:
		ScreenQuadGeometry(std::string _name = "ScreenQuadGeometry");

	protected:
		// From AbstractValidatable
		virtual void Validate() override;
	};
}