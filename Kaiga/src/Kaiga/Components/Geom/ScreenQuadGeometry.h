#pragma once

#include <ramen.h>
#include <kaiga.h>

namespace Kaiga
{
	class ScreenQuadGeometry : 
		public AbstractGeometry,
		public Ramen::ComponentBase<ScreenQuadGeometry>
	{
	protected:
		// From AbstractValidatable
		virtual void Validate() override;
	};
}