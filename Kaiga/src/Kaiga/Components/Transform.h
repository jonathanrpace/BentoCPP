#pragma once

#include <ramen.h>
#include <glm.h>

namespace Kaiga
{
	class Transform
		: public Ramen::SharedObject<Transform>
		, public Ramen::IComponent
	{
	public:
		mat4 matrix;

		// from IComponent
		virtual const std::type_info & typeInfo() override;
	};
}