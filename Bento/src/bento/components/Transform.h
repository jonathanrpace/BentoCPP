#pragma once

#include <bento/core/Component.h>
#include <bento/core/SharedObject.h>

namespace bento
{
	class Transform
		: public bento::SharedObject<Transform>
		, public bento::Component
	{
	public:
		Transform(std::string _name = "Transform");

		mat4 matrix;
	};
}