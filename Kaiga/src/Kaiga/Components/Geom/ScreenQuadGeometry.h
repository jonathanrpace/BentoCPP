#pragma once

#include <Ramen\Components\ComponentBase.h>

#include <Kaiga/Components/Geom/AbstractGeometry.h>

namespace Kaiga
{
	class ScreenQuadGeometry : 
		public AbstractGeometry,
		public Ramen::ComponentBase<ScreenQuadGeometry>
	{
	public:
		// From IGeometry
		virtual void Draw() override;

	protected:
		// From AbstractValidatable
		virtual void Validate() override;
	};
}