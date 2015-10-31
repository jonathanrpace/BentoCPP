#pragma once

#include <glm.h>

#include <bento/core/IComponent.h>
#include <bento/core/SharedObject.h>

namespace bento
{
	class Transform
		: public bento::SharedObject<Transform>
		, public bento::IComponent
	{
	public:
		mat4 matrix;

		// from IComponent
		virtual const std::type_info & typeInfo() override;
	};
}