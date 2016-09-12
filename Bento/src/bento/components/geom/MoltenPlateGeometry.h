#pragma once

#include <typeinfo>
#include <string>

#include <bento/core/SharedObject.h>
#include <bento/components/geom/Geometry.h>

namespace bento
{
	class MoltenPlateGeometry : 
		public Geometry,
		public SharedObject<MoltenPlateGeometry>
	{

	public:
		MoltenPlateGeometry(std::string _name = "MoltenPlateGeometry");

	protected:
		// From AbstractValidatable
		virtual void Validate() override;
	};
}