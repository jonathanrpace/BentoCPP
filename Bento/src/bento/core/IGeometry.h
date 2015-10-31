#pragma once

#include <bento/core/IComponent.h>

namespace bento
{
	class IGeometry 
		: public bento::IComponent
	{
		virtual void Bind() = 0;
		virtual void Draw() = 0;
	};
}