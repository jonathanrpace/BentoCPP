#pragma once

#include <Ramen/Components/ComponentBase.h>
#include <GLM/glm.hpp>

namespace Kaiga
{
	class Transform : public Ramen::ComponentBase<Transform>
	{
	public:
		glm::mat4 matrix;

		Transform() :
			matrix()
		{
		}
	};
}