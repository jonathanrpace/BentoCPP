#pragma once

#include <Kaiga/Core/ShaderStageBase.h>
#include <Kaiga/Textures/RectangleTexture.h>

namespace Kaiga
{
	struct RectTextureFrag : ShaderStageBase
	{
		RectTextureFrag() 
			: ShaderStageBase("shaders/RectTexture.frag") 
		{

		}

		void SetSourceTexture(RectangleTexture* _texture)
		{
			SetTexture("s_tex", _texture);
		}
	};
}