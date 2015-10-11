#pragma once

#include <ramen.h>
#include <glm.h>

namespace Kaiga
{
	class Transform : 
		public Ramen::ComponentBase<Transform>
	{
	public:
		mat4 matrix;

		Transform() :
			matrix()
		{
		}
	};
}