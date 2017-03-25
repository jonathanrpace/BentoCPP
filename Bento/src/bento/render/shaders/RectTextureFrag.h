#pragma once

#include <bento/components/textures/RectangleTexture.h>
#include <bento/core/ShaderStageBase.h>

namespace bento
{
	struct RectTextureFrag : ShaderStageBase
	{
		RectTextureFrag() 
			: ShaderStageBase("shaders/RectTexture.frag") 
		{

		}

		void SetSourceTexture(RectangleTexture& _texture)
		{
			SetTexture("s_tex", _texture);
		}
	};
}