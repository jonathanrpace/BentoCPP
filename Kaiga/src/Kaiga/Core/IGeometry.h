#pragma once

namespace Kaiga
{
	class IGeometry
	{
		virtual void Bind() = 0;
		virtual void Draw() = 0;
	};
}