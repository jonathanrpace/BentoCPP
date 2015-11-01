#pragma once

#include <typeinfo>

#include <bento/core/SharedObject.h>
#include <bento/core/GeometryBase.h>

namespace bento
{
	class ScreenQuadGeometry : 
		public GeometryBase,
		public bento::SharedObject<ScreenQuadGeometry>
	{

	public:
		ScreenQuadGeometry(std::string _name = "ScreenQuadGeometry");

	protected:
		// From AbstractValidatable
		virtual void Validate() override;

		// From ISceneObject
		virtual const std::type_info & TypeInfo() override;
	};
}