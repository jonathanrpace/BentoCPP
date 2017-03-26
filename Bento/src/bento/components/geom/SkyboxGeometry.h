#pragma once

#include <typeinfo>
#include <string>

#include <bento/core/SharedObject.h>
#include <bento/components/geom/Geometry.h>

namespace bento
{
	class SkyboxGeometry : 
		public Geometry,
		public SharedObject<SkyboxGeometry>
	{

	public:
		SkyboxGeometry(std::string _name = "SkyboxGeometry");

	protected:
		// From AbstractValidatable
		virtual void Validate() override;
	};
}